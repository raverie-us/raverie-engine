add_definitions(-DPLATFORM_EMSCRIPTEN=1 -DPLATFORM_HARDWARE=1 -DPLATFORM_BITS="32" -DPLATFORM_NAME="Emscripten")
add_definitions(-DCOMPILER_CLANG=1)

# When using our defined common_flags variable all -s flags following the first one are removed and results in an invalid command line argument.
# By directly setting CMAKE_CXX_FLAGS with our compiler flags the subsequent -s flags remain and the project compiles properly.
# We're not entirely certain which flags go to the compiler vs linker, so we duplicate the command line for both.

#set(WELDER_EMSCRIPTEN_OPTIONS "-DEMSCRIPTEN=1 -D__EMSCRIPTEN__=1 -s TOTAL_MEMORY=134217728 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s SINGLE_FILE=1 --no-heap-copy -s DEMANGLE_SUPPORT=1 -s FORCE_FILESYSTEM=1 -s WASM=1 -s USE_SDL=2 -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 -s BINARYEN_TRAP_MODE='clamp' -msse -msse2 -std=c++11 -O3 -fstack-protector -fdelayed-template-parsing -fexceptions -fno-omit-frame-pointer -frtti -DZeroExportDll --shell-file ${WELDER_PLATFORM_DATA_DIR}/Emscripten/ZeroShell.html")

#set(WELDER_COMPILE_OPTIONS ${WELDER_EMSCRIPTEN_OPTIONS})
#set(WELDER_STATIC_LIBRARY_FLAGS ${WELDER_EMSCRIPTEN_OPTIONS})
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEMSCRIPTEN=1 -D__EMSCRIPTEN__=1 -s TOTAL_MEMORY=134217728 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s SINGLE_FILE=1 --no-heap-copy -s DEMANGLE_SUPPORT=1 -s FORCE_FILESYSTEM=1 -s WASM=1 -s USE_SDL=2 -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 -s BINARYEN_TRAP_MODE='clamp' -msse -msse2 -std=c++11 -O3 -fstack-protector -fdelayed-template-parsing -fexceptions -fno-omit-frame-pointer -frtti -DZeroExportDll --shell-file ${zero_core_path}/PlatformData/Emscripten/ZeroShell.html")

#set(common_linker_flags                "-DEMSCRIPTEN=1 -D__EMSCRIPTEN__=1 -s TOTAL_MEMORY=134217728 -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS=1 -s SINGLE_FILE=1 --no-heap-copy -s DEMANGLE_SUPPORT=1 -s FORCE_FILESYSTEM=1 -s WASM=1 -s USE_SDL=2 -s USE_WEBGL2=1 -s FULL_ES2=1 -s FULL_ES3=1 -s BINARYEN_TRAP_MODE='clamp' -msse -msse2 -std=c++11 -O3 -fstack-protector -fdelayed-template-parsing -fexceptions -fno-omit-frame-pointer -frtti -DZeroExportDll --shell-file ${zero_core_path}/PlatformData/Emscripten/ZeroShell.html")

#set(WELDER_COMPILE_OPTIONS -std=c++11 -fdelayed-template-parsing)

add_subdirectory(Libraries/Platform/Emscripten)