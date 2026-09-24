/** ***********************************************
 * @file cCircularBufferDriver_test.cpp
 * @brief Unit tests for cCircularBufferDriver.c
 * @author Anthony Garza
 * @copyright All rights reserved 2026
*************************************************/

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <cstdarg>   // va_list, va_start, va_end
#include <cstdio>    // vsnprintf
#include <iostream>
#include "commonMacros.h"
#include "commonTypes.h"
#include "../publicInclude/cCircularBufferDriverPub.h"
#include "../publicInclude/cCircularBufferDriverConfig.h"
#include "../publicInclude/cCircularBufferDriverErrorCodes.h"

using namespace std;

namespace
{
    sErrorCompact_t fakeCreateErrorCallback( uint16_t errorCode,
                                             uint16_t fileModuleEnum,
                                             uint16_t lineNumber,
                                             bool autoStoreError,
                                             uint8_t const * const errorMessage,
                                             uint8_t const * const moduleName )
    {
        (void)autoStoreError;
        (void)errorMessage;
        (void)moduleName;

        sErrorCompact_t retValue = { 0 };
        retValue._errorCode = errorCode;
        retValue._fileModuleEnum = fileModuleEnum;
        retValue._lineNumber = lineNumber;
        return retValue;
    }

    sErrorCompact_t fakeLogCallback( uint16_t moduleId,
                                     uint16_t line,
                                     eLoggingType_t type,
                                     const char *message, ... )
    {
        sErrorCompact_t retValue = { 0 };
        char buffer[256];

        va_list args;
        va_start( args, message );
        vsnprintf( buffer, sizeof( buffer ), message, args );
        va_end( args );

        cout << "[Module " << moduleId << ", Line " << line
             << ", Type " << type << "] " << buffer << endl;

        return retValue;
    }

    /** @brief Known-stable identifiers mirrored from cCircularBufferDriver.c (private #define/static there). */
    constexpr uint16_t kCircularBufferDriverModuleId = 56145U;
    const char * const kCircularBufferDriverModuleName = "cCircularBufferDriver";

    /**
     * @brief Fixture that gives every test a freshly-uninitialized driver.
     *        Relies on resetCircularBufferDriverForTest(), a UNIT_TESTS-only
     *        hook (see cCircularBufferDriverPub.h), because the driver's
     *        control struct is a file-scope static singleton with no
     *        production way to de-initialize it.
     */
    class CircularBufferDriverTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            resetCircularBufferDriverForTest();
        }

        sErrorCompact_t initValid()
        {
            return initCircularBufferDriver( (createErrorCallback_t)&fakeCreateErrorCallback,
                                             (logCallback_t)&fakeLogCallback );
        }
    };

} // namespace

/*****************************************************************************
 * initCircularBufferDriver()
 ****************************************************************************/

TEST_F( CircularBufferDriverTest, InitWithValidParametersSucceeds )
{
    EXPECT_EQ( ERROR_NONE, initValid()._errorCode );
}

TEST_F( CircularBufferDriverTest, InitWithNullCreateErrorCallbackFailsWithNullPointer )
{
    sErrorCompact_t errorInfo = initCircularBufferDriver( NULL, (logCallback_t)&fakeLogCallback );
    EXPECT_EQ( ERROR_NULL_POINTER, errorInfo._errorCode );
}

TEST_F( CircularBufferDriverTest, InitWithNullLogCallbackFailsWithNullPointer )
{
    sErrorCompact_t errorInfo = initCircularBufferDriver( (createErrorCallback_t)&fakeCreateErrorCallback, NULL );
    EXPECT_EQ( ERROR_NULL_POINTER, errorInfo._errorCode );
}

TEST_F( CircularBufferDriverTest, InitCalledTwiceReturnsAlreadyInitialized )
{
    ASSERT_EQ( ERROR_NONE, initValid()._errorCode );
    EXPECT_EQ( ERROR_ALREADY_INITIALIZED, initValid()._errorCode );
}

/*****************************************************************************
 * getCircularBufferDriverInfoAccessors()
 ****************************************************************************/

TEST_F( CircularBufferDriverTest, AccessorsReturnNonNullStruct )
{
    sCommonDriverAccessorStruct_t const * const accessors = getCircularBufferDriverInfoAccessors();

    ASSERT_NE( nullptr, accessors );
    EXPECT_NE( nullptr, accessors->getModuleIdFunction );
    EXPECT_NE( nullptr, accessors->getModuleVersionStringFunction );
    EXPECT_NE( nullptr, accessors->getModuleNameFunction );
    EXPECT_NE( nullptr, accessors->getModuleVersionFunction );
    EXPECT_NE( nullptr, accessors->isDriverInitializedFunction );
}

TEST_F( CircularBufferDriverTest, AccessorsReportModuleIdAndName )
{
    sCommonDriverAccessorStruct_t const * const accessors = getCircularBufferDriverInfoAccessors();

    EXPECT_EQ( kCircularBufferDriverModuleId, accessors->getModuleIdFunction() );
    EXPECT_STREQ( kCircularBufferDriverModuleName, reinterpret_cast<char const *>( accessors->getModuleNameFunction() ) );
}

TEST_F( CircularBufferDriverTest, AccessorsReportVersionInfoUsableForVersionChecking )
{
    sCommonDriverAccessorStruct_t const * const accessors = getCircularBufferDriverInfoAccessors();

    uint8_t const * const versionString = accessors->getModuleVersionStringFunction();
    ASSERT_NE( nullptr, versionString );
    string versionStr( reinterpret_cast<char const *>( versionString ) );
    EXPECT_GT( versionStr.size(), 0U );

    sCommonVersionStruct_t version = accessors->getModuleVersionFunction();
    EXPECT_EQ( STATIC_LIBRARY_BUILD, version._buildType ); // COMPILE_CIRCULAR_BUFFER_DRIVER_LIBRARY_STATIC is ON

    // An app might version-check via the structured major/minor/patch fields, or
    // by parsing/logging the human-readable string -- both should agree. The
    // generated string's format is "MAJOR.MINOR.PATCH.BUILDTYPE.TIMESTAMP".
    string expectedPrefix = to_string( version._major ) + "." +
                            to_string( version._minor ) + "." +
                            to_string( version._patch ) + ".";
    EXPECT_EQ( 0U, versionStr.rfind( expectedPrefix, 0U ) )
        << "version string '" << versionStr << "' did not start with '" << expectedPrefix << "'";
}

TEST_F( CircularBufferDriverTest, AccessorsIsDriverInitializedReflectsState )
{
    sCommonDriverAccessorStruct_t const * const accessors = getCircularBufferDriverInfoAccessors();

    EXPECT_FALSE( accessors->isDriverInitializedFunction() );
    ASSERT_EQ( ERROR_NONE, initValid()._errorCode );
    EXPECT_TRUE( accessors->isDriverInitializedFunction() );
}

/*****************************************************************************
 * circularBufferInit() / circularBufferPush() / circularBufferPop() / etc.
 *
 * Not implemented yet -- see cCircularBufferDriverPub.h. Add tests here
 * alongside that API.
 ****************************************************************************/
