################################################################################
# Author: Joshua Shlemmer, Dane Curbow
# Copyright 2017, DigiPen Institute of Technology
# configured for setting the emcc flags
################################################################################

# When using our defined common_flags variable all -s flags following the first one are removed and results in an invalid command line argument.
# By directly setting CMAKE_CXX_FLAGS with our compiler flags the subsequent -s flags remain and the project compiles properly.
# We're not entirely certain which flags go to the compiler vs linker, so we duplicate the command line for both.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEMSCRIPTEN=1 -D__EMSCRIPTEN__=1 -s TOTAL_MEMORY=134217728 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 --no-heap-copy -s DEMANGLE_SUPPORT=1 -s FORCE_FILESYSTEM=1 -s WASM=1 -s USE_SDL=2 -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 -s BINARYEN_TRAP_MODE='clamp' -msse -msse2 -std=c++11 -O3 -fstack-protector -fdelayed-template-parsing -fexceptions -fno-omit-frame-pointer -frtti -DZeroExportDll --shell-file ${zero_core_path}/PlatformData/Emscripten/ZeroShell.html")
set(common_linker_flags                "-DEMSCRIPTEN=1 -D__EMSCRIPTEN__=1 -s TOTAL_MEMORY=134217728 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 --no-heap-copy -s DEMANGLE_SUPPORT=1 -s FORCE_FILESYSTEM=1 -s WASM=1 -s USE_SDL=2 -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 -s BINARYEN_TRAP_MODE='clamp' -msse -msse2 -std=c++11 -O3 -fstack-protector -fdelayed-template-parsing -fexceptions -fno-omit-frame-pointer -frtti -DZeroExportDll --shell-file ${zero_core_path}/PlatformData/Emscripten/ZeroShell.html")

# To use the EMTERPRETER add these lines: 
# -s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1 -g -s EMTERPRETIFY_WHITELIST=@${cmake_utilities_dir}/EmscriptenWhitelist.json
