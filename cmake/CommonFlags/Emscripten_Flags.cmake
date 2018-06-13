################################################################################
# Author: Joshua Shlemmer, Dane Curbow
# Copyright 2017, DigiPen Institute of Technology
# configured for setting the emcc flags
################################################################################

# When using our defined common_flags variable all -s flags following the first one are removed and results in an invalid command line argument.
# By directly setting CMAKE_CXX_FLAGS with our compiler flags the subsequent -s flags remain and the project compiles properly.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEMSCRIPTEN=1 -D__EMSCRIPTEN__=1 -s WASM=1 -s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 -msse -msse2 -std=c++11 -fstack-protector -O0 -fdelayed-template-parsing -fexceptions -fno-omit-frame-pointer -frtti -DZeroExportDll" )

set(common_linker_flags
    "-DEMSCRIPTEN=1 -D__EMSCRIPTEN__=1 -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -s USE_SDL=2 -s FORCE_FILESYSTEM=1 -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 -msse -msse2 -std=c++11"
)