--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Declarative command table: name -> { code, ["in"], ["out"] } layouts.
--- --==-==--

local M = {}

local function cmd(code, inp, outp)
    return { code = code, ["in"] = inp or {}, ["out"] = outp or {} }
end

local guid = "guid"
local v3   = "vec3"
local q4   = "quat"
local tr   = "transform"

M.COMMANDS = {

    GAME_SET_CHEATS_ENABLED = cmd(0x00100001, { bEnabled = { 0, "i1" } }),
    GAME_GET_CHEATS_ENABLED = cmd(0x00100002, nil, { bEnabled = { 0, "i1" } }),
    GAME_SET_CHEATS_NOCLIP  = cmd(0x00100003, { bNoclipEnabled = { 0, "i1" } }),
    GAME_GET_CHEATS_NOCLIP  = cmd(0x00100004, nil, { bNoclipEnabled = { 0, "i1" } }),
    GAME_SET_GRAVITY        = cmd(0x00100005, { gravity = { 0, "f4" } }),
    GAME_GET_GRAVITY        = cmd(0x00100006, nil, { gravity = { 0, "f4" } }),
    GAME_CHECK_GUID_IS_VALID= cmd(0x00100007, { guid = { 0, guid } }),
    GAME_INITIALIZE         = cmd(0x00100008),
    GAME_DEINITIALIZE       = cmd(0x00100009),

    PLAYER_SET_LOCATION     = cmd(0x00200001, { location = { 0, v3 } }),
    PLAYER_GET_LOCATION     = cmd(0x00200002, nil, { location = { 0, v3 } }),
    PLAYER_SET_ROTATION     = cmd(0x00200003, { quat = { 0, q4 } }),
    PLAYER_GET_ROTATION     = cmd(0x00200004, nil, { quat = { 0, q4 } }),
    PLAYER_SPAWN            = cmd(0x00200005),
    PLAYER_KILL             = cmd(0x00200006),

    SOLVER_GUN_SET_ENABLED  = cmd(0x00300001, { bEnabled = { 0, "i1" } }),
    SOLVER_GUN_GET_ENABLED  = cmd(0x00300002, nil, { bEnabled = { 0, "i1" } }),
    SOLVER_GUN_ACTION_USE   = cmd(0x00300003),
    SOLVER_GUN_ACTION_RELEASE = cmd(0x00300004),
    SOLVER_GUN_ACTION_THROW = cmd(0x00300005),

    FLASHLIGHT_SET_ENABLED  = cmd(0x00400001, { bEnabled = { 0, "i1" } }),
    FLASHLIGHT_GET_ENABLED  = cmd(0x00400002, nil, { bEnabled = { 0, "i1" } }),
    FLASHLIGHT_SET_STATE    = cmd(0x00400003, { bState = { 0, "i1" } }),
    FLASHLIGHT_GET_STATE    = cmd(0x00400004, nil, { bState = { 0, "i1" } }),

    STATIC_MESH_CREATE      = cmd(0x00500001, {
        transform = { 0, tr }, mesh = { 40, "u2" }, material = { 42, "u2" },
    }),

    DYNAMIC_MESH_CREATE     = cmd(0x00600001, {
        transform = { 0, tr }, mesh = { 40, "u2" }, material = { 42, "u2" },
        bVisibility = { 44, "i1" },
    }, { guid = { 0, guid } }),
    DYNAMIC_MESH_SET_MATERIAL    = cmd(0x00600002, { guid = { 0, guid }, material = { 8, "u2" } }),
    DYNAMIC_MESH_SET_MESH        = cmd(0x00600003, { guid = { 0, guid }, mesh = { 8, "u2" } }),
    DYNAMIC_MESH_SET_VISIBILITY  = cmd(0x00600004, { guid = { 0, guid }, bVisibility = { 8, "i1" } }),
    DYNAMIC_MESH_GET_VISIBILITY  = cmd(0x00600005, { guid = { 0, guid } }, { bVisibility = { 0, "i1" } }),
    DYNAMIC_MESH_SET_TRANSFORM   = cmd(0x00600006, { guid = { 0, guid }, transform = { 8, tr } }),
    DYNAMIC_MESH_GET_TRANSFORM   = cmd(0x00600007, { guid = { 0, guid } }, { transform = { 0, tr } }),
    DYNAMIC_MESH_DESTROY         = cmd(0x00600008, { guid = { 0, guid } }),

    ELEMENT_CREATE = cmd(0x00700001, {
        transform = { 0, tr }, class = { 40, "u2" }, callback = { 42, "u8" },
        state = { 50, "i1" }, bVisibility = { 51, "i1" },
    }, { guid = { 0, guid } }),
    ELEMENT_SET_CALLBACK = cmd(0x00700002, { guid = { 0, guid }, callback = { 8, "u8" } }),
    ELEMENT_GET_CALLBACK = cmd(0x00700003, { guid = { 0, guid } }, { callback = { 0, "u8" } }),
    ELEMENT_SET_STATE    = cmd(0x00700004, { guid = { 0, guid }, state = { 8, "i1" } }),
    ELEMENT_GET_STATE    = cmd(0x00700005, { guid = { 0, guid } }, { state = { 0, "i1" } }),
    ELEMENT_SET_VISIBILITY = cmd(0x00700006, { guid = { 0, guid }, bVisibility = { 8, "i1" } }),
    ELEMENT_GET_VISIBILITY = cmd(0x00700007, { guid = { 0, guid } }, { bVisibility = { 0, "i1" } }),
    ELEMENT_SET_TRANSFORM  = cmd(0x00700008, { guid = { 0, guid }, transform = { 8, tr } }),
    ELEMENT_GET_TRANSFORM  = cmd(0x00700009, { guid = { 0, guid } }, { transform = { 0, tr } }),
    ELEMENT_SET_CLASS      = cmd(0x0070000A, { guid = { 0, guid }, class = { 8, "u2" } }),
    ELEMENT_GET_CLASS      = cmd(0x0070000B, { guid = { 0, guid } }, { class = { 0, "u2" } }),
    ELEMENT_SET_REGISTER   = cmd(0x0070000C, { guid = { 0, guid }, register = { 8, "u4" }, index = { 16, "i1" } }),
    ELEMENT_GET_REGISTER   = cmd(0x0070000D, { guid = { 0, guid }, index = { 8, "i1" } }, { register = { 0, "u4" } }),
    ELEMENT_SET_ALL_REGISTERS = cmd(0x0070000E, {
        guid = { 0, guid },
        registers = { 8, { "u4", 8 } },
    }),
    ELEMENT_GET_ALL_REGISTERS = cmd(0x0070000F, { guid = { 0, guid } }, {
        registers = { 0, { "u4", 8 } },
    }),
    ELEMENT_SET_STATE_AND_CALLBACK_AND_ALL_REGISTERS = cmd(0x00700010, {
        guid = { 0, guid }, registers = { 8, { "u4", 8 } },
        callback = { 40, "u8" }, state = { 48, "i1" },
    }),
    ELEMENT_GET_STATE_AND_CALLBACK_AND_ALL_REGISTERS = cmd(0x00700011, { guid = { 0, guid } }, {
        registers = { 0, { "u4", 8 } }, callback = { 32, "u8" }, state = { 40, "i1" },
    }),
    ELEMENT_DESTROY = cmd(0x00700012, { guid = { 0, guid } }),
}

return M
