@echo off
setlocal

where emcmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: Activate the Emscripten SDK before running this script.
    exit /b 1
)

cmake -E remove_directory build_web
cmake -E remove_directory web
call emcmake cmake -S . -B build_web -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
if errorlevel 1 exit /b 1

cmake --build build_web --parallel
if errorlevel 1 exit /b 1

if not exist web\index.html exit /b 1
if not exist web\index.js exit /b 1
if not exist web\index.wasm exit /b 1
if not exist web\index.data exit /b 1
echo Web build complete: web\index.html
echo Serve it with: python -m http.server 8080 --directory web
