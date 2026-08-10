--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- PseData (60 bytes) pack/unpack via string.pack (little-endian),
--- matching the C layout in include/pse/structures.h.
--- --==-==--

local Pack = {}

local typeOf = type

Pack.SIZE = 60

local FMT = {
    i1 = "i1", u1 = "I1", i2 = "i2", u2 = "I2", i4 = "i4", u4 = "I4",
    i8 = "i8", u8 = "I8", f4 = "f",
}

function Pack.new()
    return ("\0"):rep(Pack.SIZE)
end

function Pack.size(type)
    if type == "vec3" then return 12 end
    if type == "quat" then return 16 end
    if type == "transform" then return 40 end
    if type == "guid" then return 8 end
    if type.type then return type.count * Pack.size(type.type) end
    if typeOf(type) == "table" and type[1] and type[2] then return type[2] * Pack.size(type[1]) end
    return #string.pack("<" .. FMT[type], 0)
end

local function splitAt(buf, offset, written)
    return buf:sub(1, offset), buf:sub(offset + 1 + written)
end

local function guidToBytes(v)
    if type(v) == "string" then
        if #v == 8 then return v end
        if #v == 16 then
            local hi = tonumber(v:sub(1, 8), 16)
            local lo = tonumber(v:sub(9, 16), 16)
            return string.pack("<I4I4", lo, hi)
        end
        error("guid: expected 16 hex digits or 8 raw bytes, got #" .. #v, 3)
    end
    if math.type(v) ~= "integer" then
        error("guid: expected integer or hex string, got " .. tostring(v), 3)
    end
    if v < 0 or v > 0x7FFFFFFFFFFFFFFF then
        error("guid: out of u64 range (use hex string beyond 2^63-1): " .. tostring(v), 3)
    end
    local hi = math.floor(v / 4294967296)
    local lo = v - hi * 4294967296
    return string.pack("<I4I4", lo, hi)
end

local function bytesToGuid(b)
    local lo, hi = string.unpack("<I4I4", b)
    if hi == 0 then return lo end
    return ("%08x%08x"):format(hi, lo)
end

local function setScalar(buf, offset, type, value)
    local s = string.pack("<" .. FMT[type], value)
    local h, t = splitAt(buf, offset, #s)
    return h .. s .. t
end

local function getScalar(buf, offset, type)
    return string.unpack("<" .. FMT[type], buf, offset + 1)
end

local function axis3(v)
    return v[1] or v.x or 0, v[2] or v.y or 0, v[3] or v.z or 0
end

local function axis4(v)
    return v[1] or v.x or 0, v[2] or v.y or 0, v[3] or v.z or 0, v[4] or v.w or 1
end

function Pack.set(buf, offset, type, value)
    if type.type then
        local count, base = type.count, type.type
        for i = 1, count do
            buf = Pack.set(buf, offset + (i - 1) * Pack.size(base), base, value[i])
        end
        return buf
    end
    if typeOf(type) == "table" and type[1] and type[2] then
        local count, base = type[2], type[1]
        for i = 1, count do
            buf = Pack.set(buf, offset + (i - 1) * Pack.size(base), base, value[i])
        end
        return buf
    end
    if type == "vec3" then
        local s = string.pack("<fff", axis3(value))
        local h, t = splitAt(buf, offset, 12)
        return h .. s .. t
    end
    if type == "quat" then
        local s = string.pack("<ffff", axis4(value))
        local h, t = splitAt(buf, offset, 16)
        return h .. s .. t
    end
    if type == "transform" then
        local q = value.quat or value.rotation or { 0, 0, 0, 1 }
        local l = value.location or value.position
        local sc = value.scale or { 1, 1, 1 }
        local q1, q2, q3, q4 = axis4(q)
        local l1, l2, l3 = axis3(l)
        local s1, s2, s3 = axis3(sc)
        local s = string.pack("<fffffffff f", q1, q2, q3, q4, l1, l2, l3, s1, s2, s3)
        local h, t = splitAt(buf, offset, 40)
        return h .. s .. t
    end
    if type == "guid" then
        local s = guidToBytes(value)
        local h, t = splitAt(buf, offset, 8)
        return h .. s .. t
    end
    return setScalar(buf, offset, type, value)
end

function Pack.get(buf, offset, type)
    if type.type then
        local count, base = type.count, type.type
        local out = {}
        for i = 1, count do
            out[i] = Pack.get(buf, offset + (i - 1) * Pack.size(base), base)
        end
        return out
    end
    if typeOf(type) == "table" and type[1] and type[2] then
        local count, base = type[2], type[1]
        local out = {}
        for i = 1, count do
            out[i] = Pack.get(buf, offset + (i - 1) * Pack.size(base), base)
        end
        return out
    end
    if type == "vec3" then
        local x, y, z = string.unpack("<fff", buf, offset + 1)
        return { x, y, z }
    end
    if type == "quat" then
        local a, b, c, d = string.unpack("<ffff", buf, offset + 1)
        return { a, b, c, d }
    end
    if type == "transform" then
        local a, b, c, d, e, f, g, h, i, j = string.unpack("<fffffffff f", buf, offset + 1)
        return { quat = { a, b, c, d }, location = { e, f, g }, scale = { h, i, j } }
    end
    if type == "guid" then
        return bytesToGuid(buf:sub(offset + 1, offset + 8))
    end
    return getScalar(buf, offset, type)
end

return Pack
