// Portal Solver Editor SDK

#include "pse.h"

#include <atomic>
#include <Windows.h>

// Static
static inline void* gPseSharedMemoryHandle = nullptr;
static inline PseBuffers* gPseBuffers = nullptr;
static inline PseCommandBuffer* gPseCommandBuffer = nullptr;
static inline PseEventBuffer* gPseEventBuffer = nullptr;
static inline uint32_t gPseEventBufferIndex = 0;

// Private
static void privateCommandBufferRawCopy(void* pExternalData, uint32_t absoluteIndex, uint32_t count, bool bToBuffer)
{
    uint32_t spaceAtEnd = PSE_COMMAND_BUFFER_SIZE - (absoluteIndex & (PSE_COMMAND_BUFFER_SIZE - 1));
    void* pInternalStart = pseCommandBufferRawAt(absoluteIndex);
    
    if(count <= spaceAtEnd)
    {
        std::memcpy
        (
            bToBuffer ? pInternalStart : pExternalData,
            bToBuffer ? pExternalData : pInternalStart,
            count * 64
        );
    }
    else
    {
        uint32_t BytesAtEnd = spaceAtEnd * 64;
            
        std::memcpy
        (
            bToBuffer ? pInternalStart : pExternalData,
            bToBuffer ? pExternalData : pInternalStart,
            BytesAtEnd
        );
            
        std::memcpy
        (
            bToBuffer ? (uint8_t*)&gPseCommandBuffer->buffer[0] : (uint8_t*)pExternalData + BytesAtEnd,
            bToBuffer ? (uint8_t*)pExternalData + BytesAtEnd : (uint8_t*)&gPseCommandBuffer->buffer[0],
            (count - spaceAtEnd) * 64
        );
    }
}

PseData* privateEventBufferRawAt(uint32_t absoluteIndex)
{
    return &gPseEventBuffer->buffer[absoluteIndex & (PSE_EVENT_BUFFER_SIZE - 1)];
}

inline std::atomic<uint32_t>* privateAsAtomic(uint32_t* pVariable)
{
    return reinterpret_cast<std::atomic<uint32_t>*>(pVariable);
}

// Buffers / Core
int32_t pseInitializeBuffers(void)
{
    gPseSharedMemoryHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "Global\\PortalSolverEditorSDK");
    if(!gPseSharedMemoryHandle)
    {
        // 0.2 fix: Global -> Local. Working for Backward Compatibility
        gPseSharedMemoryHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "Local\\PortalSolverEditorSDK");
        if(!gPseSharedMemoryHandle) return -1; // Could not open file mapping object
    }
    
    void* pBuffers = MapViewOfFile(gPseSharedMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(PseBuffers));
    if(!pBuffers)
    {
        CloseHandle(gPseSharedMemoryHandle);
        gPseSharedMemoryHandle = nullptr;
        return -2; // Could not map view of file
    }
    
    gPseBuffers = static_cast<PseBuffers*>(pBuffers);
    gPseCommandBuffer = &gPseBuffers->commandBuffer;
    gPseEventBuffer = &gPseBuffers->eventBuffer;
    
    memset(pBuffers, 0, sizeof(PseBuffers));
    gPseEventBufferIndex = 0;
    
    return 0;
}

void pseDeinitializeBuffers(void)
{
    if(gPseBuffers) UnmapViewOfFile(gPseBuffers);
    if(gPseSharedMemoryHandle) CloseHandle(gPseSharedMemoryHandle);

    gPseBuffers = nullptr;
    gPseCommandBuffer = nullptr;
    gPseEventBuffer = nullptr;
    gPseSharedMemoryHandle = nullptr;
}

PseBuffers*       pseGetBuffersAddress(void)       { return gPseBuffers; }
PseCommandBuffer* pseGetCommandBufferAddress(void) { return gPseCommandBuffer; }
PseEventBuffer*   pseGetEventBufferAddress(void)   { return gPseEventBuffer; }
void*             pseGetSharedMemoryHandle(void)   { return gPseSharedMemoryHandle; }

// Command Buffer / Methods
void pseCommandBufferSynchronize(void)
{
    uint32_t pse = privateAsAtomic(&gPseCommandBuffer->pseIndex)->load(std::memory_order_relaxed);
    while(true)
    {
        uint32_t game = privateAsAtomic(&gPseCommandBuffer->gameIndex)->load(std::memory_order_acquire);
        if((int32_t)(game - pse) >= 0) break;
        _mm_pause();
    }
}

uint32_t pseCommandBufferSinglePush(const PseData* pInData)
{
    while(true)
    {
        uint32_t pse = privateAsAtomic(&gPseCommandBuffer->pseIndex)->load(std::memory_order_relaxed);
        uint32_t game = privateAsAtomic(&gPseCommandBuffer->gameIndex)->load(std::memory_order_acquire);
        
        if(pse - game < PSE_COMMAND_BUFFER_SIZE)
        {
            *pseCommandBufferRawAt(pse) = *pInData;
            privateAsAtomic(&gPseCommandBuffer->pseIndex)->store(pse + 1, std::memory_order_release);
            return pse;
        }
        
        _mm_pause();
    }
}

void pseCommandBufferSingleWait(uint32_t absoluteIndex, PseData* pOutData)
{
    while(true)
    {
        uint32_t game = privateAsAtomic(&gPseCommandBuffer->gameIndex)->load(std::memory_order_acquire);
        
        if(game > absoluteIndex)
        {
            *pOutData = *pseCommandBufferRawAt(absoluteIndex);
            return;
        }
        
        _mm_pause();
    }
}

void pseCommandBufferSinglePushAndWait(PseData* pOutData)
{
    pseCommandBufferSingleWait(pseCommandBufferSinglePush(pOutData), pOutData);
}

uint32_t pseCommandBufferBatchPush(const PseData* pInData, uint32_t count)
{
    while(true)
    {
        uint32_t pse = privateAsAtomic(&gPseCommandBuffer->pseIndex)->load(std::memory_order_relaxed);
        uint32_t game = privateAsAtomic(&gPseCommandBuffer->gameIndex)->load(std::memory_order_acquire);
            
        if(pse - game + count <= PSE_COMMAND_BUFFER_SIZE)
        {
            privateCommandBufferRawCopy((void*)pInData, pse, count, true);
            privateAsAtomic(&gPseCommandBuffer->pseIndex)->store(pse + count, std::memory_order_release);
            return pse;
        }
        _mm_pause();
    }
}

void pseCommandBufferBatchWait(uint32_t absoluteIndex, PseData* pInData, uint32_t count)
{
    if(count == 0) return;
        
    while(true)
    {
        uint32_t game = privateAsAtomic(&gPseCommandBuffer->gameIndex)->load(std::memory_order_acquire);
        if((int32_t)(game - (absoluteIndex + count)) >= 0)
        {
            privateCommandBufferRawCopy((void*)pInData, absoluteIndex, count, false);
            return;
        }
        _mm_pause();
    }
}

void pseCommandBufferBatchPushAndWait(PseData* pInData, uint32_t count)
{
    pseCommandBufferBatchWait(pseCommandBufferBatchPush(pInData, count), pInData, count);
}

PseData* pseCommandBufferRawAt(uint32_t absoluteIndex)
{
    return &gPseCommandBuffer->buffer[absoluteIndex & (PSE_COMMAND_BUFFER_SIZE - 1)];
}

uint32_t pseCommandBufferRawReserve(uint32_t count)
{
    while(true)
    {
        uint32_t pse = privateAsAtomic(&gPseCommandBuffer->pseIndex)->load(std::memory_order_relaxed);
        uint32_t game = privateAsAtomic(&gPseCommandBuffer->gameIndex)->load(std::memory_order_acquire);
        if(pse - game + count <= PSE_COMMAND_BUFFER_SIZE) return pse;
        _mm_pause();
    }
}

void pseCommandBufferRawCommit(uint32_t startAbsoluteIndex, uint32_t count)
{
    privateAsAtomic(&gPseCommandBuffer->pseIndex)->store(startAbsoluteIndex + count, std::memory_order_release);
}

void pseCommandBufferRawWait(uint32_t absoluteIndex, uint32_t count)
{
    while(true)
    {
        uint32_t game = privateAsAtomic(&gPseCommandBuffer->gameIndex)->load(std::memory_order_acquire);
        if((int32_t)(game - (absoluteIndex + count)) >= 0) return;
        _mm_pause();
    }
}

void pseCommandBufferRawCommitAndWait(uint32_t startAbsoluteIndex, uint32_t count)
{
    pseCommandBufferRawCommit(startAbsoluteIndex, count);
    pseCommandBufferRawWait(startAbsoluteIndex, count);
}

// Event Buffer / Methods
int32_t pseEventBufferTryGet(PseData* pOutData)
{
    uint32_t game = privateAsAtomic(&gPseEventBuffer->gameIndex)->load(std::memory_order_acquire);
    if((int32_t)(game - gPseEventBufferIndex) <= 0) return 0;
    
    *pOutData = *privateEventBufferRawAt(gPseEventBufferIndex);
    ++gPseEventBufferIndex;
    return 1;
}

