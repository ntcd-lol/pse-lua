// .-=#####=-.
// PSE SDK Lua
// |- Creator: ntcd_lol, opencode
// \- Comment: :3
// '-=#####=-'
// ^         ^
// Как это работает:
// * Экспортирует в Lua таблицу "core" (C API): push / push_and_wait /
//   poll_event / mock_emit / set_mock / initialize / deinitialize /
//   synchronize / millis / sleep - поверх pse.dll (pse_bridge) или мока.
// * В режиме мока (gMock) хранит эмулятор мира: mock::Element с раскладкой
//   ELEMENT_CREATE (transform@0, class@40, callback@42, state@50,
//   visibility@51) + 8 x PseRegister, генерирует GUID начиная с 1 и
//   обрабатывает все команды из sdk/schema.lua.
// * Авто-эмиссия PSE_EVENT_ELEMENT_CHANGED (guid@0, callback@8, state@16)
//   при изменении state у элемента с ненулевым callback; события
//   складываются в очередь mock::events и читаются через poll_event.
// * В живом режиме те же вызовы транслируются в экспорты pse.dll.
// --==-==--

#include <Windows.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <deque>
#include <string>
#include <unordered_map>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include "pse_bridge.h"

namespace
{
    struct PseData
    {
        alignas(64) uint32_t header;
        alignas(64) uint8_t  data[60];
    };

    bool gMock = false;

    namespace mock
    {
        struct Element
        {
            uint8_t transform[40];
            uint8_t cls[2];
            uint8_t callback[8];
            int8_t  state;
            int8_t  visibility;
            uint8_t registers[32];
        };

        std::unordered_map<uint64_t, Element> world;   
        std::deque<PseData> events;                    
        uint64_t nextGuid = 1;
    }

    static uint32_t rd32(const uint8_t* p) { uint32_t v; std::memcpy(&v, p, 4); return v; }
    static uint64_t rd64(const uint8_t* p) { uint64_t v; std::memcpy(&v, p, 8); return v; }
    static void wr32(uint8_t* p, uint32_t v) { std::memcpy(p, &v, 4); }
    static void wr64(uint8_t* p, uint64_t v) { std::memcpy(p, &v, 8); }

    enum : uint32_t
    {
        CMD_GAME_SET_CHEATS_ENABLED  = 0x00100001,
        CMD_GAME_GET_CHEATS_ENABLED  = 0x00100002,
        CMD_GAME_SET_CHEATS_NOCLIP   = 0x00100003,
        CMD_GAME_GET_CHEATS_NOCLIP   = 0x00100004,
        CMD_GAME_SET_GRAVITY         = 0x00100005,
        CMD_GAME_GET_GRAVITY         = 0x00100006,
        CMD_GAME_CHECK_GUID_IS_VALID = 0x00100007,
        CMD_GAME_INITIALIZE          = 0x00100008,
        CMD_GAME_DEINITIALIZE        = 0x00100009,

        CMD_STATIC_MESH_CREATE       = 0x00500001,

        CMD_DYNAMIC_MESH_CREATE      = 0x00600001,
        CMD_DYNAMIC_MESH_SET_MATERIAL = 0x00600002,
        CMD_DYNAMIC_MESH_SET_MESH    = 0x00600003,
        CMD_DYNAMIC_MESH_SET_VISIBILITY = 0x00600004,
        CMD_DYNAMIC_MESH_GET_VISIBILITY = 0x00600005,
        CMD_DYNAMIC_MESH_SET_TRANSFORM = 0x00600006,
        CMD_DYNAMIC_MESH_GET_TRANSFORM = 0x00600007,
        CMD_DYNAMIC_MESH_DESTROY     = 0x00600008,

        CMD_ELEMENT_CREATE           = 0x00700001,
        CMD_ELEMENT_SET_CALLBACK     = 0x00700002,
        CMD_ELEMENT_GET_CALLBACK     = 0x00700003,
        CMD_ELEMENT_SET_STATE        = 0x00700004,
        CMD_ELEMENT_GET_STATE        = 0x00700005,
        CMD_ELEMENT_SET_VISIBILITY   = 0x00700006,
        CMD_ELEMENT_GET_VISIBILITY   = 0x00700007,
        CMD_ELEMENT_SET_TRANSFORM    = 0x00700008,
        CMD_ELEMENT_GET_TRANSFORM    = 0x00700009,
        CMD_ELEMENT_SET_CLASS        = 0x0070000A,
        CMD_ELEMENT_GET_CLASS        = 0x0070000B,
        CMD_ELEMENT_SET_REGISTER     = 0x0070000C,
        CMD_ELEMENT_GET_REGISTER     = 0x0070000D,
        CMD_ELEMENT_SET_ALL_REGISTERS = 0x0070000E,
        CMD_ELEMENT_GET_ALL_REGISTERS = 0x0070000F,
        CMD_ELEMENT_SET_STATE_AND_CALLBACK_AND_ALL_REGISTERS = 0x00700010,
        CMD_ELEMENT_GET_STATE_AND_CALLBACK_AND_ALL_REGISTERS = 0x00700011,
        CMD_ELEMENT_DESTROY          = 0x00700012,

        EVENT_ELEMENT_CHANGED        = 0x00000000,
        RESULT_SUCCESS               = 0x00000000,
    };

    static void emitChanged(uint64_t guid, const mock::Element& e)
    {
        PseData ev{};
        ev.header = EVENT_ELEMENT_CHANGED;
        wr64(ev.data, guid);
        wr64(ev.data + 8, rd64(e.callback));
        ev.data[16] = static_cast<uint8_t>(e.state);
        mock::events.push_back(ev);
    }

    static PseData mockProcess(const PseData& in)
    {
        PseData out{};
        out.header = RESULT_SUCCESS;
        const uint32_t cmd = in.header;
        auto& W = mock::world;
        const uint64_t guid = rd64(in.data);

        auto get = [&](uint64_t g) -> mock::Element* {
            auto it = W.find(g);
            return it == W.end() ? nullptr : &it->second;
        };

        switch (cmd)
        {
        case CMD_ELEMENT_CREATE:
        {
            mock::Element e{};
            std::memcpy(e.transform, in.data, 40);
            std::memcpy(e.cls, in.data + 40, 2);
            std::memcpy(e.callback, in.data + 42, 8);
            e.state = static_cast<int8_t>(in.data[50]);
            e.visibility = static_cast<int8_t>(in.data[51]);
            const uint64_t id = mock::nextGuid++;
            W[id] = e;
            wr64(out.data, id);
            break;
        }
        case CMD_DYNAMIC_MESH_CREATE:
        {
            mock::Element e{};
            std::memcpy(e.transform, in.data, 40);
            std::memcpy(e.cls, in.data + 40, 2);        
            std::memcpy(e.callback, in.data + 42, 2);   
            e.visibility = static_cast<int8_t>(in.data[44]);
            const uint64_t id = mock::nextGuid++;
            W[id] = e;
            wr64(out.data, id);
            break;
        }
        case CMD_STATIC_MESH_CREATE:
            break;

        case CMD_ELEMENT_SET_STATE:
            if (auto* e = get(guid))
            {
                e->state = static_cast<int8_t>(in.data[8]);
                if (rd64(e->callback) != 0) emitChanged(guid, *e);
            }
            break;
        case CMD_ELEMENT_GET_STATE:
            if (auto* e = get(guid)) out.data[0] = static_cast<uint8_t>(e->state);
            break;

        case CMD_ELEMENT_SET_VISIBILITY:
            if (auto* e = get(guid)) e->visibility = static_cast<int8_t>(in.data[8]);
            break;
        case CMD_ELEMENT_GET_VISIBILITY:
            if (auto* e = get(guid)) out.data[0] = static_cast<uint8_t>(e->visibility);
            break;

        case CMD_ELEMENT_SET_TRANSFORM:
            if (auto* e = get(guid)) std::memcpy(e->transform, in.data + 8, 40);
            break;
        case CMD_ELEMENT_GET_TRANSFORM:
            if (auto* e = get(guid)) std::memcpy(out.data, e->transform, 40);
            break;

        case CMD_ELEMENT_SET_CLASS:
            if (auto* e = get(guid)) std::memcpy(e->cls, in.data + 8, 2);
            break;
        case CMD_ELEMENT_GET_CLASS:
            if (auto* e = get(guid)) std::memcpy(out.data, e->cls, 2);
            break;

        case CMD_ELEMENT_SET_CALLBACK:
            if (auto* e = get(guid)) std::memcpy(e->callback, in.data + 8, 8);
            break;
        case CMD_ELEMENT_GET_CALLBACK:
            if (auto* e = get(guid)) std::memcpy(out.data, e->callback, 8);
            break;

        case CMD_ELEMENT_SET_REGISTER:
            if (auto* e = get(guid))
            {
                const int idx = static_cast<int8_t>(in.data[16]);
                if (idx >= 0 && idx < 8)
                    wr32(e->registers + static_cast<size_t>(idx) * 4, rd32(in.data + 8));
            }
            break;
        case CMD_ELEMENT_GET_REGISTER:
            if (auto* e = get(guid))
            {
                const int idx = static_cast<int8_t>(in.data[8]);
                if (idx >= 0 && idx < 8)
                    wr32(out.data, rd32(e->registers + static_cast<size_t>(idx) * 4));
            }
            break;
        case CMD_ELEMENT_SET_ALL_REGISTERS:
            if (auto* e = get(guid)) std::memcpy(e->registers, in.data + 8, 32);
            break;
        case CMD_ELEMENT_GET_ALL_REGISTERS:
            if (auto* e = get(guid)) std::memcpy(out.data, e->registers, 32);
            break;

        case CMD_ELEMENT_SET_STATE_AND_CALLBACK_AND_ALL_REGISTERS:
            if (auto* e = get(guid))
            {
                std::memcpy(e->registers, in.data + 8, 32);
                std::memcpy(e->callback, in.data + 40, 8);
                e->state = static_cast<int8_t>(in.data[48]);
                if (rd64(e->callback) != 0) emitChanged(guid, *e);
            }
            break;
        case CMD_ELEMENT_GET_STATE_AND_CALLBACK_AND_ALL_REGISTERS:
            if (auto* e = get(guid))
            {
                std::memcpy(out.data, e->registers, 32);
                std::memcpy(out.data + 32, e->callback, 8);
                out.data[40] = static_cast<uint8_t>(e->state);
            }
            break;

        case CMD_ELEMENT_DESTROY:
            W.erase(guid);
            break;

        case CMD_DYNAMIC_MESH_SET_MATERIAL:
            if (auto* e = get(guid)) std::memcpy(e->callback, in.data + 8, 2);
            break;
        case CMD_DYNAMIC_MESH_SET_MESH:
            if (auto* e = get(guid)) std::memcpy(e->cls, in.data + 8, 2);
            break;
        case CMD_DYNAMIC_MESH_SET_VISIBILITY:
            if (auto* e = get(guid)) e->visibility = static_cast<int8_t>(in.data[8]);
            break;
        case CMD_DYNAMIC_MESH_GET_VISIBILITY:
            if (auto* e = get(guid)) out.data[0] = static_cast<uint8_t>(e->visibility);
            break;
        case CMD_DYNAMIC_MESH_SET_TRANSFORM:
            if (auto* e = get(guid)) std::memcpy(e->transform, in.data + 8, 40);
            break;
        case CMD_DYNAMIC_MESH_GET_TRANSFORM:
            if (auto* e = get(guid)) std::memcpy(out.data, e->transform, 40);
            break;
        case CMD_DYNAMIC_MESH_DESTROY:
            W.erase(guid);
            break;

        default:
            break;
        }
        return out;
    }

    static int checkHeader(lua_State* L, int index)
    {
        return static_cast<int>(luaL_checkinteger(L, index));
    }

    static void checkPayload(lua_State* L, int index, uint8_t outData[60])
    {
        size_t len = 0;
        const char* payload = luaL_checklstring(L, index, &len);
        std::memcpy(outData, payload, len < 60 ? len : 60);
    }

    int l_initialize(lua_State* L)
    {
        if (gMock) { lua_pushinteger(L, 0); return 1; }
        lua_pushinteger(L, psebridge::initialize());
        return 1;
    }

    int l_deinitialize(lua_State* L)
    {
        if (gMock)
        {
            mock::world.clear();
            mock::events.clear();
            return 0;
        }
        psebridge::deinitialize();
        return 0;
    }

    int l_synchronize(lua_State* L)
    {
        if (!gMock) psebridge::synchronize();
        return 0;
    }

    int l_push(lua_State* L)
    {
        uint32_t h = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        uint8_t data[60] = {};
        checkPayload(L, 2, data);

        if (gMock)
        {
            PseData cmd{};
            cmd.header = h;
            std::memcpy(cmd.data, data, 60);
            mockProcess(cmd);
            return 0;
        }
        psebridge::push(h, data);
        return 0;
    }

    int l_push_and_wait(lua_State* L)
    {
        uint32_t h = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        uint8_t data[60] = {};
        checkPayload(L, 2, data);

        uint32_t outHeader = h;
        uint8_t outData[60] = {};
        std::memcpy(outData, data, 60);

        if (gMock)
        {
            PseData cmd{};
            cmd.header = h;
            std::memcpy(cmd.data, data, 60);
            PseData res = mockProcess(cmd);
            outHeader = res.header;
            std::memcpy(outData, res.data, 60);
        }
        else
        {
            psebridge::pushAndWait(outHeader, outData);
        }

        lua_pushinteger(L, static_cast<lua_Integer>(outHeader));
        lua_pushlstring(L, reinterpret_cast<const char*>(outData), 60);
        return 2;
    }

    int l_poll_event(lua_State* L)
    {
        uint32_t h = 0;
        uint8_t data[60] = {};

        if (gMock)
        {
            if (mock::events.empty())
            {
                lua_pushnil(L);
                return 1;
            }
            PseData ev = mock::events.front();
            mock::events.pop_front();
            h = ev.header;
            std::memcpy(data, ev.data, 60);
        }
        else
        {
            if (psebridge::pollEvent(h, data) == 0)
            {
                lua_pushnil(L);
                return 1;
            }
        }

        lua_pushinteger(L, static_cast<lua_Integer>(h));
        lua_pushlstring(L, reinterpret_cast<const char*>(data), 60);
        return 2;
    }

    int l_mock_emit(lua_State* L)
    {
        uint32_t h = static_cast<uint32_t>(luaL_checkinteger(L, 1));
        PseData ev{};
        ev.header = h;
        checkPayload(L, 2, ev.data);
        mock::events.push_back(ev);
        return 0;
    }

    int l_sleep(lua_State* L)
    {
        lua_Integer ms = luaL_checkinteger(L, 1);
        if (ms < 0) ms = 0;
        Sleep(static_cast<DWORD>(ms));
        return 0;
    }

    int l_millis(lua_State* L)
    {
        lua_pushinteger(L, static_cast<lua_Integer>(psebridge::millis()));
        return 1;
    }

    int l_dll_path(lua_State* L)
    {
        const std::string& p = psebridge::dllPath();
        lua_pushstring(L, p.empty() ? "(not found)" : p.c_str());
        return 1;
    }

    int l_loaded(lua_State* L)
    {
        lua_pushboolean(L, gMock || psebridge::isLoaded() ? 1 : 0);
        return 1;
    }

    int l_mock(lua_State* L)
    {
        lua_pushboolean(L, gMock ? 1 : 0);
        return 1;
    }

    int l_set_mock(lua_State* L)
    {
        gMock = lua_toboolean(L, 1) != 0;
        if (gMock)
        {
            mock::world.clear();
            mock::events.clear();
            mock::nextGuid = 1;
        }
        return 0;
    }

    const luaL_Reg kCoreFunctions[] =
    {
        { "initialize",    l_initialize },
        { "deinitialize",  l_deinitialize },
        { "synchronize",   l_synchronize },
        { "push",          l_push },
        { "push_and_wait", l_push_and_wait },
        { "poll_event",    l_poll_event },
        { "mock_emit",     l_mock_emit },
        { "sleep",         l_sleep },
        { "millis",        l_millis },
        { "dll_path",      l_dll_path },
        { "loaded",        l_loaded },
        { "mock",          l_mock },
        { "set_mock",      l_set_mock },
        { nullptr, nullptr }
    };
}

namespace psebridge
{
    void lua_register_core(lua_State* L)
    {
        luaL_newlib(L, kCoreFunctions);
        lua_setglobal(L, "core");
    }

    void setMock(bool enabled)
    {
        gMock = enabled;
        if (gMock)
        {
            mock::world.clear();
            mock::events.clear();
            mock::nextGuid = 1;
        }
    }
}
