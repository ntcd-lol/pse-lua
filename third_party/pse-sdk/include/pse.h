// Portal Solver Editor SDK

#pragma once

#if defined(_WIN32)
    #ifdef PSE_EXPORTS
        #define PSE_API __declspec(dllexport)
    #else
        #define PSE_API __declspec(dllimport)
    #endif
#else
    #define PSE_API __attribute__((visibility("default")))
#endif

#include "enums.h"
#include "structures.h"

#ifdef __cplusplus
extern "C"
{
#endif
    
    // Buffers / Core
    PSE_API int32_t pseInitializeBuffers(void);
    PSE_API void    pseDeinitializeBuffers(void);
    
    PSE_API PseBuffers*       pseGetBuffersAddress(void);
    PSE_API PseCommandBuffer* pseGetCommandBufferAddress(void);
    PSE_API PseEventBuffer*   pseGetEventBufferAddress(void);
    PSE_API void*             pseGetSharedMemoryHandle(void);
    
    // Command Buffer / Methods
    PSE_API void     pseCommandBufferSynchronize(void);
    
    PSE_API uint32_t pseCommandBufferSinglePush(const PseData* pInData);
    PSE_API void     pseCommandBufferSingleWait(uint32_t absoluteIndex, PseData* pOutData);
    PSE_API void     pseCommandBufferSinglePushAndWait(PseData* pOutData);
    
    PSE_API uint32_t pseCommandBufferBatchPush(const PseData* pInData, uint32_t count);
    PSE_API void     pseCommandBufferBatchWait(uint32_t absoluteIndex, PseData* pOutData, uint32_t count);
    PSE_API void     pseCommandBufferBatchPushAndWait(PseData* pOutData, uint32_t count);
    
    PSE_API PseData* pseCommandBufferRawAt(uint32_t absoluteIndex);
    PSE_API uint32_t pseCommandBufferRawReserve(uint32_t count);
    PSE_API void     pseCommandBufferRawCommit(uint32_t startAbsoluteIndex, uint32_t count);
    PSE_API void     pseCommandBufferRawWait(uint32_t absoluteIndex, uint32_t count);
    PSE_API void     pseCommandBufferRawCommitAndWait(uint32_t startAbsoluteIndex, uint32_t count);
    
    // Event Buffer / Methods
    PSE_API int32_t pseEventBufferTryGet(PseData* pOutData);
    
#ifdef __cplusplus
}
#endif
