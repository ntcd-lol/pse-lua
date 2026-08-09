// .-=#####=-.
// PSE SDK Lua
// |- Creator: ntcd_lol, opencode
// \- Comment: :3
// '-=#####=-'
// ^         ^
// Как это работает:
// * Заголовок C-моста между встроенным Lua и pse.dll.
// * Декларирует регистрацию таблицы "core" и тонкие обёртки над
//   C-экспортами PSE SDK (буферы команд/событий, millis).
// * setMock() переключает реализацию на офлайн-эмуляцию (см. lua_core.cpp).
// --==-==--

#pragma once

#include <cstdint>
#include <string>

struct lua_State;

namespace psebridge
{
    // Registers the "core" table (Lua C API) exposing the bridge to scripts.
    void lua_register_core(lua_State* L);

    // Loads pse.dll and resolves the needed C exports.
    // Returns true if the library is available (game may or may not be running).
    bool load();

    void unload();

    bool isLoaded();

    // Absolute path to the resolved pse.dll (empty if not found).
    const std::string& dllPath();

    // pseInitializeBuffers
    int32_t initialize();

    // pseDeinitializeBuffers
    void deinitialize();

    // pseCommandBufferSynchronize
    void synchronize();

    // pseCommandBufferSinglePush (fire-and-forget)
    void push(uint32_t header, const uint8_t data[60]);

    // pseCommandBufferSinglePushAndWait (in-place PseData round trip)
    int32_t pushAndWait(uint32_t& outHeader, uint8_t outData[60]);

    // pseEventBufferTryGet; returns 1 if an event was produced, 0 otherwise
    int32_t pollEvent(uint32_t& outHeader, uint8_t outData[60]);

    // Epoch milliseconds since 1970-01-01 (local-independent; used by the Lua logger)
    int64_t millis();

    // Switches the Lua-facing layer into offline simulation mode (no pse.dll needed).
    // Implemented by the Lua core layer (src/lua_core.cpp).
    void setMock(bool enabled);
}
