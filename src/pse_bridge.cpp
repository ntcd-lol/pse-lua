// .-=#####=-.
// PSE SDK Lua
// |- Creator: ntcd_lol, opencode
// \- Comment: :3
// '-=#####=-'
// ^         ^
// Как это работает:
// * Динамически загружает pse.dll через LoadLibraryA и резолвит экспорты:
//   pseInitializeBuffers / pseDeinitializeBuffers / pseCommandBufferSynchronize /
//   pseCommandBufferSinglePush / pseCommandBufferSinglePushAndWait /
//   pseEventBufferTryGet.
// * Поиск DLL: переменная PSE_DLL, рядом с exe, текущий каталог и несколько
//   шагов вверх по дереву (проект лежит в ...\PSE SDK Lua\pse-lua, DLL в корне).
// * Каждый экспорт обёрнут в нуль-безопасный вызов (если функция не найдена -
//   вызов игнорируется). PseData передаётся как alignas(64) header + data[60].
// * В mock-режиме эти функции не используются - lua_core эмулирует команды.
// --==-==--

#include "pse_bridge.h"

#include <Windows.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <vector>

namespace
{
    HMODULE gDll = nullptr;
    std::string gDllPath;
    std::vector<std::string> gSearchPaths;

    using PfnInitialize     = int32_t(*)();
    using PfnDeinitialize   = void(*)();
    using PfnSynchronize    = void(*)();
    using PfnSinglePush     = uint32_t(*)(void* pInData);
    using PfnPushAndWait    = void(*)(void* pOutData);
    using PfnEventTryGet    = int32_t(*)(void* pOutData);

    PfnInitialize   gInit     = nullptr;
    PfnDeinitialize gDeinit   = nullptr;
    PfnSynchronize  gSync     = nullptr;
    PfnSinglePush   gPush     = nullptr;
    PfnPushAndWait  gPushWait = nullptr;
    PfnEventTryGet  gTryGet   = nullptr;
}

namespace psebridge
{
    static std::string exeDirectory()
    {
        char buf[MAX_PATH];
        DWORD n = GetModuleFileNameA(nullptr, buf, MAX_PATH);
        std::string path(buf, n);
        auto pos = path.find_last_of('\\');
        return pos == std::string::npos ? "." : path.substr(0, pos);
    }

    static void buildSearchPaths()
    {
        gSearchPaths.clear();
        const char* env = std::getenv("PSE_DLL");
        if (env && *env) gSearchPaths.emplace_back(env);

        const std::string exeDir = exeDirectory();
        // Next to the executable, current directory, and a few relative hops up
        // (the project lives at <workspace>/PSE SDK Lua/pse-lua, the dll at <workspace>/pse.dll).
        gSearchPaths.push_back(exeDir + "\\pse.dll");
        gSearchPaths.push_back(".\\pse.dll");
        gSearchPaths.push_back(exeDir + "\\..\\pse.dll");
        gSearchPaths.push_back(exeDir + "\\..\\..\\pse.dll");
        gSearchPaths.push_back(exeDir + "\\..\\..\\..\\pse.dll");

        // De-duplicate while keeping order
        std::vector<std::string> unique;
        for (const auto& p : gSearchPaths)
        {
            bool seen = false;
            for (const auto& u : unique) if (u == p) { seen = true; break; }
            if (!seen) unique.push_back(p);
        }
        gSearchPaths.swap(unique);
    }

    static void clearResolved()
    {
        if (gDll) { FreeLibrary(gDll); gDll = nullptr; }
        gDllPath.clear();
        gInit = nullptr;
        gDeinit = nullptr;
        gSync = nullptr;
        gPush = nullptr;
        gPushWait = nullptr;
        gTryGet = nullptr;
    }

    bool load()
    {
        if (gDll) return true;

        buildSearchPaths();
        for (const auto& candidate : gSearchPaths)
        {
            HMODULE h = LoadLibraryA(candidate.c_str());
            if (!h) continue;

            gInit     = reinterpret_cast<PfnInitialize>(GetProcAddress(h, "pseInitializeBuffers"));
            gDeinit   = reinterpret_cast<PfnDeinitialize>(GetProcAddress(h, "pseDeinitializeBuffers"));
            gSync     = reinterpret_cast<PfnSynchronize>(GetProcAddress(h, "pseCommandBufferSynchronize"));
            gPush     = reinterpret_cast<PfnSinglePush>(GetProcAddress(h, "pseCommandBufferSinglePush"));
            gPushWait = reinterpret_cast<PfnPushAndWait>(GetProcAddress(h, "pseCommandBufferSinglePushAndWait"));
            gTryGet   = reinterpret_cast<PfnEventTryGet>(GetProcAddress(h, "pseEventBufferTryGet"));

            if (gInit && gDeinit && gSync && gPush && gPushWait && gTryGet)
            {
                gDll = h;
                gDllPath = candidate;
                return true;
            }

            FreeLibrary(h);
        }

        clearResolved();
        return false;
    }

    void unload() { clearResolved(); }

    bool isLoaded() { return gDll != nullptr; }

    const std::string& dllPath() { return gDllPath; }

    int32_t initialize()
    {
        if (!gInit) return -1;
        return gInit();
    }

    void deinitialize()
    {
        if (gDeinit) gDeinit();
    }

    void synchronize()
    {
        if (gSync) gSync();
    }

    void push(uint32_t header, const uint8_t data[60])
    {
        struct alignas(64) PseData { uint32_t header; uint8_t data[60]; };
        PseData cmd{};
        cmd.header = header;
        std::memcpy(cmd.data, data, 60);

        if (!gPush) return;
        gPush(&cmd);
    }

    int32_t pushAndWait(uint32_t& outHeader, uint8_t outData[60])
    {
        struct alignas(64) PseData { uint32_t header; uint8_t data[60]; };
        PseData data{};
        data.header = outHeader;
        std::memcpy(data.data, outData, 60);

        if (!gPushWait) return -1;
        gPushWait(&data);

        outHeader = data.header;
        std::memcpy(outData, data.data, 60);
        return 0;
    }

    int32_t pollEvent(uint32_t& outHeader, uint8_t outData[60])
    {
        struct alignas(64) PseData { uint32_t header; uint8_t data[60]; };
        PseData data{};

        if (!gTryGet) return 0;
        int32_t r = gTryGet(&data);
        if (r == 0) return 0;

        outHeader = data.header;
        std::memcpy(outData, data.data, 60);
        return 1;
    }

    int64_t millis()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }
}
