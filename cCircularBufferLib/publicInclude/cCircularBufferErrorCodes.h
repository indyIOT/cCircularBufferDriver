/** ***********************************************
 * @file cCircularBufferErrorCodes.h
 * @brief Public interface for the circular buffer error codes
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cErrorDriverPub.h"
#ifndef C_CIRCULAR_BUFFER_ERROR_CODES_H
#define C_CIRCULAR_BUFFER_ERROR_CODES_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef ERROR_NONE
#undef ERROR_NONE
#endif

/** Enumeration of Common Error Codes These are always going to be the first error 
 * codes of every Errorcode Enumeration */
typedef enum 
{
} eCircularBufferErrorCodes_t;

                                                       
#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_CIRCULAR_BUFFER_ERROR_CODES_H */