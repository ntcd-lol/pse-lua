// Portal Solver Editor SDK

#pragma once

#include <stdint.h>

#define PSE_COMMAND_BUFFER_SIZE 2048
#define PSE_EVENT_BUFFER_SIZE 256
#define PSE_MAX_COMMANDS_PER_TICK 128

typedef uint64_t PseGUID;
typedef uint32_t PseRegister;
typedef int8_t PseState;
typedef uint64_t PseCallback;

typedef struct { float x, y; } PseVector2;
typedef struct { float x, y, z; } PseVector3;
typedef struct { float x, y, z, w; } PseQuat;
typedef struct { PseQuat quat; PseVector3 location; PseVector3 scale; } PseTransform;
typedef struct { uint8_t r, g, b; } PseColor;

typedef struct alignas(64)
{
    uint32_t header;
    uint8_t data[60];
} PseData;

typedef struct alignas(64)
{
    alignas(64) PseData buffer[PSE_COMMAND_BUFFER_SIZE];
    alignas(64) uint32_t pseIndex;
    alignas(64) uint32_t gameIndex;
} PseCommandBuffer;

typedef struct alignas(64)
{
    alignas(64) PseData buffer[PSE_EVENT_BUFFER_SIZE];
    alignas(64) uint32_t gameIndex;
} PseEventBuffer;

typedef struct alignas(64)
{
    alignas(64) PseCommandBuffer commandBuffer;
    alignas(64) PseEventBuffer eventBuffer;
} PseBuffers;
