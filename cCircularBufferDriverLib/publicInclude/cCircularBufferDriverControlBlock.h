/*********************************************************************
 * @file cCircularBufferDriverControlBlock.h
 * @brief The real, fully-fielded circular buffer control block.
 * @author Anthony Garza
 * @copyright All rights reserved 2026
 *
 * @note This is deliberately kept out of cCircularBufferDriverPub.h. The
 *       "front door" public header only knows about sCircularBufferControlBlock_t
 *       as an opaque, correctly-sized-and-aligned uint8_t array -- callers
 *       are not meant to reach into the real fields directly. This header is
 *       the "back door": still public (nothing stops a caller from including
 *       it), but you have to go looking for it. Pair it with
 *       circularBufferGetRawControlBlock() / circularBufferGetRawControlBlockConst()
 *       in cCircularBufferDriverPub.h to actually get a typed pointer to one
 *       of these out of an sCircularBufferControlBlock_t.
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "commonMacros.h"
#include "commonTypes.h"
#include "cCircularBufferDriverConfig.h"
#if  ( ( CIRUCULAR_BUFFER_USE_WRITE_MUTEX == DEF_TRUE ) || \
       ( CIRCULAR_BUFFER_USE_READ_MUTEX == DEF_TRUE ) || \
       ( CIRCULAR_BUFFER_USE_ACCESS_MUTEX == DEF_TRUE ) )
#include "cMutexDriverPub.h"
#endif

#ifndef C_CIRCULAR_BUFFER_DRIVER_CONTROL_BLOCK_H
#define C_CIRCULAR_BUFFER_DRIVER_CONTROL_BLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack( push, 1 )
/**
 * @brief Control block for a single circular buffer instance. One of these
 *        per buffer -- unlike initCircularBufferDriver(), this is not a
 *        singleton.
 * @note This is the real layout behind the opaque sCircularBufferControlBlock_t
 *       handle every circularBufferXxx() function actually takes. If you add,
 *       remove, or reorder fields here, update CIRCULAR_BUFFER_CONTROL_BLOCK_SIZE_BYTES
 *       and CIRCULAR_BUFFER_CONTROL_BLOCK_ALIGNMENT_BYTES in
 *       cCircularBufferDriverConfig.h to match -- the static asserts next to
 *       those definitions (and in cCircularBufferDriver.c) will fail the build
 *       loudly if you forget.
 */
typedef struct sCircularBufferControlBlockRaw
{
#if ( CIRUCULAR_BUFFER_USE_WRITE_MUTEX == DEF_TRUE )
    sMutex_t _writeMutex;
#endif
#if ( CIRCULAR_BUFFER_USE_READ_MUTEX == DEF_TRUE )
    sMutex_t _readMutex;
#endif
#if ( CIRCULAR_BUFFER_USE_ACCESS_MUTEX == DEF_TRUE )
    sMutex_t _accessMutex;
#endif
    bool _isInitialized;
    bool _isFull;
#if ( ( CIRCULAR_BUFFER_ALLOW_MALLOC == DEF_TRUE ) || ( CIRCULAR_BUFFER_USE_MEMMORY_POOL_ALLOCATOR == DEF_TRUE ) )
    bool _isBufferAllocated;
    uint8_t _unused;
#else
    uint16_t _unused;
#endif
    size_t _bufferSizeInBytes;
    size_t _bufferSizeInElements;
    size_t _elementSizeInBytes;
    uint32_t _headIndex;
    uint32_t _tailIndex;
    void * _pBuffer;
} sCircularBufferControlBlockRaw_t;
#pragma pack( pop )

AG_STATIC_ASSERT( ( sizeof( sCircularBufferControlBlockRaw_t ) % CIRCULAR_BUFFER_ALLIGN_SIZE ) == 0,
                  "sCircularBufferControlBlockRaw_t must be padded to a multiple of CIRCULAR_BUFFER_ALLIGN_SIZE" );

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_CIRCULAR_BUFFER_DRIVER_CONTROL_BLOCK_H */
