@echo off

:: Get the source root directory
set ODYSSEY_SOURCE_DIR=%~dp0
set USE_NMAKE=0

:: Detect presence of cmake
cmd /c cmake --version 2>&1 | find "cmake version" > NUL || goto cmake_notfound

:: Detect build environment (MinGW)
if defined ROS_ARCH (
    echo Detected RosBE for %ROS_ARCH%
    set BUILD_ENVIRONMENT=MinGW
    set ARCH=%ROS_ARCH%
    set CMAKE_GENERATOR="MinGW Makefiles"
) else (
    echo Error: Unable to detect build environment. Configure script failure.
    exit /b
)

:: Checkpoint
if not defined ARCH (
    echo unknown build architecture
    exit /b
)

:: Create directories
set ODYSSEY_OUTPUT_PATH=output-%BUILD_ENVIRONMENT%-%ARCH%
if "%ODYSSEY_SOURCE_DIR%" == "%CD%\" (
    echo Creating directories in %ODYSSEY_OUTPUT_PATH%

    if not exist %ODYSSEY_OUTPUT_PATH% (
        mkdir %ODYSSEY_OUTPUT_PATH%
    )
    cd %ODYSSEY_OUTPUT_PATH%
)

if not exist host-tools (
    mkdir host-tools
)
if not exist odyssey (
    mkdir odyssey
)

echo Preparing host tools...
cd host-tools
if EXIST CMakeCache.txt (
    del CMakeCache.txt /q
)
set ODYSSEY_BUILD_TOOLS_DIR=%CD%

cmake -G %CMAKE_GENERATOR% -DARCH=%ARCH% %ODYSSEY_SOURCE_DIR%
cd..

echo Preparing odyssey...
cd odyssey
if EXIST CMakeCache.txt (
    del CMakeCache.txt /q
)

if "%BUILD_ENVIRONMENT%" == "MinGW" (
	cmake -G %CMAKE_GENERATOR% %* -DENABLE_CCACHE=0 -DPCH=0 -DCMAKE_TOOLCHAIN_FILE=toolchain-gcc.cmake -DARCH=%ARCH% -DODYSSEY_BUILD_TOOLS_DIR:DIR="%ODYSSEY_BUILD_TOOLS_DIR%" %ODYSSEY_SOURCE_DIR%
)

cd..

echo Configure script complete! Enter directories and execute appropriate build commands(ex: make, nmake, jom, etc...).
exit /b

:cmake_notfound
 echo Unable to find cmake, if it is installed, check your PATH variable.
 exit /b
