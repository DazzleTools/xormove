@echo off
REM xormove Windows Build Script
REM Run from VS Developer Command Prompt or PowerShell with VS tools in PATH

setlocal enabledelayedexpansion

REM Navigate to project root
cd /d "%~dp0\.."

REM Parse arguments first to check for help
set BUILD_TYPE=Release
set CLEAN_BUILD=0
set RUN_TESTS=0
set USE_PRESET=0
set SHOW_HELP=0
set CONFIGURE_ONLY=0

:parse_args
if "%1"=="" goto :done_parsing
if /i "%1"=="--help" set SHOW_HELP=1
if /i "%1"=="-h" set SHOW_HELP=1
if /i "%1"=="--debug" set BUILD_TYPE=Debug
if /i "%1"=="--release" set BUILD_TYPE=Release
if /i "%1"=="--clean" set CLEAN_BUILD=1
if /i "%1"=="--rebuild" set CLEAN_BUILD=1
if /i "%1"=="--test" set RUN_TESTS=1
if /i "%1"=="--preset" set USE_PRESET=1
if /i "%1"=="--configure" set CONFIGURE_ONLY=1
shift
goto :parse_args

:done_parsing

if %SHOW_HELP%==1 (
    echo xormove Windows Build Script
    echo.
    echo Usage: build-windows.cmd [options]
    echo.
    echo Options:
    echo   --help, -h      Show this help message
    echo   --debug         Build debug configuration
    echo   --release       Build release configuration (default)
    echo   --clean         Clean build directory before building
    echo   --rebuild       Same as --clean
    echo   --test          Run tests after building
    echo   --configure     Configure only, don't build
    echo   --preset        Use CMake presets instead of manual configuration
    echo.
    echo Examples:
    echo   build-windows.cmd                    # Release build
    echo   build-windows.cmd --debug            # Debug build
    echo   build-windows.cmd --clean --test     # Clean rebuild with tests
    echo   build-windows.cmd --preset           # Use CMake presets
    echo.
    exit /b 0
)

REM Check for required tools
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found. Install Visual Studio 2022 with C++ workload or install CMake separately.
    exit /b 1
)

REM Use CMake presets if requested
if %USE_PRESET%==1 (
    echo Using CMake presets...
    set PRESET_NAME=windows-!BUILD_TYPE:R=r!
    set PRESET_NAME=!PRESET_NAME:D=d!

    if %CLEAN_BUILD%==1 (
        echo Cleaning build directory...
        if exist "build\windows-release" rmdir /s /q "build\windows-release"
        if exist "build\windows-debug" rmdir /s /q "build\windows-debug"
    )

    echo Configuring with preset: windows-%BUILD_TYPE%
    cmake --preset windows-%BUILD_TYPE%
    if errorlevel 1 (
        echo ERROR: CMake configuration failed
        exit /b 1
    )

    if %CONFIGURE_ONLY%==1 (
        echo Configuration complete.
        exit /b 0
    )

    echo Building with preset: windows-%BUILD_TYPE%
    cmake --build --preset windows-%BUILD_TYPE%
    if errorlevel 1 (
        echo ERROR: Build failed
        exit /b 1
    )

    if %RUN_TESTS%==1 (
        echo Running tests...
        ctest --preset windows-%BUILD_TYPE%
    )

    echo.
    echo Build successful!
    exit /b 0
)

REM Find vcpkg - check common locations
set VCPKG_ROOT=
if defined VCPKG_ROOT if exist "%VCPKG_ROOT%\vcpkg.exe" goto :found_vcpkg
if exist "C:\vcpkg\vcpkg.exe" set VCPKG_ROOT=C:\vcpkg& goto :found_vcpkg
if exist "%LOCALAPPDATA%\vcpkg\vcpkg.exe" set VCPKG_ROOT=%LOCALAPPDATA%\vcpkg& goto :found_vcpkg
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\vcpkg\vcpkg.exe" (
    set "VCPKG_ROOT=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\vcpkg"
    goto :found_vcpkg
)
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\vcpkg\vcpkg.exe" (
    set "VCPKG_ROOT=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\vcpkg"
    goto :found_vcpkg
)
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\vcpkg\vcpkg.exe" (
    set "VCPKG_ROOT=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\vcpkg"
    goto :found_vcpkg
)

echo ERROR: vcpkg not found. Please set VCPKG_ROOT environment variable.
echo Common locations:
echo   - C:\vcpkg
echo   - %%LOCALAPPDATA%%\vcpkg
echo   - Visual Studio 2022 installation
exit /b 1

:found_vcpkg
echo Using vcpkg at: %VCPKG_ROOT%

REM Clean build directory if requested
if %CLEAN_BUILD%==1 (
    echo Cleaning build directory...
    if exist build rmdir /s /q build
)

REM Create build directory
if not exist build mkdir build

REM Configure with CMake
echo.
echo Configuring CMake (%BUILD_TYPE%)...
cmake -B build -S . ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

if errorlevel 1 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

if %CONFIGURE_ONLY%==1 (
    echo Configuration complete.
    exit /b 0
)

REM Build
echo.
echo Building...
cmake --build build --config %BUILD_TYPE%

if errorlevel 1 (
    echo ERROR: Build failed
    exit /b 1
)

REM Run tests if requested
if %RUN_TESTS%==1 (
    echo.
    echo Running tests...
    ctest --test-dir build --build-config %BUILD_TYPE% --output-on-failure
)

echo.
echo Build successful!
echo Executable: build\%BUILD_TYPE%\xormove.exe
