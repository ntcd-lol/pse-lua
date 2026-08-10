// .-=#####=-.
// PSE SDK Lua
// |- Creator: ntcd_lol, opencode
// \- Comment: :3
// '-=#####=-'
// ^         ^
// Host: creates a Lua 5.4 state, registers the "core" C bridge and
// loads the SDK via require("sdk.init"). --mock enables offline emulation;
// the first positional argument runs a Lua script, otherwise a REPL opens.
// --==-==--

#define NOMINMAX
#include <Windows.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "pse_bridge.h"

static const char* kVersion = "0.1.0";

static void enableVirtualTerminal()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE)
    {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode))
        {
            SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT);
        }
    }
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

static void reportError(lua_State* L)
{
    const char* msg = lua_tostring(L, -1);
    if (!msg) msg = "unknown error";
    std::cerr << "\033[31m" << msg << "\033[0m\n";
    lua_pop(L, 1);
}

static int traceback(lua_State* L)
{
    const char* msg = lua_tostring(L, 1);
    if (!msg) lua_pushliteral(L, "(error object is not a string)");
    else luaL_traceback(L, L, msg, 1);
    return 1;
}

static void printValue(lua_State* L, int index, int depth);

static void runChunkOnTop(lua_State* L)
{
    int status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (status != LUA_OK)
    {
        reportError(L);
        return;
    }
    int n = lua_gettop(L);
    for (int i = 1; i <= n; ++i)
    {
        if (i > 1) std::cout << "\t";
        printValue(L, i, 0);
    }
    if (n > 0) std::cout << "\n";
    lua_pop(L, n);
}

static void printValue(lua_State* L, int index, int depth)
{
    index = lua_absindex(L, index);
    switch (lua_type(L, index))
    {
    case LUA_TNIL:       std::cout << "nil"; break;
    case LUA_TBOOLEAN:   std::cout << (lua_toboolean(L, index) ? "true" : "false"); break;
    case LUA_TNUMBER:    std::cout << lua_tonumber(L, index); break;
    case LUA_TSTRING:    std::cout << lua_tostring(L, index); break;
    case LUA_TFUNCTION:  std::cout << "function: " << lua_topointer(L, index); break;
    case LUA_TUSERDATA:  std::cout << "userdata: " << lua_topointer(L, index); break;
    case LUA_TTHREAD:    std::cout << "thread: " << lua_topointer(L, index); break;
    case LUA_TLIGHTUSERDATA: std::cout << "userdata: " << lua_topointer(L, index); break;
    case LUA_TTABLE:
        if (depth > 3) { std::cout << "{ ... }"; break; }
        {
            std::cout << "{";
            bool first = true;
            lua_pushnil(L);
            while (lua_next(L, index) != 0)
            {
                if (!first) std::cout << ", ";
                first = false;
                if (lua_type(L, -2) == LUA_TSTRING)
                    std::cout << lua_tostring(L, -2) << "=";
                else if (lua_type(L, -2) == LUA_TNUMBER)
                    std::cout << "[" << lua_tonumber(L, -2) << "]=";
                printValue(L, -1, depth + 1);
                lua_pop(L, 1);
            }
            std::cout << "}";
        }
        break;
    default: std::cout << "<" << lua_typename(L, lua_type(L, index)) << ">"; break;
    }
}

static bool isUnfinishedSyntax(lua_State* L)
{
    if (!lua_isstring(L, -1)) return false;
    size_t len = 0;
    const char* msg = lua_tolstring(L, -1, &len);
    if (!msg) return false;
    return (len >= 5 && std::strncmp(msg + len - 5, "<eof>", 5) == 0) ||
           std::strstr(msg, "near '<eof>'") != nullptr;
}

static std::string rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(" \t\r\n");
    return end == std::string::npos ? "" : s.substr(0, end + 1);
}

static std::string ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(" \t\r");
    return start == std::string::npos ? "" : s.substr(start);
}

static bool endsWithCreate(const std::string& code)
{
    const std::string t = rtrim(code);
    const size_t pos = t.rfind(":create");
    if (pos == std::string::npos) return false;
    size_t i = pos + std::string(":create").size();
    while (i < t.size() && (t[i] == ' ' || t[i] == '\t')) ++i;
    if (i >= t.size() || t[i] != '(') return false;
    int depth = 0;
    for (; i < t.size(); ++i)
    {
        if (t[i] == '(') ++depth;
        else if (t[i] == ')')
        {
            if (--depth == 0)
                return rtrim(t.substr(i + 1)).empty();
        }
    }
    return false; 
}

static bool lineIsContinuation(const std::string& lastLine)
{
    const std::string t = ltrim(lastLine);
    if (t.empty()) return false;
    if (t.size() >= 2 && t[0] == ':' && t[1] == ':') return false; 
    if (t[0] == ':') return true;
    if (t[0] == '.') return t.size() < 2 || t[1] != '.';           
    return false;
}

static bool lastStatementHasBuilderStart(const std::string& code)
{
    const size_t cut = code.find_last_of("\n;");
    const std::string last = (cut == std::string::npos) ? code : code.substr(cut + 1);
    size_t pos = 0;
    while ((pos = last.find("PSE.create", pos)) != std::string::npos)
    {
        size_t j = pos + std::string("PSE.create").size();
        while (j < last.size() &&
               (std::isalnum(static_cast<unsigned char>(last[j])) || last[j] == '_'))
            ++j;
        while (j < last.size() && (last[j] == ' ' || last[j] == '\t')) ++j;
        if (j < last.size() && last[j] == '(') return true;
        ++pos;
    }
    return false;
}

static bool isBuilderOpen(const std::string& code)
{
    const std::string t = rtrim(code);
    if (t.empty()) return false;
    if (endsWithCreate(t)) return false;
    const size_t nl = t.find_last_of('\n');
    const std::string lastLine = (nl == std::string::npos) ? t : t.substr(nl + 1);
    if (lineIsContinuation(lastLine)) return true;
    if (lastStatementHasBuilderStart(code)) return true;
    return false;
}

static bool tryRunBuffer(lua_State* L, const std::string& buf, bool force)
{
    {
        const std::string expr = "return " + buf;
        const int eStatus = luaL_loadbuffer(L, expr.data(), expr.size(), "=pse");
        if (eStatus == LUA_OK)
        {
            if (!force && isBuilderOpen(buf))
            {
                lua_pop(L, 1); 
                return false;
            }
            runChunkOnTop(L);
            return true;
        }
        lua_pop(L, 1);
    }

    const int status = luaL_loadbuffer(L, buf.data(), buf.size(), "=pse");
    if (status == LUA_OK)
    {
        if (!force && isBuilderOpen(buf))
        {
            lua_pop(L, 1); 
            return false;
        }
        runChunkOnTop(L);
        return true;
    }

    const bool unfinished = isUnfinishedSyntax(L);
    lua_pop(L, 1);
    if (unfinished && !force) return false; 

    luaL_loadbuffer(L, buf.data(), buf.size(), "=pse");
    reportError(L);
    return true;
}

// --- line editor with tab completion ---------------------------------------

static std::string toUtf8(const std::wstring& s)
{
    if (s.empty()) return std::string();
    const int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string out(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n, nullptr, nullptr);
    return out;
}

static std::wstring toWide(const std::string& s)
{
    if (s.empty()) return std::wstring();
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring out(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), out.data(), n);
    return out;
}

static const char* const kCompletionWords[] = {
    "help", "exit", "quit",
    "local", "function", "if", "then", "else", "elseif", "end", "for", "while",
    "do", "repeat", "until", "return", "break", "true", "false", "nil",
    "and", "or", "not", "require", "print", "type", "tostring", "tonumber",
    "math", "string", "table", "os", "io", "pairs", "ipairs",
    "PSE", "pse", "core", "Logger",
    "version", "initialize", "deinitialize", "synchronize", "millis", "sleep",
    "vec", "quat", "deg", "color", "guid", "get", "register",
    "createMeshObject", "createStaticMesh", "createElement",
    "spawnMeshObject", "spawnStaticMesh", "spawnElement",
    "createButton", "createDoor", "createLamp", "createTrigger",
    "createWeightCube", "createLaserTx", "createLaserRx", "createLaserRelay",
    "createLaserPanel", "createFaithPlate", "createIndicator",
    "createPedestalButton", "createSolverButton", "createWindow",
    "on", "onElementChanged", "poll", "pollAll", "run", "mock",
    "set_mock", "call", "push", "push_and_wait", "poll_event",
    "registerCallback", "releaseCallback", "packInput",
    "info", "debug", "warn", "warning", "error", "critical",
    "setLevel", "setTag", "getTag", "setFile", "setColors", "paint",
    "CUBE", "FACE", "PLANE", "CUP_INNER", "CUP_OUTER", "II_INNER", "II_OUTER",
    "O_INNER", "O_OUTER", "U_INNER", "U_OUTER", "DOOR_FRAME",
    "LASER_FRAME", "LASER_FRAME_SHIFTED", "PLANE_Z_SHIFTED",
    "WALL_WHITE_SMALL", "WALL_WHITE_MEDIUM", "WALL_WHITE_DOUBLE", "WALL_WHITE_BIG",
    "WALL_WHITE_ABSOLUTE_SCIENCE", "WALL_BLACK_SMALL", "WALL_BLACK_MEDIUM", "WALL_BLACK_BIG",
    "FLOOR_WHITE", "FLOOR_BLACK", "WINDOW_METAL_GRID", "WINDOW_GLASS_METAL_GRID",
    "WALL_YELLOW_1_0", "WALL_YELLOW_1_5",
    "ENTRY_ELEVATOR", "EXIT_ELEVATOR", "DOOR", "BUTTON", "ANTI_EXPROPRIATION_FIELD",
    "PEDESTAL_BUTTON", "SOLVER_BUTTON", "INDICATOR", "LASER_TX", "LASER_RX",
    "LASER_RELAY", "LASER_PANEL", "LASER_CUBE", "WEIGHT_CUBE", "FAITH_PLATE",
    "PANEL", "STAIRS", "CUBE_DROPPER", "SOLVER_GUN_PEDESTAL", "FLASHLIGHT",
    "WINDOW", "TRIGGER", "LAMP",
};

static const char* const kMethodWords[] = {
    "position", "rotation", "scale", "transform",
    "geometry", "texture", "mesh", "material", "visible", "name", "create",
    "state", "onChange", "register", "registers",
    "applyTransform", "setState", "getState", "setPosition", "setRotation",
    "setScale", "setTransform", "getTransform", "setClass", "getClass",
    "setRegister", "getRegister", "setRegisters", "getRegisters",
    "setCallback", "destroy", "setVisibility", "getVisibility",
    "setMesh", "setMaterial", "setGeometry", "setTexture",
    "getPosition", "setPosition", "getRotation", "setRotation", "spawn", "kill",
    "setCheatsEnabled", "getCheatsEnabled", "setNoclip", "getNoclip",
    "setGravity", "getGravity", "checkGuid",
    "setEnabled", "getEnabled", "use", "release", "throw",
    "emit", "emitRaw",
};

struct Editor
{
    HANDLE hOut;
    COORD start;
    std::wstring prompt;
    std::wstring line;
    size_t cursor = 0;
    size_t tokenStart = 0;
    std::vector<std::wstring> matches;
    int selected = 0;
    int lastListTop = -1;
    int lastListCount = 0;

    Editor(HANDLE out, const std::wstring& p)
        : hOut(out), prompt(p)
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(hOut, &info);
        start = info.dwCursorPosition;
    }

    void drawInputRow()
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(hOut, &info);
        DWORD n = 0;
        FillConsoleOutputCharacterW(hOut, L' ', info.dwSize.X - start.X, start, &n);
        SetConsoleCursorPosition(hOut, start);
        WriteConsoleW(hOut, prompt.c_str(), (DWORD)prompt.size(), &n, nullptr);
        WriteConsoleW(hOut, line.c_str(), (DWORD)line.size(), &n, nullptr);
    }

    void drawSuggestions()
    {
        const int count = (int)matches.size();
        const int maxRows = 5;

        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(hOut, &info);
        SMALL_RECT win = info.srWindow;
        const SHORT h = win.Bottom - win.Top + 1;

        if (count > 0)
        {
            // Anchor the list to the bottom of the visible window and keep the
            // input row just above it.
            SHORT bottom = (SHORT)(start.Y + maxRows);
            if (bottom >= info.dwSize.Y) bottom = (SHORT)(info.dwSize.Y - 1);
            SHORT top = (SHORT)(bottom - h + 1);
            if (top < 0)
            {
                top = 0;
                bottom = (SHORT)(h - 1);
                if (bottom >= info.dwSize.Y) bottom = (SHORT)(info.dwSize.Y - 1);
            }
            win.Top = top;
            win.Bottom = bottom;
            SetConsoleWindowInfo(hOut, TRUE, &win);
        }

        // Clear the previously drawn list (rows that scrolled out of the window
        // are simply not visible anymore).
        if (lastListCount > 0)
        {
            DWORD n = 0;
            for (int i = 0; i < lastListCount; ++i)
            {
                COORD row = { 0, (SHORT)(lastListTop + i) };
                FillConsoleOutputCharacterW(hOut, L' ', info.dwSize.X, row, &n);
            }
        }
        lastListTop = -1;
        lastListCount = 0;

        if (count > 0)
        {
            SHORT topRow = (SHORT)(win.Bottom - count + 1);
            if (topRow <= start.Y) topRow = (SHORT)(start.Y + 1);
            const SHORT fit = (SHORT)(win.Bottom - topRow + 1);
            const int shown = std::min(count, (int)fit);
            DWORD n = 0;
            for (int i = 0; i < shown; ++i)
            {
                COORD row = { 0, (SHORT)(topRow + i) };
                FillConsoleOutputCharacterW(hOut, L' ', info.dwSize.X, row, &n);
                std::wstring s = (i == selected ? L"> " : L"  ") + matches[i];
                WriteConsoleW(hOut, s.c_str(), (DWORD)s.size(), &n, nullptr);
            }
            lastListTop = topRow;
            lastListCount = shown;
        }
    }

    void placeCursor()
    {
        COORD cur;
        cur.X = (SHORT)(start.X + (SHORT)prompt.size() + (SHORT)cursor);
        cur.Y = start.Y;
        SetConsoleCursorPosition(hOut, cur);
    }

    void redraw()
    {
        drawSuggestions();
        drawInputRow();
        placeCursor();
    }
};

static bool exprIsSafe(const std::string& expr)
{
    static const char* forbidden[] = {
        "run(", "sleep(", "pollAll(", "poll(", "dofile", "loadfile", "require(",
        "os.", "io.", "debug.", "package.",
        ":create(", ":destroy(", ":setState(", ":setPosition(", ":setRotation(",
        ":setScale(", ":setTransform(", ":setVisibility(", ":setMaterial(",
        ":setMesh(", ":setClass(", ":setRegister(", ":setRegisters(",
        ":setCallback(", ":applyTransform(", ":initialize(", ":deinitialize(",
        ":checkGuid(", ":setGravity(", ":setNoclip(", ":setCheatsEnabled(",
        ":setEnabled(", ":use(", ":release(", ":throw(", ":spawn(", ":kill(",
        "mock.emit",
    };
    for (const char* f : forbidden)
        if (std::strstr(expr.c_str(), f)) return false;
    return true;
}

static void collectObjectKeys(lua_State* L, int idx, std::vector<std::string>& out, int depth, bool metaOnly)
{
    if (depth > 4) return;
    idx = lua_absindex(L, idx);
    if (lua_istable(L, idx))
    {
        bool hasIndex = false;
        if (lua_getmetatable(L, idx))
        {
            lua_getfield(L, -1, "__index");
            if (lua_istable(L, -1))
            {
                hasIndex = true;
                collectObjectKeys(L, -1, out, depth + 1, false);
            }
            lua_pop(L, 2);
        }
        // In method space (':') a plain table like PSE.player has no __index
        // metatable, so fall back to its own keys instead of suggesting nothing
        // (and instead of falling through to the global word list).
        if (!metaOnly || !hasIndex)
        {
            lua_pushnil(L);
            while (lua_next(L, idx) != 0)
            {
                if (lua_type(L, -2) == LUA_TSTRING)
                {
                    const char* k = lua_tostring(L, -2);
                    if (k && k[0] != '_' && std::strcmp(k, "new") != 0)
                        out.push_back(k);
                }
                lua_pop(L, 1);
            }
        }
    }
    else if (lua_type(L, idx) == LUA_TUSERDATA || lua_type(L, idx) == LUA_TLIGHTUSERDATA)
    {
        if (lua_getmetatable(L, idx))
        {
            lua_getfield(L, -1, "__index");
            if (lua_istable(L, -1))
                collectObjectKeys(L, -1, out, depth + 1, false);
            lua_pop(L, 2);
        }
    }
}

static bool evaluateObject(lua_State* L, const std::string& expr, bool metaOnly, std::vector<std::string>& out)
{
    if (expr.empty() || expr.size() > 256 || !exprIsSafe(expr)) return false;
    for (char c : expr) if (c == '=' || c == ';') return false;

    const std::string code = "return " + expr;
    if (luaL_loadbuffer(L, code.data(), code.size(), "=pse-comp") != LUA_OK)
    {
        lua_pop(L, 1);
        return false;
    }
    if (lua_pcall(L, 0, 1, 0) != LUA_OK)
    {
        lua_pop(L, 1);
        return false;
    }
    collectObjectKeys(L, -1, out, 0, metaOnly);
    lua_pop(L, 1);
    return !out.empty();
}

static void computeMatches(lua_State* L, Editor& ed)
{
    const wchar_t* delims = L" \t():.=,;[]\"{}+-*/";
    size_t start = ed.cursor;
    while (start > 0 && wcschr(delims, ed.line[start - 1]) == nullptr) --start;
    ed.tokenStart = start;

    const std::wstring token = ed.line.substr(start, ed.cursor - start);
    const std::string t = toUtf8(token);
    const wchar_t sep = start > 0 ? ed.line[start - 1] : 0;

    // After ':' / '.' completion is scoped to the object that precedes the
    // separator: only that object's members are offered, never the global
    // word list. An empty head (e.g. a continuation line ":position(") is the
    // only case that falls back to the global words.
    bool scoped = false;
    std::vector<std::string> names;
    if (sep == ':' || sep == '.')
    {
        std::wstring head = ed.line.substr(0, start);
        while (!head.empty() &&
               (head.back() == L':' || head.back() == L'.' ||
                head.back() == L' ' || head.back() == L'\t'))
            head.pop_back();
        const std::string expr = toUtf8(head);
        if (!expr.empty())
        {
            scoped = true;
            if (!evaluateObject(L, expr, sep == ':', names))
                names.clear();
        }
    }

    auto addUnique = [&](const std::string& w)
    {
        if ((int)ed.matches.size() >= 5) return;
        const std::wstring ww = toWide(w);
        for (const auto& m : ed.matches) if (m == ww) return;
        ed.matches.push_back(ww);
    };

    auto push = [&](const std::string& w)
    {
        if (w.size() < t.size()) return;
        if (std::strncmp(w.c_str(), t.c_str(), t.size()) != 0) return;
        addUnique(w);
    };

    ed.matches.clear();
    if (scoped)
    {
        for (const std::string& w : names) if (std::strcmp(w.c_str(), t.c_str()) == 0) push(w);
        for (const std::string& w : names) if (std::strcmp(w.c_str(), t.c_str()) != 0) push(w);
    }
    else
    {
        for (const char* w : kCompletionWords) push(w);
        for (const char* w : kMethodWords) push(w);
    }

    if (ed.matches.size() == 1 && ed.matches[0] == token) ed.matches.clear();
    if (ed.selected >= (int)ed.matches.size()) ed.selected = 0;
}

static bool readLineInteractive(lua_State* L, const std::string& promptUtf8, std::string& out)
{
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD inMode = 0;
    if (hIn == INVALID_HANDLE_VALUE || hOut == INVALID_HANDLE_VALUE ||
        !GetConsoleMode(hIn, &inMode))
        return false;

    Editor ed(hOut, toWide(promptUtf8));
    computeMatches(L, ed);
    ed.redraw();

    SetConsoleMode(hIn, (inMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)) | ENABLE_PROCESSED_INPUT);

    auto refresh = [&]()
    {
        computeMatches(L, ed);
        ed.redraw();
    };

    for (;;)
    {
        INPUT_RECORD rec;
        DWORD n = 0;
        if (!ReadConsoleInputW(hIn, &rec, 1, &n) || n == 0) break;
        if (rec.EventType != KEY_EVENT) continue;
        const KEY_EVENT_RECORD& ke = rec.Event.KeyEvent;
        if (!ke.bKeyDown) continue;

        const WORD vk = ke.wVirtualKeyCode;
        const wchar_t ch = ke.uChar.UnicodeChar;
        const bool ctrl = (ke.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;

        if (vk == VK_RETURN)
        {
            ed.matches.clear();
            ed.redraw();
            std::cout << "\n";
            out = toUtf8(ed.line);
            SetConsoleMode(hIn, inMode);
            return true;
        }
        if (ctrl && vk == 'C')
        {
            ed.matches.clear();
            ed.redraw();
            std::cout << "\n";
            SetConsoleMode(hIn, inMode);
            std::exit(0);
        }
        if (vk == VK_TAB)
        {
            if (!ed.matches.empty())
            {
                const std::wstring& word = ed.matches[ed.selected];
                ed.line.replace(ed.tokenStart, ed.cursor - ed.tokenStart, word);
                ed.cursor = ed.tokenStart + word.size();
            }
            refresh();
            continue;
        }
        if (vk == VK_UP)
        {
            if (!ed.matches.empty())
            {
                if (--ed.selected < 0) ed.selected = (int)ed.matches.size() - 1;
                ed.redraw();
            }
            continue;
        }
        if (vk == VK_DOWN)
        {
            if (!ed.matches.empty())
            {
                ed.selected = (ed.selected + 1) % (int)ed.matches.size();
                ed.redraw();
            }
            continue;
        }
        if (vk == VK_ESCAPE)
        {
            ed.matches.clear();
            ed.selected = 0;
            ed.redraw();
            continue;
        }
        if (vk == VK_BACK) { if (ed.cursor > 0) { ed.line.erase(ed.cursor - 1, 1); --ed.cursor; refresh(); } continue; }
        if (vk == VK_DELETE) { if (ed.cursor < ed.line.size()) { ed.line.erase(ed.cursor, 1); refresh(); } continue; }
        if (vk == VK_LEFT) { if (ed.cursor > 0) { --ed.cursor; refresh(); } continue; }
        if (vk == VK_RIGHT) { if (ed.cursor < ed.line.size()) { ++ed.cursor; refresh(); } continue; }
        if (vk == VK_HOME) { ed.cursor = 0; refresh(); continue; }
        if (vk == VK_END) { ed.cursor = ed.line.size(); refresh(); continue; }

        if (ch >= 32 && ch != 127)
        {
            ed.line.insert(ed.cursor, 1, ch);
            ++ed.cursor;
            refresh();
        }
    }

    SetConsoleMode(hIn, inMode);
    return false;
}

static bool isConsoleInput()
{
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    return h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode) != 0;
}

static bool readLine(lua_State* L, const std::string& prompt, std::string& out)
{
    if (!isConsoleInput())
    {
        std::cout << prompt << std::flush;
        return (bool)std::getline(std::cin, out);
    }
    return readLineInteractive(L, prompt, out);
}

static void replLine(lua_State* L, const std::string& raw)
{
    std::string line = raw;
    if (!line.empty() && line.back() == '\r') line.pop_back();

    if (line == "help")
    {
        std::cout <<
            "\n  help                             show this text\n"
            "  exit / quit                      leave the interpreter\n"
            "  PSE.initialize()                 connect to Portal: Solver (Shared Memory)\n"
            "  PSE.game:initialize()            put the player on the SDK level\n"
            "  PSE.game:deinitialize()          return to the main menu\n"
            "  PSE.createMeshObject():geometry('FACE'):texture('FLOOR_WHITE'):position(0,0,-100):create()\n"
            "  PSE.createDoor():position(100,200,-100):state(1):create()\n"
            "  PSE.get(guid):setState(1)            PSE.get(guid):getState()\n"
            "  PSE.get(guid):setRegisters{...}\n"
            "  PSE.onElementChanged(fn)         register a global event handler\n"
            "  PSE.pollAll()                    process events from the game (callbacks)\n"
            "  PSE.run(seconds)                 run the event loop (Ctrl+C to stop)\n"
            "  pse.player:getPosition()         object-style API (pse.game/pse.gun/...)\n"
            "  core.set_mock(true)              offline simulation mode (no game required)\n"
            "\n"
            "  Object builders can span several lines; input is collected until\n"
            "  the chain ends with ':create()' (a blank line commits it as-is):\n"
            "    local door = PSE.createDoor()\n"
            "        :position(0, 100, 0)\n"
            "        :name(\"door\")\n"
            "        :create()\n"
            "\n";
        return;
    }
    if (line == "exit" || line == "quit")
    {
        std::cout << "bye!\n";
        std::exit(0);
    }

    if (line.empty()) return;

    std::string buf = line;
    while (true)
    {
        if (tryRunBuffer(L, buf, false)) return;

        std::string next;
        if (!readLine(L, "...> ", next)) break;
        if (!next.empty() && next.back() == '\r') next.pop_back();

        if (next == "exit" || next == "quit")
        {
            std::cout << "bye!\n";
            std::exit(0);
        }

        if (next.empty())
        {
            tryRunBuffer(L, buf, true);
            return;
        }

        buf += "\n" + next;
    }

    tryRunBuffer(L, buf, true);
}

static bool runScript(lua_State* L, const std::string& path)
{
    if (luaL_loadfile(L, path.c_str()) != LUA_OK)
    {
        reportError(L);
        return false;
    }
    runChunkOnTop(L);
    return true;
}

static void extendPackagePath(lua_State* L)
{
    char exe[MAX_PATH];
    DWORD n = GetModuleFileNameA(nullptr, exe, MAX_PATH);
    std::string exePath(exe, n);
    auto pos = exePath.find_last_of('\\');
    std::string dir = pos == std::string::npos ? "." : exePath.substr(0, pos);

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    std::string current = lua_tostring(L, -1) ? lua_tostring(L, -1) : "";
    lua_pop(L, 1);

    std::string extra;
    const char* candidates[] = { "lua", "../lua" };
    for (const char* c : candidates)
    {
        std::string dirPath = dir + "\\" + c;
        if (std::filesystem::is_directory(dirPath))
            extra += (extra.empty() ? "" : ";") + dirPath + "\\?.lua";
    }

    if (!extra.empty())
    {
        lua_pushstring(L, (extra + ";" + current).c_str());
        lua_setfield(L, -2, "path");
    }
    lua_pop(L, 1);
}

static bool loadSdk(lua_State* L)
{
    lua_getglobal(L, "require");
    lua_pushstring(L, "sdk.init");
    if (lua_pcall(L, 1, 0, 0) != LUA_OK)
    {
        reportError(L);
        return false;
    }
    return true;
}

static void callGlobalStr(lua_State* L, const char* global, const char* fn, int argc, const char* const* argv)
{
    lua_getglobal(L, global);
    if (!lua_istable(L, -1)) { lua_pop(L, 1); return; }
    lua_getfield(L, -1, fn);
    if (!lua_isfunction(L, -1)) { lua_pop(L, 2); return; }
    lua_remove(L, -2);
    for (int i = 0; i < argc; ++i) lua_pushstring(L, argv[i]);
    if (lua_pcall(L, argc, 0, 0) != LUA_OK) reportError(L);
}

static void printBanner()
{
    std::cout <<
        "\n"
        "  .-=:[ PSE SDK Lua ]:=-.   simplified PSE SDK + interpreter\n"
        "  version " << kVersion << "   (embedded Lua 5.4, pse-sdk statically linked)\n"
        "  type 'help' for commands, 'exit' to quit\n"
        "\n";
}

int main(int argc, char** argv)
{
    enableVirtualTerminal();

    psebridge::load();
    if (psebridge::isLoaded())
        printf("  [bridge] pse-sdk connected: %s\n", psebridge::dllPath().c_str());
    else
        printf("  [bridge] pse-sdk: game not running - use core.set_mock(true) for offline work\n");

    lua_State* L = luaL_newstate();
    if (!L) { std::cerr << "Failed to create Lua state\n"; return -1; }
    luaL_openlibs(L);

    psebridge::lua_register_core(L);
    extendPackagePath(L);

    bool sdkOk = loadSdk(L);
    if (sdkOk)
    {
        const char* dll = psebridge::isLoaded() ? psebridge::dllPath().c_str() : "(offline)";
        const char* msgs[4] = { "PSE", "PSE SDK Lua %s loaded (pse-sdk: %s)", kVersion, dll };
        callGlobalStr(L, "Logger", "info", 4, msgs);
    }

    bool mock = false;
    int start = 1;
    while (start < argc && std::strcmp(argv[start], "--mock") == 0) { mock = true; ++start; }
    if (mock)
    {
        if (luaL_dostring(L, "core.set_mock(true)") != LUA_OK) reportError(L);
        std::cout << "  [bridge] mock mode ON (no game required)\n";
    }

    if (start < argc)
    {
        runScript(L, argv[start]);
    }
    else
    {
        printBanner();
        std::string line;
        while (readLine(L, "pse> ", line))
        {
            replLine(L, line);
        }
        std::cout << "\n";
    }

    if (sdkOk)
    {
        lua_getglobal(L, "PSE");
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "deinitialize");
            if (lua_isfunction(L, -1)) lua_pcall(L, 0, 0, 0);
        }
        lua_pop(L, 1);
    }

    psebridge::deinitialize();
    lua_close(L);
    psebridge::unload();
    return 0;
}
