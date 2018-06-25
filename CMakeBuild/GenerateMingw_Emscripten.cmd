pushd %cd%

set folder_name="Mingw_Emscripten"
rd %folder_name% /s/q

mkdir %folder_name%

cd %folder_name%

cmake -DCMAKE_TOOLCHAIN_FILE=%EMSCRIPTEN%/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=Debug -G "MinGW Makefiles" -DWindows_Emscripten=ON -DBits_32=ON -DEMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES=1 ../..

popd
