# ##############################################################################
# CMakeLists.txt — cCircularBufferDriver
#
# Builds the circular buffer driver library (cCircularBufferDriver) as either a
# static or shared library depending on the COMPILE_CIRCULAR_BUFFER_DRIVER_LIBRARY_STATIC
# flag set by the parent CMakeLists.txt.
#
# Library layout:
#   source/       — private implementation (.c files compiled into the library)
#   include/      — private headers (only visible inside this library)
#   publicInclude/— public API headers (propagated to all consumers via PUBLIC)
#
# Dependencies:
#   commonMacros  — header-only INTERFACE target providing shared project macros
#   commonTypes   — header-only INTERFACE target providing shared project types.
#
# Version header:
#   cCircularBufferDriverVersion.h is auto-generated into the build tree on every
#   cmake --build invocation via generateVersion.cmake. It is written to:
#     <publicInclude>/generated/cCircularBufferDriverVersion.h
#   and exposes CIRCULAR_BUFFER_DRIVER_VERSION_MAJOR/MINOR/PATCH/BUILD_TYPE/STRING.
#   A stub is written at configure time so the file exists for dep scanning.
#
# Unit tests:
#   Conditionally compiled when UNIT_TESTS is ON and the test source file
#   unittest/cCircularBufferDriver_test.cpp exists. Linked against GTest::gtest_main
#   and discovered automatically via gtest_discover_tests().
# ##############################################################################

# --- Version header setup -----------------------------------------------------
set(VERSION_HEADER_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/publicInclude/generated)
set(VERSION_HEADER_FILE ${VERSION_HEADER_DIR}/cCircularBufferDriverVersion.h)

if( COMPILE_CIRCULAR_BUFFER_DRIVER_LIBRARY_STATIC )
    set(CIRCULAR_BUFFER_DRIVER_BUILD_TYPE "STATIC")
    set(CIRCULAR_BUFFER_DRIVER_BUILD_TYPE_ENUM STATIC_LIBRARY_BUILD)
else()
    set(CIRCULAR_BUFFER_DRIVER_BUILD_TYPE "DYNAMIC")
    set(CIRCULAR_BUFFER_DRIVER_BUILD_TYPE_ENUM DYNAMIC_LIBRARY_BUILD)
endif()

# Write a stub at configure time so the path exists and dependency scanning works
# before the real file is generated on the first build.
file(MAKE_DIRECTORY ${VERSION_HEADER_DIR})
if(NOT EXISTS ${VERSION_HEADER_FILE})
    file(WRITE ${VERSION_HEADER_FILE}
        "/* Stub — replaced at build time by generateVersion.cmake */\n"
        "#ifndef C_CIRCULAR_BUFFER_DRIVER_VERSION_H\n"
        "#define C_CIRCULAR_BUFFER_DRIVER_VERSION_H\n"
        "#endif /* C_CIRCULAR_BUFFER_DRIVER_VERSION_H */\n"
    )
endif()
# ------------------------------------------------------------------------------


if( COMPILE_CIRCULAR_BUFFER_DRIVER_LIBRARY_STATIC )
    add_library(${CIRCULAR_BUFFER_DRIVER_LIBRARY_NAME} STATIC
        source/cCircularBufferDriver.c
    )
else()
    add_library(${CIRCULAR_BUFFER_DRIVER_LIBRARY_NAME} SHARED
        source/cCircularBufferDriver.c
    )
endif()

target_include_directories(${CIRCULAR_BUFFER_DRIVER_LIBRARY_NAME}
    PRIVATE
        include
    PUBLIC
        publicInclude
)

target_link_libraries(${CIRCULAR_BUFFER_DRIVER_LIBRARY_NAME}
    PUBLIC
        cCommonMacros
        cCommonTypes        
)

if ( USE_MUTEXES )
    target_link_libraries(${CIRCULAR_BUFFER_DRIVER_LIBRARY_NAME}
        PUBLIC
            cMutexDriver
    )
endif()

# Propagate UNIT_TESTS as a compile definition (PUBLIC so it also reaches anything
# that links this library, e.g. cCircularBufferDriverTest below) so the test-only
# reset hook (resetCircularBufferDriverForTest) compiles in. It is absent whenever
# UNIT_TESTS is OFF, so it never ships in a release build.
if( UNIT_TESTS )
    target_compile_definitions(${CIRCULAR_BUFFER_DRIVER_LIBRARY_NAME}
        PUBLIC
            UNIT_TESTS
    )
endif()

# Expose the generated header directory so consumers can #include it
target_include_directories(${CIRCULAR_BUFFER_DRIVER_LIBRARY_NAME}
    PUBLIC
        ${VERSION_HEADER_DIR}
)

# Always regenerate cCircularBufferDriverVersion.h at every build invocation.
# add_custom_target runs unconditionally (no output tracking), ensuring a
# fresh timestamp in PATCH on every cmake --build call.
add_custom_target(GenerateCircularBufferDriverVersion ALL
    COMMAND ${CMAKE_COMMAND}
        -DVERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        -DVERSION_MINOR=${PROJECT_VERSION_MINOR}
        -DVERSION_PATCH=${PROJECT_VERSION_PATCH}
        -DVERSION_BUILD_TYPE=${CIRCULAR_BUFFER_DRIVER_BUILD_TYPE}
        -DVERSION_BUILD_TYPE_ENUM=${CIRCULAR_BUFFER_DRIVER_BUILD_TYPE_ENUM}
        -DOUTPUT_FILE=${VERSION_HEADER_FILE}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/generateVersion.cmake
    COMMENT "Regenerating cCircularBufferDriverVersion.h"
    VERBATIM
)

# Ensure the header exists before the library sources are compiled
add_dependencies(${CIRCULAR_BUFFER_DRIVER_LIBRARY_NAME} GenerateCircularBufferDriverVersion)


if( UNIT_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unittest/cCircularBufferDriver_test.cpp )
    add_executable(
        cCircularBufferDriverTest
        unittest/cCircularBufferDriver_test.cpp
    )

    target_include_directories(cCircularBufferDriverTest
        PRIVATE
            include
        ../../cCommonMacros/include
        ../../cCommonTypes/include
    )

if ( USE_MUTEXES )
    target_include_directories(cCircularBufferDriverTest
        PRIVATE
            ../../cMutexDriver/cMutexDriverLib/publicInclude
    )
endif()

    target_link_libraries(
        cCircularBufferDriverTest
        ${CIRCULAR_BUFFER_DRIVER_LIBRARY_NAME}
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(cCircularBufferDriverTest)
endif()
