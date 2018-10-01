################################################################################
# Author: Joshua Davis
# Copyright 2018, DigiPen Institute of Technology
################################################################################
# get our parent directory
get_filename_component(ParentDirectory ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# get our current directory
set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

################################################################################
# define all of the libraries that are in this folder
################################################################################
add_library(SPIRV-Cross "")

add_subdirectory(SpirVFiles)

################################################################################
# Explicitly define the folders as they are not organized how we want
################################################################################
set_target_properties(SPIRV-Cross PROPERTIES FOLDER "Extensions")

################################################################################
# include filelists for each library
################################################################################
include(SPIRV-Cross/CMakeLists.txt)

################################################################################
# define include directories for all of our libraries
################################################################################
target_include_directories(SPIRV-Cross 
    PUBLIC 
        ${CurrentDirectory}/SPIRV-Cross
        ${CurrentDirectory}

)

################################################################################
# set the linked libraries for all of our targets
################################################################################
target_link_libraries(SPIRV-Cross
                      PUBLIC
)

################################################################################
# set the output directories for all of our targets
################################################################################
zero_multitarget_output_directories(SPIRV-Cross
                      LIBRARY_DIRECTORY ${zero_library_dir}
                      RUNTIME_DIRECTORY ${zero_binary_dir}
)

################################################################################
# Specify any additional target options such as pdb locations
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_output_settings(
        SPIRV-Cross
        CONFIGS ${supported_configs}
        BASEPATH ${zero_build_out}
        PLATFORM ${platform}
        CONFIG ${configuration}
        BITS ${bit}
        TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
        TARGET_SUBFOLDER
    )
endif()

################################################################################
# set flags and definitions
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_compile_options(  
        SPIRV-Cross
        PRIVATE 
        PUBLIC
            -GS -analyze-  -Zc:wchar_t
        PRIVATE
            -W3
            -wd"4302" 
            -wd"4305"
            -wd"4800"
            -Gd
            -EHsc
            -Gm
            -GR-
            -fp:fast 
            -Zc:forScope 
            -Oy-
            -Od
            -Zc:inline
            -MDd
            -nologo
            ${common_flags}
    )
else()
    zero_multitarget_compile_options(
        SPIRV-Cross
        PRIVATE
        PUBLIC
        PRIVATE
        ${common_flags}
    )
    set_target_properties(
        SPIRV-Cross
        PROPERTIES
        LINK_FLAGS ${common_linker_flags}
    )
endif()

################################################################################
# Set linker flags
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    # set the correct subsystems for executable targets, and set stack size for the editor
    set_target_properties(
        SPIRV-Cross
        PROPERTIES 
        STATIC_LIBRARY_FLAGS "${common_library_flags}"
        STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
    )
endif()

################################################################################
# Group source into folders
################################################################################
zero_subfolder_source_group(${zero_core_path} Extensions/SpirV SPIRV-Cross "")