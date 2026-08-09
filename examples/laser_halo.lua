--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Animated laser ring: 24 LASER_TX emitters with HSV colors (port of 02_LaserHalo).
--- --==-==--

local N = 24
local R1 = 800
local R2 = 10
local SPEED_1 = 0.6
local SPEED_2 = 1.5
local FRAMES = 60
local FRAME_MS = 50

local function hsvToRgb(h, s, v)
    local i = math.floor(h % 6)
    local f = h - math.floor(h)
    local p = v * (1 - s)
    local q = v * (1 - s * f)
    local t = v * (1 - s * (1 - f))
    local r, g, b
    if     i == 0 then r, g, b = v, t, p
    elseif i == 1 then r, g, b = q, v, p
    elseif i == 2 then r, g, b = p, v, t
    elseif i == 3 then r, g, b = p, q, v
    elseif i == 4 then r, g, b = t, p, v
    else               r, g, b = v, p, q
    end
    return r, g, b
end

PSE.createMeshObject()
    :geometry("FACE")
    :texture("FLOOR_BLACK")
    :scale(24, 24, 1)
    :position(0, 0, -1600)
    :create()

PSE.createMeshObject()
    :geometry("FACE")
    :texture("FLOOR_BLACK")
    :scale(2, 2, 1)
    :position(0, 0, -100)
    :visible(false)
    :create()

PSE.setSolverGunEnabled(false)

local lasers = {}
for i = 0, N - 1 do
    local r, g, b = hsvToRgb(6 * i / N, 1, 1)
    local tx = PSE.createLaserTx()
        :position(0, 0, -1600)
        :state(1)
        :register(1, PSE.color(math.floor(r * 255), math.floor(g * 255), math.floor(b * 255)))
        :create()
    lasers[#lasers + 1] = tx
end
Logger.info("MAIN", "created %d laser emitters", N)

for frame = 0, FRAMES - 1 do
    local t = frame * FRAME_MS / 1000
    for i = 0, N - 1 do
        local alpha = 2 * math.pi * i / N
        local lookAngle = SPEED_1 * t + alpha + SPEED_2 * t
        local x = R1 * math.cos(SPEED_1 * t + alpha) + R2 * math.cos(SPEED_2 * t)
        local y = R1 * math.sin(SPEED_1 * t + alpha) + R2 * math.sin(SPEED_2 * t)
        PSE.Core.push("ELEMENT_SET_TRANSFORM", {
            guid = lasers[i + 1].guid,
            transform = {
                quat = PSE.deg(0, lookAngle * 180 / math.pi, 0),
                location = { x, y, -1575 },
                scale = { 1, 1, 1 },
            },
        })
    end
    PSE.Core.synchronize()
    PSE.sleep(FRAME_MS)
end

Logger.info("MAIN", "animation finished")
