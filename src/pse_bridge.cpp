// .-=#####=-.
// PSE SDK Lua
// |- Creator: ntcd_lol, opencode
// \- Comment: :3
// '-=#####=-'
// ^         ^
// Thin wrapper over the statically-linked pse-sdk (third_party/pse-sdk):
// opens the shared-memory buffers directly, no dynamic pse.dll lookup.
// Mock mode bypasses the bridge entirely (see lua_core.cpp).
// --==-==--

#include "pse_bridge.h"

#include "pse.h"

#include <chrono>
#include <cstring>

namespace
{
    bool gReady = false;
}

namespace psebridge
{
    bool load()
    {
        if (gReady) return true;
        gReady = pseInitializeBuffers() == 0;
        return gReady;
    }

    void unload()
    {
        if (gReady)
        {
            pseDeinitializeBuffers();
            gReady = false;
        }
    }

    bool isLoaded() { return gReady; }

    const std::string& dllPath()
    {
        static const std::string sStatic = "third_party/pse-sdk (statically linked)";
        return sStatic;
    }

    int32_t initialize()
    {
        if (gReady) return 0;
        gReady = pseInitializeBuffers() == 0;
        return gReady ? 0 : -1;
    }

    void deinitialize()
    {
        if (gReady)
        {
            pseDeinitializeBuffers();
            gReady = false;
        }
    }

    void synchronize()
    {
        pseCommandBufferSynchronize();
    }

    void push(uint32_t header, const uint8_t data[60])
    {
        PseData cmd{};
        cmd.header = header;
        std::memcpy(cmd.data, data, 60);

        pseCommandBufferSinglePush(&cmd);
    }

    int32_t pushAndWait(uint32_t& outHeader, uint8_t outData[60])
    {
        PseData data{};
        data.header = outHeader;
        std::memcpy(data.data, outData, 60);

        pseCommandBufferSinglePushAndWait(&data);

        outHeader = data.header;
        std::memcpy(outData, data.data, 60);
        return 0;
    }

    int32_t pollEvent(uint32_t& outHeader, uint8_t outData[60])
    {
        PseData data{};

        int32_t r = pseEventBufferTryGet(&data);
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
