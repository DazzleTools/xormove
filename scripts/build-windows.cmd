@echo off
REM xormove Windows Build Script
REM Run from VS Developer Command Prompt or PowerShell with VS tools in PATH

setlocal enabledelayedexpansion

REM Navigate to project root
cd /d "%~dp0\.."

REM Check for required tools
where cmake >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found. Install Visual Studio 2022 with C++ workload or install CMake separately.
    exit /b 1
)

REM Find vcpkg - check common locations
set VCPKG_ROOT=
if exist "%VCPKG_ROOT%\vcpkg.exe" goto :found_vcpkg
if exist "C:\vcpkg\vcpkg.exe" set VCPKG_ROOT=C:\vcpkg& goto :found_vcpkg
if exist "%LOCALAPPDATA%\vcpkg\vcpkg.exe" set VCPKG_ROOT=%LOCALAPPDATA%\vcpkg& goto :found_vcpkg
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\vcpkg\vcpkg.exe" (
    set VCPKG_ROOT=%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\vcpkg
    goto :found_vcpkg
)
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\vcpkg\vcpkg.exe" (
    set VCPKG_ROOT=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\vcpkg
    goto :found_vcpkg
)
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\vcpkg\vcpkg.exe" (
    set VCPKG_ROOT=%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\vcpkg
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

REM Parse arguments
set BUILD_TYPE=Release
set CLEAN_BUILD=0

:parse_args
if "%1"=="" goto :done_parsing
if /i "%1"=="--debug" set BUILD_TYPE=Debug
if /i "%1"=="--release" set BUILD_TYPE=Release
if /i "%1"=="--clean" set CLEAN_BUILD=1
shift
goto :parse_args

:done_parsing

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

REM Build
echo.
echo Building...
cmake --build build --config %BUILD_TYPE%

if errorlevel 1 (
    echo ERROR: Build failed
    exit /b 1
)

echo.
echo Build successful!
echo Executable: build\%BUILD_TYPE%\xormove.exe
