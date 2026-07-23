@echo off
setlocal

where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: CMake 3.20 or newer is required.
    exit /b 1
)

cmake -E remove_directory build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=ON
if errorlevel 1 exit /b 1

cmake --build build --config Release --parallel
if errorlevel 1 exit /b 1

ctest --test-dir build -C Release --output-on-failure
if errorlevel 1 exit /b 1

echo.
echo Controls: click toggles ON/OFF, drag boosts speed, ESC quits.
pushd build\Release
Fan3D.exe
set result=%errorlevel%
popd
exit /b %result%
