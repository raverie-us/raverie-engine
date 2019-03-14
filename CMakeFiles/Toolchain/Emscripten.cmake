add_definitions(-DWelderCompilerClang=1 -DWelderCompilerName="Clang")

add_definitions(-DHAVE_UNISTD_H)

#  -s DISABLE_EXCEPTION_CATCHING=0\

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(WELDER_C_CXX_FLAGS "\
  -Wno-address-of-packed-member\
  -Wno-empty-body\
  -s ALLOW_MEMORY_GROWTH=1\
  -s BINARYEN_TRAP_MODE='clamp'\
  -s WASM=1\
  -s SIMD=0\
  -s USE_SDL=2\
  -s USE_WEBGL2=1\
  -s FULL_ES2=1\
  -s FULL_ES3=1\
  -s SINGLE_FILE=1\
  --shell-file  ${WELDER_PLATFORM_DATA_DIR}/Shell.html\
  -fdelayed-template-parsing\
  -fexceptions\
  -frtti\
  -fstack-protector\
  -fno-vectorize\
  -fno-slp-vectorize\
  -fno-tree-vectorize\
  -fno-omit-frame-pointer\
")

set(WELDER_LINKER_FLAGS "\
  -s USE_WEBGL2=1\
  -s FULL_ES2=1\
  -s FULL_ES3=1\
")

set(WELDER_C_CXX_FLAGS_DEBUG "\
  -Os\
  -g\
  -s ASSERTIONS=2\
  -s GL_ASSERTIONS=1\
  -s DEMANGLE_SUPPORT=1\
  -s STACK_OVERFLOW_CHECK=2\
  -s SAFE_HEAP=1\
  -s WARN_UNALIGNED=1\
")

set(WELDER_C_CXX_FLAGS_RELWITHDEBINFO "\
  -O3\
  -g\
  -s ASSERTIONS=2\
  -s GL_ASSERTIONS=1\
  -s DEMANGLE_SUPPORT=1\
  -s STACK_OVERFLOW_CHECK=2\
  -s SAFE_HEAP=1\
  -s WARN_UNALIGNED=1\
")

set(WELDER_C_CXX_FLAGS_RELEASE "\
  -O3\
")

set(WELDER_C_CXX_FLAGS_MINSIZEREL "\
  -Oz\
")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}\
  --no-heap-copy\
  --embed-file \"${WELDER_VIRTUAL_FILE_SYSTEM_ZIP}\"@/FileSystem.zip\
")

set(WELDER_C_CXX_EXTERNAL_FLAGS -Wno-everything)

function(welder_use_precompiled_header target directory)
  #add_library(${target}PrecompiledHeader)
  #
  #target_sources(${target}PrecompiledHeader
  #  PRIVATE
  #    ${directory}/Precompiled.hpp
  #    ${directory}/Precompiled.cpp
  #)
  #
  #set_source_files_properties(${directory}/Precompiled.hpp PROPERTIES
  #  COMPILE_FLAGS "-xc++-header -c"
  #  LANGUAGE CXX
  #)
  #
  #get_target_property(targetIncludeDirectories ${target} INCLUDE_DIRECTORIES)
  #set_target_properties(${target}PrecompiledHeader PROPERTIES INCLUDE_DIRECTORIES "${targetIncludeDirectories}")
  #
  #get_target_property(binaryDir "${target}PrecompiledHeader" BINARY_DIR)
  #
  #set_target_properties(${target} PROPERTIES COMPILE_FLAGS "-include-pch ${binaryDir}/CMakeFiles/${target}PrecompiledHeader.dir/Precompiled.hpp.o")
  #
  #add_dependencies(${target} ${target}PrecompiledHeader)
endfunction()

function(welder_source_ignore_precompiled_header source)
endfunction()
