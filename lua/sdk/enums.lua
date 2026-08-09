--- .-=#####=-.
--- PSE SDK Lua
--- |- Creator: ntcd_lol, opencode
--- \- Comment: :3
--- '-=#####=-'
--- ^         ^
--- Numeric constants mirroring include/pse/enums.h, with name resolvers.
--- --==-==--

local M = {}

local function makeResolver(prefix, map)
    return function(value)
        if type(value) == "number" then return value end
        local v = tostring(value):upper()
        if map[v] ~= nil then return map[v] end
        if prefix and map[prefix .. v] ~= nil then return map[prefix .. v] end
        error("unknown " .. prefix .. " name: " .. tostring(value), 3)
    end
end

M.byValue = function(map, value)
    local v = map[value]
    if v ~= nil then return v end
    for k, val in pairs(map) do
        if val == value then return k end
    end
    return ("0x%08X"):format(value)
end

local C = {}
C.GAME                                    = 0x00100000
C.GAME_SET_CHEATS_ENABLED                 = 0x00100001
C.GAME_GET_CHEATS_ENABLED                 = 0x00100002
C.GAME_SET_CHEATS_NOCLIP                  = 0x00100003
C.GAME_GET_CHEATS_NOCLIP                  = 0x00100004
C.GAME_SET_GRAVITY                        = 0x00100005
C.GAME_GET_GRAVITY                        = 0x00100006
C.GAME_CHECK_GUID_IS_VALID                = 0x00100007
C.GAME_INITIALIZE                         = 0x00100008
C.GAME_DEINITIALIZE                       = 0x00100009
C.PLAYER                                  = 0x00200000
C.PLAYER_SET_LOCATION                     = 0x00200001
C.PLAYER_GET_LOCATION                     = 0x00200002
C.PLAYER_SET_ROTATION                     = 0x00200003
C.PLAYER_GET_ROTATION                     = 0x00200004
C.PLAYER_SPAWN                            = 0x00200005
C.PLAYER_KILL                             = 0x00200006
C.SOLVER_GUN                              = 0x00300000
C.SOLVER_GUN_SET_ENABLED                  = 0x00300001
C.SOLVER_GUN_GET_ENABLED                  = 0x00300002
C.SOLVER_GUN_ACTION_USE                   = 0x00300003
C.SOLVER_GUN_ACTION_RELEASE               = 0x00300004
C.SOLVER_GUN_ACTION_THROW                 = 0x00300005
C.FLASHLIGHT                              = 0x00400000
C.FLASHLIGHT_SET_ENABLED                  = 0x00400001
C.FLASHLIGHT_GET_ENABLED                  = 0x00400002
C.FLASHLIGHT_SET_STATE                    = 0x00400003
C.FLASHLIGHT_GET_STATE                    = 0x00400004
C.STATIC_MESH                             = 0x00500000
C.STATIC_MESH_CREATE                      = 0x00500001
C.REPLACE                                 = 0x05062025
C.DYNAMIC_MESH                            = 0x00600000
C.DYNAMIC_MESH_CREATE                     = 0x00600001
C.DYNAMIC_MESH_SET_MATERIAL               = 0x00600002
C.DYNAMIC_MESH_SET_MESH                   = 0x00600003
C.DYNAMIC_MESH_SET_VISIBILITY             = 0x00600004
C.DYNAMIC_MESH_GET_VISIBILITY             = 0x00600005
C.DYNAMIC_MESH_SET_TRANSFORM              = 0x00600006
C.DYNAMIC_MESH_GET_TRANSFORM              = 0x00600007
C.DYNAMIC_MESH_DESTROY                    = 0x00600008
C.ELEMENT                                 = 0x00700000
C.ELEMENT_CREATE                          = 0x00700001
C.ELEMENT_SET_CALLBACK                    = 0x00700002
C.ELEMENT_GET_CALLBACK                    = 0x00700003
C.ELEMENT_SET_STATE                       = 0x00700004
C.ELEMENT_GET_STATE                       = 0x00700005
C.ELEMENT_SET_VISIBILITY                  = 0x00700006
C.ELEMENT_GET_VISIBILITY                  = 0x00700007
C.ELEMENT_SET_TRANSFORM                   = 0x00700008
C.ELEMENT_GET_TRANSFORM                   = 0x00700009
C.ELEMENT_SET_CLASS                       = 0x0070000A
C.ELEMENT_GET_CLASS                       = 0x0070000B
C.ELEMENT_SET_REGISTER                    = 0x0070000C
C.ELEMENT_GET_REGISTER                    = 0x0070000D
C.ELEMENT_SET_ALL_REGISTERS               = 0x0070000E
C.ELEMENT_GET_ALL_REGISTERS               = 0x0070000F
C.ELEMENT_SET_STATE_AND_CALLBACK_AND_ALL_REGISTERS = 0x00700010
C.ELEMENT_GET_STATE_AND_CALLBACK_AND_ALL_REGISTERS = 0x00700011
C.ELEMENT_DESTROY                         = 0x00700012
C.NONE                                    = 0xFFFFFFFF
M.COMMAND = C
M.command = makeResolver("PSE_COMMAND_", C)

local E = {}
E.ELEMENT_CHANGED    = 0x00000000
E.GAME_TICK_OVERFLOW = 0x00000001
E.NONE               = 0xFFFFFFFF
M.EVENT = E
M.event = makeResolver("PSE_EVENT_", E)

local R = {}
R.SUCCESS                        = 0x00000000
R.ERROR_COMMAND_NOT_FOUND        = 0x00000001
R.ERROR_COMMAND_NOT_IMPLEMENTED  = 0x00000002
R.ERROR_EXECUTION_FAILED         = 0x00000003
R.ERROR_ARRAY_INDEX_OUT_OF_RANGE = 0x00000004
R.ERROR_GUID_NOT_FOUND           = 0x00000005
R.ERROR_INVALID_OBJECT_TYPE      = 0x00000006
R.ERROR_GAME_NOT_INITIALIZED     = 0x00000007
R.NONE                           = 0xFFFFFFFF
M.RESULT = R
M.result = makeResolver("PSE_RESULT_", R)

local MESH = {}
MESH.PLANE               = 0x0000
MESH.FACE                = 0x0001
MESH.CUBE                = 0x0002
MESH.CUP_INNER           = 0x0003
MESH.CUP_OUTER           = 0x0004
MESH.II_INNER            = 0x0005
MESH.II_OUTER            = 0x0006
MESH.O_INNER             = 0x0007
MESH.O_OUTER             = 0x0008
MESH.U_INNER             = 0x0009
MESH.U_OUTER             = 0x000A
MESH.DOOR_FRAME          = 0x000B
MESH.LASER_FRAME         = 0x000C
MESH.LASER_FRAME_SHIFTED = 0x000D
MESH.PLANE_Z_SHIFTED     = 0x000E
M.MESH = MESH
M.mesh = makeResolver("PSE_MESH_", MESH)

local MAT = {}
MAT.WALL_WHITE_SMALL            = 0x0000
MAT.WALL_WHITE_MEDIUM           = 0x0001
MAT.WALL_WHITE_DOUBLE           = 0x0002
MAT.WALL_WHITE_BIG              = 0x0003
MAT.WALL_WHITE_ABSOLUTE_SCIENCE = 0x0004
MAT.WALL_BLACK_SMALL            = 0x0005
MAT.WALL_BLACK_MEDIUM           = 0x0006
MAT.WALL_BLACK_BIG              = 0x0007
MAT.FLOOR_WHITE                 = 0x0008
MAT.FLOOR_BLACK                 = 0x0009
MAT.WINDOW_METAL_GRID           = 0x000A
MAT.WINDOW_GLASS_METAL_GRID     = 0x000B
MAT.WALL_YELLOW_1_0             = 0x000C
MAT.WALL_YELLOW_1_5             = 0x000D
M.MATERIAL = MAT
M.material = makeResolver("PSE_MATERIAL_", MAT)

local CLS = {}
CLS.ENTRY_ELEVATOR           = 0x0000
CLS.EXIT_ELEVATOR            = 0x0001
CLS.DOOR                     = 0x0002
CLS.BUTTON                   = 0x0003
CLS.ANTI_EXPROPRIATION_FIELD = 0x0004
CLS.PEDESTAL_BUTTON          = 0x0005
CLS.SOLVER_BUTTON            = 0x0006
CLS.INDICATOR                = 0x0007
CLS.LASER_TX                 = 0x0008
CLS.LASER_RX                 = 0x0009
CLS.LASER_RELAY              = 0x000A
CLS.LASER_PANEL              = 0x000B
CLS.LASER_CUBE               = 0x000C
CLS.WEIGHT_CUBE              = 0x000D
CLS.FAITH_PLATE              = 0x000E
CLS.PANEL                    = 0x000F
CLS.STAIRS                   = 0x0010
CLS.CUBE_DROPPER             = 0x0011
CLS.SOLVER_GUN_PEDESTAL      = 0x0012
CLS.FLASHLIGHT               = 0x0013
CLS.WINDOW                   = 0x0014
CLS.TRIGGER                  = 0x0015
CLS.LAMP                     = 0x0016
M.CLASS = CLS
M.class = makeResolver("PSE_CLASS_", CLS)

return M
