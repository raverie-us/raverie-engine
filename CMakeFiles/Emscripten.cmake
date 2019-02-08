add_definitions(-DPLATFORM_EMSCRIPTEN=1 -DPLATFORM_NAME="Emscripten")
add_definitions(-DCOMPILER_CLANG=1)

add_definitions(-DHAVE_UNISTD_H)

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(WELDER_C_CXX_FLAGS "\
  -Wno-address-of-packed-member\
  -s ALLOW_MEMORY_GROWTH=1\
  -s WASM=1\
  -s SIMD=0\
  -s USE_SDL=2\
  -s USE_WEBGL2=1\
  -s FULL_ES2=1\
  -s FULL_ES3=1\
  -s BINARYEN_TRAP_MODE='clamp'\
  -s SINGLE_FILE=1\
  --shell-file  ${WELDER_PLATFORM_DATA_DIR}/Shell.html\
  -fdelayed-template-parsing\
  -fexceptions\
  -frtti\
  -fno-vectorize\
  -fno-slp-vectorize\
  -fno-tree-vectorize\
")

set(WELDER_LINKER_FLAGS "\
  -s USE_WEBGL2=1\
  -s FULL_ES2=1\
  -s FULL_ES3=1\
")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(WELDER_C_CXX_FLAGS "${WELDER_C_CXX_FLAGS}\
    -Os\
    -g4\
    -s ASSERTIONS=2\
    -s GL_ASSERTIONS=1\
    -s DEMANGLE_SUPPORT=1\
    -s STACK_OVERFLOW_CHECK=2\
    -s SAFE_HEAP=1\
    -s ALIASING_FUNCTION_POINTERS=0\
  ")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(WELDER_C_CXX_FLAGS "${WELDER_C_CXX_FLAGS}\
    -O3\
    -g2\
  ")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}\
  --no-heap-copy\
  --embed-file \"${WELDER_VIRTUAL_FILE_SYSTEM_ZIP}\"@/FileSystem.zip\
")

set(WELDER_C_CXX_EXTERNAL_FLAGS -Wno-everything)
