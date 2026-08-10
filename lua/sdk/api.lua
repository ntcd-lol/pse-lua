--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Main SDK API: global PSE table with fluent element/mesh factories,
--- name resolvers, element registry, events, mock helpers and utilities.
--- --==-==--

local Enums = require("sdk.enums")
local Pack = require("sdk.pack")
local Registers = require("sdk.registers")
local Logger = require("sdk.logger")
local Core = require("sdk.core")

local PSE = {}

PSE.version = "0.1.0"

PSE.Core = Core
PSE.Logger = Logger
PSE.Registers = Registers

PSE.Mesh = Enums.MESH
PSE.Material = Enums.MATERIAL
PSE.Class = Enums.CLASS
PSE.Command = Enums.COMMAND
PSE.Event = Enums.EVENT
PSE.Result = Enums.RESULT

PSE.names = {
    mesh = function(v) return Enums.byValue(Enums.MESH, v) end,
    material = function(v) return Enums.byValue(Enums.MATERIAL, v) end,
    class = function(v) return Enums.byValue(Enums.CLASS, v) end,
    command = function(v) return Enums.byValue(Enums.COMMAND, v) end,
    result = function(v) return Enums.byValue(Enums.RESULT, v) end,
}

function PSE.deinitialize()
    Core.deinitialize()
end

function PSE.initialize()
    if core.mock() then return false end
    local r = Core.initialize()
    if r == 0 then
        Logger.info("MAIN", "connected to game (live mode)")
        local ok, err = pcall(Core.call, "GAME_INITIALIZE")
        if ok then
            Logger.info("MAIN", "game initialized: player moved to the SDK level")
        else
            Logger.warning("MAIN", "initializeGame() failed: %s", tostring(err))
        end
        return true
    end
    Logger.warning("MAIN", "game not reachable (initialize=%d) - switching to mock mode", r)
    core.set_mock(true)
    return false
end

PSE.synchronize = Core.synchronize
PSE.millis = Core.millis

function PSE.sleep(ms)
    Core.raw.sleep(ms)
end

function PSE.vec(x, y, z)
    if type(x) == "table" then return { x[1], x[2], x[3] } end
    return { x, y, z }
end

function PSE.quat(x, y, z, w)
    if type(x) == "table" then return { x[1], x[2], x[3], x[4] or 1 } end
    return { x, y, z, w or 1 }
end

function PSE.deg(pitch, yaw, roll)
    local d2r = math.pi / 180
    local p, y, r = pitch * d2r, yaw * d2r, roll * d2r
    local sp, cp = math.sin(p / 2), math.cos(p / 2)
    local sy, cy = math.sin(y / 2), math.cos(y / 2)
    local sr, cr = math.sin(r / 2), math.cos(r / 2)
    return {
        x = cr * sp * sy + sr * cp * cy,
        y = cr * sp * cy - sr * cp * sy,
        z = cr * cp * sy - sr * sp * cy,
        w = cr * cp * cy + sr * sp * sy,
    }
end

function PSE.color(r, g, b)
    return Registers.packRgb(r, g, b)
end

PSE.palette = Registers.COLORS

function PSE.guid(v)
    if type(v) == "table" then v = v.guid end
    return v
end

local registry = { byName = {}, byGuid = {} }

local function register(name, guid, entity)
    if name then registry.byName[name] = entity end
    if guid ~= nil then registry.byGuid[tostring(guid)] = entity end
end

function PSE.register(name, guidOrEntity)
    local entity = guidOrEntity
    if type(guidOrEntity) ~= "table" then
        entity = { guid = guidOrEntity }
        register(name, guidOrEntity, entity)
        return entity
    end
    register(name, entity.guid, entity)
    return entity
end

function PSE.get(key)
    if type(key) == "table" then return key end
    if type(key) == "string" then
        local e = registry.byName[key]
        if e then return e end
    end
    return registry.byGuid[tostring(key)]
end

local function defaultTransform()
    return { quat = { 0, 0, 0, 1 }, location = { 0, 0, 0 }, scale = { 1, 1, 1 } }
end

local function normalizeTransform(t)
    if not t then return defaultTransform() end
    local out = { quat = { 0, 0, 0, 1 }, location = { 0, 0, 0 }, scale = { 1, 1, 1 } }
    local q = t.quat or t.rotation or (t.x ~= nil and t or nil)
    local l = t.location or t.position
    local s = t.scale
    if q then
        out.quat = { q.x or q[1] or 0, q.y or q[2] or 0, q.z or q[3] or 0, q.w or q[4] or 1 }
    end
    if l then out.location = { l.x or l[1] or 0, l.y or l[2] or 0, l.z or l[3] or 0 } end
    if s then out.scale = { s.x or s[1] or 1, s.y or s[2] or 1, s.z or s[3] or 1 } end
    return out
end

local function flag(b)
    return b and 1 or 0
end

local MeshObject = {}
MeshObject.__index = MeshObject

function MeshObject.new(opts)
    opts = opts or {}
    local self = {
        transform = normalizeTransform(opts.transform),
        mesh = Enums.mesh(opts.mesh or opts.geometry or "CUBE"),
        material = Enums.material(opts.material or opts.texture or "WALL_WHITE_MEDIUM"),
        visibility = opts.visibility ~= false,
        static = opts.static == true,
        guid = nil,
        name = opts.name,
    }
    return setmetatable(self, MeshObject)
end

function MeshObject:position(x, y, z)
    if type(x) == "table" then self.transform.location = { x[1], x[2], x[3] }
    else self.transform.location = { x, y, z } end
    return self
end

function MeshObject:rotation(x, y, z, w)
    if type(x) == "table" then self.transform.quat = { x[1], x[2], x[3], x[4] or 1 }
    else self.transform.quat = { x, y, z, w or 1 } end
    return self
end

function MeshObject:scale(x, y, z)
    if type(x) == "table" then self.transform.scale = { x[1], x[2], x[3] }
    else self.transform.scale = { x, y, z } end
    return self
end

function MeshObject:transform(t)
    self.transform = normalizeTransform(t)
    return self
end

function MeshObject:geometry(m)
    self.mesh = Enums.mesh(m)
    return self
end

function MeshObject:texture(m)
    self.material = Enums.material(m)
    return self
end

function MeshObject:visible(b)
    self.visibility = b ~= false
    return self
end

function MeshObject:name(n)
    self.name = n
    return self
end

function MeshObject:create()
    if self.static then
        Core.call("STATIC_MESH_CREATE", {
            transform = self.transform, mesh = self.mesh, material = self.material,
        })
        register(self.name, nil, self)
        Logger.info("PSE", "created static %s '%s' (no guid)",
            Enums.byValue(Enums.MESH, self.mesh), rawget(self, "name") or "unnamed")
        return self
    end
    local out = Core.call("DYNAMIC_MESH_CREATE", {
        transform = self.transform,
        mesh = self.mesh,
        material = self.material,
        bVisibility = flag(self.visibility),
    }, true)
    self.guid = out.guid
    register(self.name, self.guid, self)
    Logger.info("PSE", "created %s '%s', guid = %s",
        Enums.byValue(Enums.MESH, self.mesh), rawget(self, "name") or "unnamed", self.guid)
    return self
end

function MeshObject:applyTransform()
    if self.static then
        error("MeshObject: static meshes cannot be transformed", 2)
    end
    Core.call("DYNAMIC_MESH_SET_TRANSFORM", { guid = self.guid, transform = self.transform })
    return self
end

function MeshObject:setMaterial(m)
    Core.call("DYNAMIC_MESH_SET_MATERIAL", { guid = self.guid, material = Enums.material(m) })
    self.material = Enums.material(m)
    return self
end

function MeshObject:setMesh(m)
    Core.call("DYNAMIC_MESH_SET_MESH", { guid = self.guid, mesh = Enums.mesh(m) })
    self.mesh = Enums.mesh(m)
    return self
end

function MeshObject:setVisibility(b)
    Core.call("DYNAMIC_MESH_SET_VISIBILITY", { guid = self.guid, bVisibility = flag(b) })
    self.visibility = b ~= false
    return self
end

function MeshObject:getVisibility()
    local o = Core.call("DYNAMIC_MESH_GET_VISIBILITY", { guid = self.guid }, true)
    return o.bVisibility == 1
end

function MeshObject:setPosition(x, y, z)
    return self:position(x, y, z):applyTransform()
end

function MeshObject:setRotation(x, y, z, w)
    return self:rotation(x, y, z, w):applyTransform()
end

function MeshObject:setScale(x, y, z)
    return self:scale(x, y, z):applyTransform()
end

function MeshObject:setTransform(t)
    return self:transform(t):applyTransform()
end

function MeshObject:getTransform()
    local o = Core.call("DYNAMIC_MESH_GET_TRANSFORM", { guid = self.guid }, true)
    self.transform = normalizeTransform(o.transform)
    return self.transform
end

function MeshObject:destroy()
    if self.static then return self end
    Core.call("DYNAMIC_MESH_DESTROY", { guid = self.guid })
    self.guid = nil
    return self
end

local Element = {}
Element.__index = Element

function Element.new(class, opts)
    opts = opts or {}
    local self = {
        class = Enums.class(class),
        transform = normalizeTransform(opts.transform),
        _state = opts.state or 0,
        visibility = opts.visibility ~= false,
        callback = opts.callback or opts.onChange,
        callbackId = nil,
        registers = {},
        guid = nil,
        name = opts.name,
    }
    if opts.registers then
        for i = 1, 8 do self.registers[i] = opts.registers[i] or 0 end
    end
    return setmetatable(self, Element)
end

function Element:position(x, y, z)
    if type(x) == "table" then self.transform.location = { x[1], x[2], x[3] }
    else self.transform.location = { x, y, z } end
    return self
end

function Element:rotation(x, y, z, w)
    if type(x) == "table" then self.transform.quat = { x[1], x[2], x[3], x[4] or 1 }
    else self.transform.quat = { x, y, z, w or 1 } end
    return self
end

function Element:scale(x, y, z)
    if type(x) == "table" then self.transform.scale = { x[1], x[2], x[3] }
    else self.transform.scale = { x, y, z } end
    return self
end

function Element:transform(t)
    self.transform = normalizeTransform(t)
    return self
end

function Element:state(s)
    self._state = s
    return self
end

function Element:visible(b)
    self.visibility = b ~= false
    return self
end

function Element:onChange(fn)
    self.callback = fn
    return self
end

function Element:register(i, v)
    self.registers[i] = v
    return self
end

function Element:registers(t)
    for i = 1, 8 do self.registers[i] = t[i] or 0 end
    return self
end

function Element:name(n)
    self.name = n
    return self
end

function Element:create()
    if self.callback and not self.callbackId then
        self.callbackId = Core.registerCallback(self.callback)
    end
    local out = Core.call("ELEMENT_CREATE", {
        transform = self.transform,
        class = self.class,
        callback = self.callbackId or 0,
        state = self._state,
        bVisibility = flag(self.visibility),
    }, true)
    self.guid = out.guid
    register(self.name, self.guid, self)
    Logger.info("PSE", "created %s '%s', guid = %s",
        Enums.byValue(Enums.CLASS, self.class), rawget(self, "name") or "unnamed", self.guid)
    return self
end

function Element:applyTransform()
    Core.call("ELEMENT_SET_TRANSFORM", { guid = self.guid, transform = self.transform })
    return self
end

function Element:setState(s)
    self._state = s
    Core.call("ELEMENT_SET_STATE", { guid = self.guid, state = s })
    return self
end

function Element:getState()
    local o = Core.call("ELEMENT_GET_STATE", { guid = self.guid }, true)
    return o.state
end

function Element:setVisibility(b)
    Core.call("ELEMENT_SET_VISIBILITY", { guid = self.guid, bVisibility = flag(b) })
    self.visibility = b ~= false
    return self
end

function Element:getVisibility()
    local o = Core.call("ELEMENT_GET_VISIBILITY", { guid = self.guid }, true)
    return o.bVisibility == 1
end

function Element:setPosition(x, y, z)
    return self:position(x, y, z):applyTransform()
end

function Element:setRotation(x, y, z, w)
    return self:rotation(x, y, z, w):applyTransform()
end

function Element:setScale(x, y, z)
    return self:scale(x, y, z):applyTransform()
end

function Element:setTransform(t)
    return self:transform(t):applyTransform()
end

function Element:getTransform()
    local o = Core.call("ELEMENT_GET_TRANSFORM", { guid = self.guid }, true)
    self.transform = normalizeTransform(o.transform)
    return self.transform
end

function Element:setClass(c)
    self.class = Enums.class(c)
    Core.call("ELEMENT_SET_CLASS", { guid = self.guid, class = self.class })
    return self
end

function Element:getClass()
    local o = Core.call("ELEMENT_GET_CLASS", { guid = self.guid }, true)
    return o.class
end

function Element:setRegister(i, v)
    self.registers[i] = v
    Core.call("ELEMENT_SET_REGISTER", { guid = self.guid, register = v, index = i })
    return self
end

function Element:getRegister(i)
    local o = Core.call("ELEMENT_GET_REGISTER", { guid = self.guid, index = i }, true)
    return o.register
end

function Element:setRegisters(t)
    for i = 1, 8 do self.registers[i] = t[i] or 0 end
    Core.call("ELEMENT_SET_ALL_REGISTERS", { guid = self.guid, registers = self.registers })
    return self
end

function Element:getRegisters()
    local o = Core.call("ELEMENT_GET_ALL_REGISTERS", { guid = self.guid }, true)
    for i = 1, 8 do self.registers[i] = o.registers[i] end
    return self.registers
end

function Element:setCallback(fn)
    if self.callbackId then Core.releaseCallback(self.callbackId) end
    self.callback = fn
    self.callbackId = Core.registerCallback(fn)
    Core.call("ELEMENT_SET_CALLBACK", { guid = self.guid, callback = self.callbackId })
    return self
end

function Element:destroy()
    if self.callbackId then Core.releaseCallback(self.callbackId); self.callbackId = nil end
    if self.guid then Core.call("ELEMENT_DESTROY", { guid = self.guid }) end
    self.guid = nil
    return self
end

function PSE.createMeshObject(opts)
    return MeshObject.new(opts)
end

function PSE.createStaticMesh(opts)
    opts = opts or {}
    opts.static = true
    return MeshObject.new(opts)
end

function PSE.createElement(class, opts)
    return Element.new(class, opts)
end

function PSE.spawnMeshObject(opts)
    return PSE.createMeshObject(opts):create()
end

function PSE.spawnStaticMesh(opts)
    return PSE.createStaticMesh(opts):create()
end

function PSE.spawnElement(class, opts)
    return PSE.createElement(class, opts):create()
end

function PSE.createButton(opts)     return PSE.createElement("BUTTON", opts) end
function PSE.createDoor(opts)       return PSE.createElement("DOOR", opts) end
function PSE.createLamp(opts)       return PSE.createElement("LAMP", opts) end
function PSE.createTrigger(opts)    return PSE.createElement("TRIGGER", opts) end
function PSE.createWeightCube(opts) return PSE.createElement("WEIGHT_CUBE", opts) end
function PSE.createLaserTx(opts)    return PSE.createElement("LASER_TX", opts) end
function PSE.createLaserRx(opts)    return PSE.createElement("LASER_RX", opts) end
function PSE.createLaserRelay(opts) return PSE.createElement("LASER_RELAY", opts) end
function PSE.createLaserPanel(opts) return PSE.createElement("LASER_PANEL", opts) end
function PSE.createFaithPlate(opts) return PSE.createElement("FAITH_PLATE", opts) end
function PSE.createIndicator(opts)  return PSE.createElement("INDICATOR", opts) end
function PSE.createPedestalButton(opts) return PSE.createElement("PEDESTAL_BUTTON", opts) end
function PSE.createSolverButton(opts)   return PSE.createElement("SOLVER_BUTTON", opts) end
function PSE.createWindow(opts)         return PSE.createElement("WINDOW", opts) end

PSE.player = {
    getPosition = function(self) return Core.call("PLAYER_GET_LOCATION", nil, true).location end,
    setPosition = function(self, x, y, z) Core.call("PLAYER_SET_LOCATION", { location = PSE.vec(x, y, z) }); return self end,
    getRotation = function(self) return Core.call("PLAYER_GET_ROTATION", nil, true).quat end,
    setRotation = function(self, x, y, z, w) Core.call("PLAYER_SET_ROTATION", { quat = PSE.quat(x, y, z, w) }); return self end,
    spawn       = function(self) Core.call("PLAYER_SPAWN"); return self end,
    kill        = function(self) Core.call("PLAYER_KILL"); return self end,
}

PSE.game = {
    initialize       = function(self) Core.call("GAME_INITIALIZE"); return self end,
    deinitialize     = function(self) Core.call("GAME_DEINITIALIZE"); return self end,
    setCheatsEnabled = function(self, b) Core.call("GAME_SET_CHEATS_ENABLED", { bEnabled = flag(b) }); return self end,
    getCheatsEnabled = function(self) return Core.call("GAME_GET_CHEATS_ENABLED", nil, true).bEnabled == 1 end,
    setNoclip        = function(self, b) Core.call("GAME_SET_CHEATS_NOCLIP", { bNoclipEnabled = flag(b) }); return self end,
    getNoclip        = function(self) return Core.call("GAME_GET_CHEATS_NOCLIP", nil, true).bNoclipEnabled == 1 end,
    setGravity       = function(self, g) Core.call("GAME_SET_GRAVITY", { gravity = g }); return self end,
    getGravity       = function(self) return Core.call("GAME_GET_GRAVITY", nil, true).gravity end,
    checkGuid        = function(self, guid) Core.call("GAME_CHECK_GUID_IS_VALID", { guid = PSE.guid(guid) }); return self end,
}

PSE.gun = {
    setEnabled = function(self, b) Core.call("SOLVER_GUN_SET_ENABLED", { bEnabled = flag(b) }); return self end,
    getEnabled = function(self) return Core.call("SOLVER_GUN_GET_ENABLED", nil, true).bEnabled == 1 end,
    use        = function(self) Core.call("SOLVER_GUN_ACTION_USE"); return self end,
    release    = function(self) Core.call("SOLVER_GUN_ACTION_RELEASE"); return self end,
    throw      = function(self) Core.call("SOLVER_GUN_ACTION_THROW"); return self end,
}

PSE.flashlight = {
    setEnabled = function(self, b) Core.call("FLASHLIGHT_SET_ENABLED", { bEnabled = flag(b) }); return self end,
    getEnabled = function(self) return Core.call("FLASHLIGHT_GET_ENABLED", nil, true).bEnabled == 1 end,
    setState   = function(self, b) Core.call("FLASHLIGHT_SET_STATE", { bState = flag(b) }); return self end,
    getState   = function(self) return Core.call("FLASHLIGHT_GET_STATE", nil, true).bState == 1 end,
}

local handlers = {}

function PSE.on(eventName, fn)
    if type(fn) ~= "function" then error("PSE.on: expected function", 2) end
    local list = handlers[eventName] or {}
    list[#list + 1] = fn
    handlers[eventName] = list
    return PSE
end

function PSE.onElementChanged(fn)
    return PSE.on("ELEMENT_CHANGED", fn)
end

function PSE.poll()
    local ev = Core.poll()
    if not ev then return nil end
    local list = handlers[ev.event]
    if list then
        for _, fn in ipairs(list) do
            local ok, err = pcall(fn, ev)
            if not ok then Logger.error("HANDLER", "handler error: %s", tostring(err)) end
        end
    end
    return ev
end

function PSE.pollAll()
    local out = {}
    while true do
        local ev = PSE.poll()
        if not ev then break end
        out[#out + 1] = ev
    end
    return out
end

function PSE.run(duration)
    local start = Core.millis()
    while true do
        PSE.pollAll()
        if duration then
            if (Core.millis() - start) >= duration * 1000 then break end
        end
        Core.raw.sleep(1)
    end
end

PSE.mock = {}

function PSE.mock.emit(target, state)
    local entity = PSE.get(target)
    if not entity then error("PSE.mock.emit: unknown element " .. tostring(target), 2) end
    local payload = Pack.new()
    payload = Pack.set(payload, 0, "guid", entity.guid)
    payload = Pack.set(payload, 8, "u8", entity.callbackId or 0)
    payload = Pack.set(payload, 16, "i1", state or 0)
    Core.raw.mock_emit(Enums.EVENT.ELEMENT_CHANGED, payload)
end

function PSE.mock.emitRaw(guid, callbackId, state)
    local payload = Pack.new()
    payload = Pack.set(payload, 0, "guid", guid)
    payload = Pack.set(payload, 8, "u8", callbackId or 0)
    payload = Pack.set(payload, 16, "i1", state or 0)
    Core.raw.mock_emit(Enums.EVENT.ELEMENT_CHANGED, payload)
end

return PSE
