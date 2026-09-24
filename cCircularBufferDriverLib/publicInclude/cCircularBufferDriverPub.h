/*********************************************************************
 * @file cCircularBufferDriverPub.h
 * @brief Public interface for the circular buffer driver.
 * @author Anthony Garza
 * @copyright All rights reserved 2026
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "commonMacros.h"
#include "commonTypes.h"
#include "cCircularBufferDriverConfig.h"
#include "cCircularBufferDriverErrorCodes.h"
#ifndef C_CIRCULAR_BUFFER_DRIVER_PUB_H
#define C_CIRCULAR_BUFFER_DRIVER_PUB_H

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * TYPES AND STRUCTURES
 *============================================================================*/

/**
 * @brief Forward declaration only -- the real, fully-fielded control block
 *        (mutexes, buffer pointer, indices, etc.) lives in
 *        cCircularBufferDriverControlBlock.h, which this header deliberately
 *        does not include. Every circularBufferXxx() function below takes
 *        the opaque sCircularBufferControlBlock_t instead. If you need the
 *        real fields, include that header yourself and go through
 *        circularBufferGetRawControlBlock() / circularBufferGetRawControlBlockConst().
 */
typedef struct sCircularBufferControlBlockRaw sCircularBufferControlBlockRaw_t;

/**
 * @brief Opaque handle for a circular buffer control block. One of these per
 *        buffer instance -- unlike initCircularBufferDriver(), this is not a
 *        singleton.
 * @note This is intentionally just a byte array, not the real struct -- see
 *       cCircularBufferDriverControlBlock.h for why. It's sized and left
 *       unaligned to exactly match sCircularBufferControlBlockRaw_t
 *       (CIRCULAR_BUFFER_CONTROL_BLOCK_SIZE_BYTES in
 *       cCircularBufferDriverConfig.h, enforced by a static assert in
 *       cCircularBufferDriver.c), so it can be zero-initialized, embedded in
 *       other structs, or put on the stack exactly like the real thing.
 */
typedef struct
{
    uint8_t _opaque[ CIRCULAR_BUFFER_CONTROL_BLOCK_SIZE_BYTES ];
} sCircularBufferControlBlock_t;

/**
 * @brief Reinterprets an opaque control block handle as a pointer to the
 *        real, fully-fielded sCircularBufferControlBlockRaw_t.
 * @note This is the escape hatch for a developer who wants direct field
 *       access instead of going through circularBufferXxx(). Nothing stops
 *       you from calling it -- but nothing protects you from it either: the
 *       mutexes and bookkeeping fields it exposes are exactly what
 *       circularBufferXxx() relies on to stay consistent, so reaching in
 *       directly is on you. Include cCircularBufferDriverControlBlock.h to
 *       get the real type definition before you can do anything with the
 *       pointer this returns.
 * @param handle Pointer to the opaque control block to reinterpret.
 * @return Pointer to the same memory, typed as sCircularBufferControlBlockRaw_t.
 *         NULL if handle is NULL.
 */
extern sCircularBufferControlBlockRaw_t * circularBufferGetRawControlBlock( sCircularBufferControlBlock_t * const handle );

/**
 * @brief Const-qualified counterpart to circularBufferGetRawControlBlock(),
 *        for read-only access to the real fields.
 * @param handle Pointer to the opaque control block to reinterpret.
 * @return Pointer to the same memory, typed as sCircularBufferControlBlockRaw_t
 *         const. NULL if handle is NULL.
 */
extern sCircularBufferControlBlockRaw_t const * circularBufferGetRawControlBlockConst( sCircularBufferControlBlock_t const * const handle );



/*============================================================================
 * DRIVER-WIDE PUBLIC INTERFACE
 *============================================================================*/

/**
 * @brief Initializes the circular buffer driver. Must be called once before
 *        any other circularBufferXxx() function is used.
 * @param createErrorCallback Pointer to a function for creating errors.
 * @param logCallback Pointer to a function for logging messages.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
extern sErrorCompact_t initCircularBufferDriver( createErrorCallback_t createErrorCallback,
                                                 logCallback_t logCallback );

#ifdef UNIT_TESTS
/**
 * @brief Test-only hook that resets the circular buffer driver back to an
 *        uninitialized state.
 * @note Compiled only when UNIT_TESTS is defined (see cCircularBufferDriverLib/CMakeLists.txt).
 *       Not present in production/release builds. Lets unit tests call
 *       initCircularBufferDriver() from a clean state instead of hitting
 *       ERROR_ALREADY_INITIALIZED.
 */
extern void resetCircularBufferDriverForTest( void );
#endif

/**
 * @brief Function to get the circular buffer driver information. This will
 *        return a structure containing accessors to get the module ID,
 *        version string, and other information about the circular buffer driver.
 * @return sCommonDriverAccessorStruct_t Structure containing accessors to
 *         get the module ID, version string, and other information about the
 *         circular buffer driver.
 */
extern sCommonDriverAccessorStruct_t const * const getCircularBufferDriverInfoAccessors( void );

/**
 * @brief initialize the ciruclar buffer with a given buffer and size.
 * @note You must call this function before using the circular buffer.
 * @param handle Pointer to the circular buffer control block.
 * @param buffer Pointer to the buffer to be used for the circular buffer.
 * @param bufferSizeInElements Size of the buffer in elements.
 * @param elementSizeInBytes Size of each element in the buffer in bytes.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
extern sErrorCompact_t circularBufferInit( sCircularBufferControlBlock_t * const handle, 
                                           void * const buffer, size_t const bufferSizeInElements, 
                                           size_t const elementSizeInBytes );


/**
 * @brief Reset the cirucular buffer clear it and head == tail.
 * @param handle Pointer to the circular buffer control block.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
extern sErrorCompact_t circularBufferReset( sCircularBufferControlBlock_t * const handle );

/**
 * @brief add data to the buffer
 * @note This adds many elements, and if they are overwriting old it is
 * entirely possible. For example if you add 20 items to a ten item buffer
 * then you will only have the last ten items. And the buffer will be full.
 * @note incoming data can only be NULL if zero address is available, meaning
 * you are on an embedded system that addresses ram at zero and you have placed
 * the data there. This is a configuration value. 
 * @param handle Pointer to the circular buffer control block.
 * @param incomingData Pointer to the data to be added.
 * @param Count count of the data to be added in elements.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
extern sErrorCompact_t circularBufferPush( sCircularBufferControlBlock_t * const handle, 
                                           void const * const incomingData, 
                                           size_t const count );


/**
 * @brief get data from the buffer
 * @param handle Pointer to the circular buffer control block.
 * @param data Pointer to the buffer where the retrieved data will be stored.
 * @param maxLength Maximum length of the data to be retrieved in elements.
 * @param amountCopied Pointer to a variable that will store the actual amount of data copied in elements.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
extern sErrorCompact_t circularBufferPop( sCircularBufferControlBlock_t * const handle, void * const data, size_t const maxLength, size_t * const amountCoppied );

/**
 * @brief Peek the buffer for a select number of elements without removing them from the buffer.
 * @param handle Pointer to the circular buffer control block.
 * @param data Pointer to the buffer where the peeked data will be stored.
 * @param maxLength Maximum length of the data to be peeked in elements.
 * @param amountPeeked Pointer to a variable that will store the actual amount of data peeked.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
extern sErrorCompact_t circularBufferPeek( sCircularBufferControlBlock_t * const handle, void * const data, size_t const maxLength, size_t * const amountPeeked );

/**
 * @brief Check if the buffer is empty
 * @param handle Pointer to the circular buffer control block.
 * @param isEmpty Pointer to a variable that will store the result.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
extern sErrorCompact_t circularBufferIsEmpty( sCircularBufferControlBlock_t * const handle, bool * const isEmpty );


/**
 * @brief Check if the buffer is full
 * @param handle Pointer to the circular buffer control block.
 * @param isFull Pointer to a variable that will store the result.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
extern sErrorCompact_t circularBufferIsFull( sCircularBufferControlBlock_t * const handle, bool * const isFull );


/** @brief accessor to get the current number of available elements.
 * @param handle Pointer to the circular buffer control block.
 * @param currentCount Pointer to a variable that will store the current number of available elements.
 * @return sErrorCompact_t structure containing error information if an error occurred.
*/
extern sErrorCompact_t circularBufferGetSize( sCircularBufferControlBlock_t const * const handle, size_t * const currentCount );

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_CIRCULAR_BUFFER_DRIVER_PUB_H */
