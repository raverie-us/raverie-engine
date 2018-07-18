################################################################################
# Author: Joshua Shlemmer, Dane Curbow
# Copyright 2017, DigiPen Institute of Technology
# configured for using the emsdk emscripten toolset with Mingw makefiles
################################################################################
include(${cmake_os_dir}/Emscripten.cmake)
include(${cmake_compiler_dir}/Clang.cmake)
include(${cmake_flags_dir}/Emscripten_Flags.cmake)

set(configuration Emscripten)
