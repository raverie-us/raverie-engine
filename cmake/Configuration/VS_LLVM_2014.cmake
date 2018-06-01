################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# configuration for using the LLVM_2014 toolset on the windows platform
################################################################################
include(${cmake_os_dir}/Windows.cmake)
include(${cmake_compiler_dir}/Clang.cmake)
include(${cmake_flags_dir}/MSVC_FLAGS.cmake)

add_compile_options(-FIPrecompiled.hpp)
