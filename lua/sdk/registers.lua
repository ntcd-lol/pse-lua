--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Как это работает:
--- * Битовые операции над u32-регистрами: getBit/setBit/getBits/setBits.
---   Бит-строка в docs/ru/concept/elements.md читается справа налево:
---   самый правый символ = бит 0.
--- * Раскладки регистров по классам: M.LAYOUT[class] = { reg -> descr }.
--- * Цвета: биты 8..23 как RRGGBB ("________RRRRRRRRGGGGGGGGBBBBBBBB"),
---   т.е. value = R<<16 | G<<8 | B (M.packRgb / M.unpackRgb).
--- * build(class, tbl): применяет поля из tbl к регистрам и возвращает
---   готовый массив из 8 регистров для команды ELEMENT_SET_ALL_REGISTERS.
--- --==-==--

local M = {}

function M.getBit(reg, offset)
    return (reg >> offset) & 1
end

function M.setBit(reg, offset, value)
    return (reg & ~(1 << offset)) | ((value & 1) << offset)
end

function M.getBits(reg, offset, count)
    if count == 32 then return (reg & 0xFFFFFFFF) end
    local mask = (1 << count) - 1
    return (reg >> offset) & mask
end

function M.setBits(reg, offset, count, value)
    if count == 32 then return value & 0xFFFFFFFF end
    local mask = (1 << count) - 1
    return (reg & ~(mask << offset)) | ((value & mask) << offset)
end

local RGB = { rgb = true }
local RGB_ONLY = { rgb = true, only = true }

M.LAYOUT = {

    DOOR = {
        [0] = { name = "flags", bits = { { "C", 0, 1 }, { "A", 1, 1 } } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    BUTTON = {
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    ANTI_EXPROPRIATION_FIELD = {
        [0] = { name = "flags", bits = {
            { "O", 0, 1 }, { "P", 1, 1 }, { "SolverGun", 2, 4 },
        } },
        [1] = { name = "color", rgb = true },
        [2] = { name = "size", pair = { "X", "Y" } },
    },

    PEDESTAL_BUTTON = {
        [0] = { name = "flags", bits = { { "TimerSeconds", 2, 7 }, { "Type", 0, 2 } } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    SOLVER_BUTTON = {
        [0] = { name = "flags", bits = {
            { "C", 10, 1 }, { "P", 9, 1 }, { "TimerSeconds", 2, 7 }, { "Type", 0, 2 },
        } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    INDICATOR = {
        [0] = { name = "flags", bits = { { "P", 1, 1 }, { "T", 0, 1 } } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    LASER_TX = {
        [0] = { name = "flags", bits = { { "Shifted", 0, 1 } } },
        [1] = { name = "laserColor", rgb = true },
    },

    LASER_RX = {
        [0] = { name = "flags", bits = { { "Shifted", 0, 1 } } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
        [3] = { name = "flagAndColor", bits = {
            { "V", 24, 1 }, { "R", 23, 8 }, { "G", 15, 8 }, { "B", 7, 8 },
        } },
    },

    LASER_RELAY = {
        [0] = { name = "flagAndColor", bits = {
            { "V", 24, 1 }, { "R", 23, 8 }, { "G", 15, 8 }, { "B", 7, 8 },
        } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    LASER_PANEL = {
        [0] = { name = "flags", bits = {
            { "B", 24, 1 }, { "MaxTilt", 23, 8 }, { "MinTilt", 15, 8 }, { "Angle", 7, 8 },
        } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    WEIGHT_CUBE = {
        [0] = { name = "flags", bits = { { "C", 0, 1 } } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    FAITH_PLATE = {
        [0] = { name = "flags", bits = { { "R", 1, 1 }, { "G", 0, 1 } } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
        [3] = { name = "targetXY", pair = { "X", "Y" } },
        [4] = { name = "targetZHeight", pair = { "Z", "Height" } },
    },

    PANEL = {
        [0] = { name = "animation", bits = { { "Animation", 0, 8 } } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    STAIRS = {
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    CUBE_DROPPER = {
        [0] = { name = "flags", bits = { { "R", 4, 1 }, { "CubeType", 0, 4 } } },
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    FLASHLIGHT = {},

    WINDOW = {
        [0] = { name = "flags", bits = {
            { "Material", 16, 16 },
            { "CornerBR", 10, 1 }, { "CornerBL", 9, 1 }, { "CornerTR", 8, 1 }, { "CornerTL", 7, 1 },
            { "EdgeR", 6, 1 }, { "EdgeL", 5, 1 }, { "EdgeB", 4, 1 }, { "EdgeT", 3, 1 },
            { "Center", 2, 1 }, { "PassSolver", 1, 1 }, { "PassLaser", 0, 1 },
        } },
        [1] = { name = "size", pair = { "X", "Y" } },
    },

    TRIGGER = {
        [0] = { name = "flags", bits = {
            { "Target", 8, 4 }, { "Condition", 4, 4 }, { "Action", 0, 4 },
        } },
    },

    LAMP = {
        [1] = { name = "colorActivated", rgb = true },
        [2] = { name = "colorDeactivated", rgb = true },
    },

    LASER_CUBE = {},
    SOLVER_GUN_PEDESTAL = {},
}

M.LAYOUT.ENTRY_ELEVATOR = M.LAYOUT.DOOR
M.LAYOUT.EXIT_ELEVATOR = M.LAYOUT.DOOR

function M.layout(class)
    return M.LAYOUT[class]
end

function M.build(class, regIndex, fields)
    local desc = M.LAYOUT[class]
    if not desc then return 0 end
    local d = desc[regIndex]
    if not d then return 0 end
    local reg = fields.raw or 0
    if d.bits then
        for _, f in ipairs(d.bits) do
            local v = fields[f[1]]
            if v ~= nil then reg = M.setBits(reg, f[2], f[3], v) end
        end
    elseif d.pair then
        local a, b = fields[d.pair[1]], fields[d.pair[2]]
        if a ~= nil then reg = M.setBits(reg, 16, 16, a) end
        if b ~= nil then reg = M.setBits(reg, 0, 16, b) end
    elseif d.rgb and fields.rgb then
        reg = fields.rgb
    end
    return reg
end

function M.extract(class, regIndex, reg)
    local desc = M.LAYOUT[class]
    if not desc then return { raw = reg } end
    local d = desc[regIndex]
    if not d then return { raw = reg } end
    local out = { raw = reg }
    if d.bits then
        for _, f in ipairs(d.bits) do
            out[f[1]] = M.getBits(reg, f[2], f[3])
        end
    elseif d.pair then
        out[d.pair[1]] = M.getBits(reg, 16, 16)
        out[d.pair[2]] = M.getBits(reg, 0, 16)
    end
    return out
end

function M.packRgb(r, g, b)
    return ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF)
end

function M.unpackRgb(v)
    return (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF
end

M.COLORS = {
    white = M.packRgb(255, 255, 255),
    black = M.packRgb(0, 0, 0),
    orange = M.packRgb(255, 120, 40),
    blue = M.packRgb(80, 140, 255),
    red = M.packRgb(255, 60, 60),
    green = M.packRgb(60, 255, 60),
    yellow = M.packRgb(255, 220, 60),
    pink = M.packRgb(255, 80, 160),
    purple = M.packRgb(160, 80, 255),
}

function M.color(color)
    if type(color) == "number" then return color end
    return M.COLORS[tostring(color)] or error("registers: unknown color " .. tostring(color), 2)
end

return M
