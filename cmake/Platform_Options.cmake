################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# Check passed options and set any missing options based on current platform
################################################################################

# Configuration
option(VS_LLVM_2014 "If set to 'ON', project will be configured for using the LLVM_2014 toolset for VS 2015." OFF)
option(Windows_VS_2015 "If set to 'ON', project will be configured for using the v140 Visual C++ toolset for VS 2015." OFF)
option(Windows_Emscripten "If set to 'ON', project will be configured for using the emsdk emscripten toolset with Mingw makefiles." OFF)

set(options_list "Windows_VS_2015 VS_LLVM_2014 Windows_Emscripten")

option(Bits_32 "If set to 'ON' the project generated will be for 32bit builds" OFF)
option(Bits_64 "If set to 'ON' the project generated will be for 64bit builds" OFF)

# Get configuration for current platform if none were selected
if ((VS_LLVM_2014 EQUAL OFF) AND (WINDOWS_VS_2015 EQUAL OFF))
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang OR CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014")
            set(VS_LLVM_2014 ON)
        elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
            set(Windows_VS_2015 ON)
        endif()
    endif()
endif()

# get the bit flags to use if none were passed by the user
if ((Bits_32 EQUAL OFF) AND (Bits_64 EQUAL OFF))
    # 64 bit
    if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
        set(Bits_64 ON)
    # 32 bit
    else()
        set(Bits_32 ON)
    endif()
endif()

# set any flags not dictated by the configuration
if (Bits_32)
    include(${cmake_flags_dir}/32Bit.cmake)
else()
    include(${cmake_flags_dir}/64Bit.cmake)
endif()

# include the correct configuration files
if(VS_LLVM_2014)
    include(${cmake_config_dir}/VS_LLVM_2014.cmake)
elseif(Windows_VS_2015)
    include(${cmake_config_dir}/Windows_VS_2015.cmake)
elseif(Windows_VS_2015_SDLSTLEmpty)
    include(${cmake_config_dir}/Windows_VS_2015_SDLSTLEmpty.cmake)
elseif(Windows_Emscripten)
    include(${cmake_config_dir}/Windows_Emscripten.cmake)
else()
    message(FATAL_ERROR "No supported platform option given and no supported config for current platform found.")
endif()

# detect any errors for a non-supported configuration being selected