/** ***********************************************
 * @file cCircularBufferDriverErrorCodes.h
 * @brief Error codes specific to the circular buffer driver, past the common
 *        error codes shared by every module (see commonTypes.h's eCommonErrorCodes_t).
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/
#include <stdint.h>
#include "commonTypes.h"
#ifndef C_CIRCULAR_BUFFER_DRIVER_ERROR_CODES_H
#define C_CIRCULAR_BUFFER_DRIVER_ERROR_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CIRCULAR_BUFFER_DRIVER_ERROR_CODES
#define CIRCULAR_BUFFER_DRIVER_ERROR_CODES
/**
 * @brief Circular buffer driver error codes. Starts at END_OF_COMMON_ERRORS
 *        so these never collide with the common error codes every module
 *        shares. Add real codes here as they're needed (e.g. buffer full,
 *        buffer empty, element size mismatch).
 */
typedef enum
{
    CIRCULAR_BUFFER_SIZE_MATH_ERROR = END_OF_COMMON_ERRORS,
    LAST_CIRCULAR_BUFFER_DRIVER_ERROR_CODE
} eCircularBufferDriverErrorCodes_t;
#endif // CIRCULAR_BUFFER_DRIVER_ERROR_CODES

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_CIRCULAR_BUFFER_DRIVER_ERROR_CODES_H */
