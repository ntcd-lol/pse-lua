# .-=#####=-.
# PSE SDK Lua
# |- Creator: ntcd_lol, opencode
# \- Comment: :3
# '-=#####=-'
#
# Builds the C++ bridge (src/pse_bridge.cpp + src/lua_core.cpp) and the
# interpreter (host + bundled Lua 5.4.8).
#
#   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
#   cmake --build build --config Release
#   build\bin\pse_lua.exe --mock examples\hello.lua
#
# Options:
#   PSE_LUA_USE_SYSTEM_LUA  - use a system-installed Lua instead of third_party (OFF)
#   PSE_LUA_COPY_SDK        - copy lua/ and examples/ next to the exe (ON)
# --==-==--

cmake_minimum_required(VERSION 3.16)
project(pse_lua VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(PSE_LUA_USE_SYSTEM_LUA "Build against a system-installed Lua instead of the bundled copy" OFF)
option(PSE_LUA_COPY_SDK "Copy the Lua SDK and examples next to the built binary" ON)

# ---------------------------------------------------------------------------
# Lua 5.4: bundled (default) or system
# ---------------------------------------------------------------------------
set(PSE_LUA_BUNDLED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/lua-5.4.8")
set(PSE_LUA_SOURCES)
set(PSE_LUA_INCLUDE_DIRS)

if(PSE_LUA_USE_SYSTEM_LUA)
    find_package(Lua 5.4 REQUIRED)
    set(PSE_LUA_INCLUDE_DIRS ${LUA_INCLUDE_DIR})
else()
    if(NOT EXISTS "${PSE_LUA_BUNDLED_DIR}")
        message(FATAL_ERROR
            "Bundled Lua 5.4.8 not found at ${PSE_LUA_BUNDLED_DIR}. "
            "Put it into third_party/ or enable PSE_LUA_USE_SYSTEM_LUA.")
    endif()
    file(GLOB PSE_LUA_SOURCES "${PSE_LUA_BUNDLED_DIR}/*.c")
    list(FILTER PSE_LUA_SOURCES EXCLUDE REGEX "/(lua|luac)\\.c$")
    set(PSE_LUA_INCLUDE_DIRS "${PSE_LUA_BUNDLED_DIR}")
endif()

# ---------------------------------------------------------------------------
# C++ bridge: Lua <-> pse.dll (reusable static library)
# ---------------------------------------------------------------------------
add_library(pse_lua_bridge STATIC
    src/pse_bridge.cpp
    src/lua_core.cpp
)
target_include_directories(pse_lua_bridge
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PRIVATE ${PSE_LUA_INCLUDE_DIRS}
)

if(PSE_LUA_USE_SYSTEM_LUA)
    target_link_libraries(pse_lua_bridge PRIVATE ${LUA_LIBRARIES})
endif()

# ---------------------------------------------------------------------------
# Interpreter (entry point + bundled Lua)
# ---------------------------------------------------------------------------
add_executable(pse_lua
    src/main.cpp
    ${PSE_LUA_SOURCES}
)
target_include_directories(pse_lua PRIVATE ${PSE_LUA_INCLUDE_DIRS})
target_link_libraries(pse_lua PRIVATE pse_lua_bridge)

if(PSE_LUA_USE_SYSTEM_LUA)
    target_link_libraries(pse_lua PRIVATE ${LUA_LIBRARIES})
endif()

if(MSVC)
    target_compile_options(pse_lua_bridge PRIVATE /utf-8)
    target_compile_options(pse_lua PRIVATE /utf-8)
endif()

# ---------------------------------------------------------------------------
# Copy the SDK and examples next to the exe so runs from build\bin work
# ---------------------------------------------------------------------------
if(PSE_LUA_COPY_SDK)
    add_custom_command(TARGET pse_lua POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/lua" "${CMAKE_BINARY_DIR}/bin/lua"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/examples" "${CMAKE_BINARY_DIR}/bin/examples"
        COMMENT "Copying lua/ and examples/ next to pse_lua.exe"
    )
endif()
