add_definitions(-DPLATFORM_EMSCRIPTEN=1 -DPLATFORM_NAME="Emscripten")
add_definitions(-DCOMPILER_CLANG=1)

add_definitions(-DHAVE_UNISTD_H)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(WELDER_C_CXX_FLAGS "\
  -Wno-expansion-to-defined\
  -Wno-address-of-packed-member\
  -Wno-incompatible-pointer-types\
  -Wno-undefined-var-template\
  -Wno-switch\
  -Wno-tautological-undefined-compare\
  -Wno-#warnings\
  -s TOTAL_MEMORY=134217728\
  -s ALLOW_MEMORY_GROWTH=1\
  -s ASSERTIONS=1\
  -s DEMANGLE_SUPPORT=1\
  -s FORCE_FILESYSTEM=1\
  -s WASM=1\
  -s SIMD=0\
  -s USE_SDL=2\
  -s USE_WEBGL2=1\
  -s FULL_ES2=1\
  -s FULL_ES3=1\
  -s BINARYEN_TRAP_MODE='clamp'\
  -s SINGLE_FILE=1\
  --shell-file ${WELDER_PLATFORM_DATA_DIR}/PlatformData/Emscripten/ZeroShell.html\
  --no-heap-copy\
  -fdelayed-template-parsing\
  -fexceptions\
  -fno-omit-frame-pointer\
  -fno-vectorize\
  -fno-slp-vectorize\
  -fno-tree-vectorize\
  -frtti\
  -O3\
")
