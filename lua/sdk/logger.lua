--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Logger: timestamp/level/tag formatting, ANSI colors and optional file output.
--- --==-==--

local core = _G.core

local M = {}

local LEVELS = { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3, CRITICAL = 4 }
local LEVEL_COLOR = {
    DEBUG    = "\27[90m",
    INFO     = "\27[32m",
    WARNING  = "\27[33m",
    ERROR    = "\27[31m",
    CRITICAL = "\27[41;37m",
}

local PALETTE = {
    black = 0, red = 196, green = 46, yellow = 226, blue = 21,
    magenta = 201, cyan = 51, white = 15, gray = 244, darkgray = 240,
    lightgray = 250, orange = 208, pink = 213, purple = 129, brown = 130,
    teal = 37, navy = 18, gold = 220, silver = 7,
}

local defaultTag = "PSE"
local minLevel = LEVELS.DEBUG
local colors = true
local fileHandle = nil
local filePath = nil

local function timestamp()
    local ms = (core and core.millis) and core.millis() or (os.time() * 1000)
    local sec = math.floor(ms / 1000)
    local t = os.date("*t", sec)
    return ("%s:%02d:%02d:%02d.%03d")
        :format((":%02d"):format(t.day), t.hour, t.min, t.sec, ms % 1000)
end

local function parseSpecCodes(input, toAnsi)
    local out = {}
    local i, n = 1, #input
    while i <= n do
        local ch = input:sub(i, i)
        if ch == "&" and input:sub(i + 1, i + 2) == "R&" then
            if toAnsi then out[#out + 1] = "\27[0m" end
            i = i + 3
        elseif ch == "&" and input:sub(i + 1, i + 2) == "CD" then
            local close = input:find("&", i + 3)
            local handled = false
            if close then
                local inner = input:sub(i + 3, close - 1)
                if #inner >= 3 then
                    local cidx = tonumber(inner:sub(1, 2), 16)
                    if cidx and cidx >= 0 and cidx <= 255 then
                        local si = tonumber(inner:sub(3, 3))
                        local style = ({ [0] = 0, [1] = 1, [2] = 3, [3] = 4 })[si] or 0
                        if toAnsi then
                            local s = "\27["
                            if style ~= 0 then s = s .. style .. ";" end
                            s = s .. "38;5;" .. cidx .. "m"
                            out[#out + 1] = s
                        end
                        i = close + 1
                        handled = true
                    end
                end
            end
            if not handled then
                out[#out + 1] = ch
                i = i + 1
            end
        else
            out[#out + 1] = ch
            i = i + 1
        end
    end
    return table.concat(out)
end

local function doFormat(fmt, ...)
    local n = select("#", ...)
    if n == 0 then return tostring(fmt) end
    local ok, res = pcall(string.format, fmt, ...)
    if ok then return res end
    local parts = { tostring(fmt) }
    for i = 1, n do parts[#parts + 1] = tostring((select(i, ...))) end
    return table.concat(parts, " ")
end

local function write(levelName, ...)
    local lv = LEVELS[levelName]
    if not lv or lv < minLevel then return end

    local n = select("#", ...)
    local tag, fmt
    if n >= 2 and type(select(1, ...)) == "string" and type(select(2, ...)) == "string" then
        tag = select(1, ...)
        fmt = select(2, ...)
    else
        tag = defaultTag
        fmt = select(1, ...)
    end

    local raw = doFormat(fmt, select(3, ...))
    local ts = timestamp()

    if colors then
        local msg = parseSpecCodes(raw, true)
        local line = ("\27[90m%s\27[0m %s[%s]\27[0m \27[1m(%s)\27[0m >>> %s\27[0m")
            :format(ts, LEVEL_COLOR[levelName], levelName, tag, msg)
        print(line)
    else
        local msg = parseSpecCodes(raw, false)
        print(("%s [%s] (%s) >>> %s"):format(ts, levelName, tag, msg))
    end

    if fileHandle then
        local msg = parseSpecCodes(raw, false)
        fileHandle:write(("%s [%s] (%s) >>> %s\n"):format(ts, levelName, tag, msg))
        fileHandle:flush()
    end
end

function M.init()
    return true
end

function M.setTag(tag)
    local prev = defaultTag
    defaultTag = tostring(tag)
    return prev
end

function M.getTag()
    return defaultTag
end

function M.setFile(path)
    local prev = filePath
    if fileHandle then fileHandle:close(); fileHandle = nil end
    filePath = path
    if path then
        local f, err = io.open(path, "a")
        if not f then error("Logger: cannot open file: " .. tostring(err), 2) end
        fileHandle = f
        fileHandle:setvbuf("line")
    end
    return prev
end

function M.close()
    if fileHandle then fileHandle:close(); fileHandle = nil end
    filePath = nil
end

function M.setLevel(level)
    local prev = minLevel
    if type(level) == "number" then
        minLevel = level
    elseif LEVELS[level] then
        minLevel = LEVELS[level]
    else
        error("Logger: unknown level " .. tostring(level), 2)
    end
    return prev
end

function M.setColors(enabled)
    local prev = colors
    colors = enabled and true or false
    return prev
end

function M.color(color)
    if type(color) == "number" then
        return math.max(0, math.min(255, math.floor(color)))
    end
    return PALETTE[tostring(color)] or error("Logger: unknown color " .. tostring(color), 2)
end

function M.paint(text, color, style)
    local c = M.color(color)
    local s = tonumber(style) or 0
    return ("&CD%02X%d&%s&R&"):format(c, s, tostring(text))
end

function M.debug(...)      write("DEBUG", ...)    end
function M.info(...)       write("INFO", ...)     end
function M.warn(...)       write("WARNING", ...)  end
function M.warning(...)    write("WARNING", ...)  end
function M.error(...)      write("ERROR", ...)    end
function M.critical(...)   write("CRITICAL", ...) end

M.Levels = LEVELS
M.Palette = PALETTE

return M
