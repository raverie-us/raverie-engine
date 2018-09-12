################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# configuration for using v140 toolset with visual studio 2015
################################################################################
include(${cmake_os_dir}/Windows.cmake)
include(${cmake_compiler_dir}/MSVC.cmake)
include(${cmake_flags_dir}/MSVC_FLAGS.cmake)
include(${cmake_os_dir}/SDLSTLEmpty.cmake)

set(configuration Windows_VS_2015_SDLSTLEmpty)

