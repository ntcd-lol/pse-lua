// Portal Solver Editor SDK

#pragma once

#include <cstdint>

enum PseCommand : uint32_t
{
	// Format (G - Group, C - Command)                           = 0xGGGСCCCC,
	PSE_COMMAND_GAME                                             = 0x00100000,
	PSE_COMMAND_GAME_SET_CHEATS_ENABLED                          = 0x00100001,
	PSE_COMMAND_GAME_GET_CHEATS_ENABLED                          = 0x00100002,
	PSE_COMMAND_GAME_SET_CHEATS_NOCLIP                           = 0x00100003,
	PSE_COMMAND_GAME_GET_CHEATS_NOCLIP                           = 0x00100004,
	PSE_COMMAND_GAME_SET_GRAVITY                                 = 0x00100005,
	PSE_COMMAND_GAME_GET_GRAVITY                                 = 0x00100006,
	PSE_COMMAND_GAME_CHECK_GUID_IS_VALID                         = 0x00100007,
	PSE_COMMAND_GAME_INITIALIZE                                  = 0x00100008,
	PSE_COMMAND_GAME_DEINITIALIZE                                = 0x00100009,
	
	PSE_COMMAND_PLAYER                                           = 0x00200000,
	PSE_COMMAND_PLAYER_SET_LOCATION                              = 0x00200001,
	PSE_COMMAND_PLAYER_GET_LOCATION                              = 0x00200002,
	PSE_COMMAND_PLAYER_SET_ROTATION                              = 0x00200003,
	PSE_COMMAND_PLAYER_GET_ROTATION                              = 0x00200004,
	PSE_COMMAND_PLAYER_SPAWN                                     = 0x00200005,
	PSE_COMMAND_PLAYER_KILL                                      = 0x00200006,
	
	PSE_COMMAND_SOLVER_GUN                                       = 0x00300000,
	PSE_COMMAND_SOLVER_GUN_SET_ENABLED                           = 0x00300001,
	PSE_COMMAND_SOLVER_GUN_GET_ENABLED                           = 0x00300002,
	PSE_COMMAND_SOLVER_GUN_ACTION_USE                            = 0x00300003,
	PSE_COMMAND_SOLVER_GUN_ACTION_RELEASE                        = 0x00300004,
	PSE_COMMAND_SOLVER_GUN_ACTION_THROW                          = 0x00300005,
	
	PSE_COMMAND_FLASHLIGHT                                       = 0x00400000,
	PSE_COMMAND_FLASHLIGHT_SET_ENABLED                           = 0x00400001,
	PSE_COMMAND_FLASHLIGHT_GET_ENABLED                           = 0x00400002,
	PSE_COMMAND_FLASHLIGHT_SET_STATE                             = 0x00400003,
	PSE_COMMAND_FLASHLIGHT_GET_STATE                             = 0x00400004,
	
	PSE_COMMAND_STATIC_MESH                                      = 0x00500000,
	PSE_COMMAND_STATIC_MESH_CREATE                               = 0x00500001,
	
	PSE_COMMAND_REPLACE                                          = 0x05062025,
	
	PSE_COMMAND_DYNAMIC_MESH                                     = 0x00600000,
	PSE_COMMAND_DYNAMIC_MESH_CREATE                              = 0x00600001,
	PSE_COMMAND_DYNAMIC_MESH_SET_MATERIAL                        = 0x00600002,
	PSE_COMMAND_DYNAMIC_MESH_SET_MESH                            = 0x00600003,
	PSE_COMMAND_DYNAMIC_MESH_SET_VISIBILITY                      = 0x00600004,
	PSE_COMMAND_DYNAMIC_MESH_GET_VISIBILITY                      = 0x00600005,
	PSE_COMMAND_DYNAMIC_MESH_SET_TRANSFORM                       = 0x00600006,
	PSE_COMMAND_DYNAMIC_MESH_GET_TRANSFORM                       = 0x00600007,
	PSE_COMMAND_DYNAMIC_MESH_DESTROY                             = 0x00600008,
	
	PSE_COMMAND_ELEMENT                                          = 0x00700000,
	PSE_COMMAND_ELEMENT_CREATE                                   = 0x00700001,
	PSE_COMMAND_ELEMENT_SET_CALLBACK                             = 0x00700002,
	PSE_COMMAND_ELEMENT_GET_CALLBACK                             = 0x00700003,
	PSE_COMMAND_ELEMENT_SET_STATE                                = 0x00700004,
	PSE_COMMAND_ELEMENT_GET_STATE                                = 0x00700005,
	PSE_COMMAND_ELEMENT_SET_VISIBILITY                           = 0x00700006,
	PSE_COMMAND_ELEMENT_GET_VISIBILITY                           = 0x00700007,
	PSE_COMMAND_ELEMENT_SET_TRANSFORM                            = 0x00700008,
	PSE_COMMAND_ELEMENT_GET_TRANSFORM                            = 0x00700009,
	PSE_COMMAND_ELEMENT_SET_CLASS                                = 0x0070000A,
	PSE_COMMAND_ELEMENT_GET_CLASS                                = 0x0070000B,
	PSE_COMMAND_ELEMENT_SET_REGISTER                             = 0x0070000C,
	PSE_COMMAND_ELEMENT_GET_REGISTER                             = 0x0070000D,
	PSE_COMMAND_ELEMENT_SET_ALL_REGISTERS                        = 0x0070000E,
	PSE_COMMAND_ELEMENT_GET_ALL_REGISTERS                        = 0x0070000F,
	PSE_COMMAND_ELEMENT_SET_STATE_AND_CALLBACK_AND_ALL_REGISTERS = 0x00700010,
	PSE_COMMAND_ELEMENT_GET_STATE_AND_CALLBACK_AND_ALL_REGISTERS = 0x00700011,
	PSE_COMMAND_ELEMENT_DESTROY                                  = 0x00700012,
	
	PSE_COMMAND_NONE                                             = 0xFFFFFFFF
};

enum PseEvent : uint32_t
{
	PSE_EVENT_ELEMENT_CHANGED    = 0x00000000,
	PSE_EVENT_GAME_TICK_OVERFLOW = 0x00000001,
	
	PSE_EVENT_NONE               = 0xFFFFFFFF
};

enum PseResult : uint32_t
{
	PSE_RESULT_SUCCESS                        = 0x00000000,
	
	PSE_RESULT_ERROR_COMMAND_NOT_FOUND        = 0x00000001,
	PSE_RESULT_ERROR_COMMAND_NOT_IMPLEMENTED  = 0x00000002,
	PSE_RESULT_ERROR_EXECUTION_FAILED         = 0x00000003,
	PSE_RESULT_ERROR_ARRAY_INDEX_OUT_OF_RANGE = 0x00000004,
	PSE_RESULT_ERROR_GUID_NOT_FOUND           = 0x00000005,
	PSE_RESULT_ERROR_INVALID_OBJECT_TYPE      = 0x00000006,
	PSE_RESULT_ERROR_GAME_NOT_INITIALIZED     = 0x00000007,
	
	PSE_RESULT_NONE                           = 0xFFFFFFFF
};

enum PseMesh : uint16_t
{
	PSE_MESH_PLANE               = 0x0000,
	PSE_MESH_FACE                = 0x0001,
	PSE_MESH_CUBE                = 0x0002,
	
	PSE_MESH_CUP_INNER           = 0x0003,
	PSE_MESH_CUP_OUTER           = 0x0004,
	PSE_MESH_II_INNER            = 0x0005,
	PSE_MESH_II_OUTER            = 0x0006,
	PSE_MESH_O_INNER             = 0x0007,
	PSE_MESH_O_OUTER             = 0x0008,
	PSE_MESH_U_INNER             = 0x0009,
	PSE_MESH_U_OUTER             = 0x000A,
	
	PSE_MESH_DOOR_FRAME          = 0x000B,
	PSE_MESH_LASER_FRAME         = 0x000C,
	PSE_MESH_LASER_FRAME_SHIFTED = 0x000D,
	PSE_MESH_PLANE_Z_SHIFTED     = 0x000E,
};

enum PseMaterial : uint16_t
{
	PSE_MATERIAL_WALL_WHITE_SMALL            = 0x0000,
	PSE_MATERIAL_WALL_WHITE_MEDIUM           = 0x0001,
	PSE_MATERIAL_WALL_WHITE_DOUBLE           = 0x0002,
	PSE_MATERIAL_WALL_WHITE_BIG              = 0x0003,
	PSE_MATERIAL_WALL_WHITE_ABSOLUTE_SCIENCE = 0x0004,
	
	PSE_MATERIAL_WALL_BLACK_SMALL            = 0x0005,
	PSE_MATERIAL_WALL_BLACK_MEDIUM           = 0x0006,
	PSE_MATERIAL_WALL_BLACK_BIG              = 0x0007,
	
	PSE_MATERIAL_FLOOR_WHITE                 = 0x0008,
	PSE_MATERIAL_FLOOR_BLACK                 = 0x0009,
	
	PSE_MATERIAL_WINDOW_METAL_GRID           = 0x000A,
	PSE_MATERIAL_WINDOW_GLASS_METAL_GRID     = 0x000B,
	
	PSE_MATERIAL_WALL_YELLOW_1_0             = 0x000C,
	PSE_MATERIAL_WALL_YELLOW_1_5             = 0x000D,
};

enum PseClass : uint16_t
{
	PSE_CLASS_ENTRY_ELEVATOR           = 0x0000,
	PSE_CLASS_EXIT_ELEVATOR            = 0x0001,
	
	PSE_CLASS_DOOR                     = 0x0002,
	PSE_CLASS_BUTTON                   = 0x0003,
	PSE_CLASS_ANTI_EXPROPRIATION_FIELD = 0x0004,
	PSE_CLASS_PEDESTAL_BUTTON          = 0x0005,
	PSE_CLASS_SOLVER_BUTTON            = 0x0006,
	PSE_CLASS_INDICATOR                = 0x0007,
	
	PSE_CLASS_LASER_TX                 = 0x0008,
	PSE_CLASS_LASER_RX                 = 0x0009,
	PSE_CLASS_LASER_RELAY              = 0x000A,
	PSE_CLASS_LASER_PLANE              = 0x000B,
	
	PSE_CLASS_LASER_CUBE               = 0x000C,
	PSE_CLASS_WEIGHT_CUBE              = 0x000D,
	
	PSE_CLASS_FAITH_PLATE              = 0x000E,
	PSE_CLASS_PANEL                    = 0x000F,
	PSE_CLASS_STAIRS                   = 0x0010,
	PSE_CLASS_CUBE_DROPPER             = 0x0011,
	PSE_CLASS_SOLVER_GUN_PEDESTAL      = 0x0012,
	PSE_CLASS_FLASHLIGHT               = 0x0013,
	PSE_CLASS_WINDOW                   = 0x0014,
	
	PSE_CLASS_TRIGGER                  = 0x0015,
	PSE_CLASS_LAMP                     = 0x0016
};
