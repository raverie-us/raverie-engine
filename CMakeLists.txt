################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################
################################################################################
# Set required version of cmake and any version specific policies
################################################################################
cmake_minimum_required(VERSION 3.9.1 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)

################################################################################
# Declare project(s) and set used languages
################################################################################
project(Zero CXX)

################################################################################
# Do platform specific checks for chunks of the library paths
################################################################################

# windows libraries
if( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(platform Windows)
    # 64 bit windows libraries
    if( ${CMAKE_SIZEOF_VOID_P} EQUAL 8 )
        message(FATAL_ERROR "64bit windows is currently not supported.")
    # 32 bit windows libraries
    else()
        set(bit x86)
    endif()
# non-windows libraries
else()
    message(FATAL_ERROR "no other platforms are currently supported besides windows. Current Sys: ${CMAKE_SYSTEM_NAME}")
endif()

################################################################################
# Set up global variables
################################################################################

#paths
set(cmake_include ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(zero_core_path ${CMAKE_CURRENT_SOURCE_DIR})
message("CorePath: ${zero_core_path}\n")
set(zero_build_out ${zero_core_path}/BuildOutput)
set(supported_configs $<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>)
set(zero_library_dir ${zero_build_out}/Out/${supported_configs})
set(zero_binary_dir ${zero_build_out}/Out/${supported_configs})
set(zero_intermediates_dir ${zero_build_out}/Int/${supported_configs})
set(zero_unit_test_dir ${zero_core_path}/UnitTests)

#flags
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(common_flags     
            $<$<CONFIG:Debug>:-GS>
            $<$<CONFIG:Release>:-MP>
            $<$<CONFIG:Release>:-GS->
            $<$<CONFIG:Release>:-GL>
    
            -analyze-
            -W3 
            -wd"4302"
            -Zc:wchar_t
    
            $<$<CONFIG:Debug>:-ZI>
            $<$<CONFIG:Release>:-Zi>
            $<$<CONFIG:Debug>:-Gm>
            $<$<CONFIG:Release>:-Gm->
            $<$<CONFIG:Debug>:-Od>
            $<$<CONFIG:Release>:-O2>
    
            -Zc:inline 
            -fp:fast 
            -errorReport:prompt 
            -WX 
            -Zc:forScope 
            -GR- 
            -Gd
            $<$<CONFIG:Release>:-arch:SSE2>
            -Oy-
            $<$<CONFIG:Debug>:-MDd>
            $<$<CONFIG:Release>:-Oi>
            $<$<CONFIG:Release>:-MT>
            -EHsc 
            -nologo
            -DEBUG  
            "-D \"_CRT_SECURE_NO_WARNINGS\"" 
    )

    set(common_library_flags "/ignore:4099,4221,4075")
    set(common_linker_flags "${common_library_flags} /SAFESEH:NO")

endif()

################################################################################
# Define any user options
################################################################################
option(generate_with_unit_tests "If set to 'ON', project will be generated with the unit tests included." OFF)

################################################################################
# Path for finding external CMake modules
################################################################################
set(CMAKE_MODULE_PATH "${Source_Root}/cmake/modules/")

################################################################################
# Includes
################################################################################
include(${cmake_include}/zero_source_group.cmake)
include(${cmake_include}/Zero_Mulitarget_Functions.cmake)
include(${cmake_include}/Zero_Custom_Commands.cmake)

################################################################################
# Separate Projects into folders for IDEs.
################################################################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################
# Call CMakeLists.txt in source folders, adding them to the project non-globally
################################################################################
# external libraries
add_subdirectory(External)

# zero source and related projects
add_subdirectory(Extensions)
add_subdirectory(Systems)
add_subdirectory(ZeroLibraries)
add_subdirectory(Projects)

if (generate_with_unit_tests)
    if (NOT EXISTS ${zero_unit_test_dir})
        message(WARNING "option 'generate_with_unit_tests' set but unit test directory: '${zero_unit_test_dir}' doesn't exist.\n Skipping unit tests.\n")
    else()
        message("\nAdding unit tests to project...\n")
        add_subdirectory(UnitTests)
    endif()
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ZeroEditor) 

