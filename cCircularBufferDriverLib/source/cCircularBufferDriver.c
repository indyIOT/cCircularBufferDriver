/** ***********************************************
 * @file cCircularBufferDriver.c
 * @brief Implementation of the circular buffer driver
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "commonMacros.h"
#include "commonTypes.h"
#include "cCircularBufferDriverConfig.h"
#include "cCircularBufferDriverPub.h"
#include "cCircularBufferDriverVersion.h"
#include "cCircularBufferDriver.h"
#ifdef __cplusplus
extern "C" {
#endif

/******************************** Type definitions ****************************/

/* sCircularBufferControlBlock_t (the opaque handle every circularBufferXxx()
 * takes -- see cCircularBufferDriverPub.h) is just a byte array sized to
 * match sCircularBufferControlBlockRaw_t (the real struct, see
 * cCircularBufferDriverControlBlock.h). These fail the build loudly the
 * moment the two drift apart -- see CIRCULAR_BUFFER_CONTROL_BLOCK_SIZE_BYTES
 * in cCircularBufferDriverConfig.h for what to do when they fire. */
AG_STATIC_ASSERT( sizeof( sCircularBufferControlBlock_t ) == sizeof( sCircularBufferControlBlockRaw_t ),
                  "sCircularBufferControlBlock_t (opaque) and sCircularBufferControlBlockRaw_t (real) have drifted apart in size -- update CIRCULAR_BUFFER_CONTROL_BLOCK_SIZE_BYTES" );
AG_STATIC_ASSERT( AG_ALIGNOF( sCircularBufferControlBlock_t ) == AG_ALIGNOF( sCircularBufferControlBlockRaw_t ),
                  "sCircularBufferControlBlock_t (opaque) and sCircularBufferControlBlockRaw_t (real) have drifted apart in alignment" );

/********************************Static functions Prototypes *************/
static uint16_t getModuleId( void );
static uint8_t const * getModuleVersionString( void );
static sCommonVersionStruct_t getModuleVersion( void );
static uint8_t const * getModuleName( void );
static bool isDriverInitialized( void );
/** @Note these internal functions expect the caller to have handled mutexes */
static inline size_t circularBufferGetSizeInternal( sCircularBufferControlBlockRaw_t const * const handle );
static inline size_t addOneItemToBuffer( sCircularBufferControlBlockRaw_t * const handle, void const * const incomingData );
static inline size_t popOneItemFromTheBuffer( sCircularBufferControlBlockRaw_t * const handle, void * const copiedData );
static inline size_t circularBufferIncrementHeadInternal( sCircularBufferControlBlockRaw_t * const handle );
static inline size_t circularBufferIncrementTailInternal( sCircularBufferControlBlockRaw_t * const handle );
/******************************** Static Global Variables **********************/
static const uint8_t moduleName[] = "cCircularBufferDriver";
#define MODULE_ID 56145

static sCircularBufferDriverControlStruct_t circularBufferDriverControlStruct = {
    ._driverControl = {
        ._driverInfo = {
                        ._moduleName = moduleName,
                        ._moduleVersionString = CIRCULAR_BUFFER_DRIVER_VERSION_STRING,
                        ._moduleID = MODULE_ID,
                        ._moduleVersion = { ._major = CIRCULAR_BUFFER_DRIVER_VERSION_MAJOR,
                                            ._minor = CIRCULAR_BUFFER_DRIVER_VERSION_MINOR,
                                            ._patch = CIRCULAR_BUFFER_DRIVER_VERSION_PATCH,
                                            ._buildType = CIRCULAR_BUFFER_DRIVER_VERSION_BUILD_TYPE_ENUM
                        },
                        ._isInitialized = false
        },
        ._driverAccessors = {
                            .getModuleIdFunction = getModuleId,
                            .getModuleVersionStringFunction = getModuleVersionString,
                            .getModuleNameFunction = getModuleName,
                            .getModuleVersionFunction = getModuleVersion,
                            .isDriverInitializedFunction = isDriverInitialized },
    },
    .logMessageFunction = NULL,
    .createErrorFunction = NULL
};

/************************************Driver wide variables ********************/
sCircularBufferDriverControlStruct_t * const THIS = &circularBufferDriverControlStruct;

/**************************** HELPER MACROS ************************************/

/****************************** Function implementations ***************/

/**
 * @brief Creates an sErrorCompact_t, going through the registered create-error
 *        callback if the driver has been initialized, or filling the struct
 *        directly if it has not.
 * @note This exists so CREATE_ERROR is safe to use even when the driver was
 *       never initialized -- calling straight through THIS->createErrorFunction
 *       in that state would dereference a null function pointer, since nothing
 *       has set it yet.
 */
sErrorCompact_t createCircularBufferDriverErrorSafe( uint16_t errorCode,
                                                     uint16_t fileModuleEnum,
                                                     uint16_t lineNumber,
                                                     uint8_t const * const errorMessage,
                                                     uint8_t const * const callerModuleName )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;

    if( THIS->createErrorFunction != NULL )
    {
        retValue = THIS->createErrorFunction( errorCode, fileModuleEnum, lineNumber, false, errorMessage, callerModuleName );
    }
    else
    {
        /* Driver isn't initialized (or createErrorFunction was never set), so
           there is no registered callback to call through -- fill the struct
           directly instead of dereferencing a null function pointer. */
        retValue._errorCode = errorCode;
        retValue._fileModuleEnum = fileModuleEnum;
        retValue._lineNumber = lineNumber;
        retValue._flags = 0;
    }

    return ( retValue );
}

/**
 * @brief Reinterprets an opaque control block handle as a pointer to the
 *        real sCircularBufferControlBlockRaw_t. See cCircularBufferDriverPub.h.
 */
sCircularBufferControlBlockRaw_t * circularBufferGetRawControlBlock( sCircularBufferControlBlock_t * const handle )
{
    return ( ( handle == NULL ) ? NULL : (sCircularBufferControlBlockRaw_t *)( (void *)handle ) );
}

/**
 * @brief Const-qualified counterpart to circularBufferGetRawControlBlock().
 *        See cCircularBufferDriverPub.h.
 */
sCircularBufferControlBlockRaw_t const * circularBufferGetRawControlBlockConst( sCircularBufferControlBlock_t const * const handle )
{
    return ( ( handle == NULL ) ? NULL : (sCircularBufferControlBlockRaw_t const *)( (void const *)handle ) );
}

/**
 * @brief Function to initialize the circular buffer driver. This should be
 *        called before any other functions are used.
 * @param createErrorCallback Pointer to a function for creating errors for the circular buffer driver.
 * @param logCallback Pointer to a function for logging messages for the circular buffer driver.
 * @return sErrorCompact_t structure containing the error information if an error occurred.
 */
sErrorCompact_t initCircularBufferDriver( createErrorCallback_t createErrorCallback,
                                          logCallback_t logCallback )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;

    if( THIS->_driverControl._driverInfo._isInitialized == false )
    {
        if( createErrorCallback == NULL )
        {
            retValue = CREATE_ERROR( ERROR_NULL_POINTER,
                                     NULL );
            if( logCallback != NULL )
            {
                (void)logCallback( THIS->_driverControl._driverInfo._moduleID,
                                   __LINE__,
                                   LOGGING_TYPE_CRITICAL,
                                   "Circular Buffer Driver Initialization Failed: Create Error Callback function pointer is NULL." );
            }
        }
        else if( logCallback == NULL )
        {
            retValue = CREATE_ERROR( ERROR_NULL_POINTER,
                                     NULL );
        }
        else
        {
            THIS->createErrorFunction = createErrorCallback;
            THIS->logMessageFunction = logCallback;
            THIS->_driverControl._driverInfo._isInitialized = true;
        }
    }
    else
    {
        retValue = CREATE_ERROR( ERROR_ALREADY_INITIALIZED, NULL );
    }

    return ( retValue );
}

#ifdef UNIT_TESTS
/**
 * @brief Test-only hook that resets the circular buffer driver back to an uninitialized state.
 * @note Compiled only when UNIT_TESTS is defined. See cCircularBufferDriverPub.h.
 */
void resetCircularBufferDriverForTest( void )
{
    THIS->_driverControl._driverInfo._isInitialized = false;
    THIS->logMessageFunction = NULL;
    THIS->createErrorFunction = NULL;
}
#endif

/**
 * @brief Function to get the circular buffer driver information.
 */
sCommonDriverAccessorStruct_t const * const getCircularBufferDriverInfoAccessors( void )
{
    return (sCommonDriverAccessorStruct_t const * const)( &THIS->_driverControl._driverAccessors );
}


/**
 * @brief initialize the ciruclar buffer with a given buffer and size.
 * @note You must call this function before using the circular buffer.
 * @note You must provide a buffer for this function unless CIRCULAR_BUFFER_ALLOW_ADDRESS_0 
 * is set to DEF_TRUE in cCircularBufferDriverConfig.h. Or if allow Malloc is set to DEF_TRUE 
 * in cCircularBufferDriverConfig.h, then the buffer will be allocated for you.
 * @param handle Pointer to the circular buffer control block.
 * @param buffer Pointer to the buffer to be used for the circular buffer.
 * @param bufferSizeInElements Size of the buffer in elements.
 * @param elementSizeInBytes Size of each element in the buffer in bytes.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
sErrorCompact_t circularBufferInit( sCircularBufferControlBlock_t * const handle, 
                                           void * const buffer, 
                                           size_t const bufferSizeInElements, 
                                           size_t const elementSizeInBytes )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;

    if( handle == NULL )
    {
        retValue = CREATE_ERROR( ERROR_NULL_POINTER, NULL );
    }
    else if( handle->_isInitialized == true )
    {
        retValue = CREATE_ERROR( ERROR_ALREADY_INITIALIZED, NULL );
    }
    else if( bufferSizeInElements == 0 || elementSizeInBytes == 0 )
    {
        retValue = CREATE_ERROR( ERROR_INVALID_PARAMETER, NULL );
    }
    else
    {
        if( buffer == NULL )
        {
#if ( CIRCULAR_BUFFER_ALLOW_ADDRESS_0 == DEF_TRUE )
            handle->_pBuffer = buffer;
elif ( CIRCULAR_BUFFER_USE_MEMMORY_POOL_ALLOCATOR == DEF_TRUE )
            #error "Not yet implemented: CIRCULAR_BUFFER_USE_MEMMORY_POOL_ALLOCATOR is set to DEF_TRUE, but memory pool allocation is not yet implemented."
            handle->_isBufferAllocated = true;
#elif ( CIRCULAR_BUFFER_ALLOW_MALLOC == DEF_TRUE )
            handle->_pBuffer = malloc( bufferSizeInElements * elementSizeInBytes );
            if( handle->_pBuffer == NULL )
            {
                retValue = CREATE_ERROR( ERROR_OUT_OF_MEMORY, NULL );
            }
            else
            {
                handle->_isBufferAllocated = true;
            }
#else
            retValue = CREATE_ERROR( ERROR_NULL_POINTER, NULL );
#endif
        }
        else
        {
            handle->_pBuffer = buffer;
        }
        /* only continue if no previous error has occurred */
        if( retValue._errorCode == ERROR_NONE )
        {
            handle->_bufferSizeInBytes = ( bufferSizeInElements * elementSizeInBytes );
            handle->_elementSizeInBytes = elementSizeInBytes;
            handle->_bufferSizeInElements = bufferSizeInElements;
            handle->_isInitialized = true;
#if ( CIRUCULAR_BUFFER_USE_WRITE_MUTEX == DEF_TRUE )
            retValue = mutexInit( &handle->_writeMutex );
#endif
#if ( CIRCULAR_BUFFER_USE_READ_MUTEX == DEF_TRUE )
            if( retValue._errorCode == ERROR_NONE )
            {
                retValue = mutexInit( &handle->_readMutex );
            }
#endif
#if ( CIRCULAR_BUFFER_USE_ACCESS_MUTEX == DEF_TRUE )
            if( retValue._errorCode == ERROR_NONE )
            {
                retValue = mutexInit( &handle->_accessMutex );
            }
#endif
            if( retValue._errorCode != ERROR_NONE )
            {
                /* If any of the mutex initializations failed, free the buffer if it was allocated */
                if( handle->_isBufferAllocated )
                {
#if ( CIRCULAR_BUFFER_ALLOW_MALLOC == DEF_TRUE )
                    free( handle->_pBuffer );
                    handle->_pBuffer = NULL;
#endif
                    handle->_isBufferAllocated = false;
                }
                handle->_isInitialized = false;
            }
            else
            {
                /* Reset the buffer to clear it and set head == tail */
                retValue = circularBufferReset( handle );
            }
        }
    }

    return ( retValue );
}


/**
 * @brief Reset the cirucular buffer clear it and head == tail.
 * @param handle Pointer to the circular buffer control block.
 * @return sErrorCompact_t structure containing error information if an 
 *  error occurred.
 */
extern sErrorCompact_t circularBufferReset( sCircularBufferControlBlock_t * const handle )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;
    // Do some checks then reset the buffer.
    if( NULL == handle  )
    {
        retValue = CREATE_ERROR( ERROR_NULL_POINTER, NULL );
    }
    else if( false == handle->_isInitialized )
    {
        retValue = CREATE_ERROR( ERROR_UNINITIALIZED , NULL );
    }
    else
    {
#if ( CIRUCULAR_BUFFER_USE_WRITE_MUTEX == DEF_TRUE )
        retValue = mutexLock( &handle->_writeMutex, CIRCULAR_BUFFER_MUTEX_TIMEOUT_MS );
        if( retValue._errorCode == ERROR_NONE )
        {
#endif
#if ( CIRCULAR_BUFFER_USE_READ_MUTEX == DEF_TRUE )
            retValue = mutexLock( &handle->_readMutex, CIRCULAR_BUFFER_MUTEX_TIMEOUT_MS );
            if( retValue._errorCode == ERROR_NONE )
            {
#endif
#if ( CIRCULAR_BUFFER_USE_ACCESS_MUTEX == DEF_TRUE )
                retValue = mutexLock( &handle->_accessMutex, CIRCULAR_BUFFER_MUTEX_TIMEOUT_MS );
                if( retValue._errorCode == ERROR_NONE )
                {
#endif

                    (void)memset( handle->_pBuffer, 0, handle->_bufferSizeInBytes );
                    handle->_headIndex = 0;
                    handle->_tailIndex = 0;
                    handle->_isFull = false;                    
#if ( CIRCULAR_BUFFER_USE_ACCESS_MUTEX == DEF_TRUE )
                    retValue = mutexUnlock( &handle->_accessMutex );
                }
#endif
#if ( CIRCULAR_BUFFER_USE_READ_MUTEX == DEF_TRUE )
                retValue = RETURN_FIRST_ERROR( retValue, mutexUnlock( &handle->_readMutex ) );
            }
#endif
#if ( CIRUCULAR_BUFFER_USE_WRITE_MUTEX == DEF_TRUE )
            retValue =  RETURN_FIRST_ERROR( retValue, mutexUnlock( &handle->_writeMutex ) );
        }
#endif                        
    }

    return ( retValue );
}

/**
 * @brief add data to the buffer
 * @param handle Pointer to the circular buffer control block.
 * @note This adds elements one at a time, and if they are overwriting old it is
 * entirely possible. For example if you add 20 items to a ten item buffer
 * then you will only have the last ten items. And the buffer will be full.
 * @note incoming data can only be NULL if zero address is available, meaning
 * you are on an embedded system that addresses ram at zero and you have placed
 * the data there. This is a configuration value. 
 * @param incomingData Pointer to the data to be added.
 * @param Count Count of the data to be added in elements.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
sErrorCompact_t circularBufferPush( sCircularBufferControlBlock_t * const handle, 
                                    void const * const incomingData, 
                                    size_t const count )
{
    /* Make sure everything is good here. */
    size_t currentBufferCount = 0U;
    size_t countTillEnd = 0U;
    int32_t overage = 0;
    int32_t actualCount = count;
    size_t copyCount = count * handle->_elementSizeInBytes;
    void const * currentIP = incomingData;
    void * currentWriteP = handle->_pBuffer;
    size_t currentIncomingIndex = 0;
    size_t currentWriteIndex = handle->_headIndex * handle->_elementSizeInBytes;
    if(  NULL == handle )
    {
        retValue = CREATE_ERROR(ERROR_NULL_POINTER, NULL );
    }
    else if( false == handle->_isInitialized )
    {
        retValue = CREATE_ERROR( ERROR_UNINITIALIZED , NULL );
    }
    else if( count == 0 )
    {
        retValue = CREATE_ERROR( ERROR_INVALID_PARAMETER, NULL );
    }
    /* Do the Mutex dance */
#if ( CIRCULAR_BUFFER_ALLOW_ADDRESS_0 == DEF_FASLE )
    else if( NULL == incomingData )
    {
        retValue = CREATE_ERROR(ERROR_NULL_POINTER, NULL );
    }
#endif
    else
    {
        #if ( CIRUCULAR_BUFFER_USE_WRITE_MUTEX == DEF_TRUE )
        retValue = mutexLock( &handle->_writeMutex, CIRCULAR_BUFFER_MUTEX_TIMEOUT_MS );
        if( retValue._errorCode == ERROR_NONE )
        {
#endif
#if ( CIRCULAR_BUFFER_USE_READ_MUTEX == DEF_TRUE )
            retValue = mutexLock( &handle->_readMutex, CIRCULAR_BUFFER_MUTEX_TIMEOUT_MS );
            if( retValue._errorCode == ERROR_NONE )
            {
#endif
#if ( CIRCULAR_BUFFER_USE_ACCESS_MUTEX == DEF_TRUE )
                retValue = mutexLock( &handle->_accessMutex, CIRCULAR_BUFFER_MUTEX_TIMEOUT_MS );
                if( retValue._errorCode == ERROR_NONE )
                {
#endif
                    if( count == 1U )
                    {
                        currentBufferCount = addOneItemToBuffer( handle,  incomingData );
                    }
                    else
                    {
                        /* If we are writing more elements than the buffer can hold
                         Just reset the buffer and write them to the beginning setting
                         isFull to true */
                        if( handle->_bufferSizeInElements < count )
                        {
                            copyCount = handle->_bufferSizeInBytes;
                            currentIncomingIndex = ( count * handle->__elementSizeInBytes ) - handle->_bufferSizeInBytes;
                            currentIP = &incomingData[ currentIncomingIndex ];
                            handle->_headIndex = 0;
                            handle->_tailIndex = 0;
                            handle->_isFull = true;
                            (void)memcpy( currentWriteP, currentIP, copyCount); 
                        }
                        else
                        {
                            /* Need to make sure we can just plop it on or if we need
                            to do some wrapping. Also count on whether or not 
                            we will need to move the tail. */
                            currentBufferCount = circularBufferGetSizeInternal( handle );
                            countTillEnd = handle->_bufferSizeInElements -  handle->_headIndex;
                            overage = count - ( handle->_bufferSizeInElements - currentBufferCount );
                            /* Wrap required?*/
                            if( countTillEnd < count )
                            {
                                currentWriteP = &handle->_pBuffer[currentWriteIndex];
                                copyCount = countTillEnd * handle->_elementSizeInBytes;
                                (void)memcpy( currentWriteP, currentIP, copyCount );                                
                                handle->_headIndex = 0;
                                currentWriteIndex = 0;
                                actualCount = count - countTillEnd;
                                copyCount = ( count - countTillEnd )* handle->_elementSizeInBytes;
                            }

                            /* Tail Movement requried */
                            if( overage > 0 )
                            { 
                                handle->_tailIndex = handle->_headIndex;
                                handle->_isFull = true;
                            }



                        }
                        
                        
                    }
#if ( CIRCULAR_BUFFER_USE_ACCESS_MUTEX == DEF_TRUE )
                    retValue = mutexUnlock( &handle->_accessMutex );
                }
#endif
#if ( CIRCULAR_BUFFER_USE_READ_MUTEX == DEF_TRUE )
                retValue = RETURN_FIRST_ERROR( retValue, mutexUnlock( &handle->_readMutex ) );
            }
#endif
#if ( CIRUCULAR_BUFFER_USE_WRITE_MUTEX == DEF_TRUE )
            retValue =  RETURN_FIRST_ERROR( retValue, mutexUnlock( &handle->_writeMutex ) );
        }
#endif    
    }
}


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
 * @param amountPeeked Pointer to a variable that will store the actual amount of data peek
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
extern sErrorCompact_t circularBufferPeek( sCircularBufferControlBlock_t * const handle, void * const data, size_t const maxLength, size_t * const amountPeeked );

/**
 * @brief Check if the buffer is empty
 * @param handle Pointer to the circular buffer control block.
 * @param isEmpty Pointer to a variable that will store the result.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
sErrorCompact_t circularBufferIsEmpty( sCircularBufferControlBlock_t * const handle, bool * const isEmpty )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;
    /* handle is the opaque sCircularBufferControlBlock_t -- reinterpret it as
     * the real struct once, up front, then work against raw from here down.
     * This is the pattern every other circularBufferXxx() function below
     * still needs to adopt (they currently dereference handle->_xxx directly,
     * which no longer compiles now that sCircularBufferControlBlock_t is just
     * a byte array -- see cCircularBufferDriverPub.h / cCircularBufferDriverControlBlock.h). */
    sCircularBufferControlBlockRaw_t * const raw = circularBufferGetRawControlBlock( handle );

    // Do some checks then pass back whether or not the buffer is full.
    if( ( NULL == raw ) || ( NULL == isEmpty ) )
    {
        retValue = CREATE_ERROR(ERROR_NULL_POINTER, NULL );
    }
    else if( false == raw->_isInitialized )
    {
        retValue = CREATE_ERROR( ERROR_UNINITIALIZED , NULL );
    }
    else
    {
        *isEmpty = false;

        if( false == raw->_isFull )
        {
            *isEmpty = ( raw->_headIndex == raw->_tailIndex );
        }
    }

    return ( retValue );
}

/**
 * @brief Check if the buffer is full
 * @param handle Pointer to the circular buffer control block.
 * @param isFull Pointer to a variable that will store the result.
 * @return sErrorCompact_t structure containing error information if an error occurred.
 */
sErrorCompact_t circularBufferIsFull( sCircularBufferControlBlock_t * const handle, 
                                      bool * const isFull )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;

    // Do some checks then pass back whether or not the buffer is full.
    if( ( NULL == handle ) || ( NULL == isEmpty ) )
    {
        retValue = CREATE_ERROR(ERROR_NULL_POINTER, NULL );
    }
    else if( false == handle->_isInitialized )
    {
        retValue = CREATE_ERROR( ERROR_UNINITIALIZED , NULL );
    }
    else
    {
        *isFull = handle->_isFull;
    }

    return ( retValue );
}


/** 
 * @brief accessor to get the current number of available elements.
 * @param handle Pointer to the circular buffer control block.
 * @param currentCount Pointer to a variable that will store the current number of available elements.
 * @return sErrorCompact_t structure containing error information if an error occurred.
*/
sErrorCompact_t circularBufferGetSize( sCircularBufferControlBlock_t const * const handle, size_t * const currentCount )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;

    // Do some checks Null pointer, init size
    if( ( NULL == handle ) || ( NULL == currentCount ) )
    {
        retValue = CREATE_ERROR(ERROR_NULL_POINTER, NULL );
    }
    else if( false == handle->_isInit )
    {
        retValue = CREATE_ERROR(CIRCULAR_BUFFER_NOT_INIT);
    }
    else
    {
        *currentCount = circularBufferGetSizeInternal( handle );
    }

    return ( retValue );
}

/************************ Static Function Implementations ***************/
/**
 * @brief Function to get the module ID of the circular buffer driver.
 */
static uint16_t getModuleId( void )
{
    return ( THIS->_driverControl._driverInfo._moduleID );
}

/**
 * @brief Function to get the module version string of the circular buffer driver.
 */
static uint8_t const * getModuleVersionString( void )
{
    return ( THIS->_driverControl._driverInfo._moduleVersionString );
}

/**
 * @brief Function to get the module version of the circular buffer driver.
 */
static sCommonVersionStruct_t getModuleVersion( void )
{
    return ( THIS->_driverControl._driverInfo._moduleVersion );
}

/**
 * @brief Function to get the module name of the circular buffer driver.
 */
static uint8_t const * getModuleName( void )
{
    return ( THIS->_driverControl._driverInfo._moduleName );
}

/**
 * @brief Function to check if the circular buffer driver is initialized.
 */
static bool isDriverInitialized( void )
{
    return ( THIS->_driverControl._driverInfo._isInitialized );
}

/**
 * @brief Static function that will check the current size of the buffer, it will
 * only do zero errorchecking to do so expects handle to be initialized and not equal
 * to NULL.
 * @param handle to a circular buffer control block
 * @return The curent Count of items in the circular buffer.
 */
static inline size_t circularBufferGetSizeInternal( sCircularBufferControlBlock_t const * const handle )
{
    size_t retValue = handle->_bufferSizeInElements;
    if( false == handle->_isFull )
    {
        if( handle->_headIndex == handle->_tailIndex )
        {
            retValue = 0;
        }
        else
        {
            retValue = handle->_headIndex;
            if( handle->_headIndex < handle->_tailIndex )
            {
                retValue += handle->_bufferSizeInElements;
            }
            retValue -= handle->_tailIndex;
        }
    }

    return ( retValue );
}

/**
 * @brief Static function that will add one item to the buffer. IT assumes that
 * all mutexes are handled externally. And does no error checking.
 * @param handle A handle to the circular control block.
 * @param incomingData One item to be added to the buffer.
 * @retrun current size of the buffer.
 */
static inline size_t addOneItemToBuffer( sCircularBufferControlBlock_t * const handle, void  const * const incomingData )
{
    size_t retValue = 0u;
    void * currentP =  &handle->pBuffer[handle->_headIndex * handle->_elementSizeInBytes];
    (void)memcpy( currentP, incomingData, handle->_elementSizeInBytes);
    retValue = circularBufferIncrementHeadInternal( handle );
    return ( retValue );
}

/** 
 * @brief Static function that will pop one item from the buffer to the copiedData pointer. It asssumes
 * all mutexes and pointers are already checked, it does no error checking.
 */
static inline size_t popOneItemFromTheBuffer( sCircularBufferControlBlock_t * const handle, void * const copiedData )
{
    size_t retValue = 0u;
    void * currentP =  &handle->pBuffer[handle->_tailIndex * handle->_elementSizeInBytes];
    (void)memcpy( incomingData, currentP, handle->_elementSizeInBytes );
    retValue = circularBufferIncrementTailInternal( handle );
    return ( retValue );
}
/**
 * @brief Static function that will increment the head index, it will do this with
 * wrap around and set isfull if neccissary. It will not check for initialized
 * or NULL.
 * @note I realize mod can be used but mod usess alot of instructions where as
 * just coding it out uses a lot less. 
 * @param handle to a circular buffer control block
 * @return The curent Count of items in the circular buffer.
 */
static inline size_t circularBufferIncrementHeadInternal( sCircularBufferControlBlock_t * const handle )
{
    size_t currentSize = circularBufferGetSizeInternal( handle );
    bool fullAtIncrement = handle->_isFull;
    if( fullAtIncrement == true )
    {
        currentSize = circularBufferIncrementTailInternal( handle );
    }

    /* Increment the head. */
    handle->_headIndex++;
    if( handle->_headIndex >= handle->_bufferSizeInElements )
    {
        handle->_headIndex = 0;
    }
    handle->_isFull = fullAtIncrement;
    currentSize++;

    return ( currentSize );
}

/**
 * @brief Static function that will increment the tail index, it will do this with
 * wrap around and clear isfull if neccissary. It will not check for initialized
 * or NULL.
 * @note I realize mod can be used but mod usess alot of instructions where as
 * just coding it out uses a lot less. 
 * @param handle to a circular buffer control block
 * @return The curent Count of items in the circular buffer.
 */
static inline size_t circularBufferIncrementTailInternal( sCircularBufferControlBlock_t * const handle )
{
    size_t currentSize = circularBufferGetSizeInternal( handle );

    /* Only decrement buffer if there is anything in there. */
    if( 0 < currentSize )
    {
        handle->_isFull = false;
        handle->_tailIndex++;
        if ( handle->_tailIndex >= handle->_bufferSizeInElements )
        {
            handle->_tailIndex = 0;
        }
        currentSize--;
    }

    return ( currentSize );
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
/***********************************************************
 * Static initializers
 */



/**
 * \brief add data to the buffer
 * \param handle - pointer to the circular buffer handle
 * \param incomingData - pointer to the data to add
 * \param length - length of the data to add
 * \return error code
 * \note This function will add data to the buffer and will wrap around if needed.
 */
sErrorStruct_t circularBufferPut( sCircularBuffer_t * const handle,
                                uint8_t const * const incomingData,
                                size_t const length )
{
    sErrorStruct_t retValue = BLANK_ERROR_STRUCT;
    bool willBeFull = false;

    // Do some checks then do the add
    if( ( NULL == handle ) || ( NULL == incomingData ) || ( NULL == handle->_buffer ) )
    {
        retValue = CREATE_ERROR(ERROR_NULL_PTR);
    }
    else if( false == handle->_isInit )
    {
        retValue = CREATE_ERROR(CIRCULAR_BUFFER_NOT_INIT);
    }
    else
    {
        // Have to put a valid amount of data in the buffer.
        if( ( length > 0 ) && ( handle->_maxSize >= length ) )
        {
            // If we ended up wrapping around if we did the head will be managed.
            for( size_t i = 0; ( i < length  ); i++ )
            {
                // Is the handle currently full
                if( true == handle->_full )
                {
                    handle->_tail++;
                    if ( handle->_tail >= handle->_maxSize )
                    {
                        handle->_tail = 0;
                    }
                }

                // Will it be full after this
                if( ( ( handle->_head + 1 )== handle->_tail ) ||
                    ( ( 0 == handle->_tail ) && ( ( handle->_head + 1) == handle->_maxSize ) ) )
                {
                    willBeFull = true;
                }

                handle->_buffer[handle->_head++] = incomingData[i];

                if ( handle->_head >= handle->_maxSize )
                {
                    handle->_head  = 0;
                }

                if( ( handle->_head == handle->_tail ) && ( true == willBeFull ))
                {
                    handle->_full = true;
                }
            }
        }
        else
        {
            // Only an error if length is greater than max size
            if( length > handle->_maxSize )
            {
                retValue = CREATE_ERROR(CIRCULAR_BUFFER_INVALID_SIZE);
            }
        }
    }

    return ( retValue );
}


/**
 * \brief get data from the buffer, max length is the largest amount of data that can
 * be coppied to the new buffer.
 * \param handle - pointer to the circular buffer handle
 * \param data - pointer to the data to get
 * \param maxLength - max length of the data to get
 * \param amountCoppied - pointer to the amount of data that was coppied
 * \return error code
 */
sErrorStruct_t circularBufferGet( sCircularBuffer_t * const handle,
                                uint8_t * const data,
                                size_t const maxLength,
                                size_t * const amountCoppied )
{
    sErrorStruct_t retValue = BLANK_ERROR_STRUCT;
    size_t tempLength = 0;
    size_t currentSize = 0;
    size_t i = 0;

    // Do some checks then do the copy
    if( ( NULL == handle ) || ( NULL == data ) || ( NULL == amountCoppied ) || ( NULL == handle->_buffer ) )
    {
        retValue = CREATE_ERROR(ERROR_NULL_PTR);
    }
    else
    {
        *amountCoppied = 0;
        if( false == handle->_isInit )
        {
            retValue = CREATE_ERROR(CIRCULAR_BUFFER_NOT_INIT);
        }
        else
        {
            // Changed this logic to be more readable.
            if( maxLength > 0 )
            {

                // Get the size of the buffer
                tempLength = circularBufferGetSizeInternal( handle );
                // If we have more inside the buffer than the consumer can take.
                if( tempLength > maxLength )
                {
                    tempLength = maxLength;
                }

                currentSize =  circularBufferGetSizeInternal( handle );
                for( i = 0; ( ( i < tempLength ) && ( 0 < currentSize )); i++ )
                {
                    data[i] = handle->_buffer[handle->_tail++];
                    handle->_full = false;
                    if ( handle->_tail >= handle->_maxSize )
                    {
                        handle->_tail = 0;
                    }
                    currentSize =  circularBufferGetSizeInternal( handle );
                }
                *amountCoppied = i;
                if( i != tempLength )
                {
                    retValue = CREATE_ERROR(CIRCULAR_BUFFER_INVALID_DECREMENT);
                }
            }
            else
            {
                retValue = CREATE_ERROR(CIRCULAR_BUFFER_INVALID_SIZE);
            }
        }
    }
    return( retValue );
}
