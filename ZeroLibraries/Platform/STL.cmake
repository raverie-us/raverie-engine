################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################

################################################################################
# define all of the libraries that are in this folder
################################################################################
add_library(Platform "")

################################################################################
# Explicitly define the folders as they are not organized how we want
################################################################################
set_target_properties(Platform PROPERTIES FOLDER "ZeroLibraries/Platform")

################################################################################
# include filelists for each library
################################################################################
include(STL/CMakeLists.txt)

################################################################################
# define include directories for all of our libraries
################################################################################
target_include_directories(Platform
    PUBLIC
        ${CurrentDirectory}/STL 
        ${WinHidHeaders}
        ${GLEWHeaders}
        ${CEFHeaders}
)

################################################################################
# set the linked libraries for all of our targets
################################################################################
target_link_libraries(Platform
                      PUBLIC
                      Common
                      ${WinHidStatic}
                      ${GLEWStatic}
                      ${CEFStatic}
)

################################################################################
# set the output directories for all of our targets
################################################################################
zero_multitarget_output_directories(
    Platform
    LIBRARY_DIRECTORY ${zero_library_dir}
    RUNTIME_DIRECTORY ${zero_binary_dir}
)

################################################################################
# Specify any additional target options such as pdb locations
################################################################################
zero_multitarget_output_settings(
    Platform
    CONFIGS ${supported_configs}
    BASEPATH ${zero_build_out}
    PLATFORM ${platform}
    BITS ${bit}
    TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
    PRECOMPILED_HEADER_NAME "Precompiled.hpp"
    PRECOMPILED_SOURCE_NAME "Precompiled.cpp"
    TARGET_SUBFOLDER "Platform"
)

################################################################################
# set flags and definitions
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_compile_options(  
        Platform
        PRIVATE
         
        PUBLIC
            -GS -analyze-  -Zc:wchar_t
            
        PRIVATE
            -W3 -wd"4302"
            ${common_flags}
    )
endif()

################################################################################
# Set linker flags
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    # set the correct subsystems for executable targets, and set stack size for the editor
    set_target_properties(
        Platform
        PROPERTIES 
        STATIC_LIBRARY_FLAGS "${common_library_flags}"
        STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
    )
endif()

################################################################################
# Group source into folders
################################################################################
zero_subfolder_source_group_ignore_target_folder(${zero_core_path} ZeroLibraries/Platform Platform "${zero_core_path/Platform}")
