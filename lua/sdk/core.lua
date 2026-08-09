--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Как это работает:
--- * Обёртка над C-мостом "core" (предоставляется хостом/main.cpp).
--- * call(): декларативный вызов команд из sdk.schema - упаковывает
---   входные поля через sdk.pack, создаёт PseData, вызывает C-сторону,
---   распаковывает выходные поля.
--- * push_and_wait(): синхронный вызов с ожиданием результата (C).
--- * poll(): берёт событие из очереди хоста и декодирует его в таблицу
---   { header, guid, callback, state, data } через sdk.pack.
--- * invokeCallback(): защищённый вызов Lua-колбэка через xpcall
---   с печатью полного traceback при ошибке.
--- * Реестр колбэков: функция Lua <-> opaque u64 id (для поля callback).
--- * Делегирует в C: initialize/deinitialize/synchronize/millis/sleep.
--- --==-==--

local Enums = require("sdk.enums")
local Pack = require("sdk.pack")
local Schema = require("sdk.schema")

local c = _G.core
if not c then
    error("sdk.core: the C host bridge ('core' table) is not available", 2)
end

local M = {}

M.raw = c

local callbacks = {}
local nextCallbackId = 1

function M.registerCallback(fn)
    if type(fn) ~= "function" then error("Core.registerCallback: expected function", 2) end
    local id = nextCallbackId
    nextCallbackId = nextCallbackId + 1
    callbacks[id] = fn
    return id
end

function M.releaseCallback(id)
    callbacks[id] = nil
end

function M.invokeCallback(id, guid, state)
    local fn = callbacks[id]
    if fn then
        local ok, err = xpcall(fn, function(e) return debug.traceback(tostring(e), 2) end, state, guid)
        if not ok then
            local Logger = require("sdk.logger")
            Logger.error("CALLBACK", "callback error: %s", tostring(err))
        end
    end
end

local function normalizeValue(t, value)
    if t == "i1" and type(value) == "boolean" then return value and 1 or 0 end
    if t == "guid" and type(value) == "table" and value.guid ~= nil then return value.guid end
    if (t == "vec3" or t == "quat") and type(value) == "table" and value.x ~= nil then
        if value.w ~= nil then return { value.x, value.y, value.z, value.w } end
        return { value.x, value.y, value.z }
    end
    if t == "u8" and type(value) == "function" then return M.registerCallback(value) end
    return value
end

function M.packInput(cmd, inFields)
    local payload = Pack.new()
    if inFields then
        for fname, spec in pairs(cmd["in"]) do
            local value = inFields[fname]
            if value ~= nil then
                payload = Pack.set(payload, spec[1], spec[2], normalizeValue(spec[2], value))
            end
        end
    end
    return payload
end

function M.call(command, inFields, outFields, opts)
    opts = opts or {}
    local name = tostring(command):upper()
    local cmd = Schema.COMMANDS[name]
    if not cmd then error("Core.call: unknown command " .. name, 2) end

    local payload = M.packInput(cmd, inFields)
    local code, data = c.push_and_wait(cmd.code, payload)
    local res = { result = code, data = data }

    if code ~= 0 then
        local rname = Enums.byValue(Enums.RESULT, code)
        if opts.assert == false then
            return nil, rname, res
        end
        error(("Core.call: %s failed -> %s (0x%08X)"):format(name, rname, code), 2)
    end

    local out = {}
    if outFields and cmd["out"] then
        for fname, spec in pairs(cmd["out"]) do
            out[fname] = Pack.get(res.data, spec[1], spec[2])
        end
    end
    return out
end

function M.push(command, inFields)
    local name = tostring(command):upper()
    local cmd = Schema.COMMANDS[name]
    if not cmd then error("Core.push: unknown command " .. name, 2) end
    c.push(cmd.code, M.packInput(cmd, inFields))
end

function M.synchronize()
    c.synchronize()
end

function M.poll()
    local header, data = c.poll_event()
    if not header then return nil end
    if header == Enums.EVENT.ELEMENT_CHANGED then
        local guid = Pack.get(data, 0, "guid")
        local callbackId = Pack.get(data, 8, "u8")
        local state = Pack.get(data, 16, "i1")
        if callbackId ~= 0 then
            M.invokeCallback(callbackId, guid, state)
        end
        return { event = "ELEMENT_CHANGED", guid = guid, state = state }
    end
    if header == Enums.EVENT.GAME_TICK_OVERFLOW then
        return { event = "GAME_TICK_OVERFLOW" }
    end
    return { event = "UNKNOWN", header = header }
end

function M.pollAll()
    local out = {}
    while true do
        local ev = M.poll()
        if not ev then break end
        out[#out + 1] = ev
    end
    return out
end

function M.initialize()
    return c.initialize()
end

function M.deinitialize()
    c.deinitialize()
end

function M.millis()
    return c.millis()
end

M.command = Enums.command
M.result = Enums.result
M.event = Enums.event
M.Result = Enums.RESULT
M.Event = Enums.EVENT
M.Command = Enums.COMMAND

function M.byValue(map, value)
    return Enums.byValue(map, value)
end

return M
