add_definitions(-DWelderCompilerClang=1 -DWelderCompilerName="Clang")

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
  -pthread\
")

set(WELDER_C_CXX_FLAGS_DEBUG "\
  -g\
")

set(WELDER_C_CXX_FLAGS_RELWITHDEBINFO "\
  -O3\
  -g\
")

set(WELDER_C_CXX_FLAGS_RELEASE "\
  -O3\
")

set(WELDER_C_CXX_FLAGS_MINSIZEREL "\
  -Oz\
")

#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker /ignore:4049,4217")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker --start-group")

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
