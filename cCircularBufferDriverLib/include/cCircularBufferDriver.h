/*********************************************************************
 * @file cCircularBufferDriver.h
 * @brief Private interface for the circular buffer driver.
 * @author Anthony Garza
 * @copyright All rights reserved 2026
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "commonMacros.h"
#include "commonTypes.h"
#include "cCircularBufferDriverConfig.h"
#include "cCircularBufferDriverErrorCodes.h"
#include "cCircularBufferDriverPub.h"
#include "cCircularBufferDriverControlBlock.h"

#ifndef C_CIRCULAR_BUFFER_DRIVER_H
#define C_CIRCULAR_BUFFER_DRIVER_H
#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Type Definitions used by driver */
typedef struct
{
    sCommonDriverControlStruct_t _driverControl; /* Control structure for the circular buffer driver */
    logCallback_t logMessageFunction; /* Pointer to a function for logging information */
    createErrorCallback_t createErrorFunction; /* Pointer to a function for creating errors */
} sCircularBufferDriverControlStruct_t;



/*********************  ***External Circular Buffer Driver Private Interface ********/

extern sCircularBufferDriverControlStruct_t * const THIS;

/**************************** HELPER MACROS ************************************/
#ifndef ERROR_NONE
#define ERROR_NONE 0U
#endif

#ifndef NO_ERROR
#define NO_ERROR 0U
#endif

#define ACCESSOR_MUTEX_LOCKED                                               0x01
#define READ_MUTEX_LOCKED                                                   0x02
#define WRITE_MUTEX_LOCKED                                                  0x04
/**
 * @brief Creates an sErrorCompact_t, going through the registered create-error
 *        callback if the driver has been initialized, or filling the struct
 *        directly if it has not.
 * @note This exists so CREATE_ERROR is safe to use even when the driver was
 *       never initialized -- calling straight through THIS->createErrorFunction
 *       in that state would dereference a null function pointer, since nothing
 *       has set it yet. Mirrors createMutexDriverErrorSafe() in cMutexDriver /
 *       createIntegrityDriverErrorSafe() in cIntegrityDriver.
 * @param errorCode The error code for this error.
 * @param fileModuleEnum The module ID where the error occurred.
 * @param lineNumber The source line where the error occurred.
 * @param errorMessage A message describing the error.
 * @param callerModuleName The name of the module reporting the error.
 * @return sErrorCompact_t structure containing the error information.
 */
extern sErrorCompact_t createCircularBufferDriverErrorSafe( uint16_t errorCode,
                                                             uint16_t fileModuleEnum,
                                                             uint16_t lineNumber,
                                                             uint8_t const * const errorMessage,
                                                             uint8_t const * const callerModuleName );

#ifndef CREATE_ERROR
#define CREATE_ERROR( errorCode, errorMessage ) \
    createCircularBufferDriverErrorSafe( errorCode, MODULE_ID, __LINE__, errorMessage, moduleName )
#endif

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_CIRCULAR_BUFFER_DRIVER_H */
