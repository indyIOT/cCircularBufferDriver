/** ***********************************************
 * @file cCircularBufferDriverConfig.h
 * @brief Overridable configuration of the circular buffer driver.
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "commonMacros.h"
#ifndef C_CIRCULAR_BUFFER_DRIVER_CONFIG_H
#define C_CIRCULAR_BUFFER_DRIVER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CUSTOM_CIRCULAR_BUFFER_DRIVER_CONFIG

/* No overridable knobs yet -- add them here (guarded by #ifndef, matching
   every other config value in this codebase) as the driver grows. */
#define CIRUCULAR_BUFFER_USE_WRITE_MUTEX                  DEF_TRUE
#define CIRCULAR_BUFFER_USE_READ_MUTEX                    DEF_TRUE
#define CIRCULAR_BUFFER_USE_ACCESS_MUTEX                  DEF_TRUE
#if ( ( CIRUCULAR_BUFFER_USE_WRITE_MUTEX == DEF_TRUE ) || \
      ( CIRCULAR_BUFFER_USE_READ_MUTEX == DEF_TRUE ) || \
      ( CIRCULAR_BUFFER_USE_ACCESS_MUTEX == DEF_TRUE ) )
#define CIRCULAR_BUFFER_USE_MUTEX_DRIVER                  DEF_TRUE
#else
#define CIRCULAR_BUFFER_USE_MUTEX_DRIVER                  DEF_FALSE
#endif

#define CIRCULAR_BUFFER_USE_LOGGING                       DEF_TRUE
/* This will keep the buffer from being initialized with a NULL pointer */
#define CIRCULAR_BUFFER_ALLOW_ADDRESS_0                   DEF_FALSE
/* If both allow malloc and allow memory pools is set to true then memory pools will be used!*/
#define CIRCULAR_BUFFER_ALLOW_MALLOC                      DEF_TRUE
#define CIRCULAR_BUFFER_USE_MEMMORY_POOL_ALLOCATOR        DEF_FALSE
#endif // CUSTOM_CIRCULAR_BUFFER_DRIVER_CONFIG
#ifndef CIRCULAR_BUFFER_MUTEX_TIMEOUT_MS
#define CIRCULAR_BUFFER_MUTEX_TIMEOUT_MS                       1000
#endif

#ifndef CIRCULAR_BUFFER_ALLIGN_SIZE
#define CIRCULAR_BUFFER_ALLIGN_SIZE                               4
#endif

/* This was handcounted so do not change unless you change it manually */
#ifndef CIRUCLAR_CONTROL_PRIMATIVE_SIZE
#define CIRUCLAR_CONTROL_PRIMATIVE_SIZE                        ( 7 * sizeof( uint32_t ) )
#endif

/* Hand-measured size (in bytes) of sCircularBufferControlBlockRaw_t, the real
 * control block defined in cCircularBufferDriverControlBlock.h -- NOT
 * computed from sizeof(sMutex_t) here on purpose, so this header (and
 * cCircularBufferDriverPub.h, which uses this to size the opaque
 * sCircularBufferControlBlock_t handle) never has to include cMutexDriverPub.h
 * or know mutexes are involved at all. Measured for the default config
 * (all three mutexes enabled, CIRCULAR_BUFFER_ALLOW_MALLOC == DEF_TRUE) on a
 * 64-bit Windows build (MUTEX_BACKEND_WIN32, 8-byte size_t). If you change
 * the real struct's fields, MUTEX_BACKEND, or the target's word size, this
 * WILL be wrong -- cCircularBufferDriver.c has a static assert that fails
 * the build loudly the moment it drifts, so re-measure sizeof(sCircularBufferControlBlockRaw_t)
 * and update this constant when that fires. */
#ifndef CIRCULAR_BUFFER_CONTROL_BLOCK_SIZE_BYTES
#define CIRCULAR_BUFFER_CONTROL_BLOCK_SIZE_BYTES                       188
#endif

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* C_CIRCULAR_BUFFER_DRIVER_CONFIG_H */
