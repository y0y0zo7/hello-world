#!/usr/bin/env sh
set -eu

command -v emcmake >/dev/null 2>&1 || {
    echo "ERROR: Activate the Emscripten SDK before running this script." >&2
    exit 1
}

cmake -E remove_directory build_web
cmake -E remove_directory web
emcmake cmake -S . -B build_web -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build_web --parallel

test -f web/index.html
test -f web/index.js
test -f web/index.wasm
test -f web/index.data
echo "Web build complete: web/index.html"
echo "Serve it with: python3 -m http.server 8080 --directory web"
