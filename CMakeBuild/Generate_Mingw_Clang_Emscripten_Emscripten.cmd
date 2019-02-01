pushd %~dp0

set folder_name="Mingw_Clang_Emscripten_Emscripten"
rd %folder_name% /s/q

mkdir %folder_name%

pushd %folder_name%

cmake -DCMAKE_TOOLCHAIN_FILE=%EMSCRIPTEN%/cmake/Modules/Platform/Emscripten.cmake -G "MinGW Makefiles" -DEMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES=1 ../..

popd

popd

REM MakeFileSystemZip.bat
