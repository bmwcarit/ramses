::  -------------------------------------------------------------------------
::  Copyright (C) 2018 BMW Car IT GmbH
::  -------------------------------------------------------------------------
::  This Source Code Form is subject to the terms of the Mozilla Public
::  License, v. 2.0. If a copy of the MPL was not distributed with this
::  file, You can obtain one at https://mozilla.org/MPL/2.0/.
::  -------------------------------------------------------------------------

@ECHO OFF

:: check number of arguments
IF "%~4"=="" (
    ECHO "Usage: %~n0 <config> <build-dir> <install-dir> <generator> [<gl-version>]"
    EXIT /B 1
)
IF NOT "%~6"=="" (
    ECHO "Usage: %~n0 <config> <build-dir> <install-dir> <generator> [<gl-version>]"
    EXIT /B 1
)

SET BUILD_CONFIG=%1
SET TEST_DIR=%2
SET INSTALL_DIR=%3
SET CMAKE_GENERATOR=%4
SET GL_VERSION=%5
SET RENDERER_PLATFORM="WINDOWS"
SET CURRENT_WORKING_DIRECTORY=%cd%
SET SCRIPT_DIR=%~dp0

echo "++++ Create test environment for install check of shared lib (%BUILD_CONFIG%, %GL_VERSION%) ++++"

:: test here
rd /S /Q %TEST_DIR%
mkdir %TEST_DIR%
cd %TEST_DIR%

:: run cmake config
echo "++++ Build with cmake config for install check of shared lib++++"
cd %TEST_DIR%
rd /S /Q test-cmake.config
mkdir test-cmake.config
cd  test-cmake.config

cmake -G%CMAKE_GENERATOR% -DCMAKE_PREFIX_PATH="%INSTALL_DIR%/lib" -DRAMSES_RENDERER_PLATFORM=%RENDERER_PLATFORM% -DGL_VERSION=%GL_VERSION% --build test-cmake.config %SCRIPT_DIR%/shared-lib-check/
cmake --build . --config %BUILD_CONFIG%

SET EXECUTABLE_PATH=%cd%\%BUILD_CONFIG%
start /d %EXECUTABLE_PATH% ramses-shared-lib-check

::check for errors
IF /I "%ERRORLEVEL%" NEQ "0" (
    cd %CURRENT_WORKING_DIRECTORY%
    ECHO %~n0: build failed
    EXIT /B 1
)

echo "++++ build check done for install check of shared lib++++"
cd %CURRENT_WORKING_DIRECTORY%
