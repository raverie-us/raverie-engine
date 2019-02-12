add_definitions(-DPLATFORM_EMSCRIPTEN=1 -DPLATFORM_NAME="Stub")
add_definitions(-DCOMPILER_CLANG=1)

# If we're compiling under Windows...
add_definitions(-D_UNICODE -DUNICODE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SECURE_SCL=0 -D_NO_CVCONST_H)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(WELDER_C_CXX_FLAGS "\
  -Wno-address-of-packed-member\
  -Wno-empty-body\
  -fdelayed-template-parsing\
  -fexceptions\
  -frtti\
  -fno-vectorize\
  -fno-slp-vectorize\
  -fno-tree-vectorize\
")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(WELDER_C_CXX_FLAGS "${WELDER_C_CXX_FLAGS}\
    -g\
  ")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(WELDER_C_CXX_FLAGS "${WELDER_C_CXX_FLAGS}\
    -O3\
  ")
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker /ignore:4049,4217")

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
