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
    void lua_register_core(lua_State* L);

    bool load();

    void unload();

    bool isLoaded();

    const std::string& dllPath();

    int32_t initialize();

    void deinitialize();

    void synchronize();

    void push(uint32_t header, const uint8_t data[60]);

    int32_t pushAndWait(uint32_t& outHeader, uint8_t outData[60]);

    int32_t pollEvent(uint32_t& outHeader, uint8_t outData[60]);

    int64_t millis();

    void setMock(bool enabled);
}
