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
add_library(SPIRV-Tools "")
add_library(SPIRV-Tools-opt "")

################################################################################
# Explicitly define the folders as they are not organized how we want
################################################################################
set_target_properties(SPIRV-Tools PROPERTIES FOLDER "Extensions")
set_target_properties(SPIRV-Tools-opt PROPERTIES FOLDER "Extensions")

################################################################################
# include filelists for each library
################################################################################
add_subdirectory(build)
add_subdirectory(external)
add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(tools)

################################################################################
# define include directories for all of our libraries
################################################################################
target_include_directories(SPIRV-Tools 
    PUBLIC 
        ${CurrentDirectory}/build
        ${CurrentDirectory}/external/SPIRV-Headers
        ${CurrentDirectory}/external/SPIRV-Headers/include
        ${CurrentDirectory}/external/SPIRV-Headers/include/spirv
        ${CurrentDirectory}/include
        ${CurrentDirectory}/include/spirv-tools
        ${CurrentDirectory}/source
        ${CurrentDirectory}/source/comp
        ${CurrentDirectory}/source/link
        ${CurrentDirectory}/source/opt
        ${CurrentDirectory}/source/util
        ${CurrentDirectory}/source/val
        ${CurrentDirectory}/tools
        ${CurrentDirectory}/tools/as
        ${CurrentDirectory}/tools/cfg
        ${CurrentDirectory}/tools/comp
        ${CurrentDirectory}/tools/dis
        ${CurrentDirectory}/tools/link
        ${CurrentDirectory}/tools/opt
        ${CurrentDirectory}/tools/stats
        ${CurrentDirectory}/tools/val
        ${CurrentDirectory}

)
target_include_directories(SPIRV-Tools-opt
    PUBLIC 
        ${CurrentDirectory}/build
        ${CurrentDirectory}/external/SPIRV-Headers
        ${CurrentDirectory}/external/SPIRV-Headers/include
        ${CurrentDirectory}/external/SPIRV-Headers/include/spirv
        ${CurrentDirectory}/include
        ${CurrentDirectory}/include/spirv-tools
        ${CurrentDirectory}/source
        ${CurrentDirectory}/source/comp
        ${CurrentDirectory}/source/link
        ${CurrentDirectory}/source/opt
        ${CurrentDirectory}/source/util
        ${CurrentDirectory}/source/val
        ${CurrentDirectory}/tools
        ${CurrentDirectory}/tools/as
        ${CurrentDirectory}/tools/cfg
        ${CurrentDirectory}/tools/comp
        ${CurrentDirectory}/tools/dis
        ${CurrentDirectory}/tools/link
        ${CurrentDirectory}/tools/opt
        ${CurrentDirectory}/tools/stats
        ${CurrentDirectory}/tools/val
        ${CurrentDirectory}

)

################################################################################
# set the linked libraries for all of our targets
################################################################################
target_link_libraries(SPIRV-Tools
                      PUBLIC
)
target_link_libraries(SPIRV-Tools-opt
                      PUBLIC
                      SPIRV-Tools
)

################################################################################
# set the output directories for all of our targets
################################################################################
zero_multitarget_output_directories(SPIRV-Tools
                      SPIRV-Tools-opt
                      LIBRARY_DIRECTORY ${zero_library_dir}
                      RUNTIME_DIRECTORY ${zero_binary_dir}
)

################################################################################
# Specify any additional target options such as pdb locations
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_output_settings(
        SPIRV-Tools
        SPIRV-Tools-opt
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
        SPIRV-Tools
        SPIRV-Tools-opt
        PRIVATE 
        PUBLIC
            -GS -analyze-  -Zc:wchar_t
        PRIVATE
            -W3
            -wd"4302" 
            -wd"4305"
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
            -WX
            -D_SCL_SECURE_NO_WARNINGS
            ${common_flags}
    )
else()
    zero_multitarget_compile_options(
        SPIRV-Tools
        PRIVATE
        PUBLIC
        PRIVATE
        ${common_flags}
    )
    set_target_properties(
        SPIRV-Tools
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
        SPIRV-Tools
        PROPERTIES 
        STATIC_LIBRARY_FLAGS "${common_library_flags}"
        STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
    )
    set_target_properties(
        SPIRV-Tools-opt
        PROPERTIES 
        STATIC_LIBRARY_FLAGS "${common_library_flags}"
        STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
    )
endif()

################################################################################
# Group source into folders
################################################################################
#zero_subfolder_source_group(${zero_core_path} Extensions/SpirV/SpirVFiles SPIRV-Tools "")