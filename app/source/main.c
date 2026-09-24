/** **************************************************************
 * @file: main.c
 * @brief Main entry point of the test application.
 * @author Anthony Garza
 * @copyright Copyright 2023 All Rights Reserved.
****************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include "commonMacros.h"
#include "cCircularBufferDriverPub.h"

static sErrorCompact_t fakeLogCallback( uint16_t moduleId,
                                        uint16_t line,
                                        eLoggingType_t type,
                                        const char *message, ... )
{
    char buffer[256];

    va_list args;
    va_start( args, message );
    vsnprintf( buffer, sizeof( buffer ), message, args );
    va_end( args );

    printf( "[Module %u, Line %u, Type %u] %s\n", moduleId, line, type, buffer );

    sErrorCompact_t retValue = { 0 };
    return retValue;
}

static sErrorCompact_t fakeCreateErrorCallback( uint16_t errorCode,
                                                uint16_t fileModuleEnum,
                                                uint16_t lineNumber,
                                                bool autoStoreError,
                                                uint8_t const * const errorMessage,
                                                uint8_t const * const moduleName )
{
    sErrorCompact_t retValue = BLANK_ERROR_STRUCT;
    printf( "Error Code: %u  Module: %u  Line: %u  Message: %s\n",
           (unsigned int)errorCode, (unsigned int)fileModuleEnum, (unsigned int)lineNumber,
           ( errorMessage ? (const char *)errorMessage : "NULL" ) );
    (void)autoStoreError;
    (void)moduleName;
    return retValue;
}

/**
 * @brief Program Main entry for testing libraries.
 *
 * @return int
 */
int main( void )
{
    int retValue = ERROR_NONE;
    sErrorCompact_t errorInfo = initCircularBufferDriver( (createErrorCallback_t)&fakeCreateErrorCallback,
                                                          (logCallback_t)&fakeLogCallback );

    printf( "initCircularBufferDriver: errorCode=%u\n", (unsigned int)errorInfo._errorCode );

    return( retValue );
}
