pushd %~dp0

set folder_name="Mingw_Emscripten_Emscripten_WASM"
rd %folder_name% /s/q

mkdir %folder_name%

pushd %folder_name%

cmake -G "MinGW Makefiles" -DWELDER_TOOLCHAIN=Emscripten -DWELDER_PLATFORM=Emscripten -DCMAKE_TOOLCHAIN_FILE=%EMSCRIPTEN%/cmake/Modules/Platform/Emscripten.cmake -DEMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES=1 -DPLATFORM_ARCHITECTURE=WASM ../..

popd

popd
