################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################
# get our current directory
set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

set(unit_tests_out ${zero_unit_test_dir}/BuildOutput)
set(unit_tests_library_dir ${unit_tests_out}/Out/${supported_configs})
set(unit_tests_binary_dir ${unit_tests_out}/Out/${supported_configs})
#set(unit_tests_intermediates_dir ${unit_tests_out}/Int/${supported_configs})

################################################################################
# Define any user options specific to this folder
################################################################################
option(generate_with_zilch_shader_tests "If set to 'ON', project will be generated with the zilch shader unit tests included." OFF)

################################################################################
# define all of the libraries that are in this folder
################################################################################
add_executable(ContainersTests "")
add_library(CppUnitLite2 "")
add_executable(FilePathTests "")
add_executable(IntersectionTests "")
add_executable(LcpSolvers "")
add_executable(MathTests "")
add_executable(SerializationTests "")
add_executable(SSETests "")
if(${generate_with_zilch_shader_tests})
    add_executable(ZilchshadersTests "")
endif()

################################################################################
# Explicitly define the folders
################################################################################
set_target_properties(ContainersTests    PROPERTIES FOLDER "UnitTests")
set_target_properties(CppUnitLite2       PROPERTIES FOLDER "UnitTests")
set_target_properties(FilePathTests      PROPERTIES FOLDER "UnitTests") 
set_target_properties(IntersectionTests  PROPERTIES FOLDER "UnitTests") 
set_target_properties(LcpSolvers         PROPERTIES FOLDER "UnitTests") 
set_target_properties(MathTests          PROPERTIES FOLDER "UnitTests")
set_target_properties(SerializationTests PROPERTIES FOLDER "UnitTests") 
set_target_properties(SSETests           PROPERTIES FOLDER "UnitTests")

if(${generate_with_zilch_shader_tests})
    set_target_properties(ZilchShadersTests  PROPERTIES FOLDER "UnitTests")
endif()

################################################################################
# include filelists for each library
################################################################################
include(ContainersTests/CMakeLists.txt)
include(CppUnitLite2/CMakeLists.txt)
include(FilePathTests/CMakeLists.txt)
include(IntersectionTests/CMakeLists.txt)
include(LcpSolvers/CMakeLists.txt)
include(MathTests/CMakeLists.txt)
include(SerializationTests/CMakeLists.txt)
include(SSETests/CMakeLists.txt)
if(${generate_with_zilch_shader_tests})
    include(ZilchShadersTests/CMakeLists.txt)
endif()

################################################################################
# define include directories for all of our libraries
################################################################################
target_include_directories(ContainersTests 
    PUBLIC 
        ${CurrentDirectory}/ContainersTests
        ${CurrentDirectory}
)
target_include_directories(CppUnitLite2 
    PUBLIC 
        ${CurrentDirectory}/CppUnitLite2
        ${CurrentDirectory}
        ${FreetypeHeaders}
)
target_include_directories(FilePathTests 
    PUBLIC 
        ${CurrentDirectory}/FilePathTests
        ${CurrentDirectory}
)
target_include_directories(IntersectionTests 
    PUBLIC 
        ${CurrentDirectory}/IntersectionTests
        ${CurrentDirectory}
)
target_include_directories(LcpSolvers 
    PUBLIC 
        ${CurrentDirectory}/LcpSolvers
        ${CurrentDirectory}
)
target_include_directories(MathTests 
    PUBLIC 
        ${CurrentDirectory}/MathTests
        ${CurrentDirectory}
)
target_include_directories(SerializationTests 
    PUBLIC 
        ${CurrentDirectory}/SerializationTests
        ${CurrentDirectory}
)
target_include_directories(SSETests 
    PUBLIC 
        ${CurrentDirectory}/SSETests
        ${CurrentDirectory}
)

if(${generate_with_zilch_shader_tests})
    target_include_directories(ZilchShadersTests 
        PUBLIC 
            ${CurrentDirectory}/ZilchShadersTests
            ${CurrentDirectory}
    )
endif()

################################################################################
# set the linked libraries for all of our targets
################################################################################
target_link_libraries(ContainersTests
                      PUBLIC
                      Common
                      Math
                      Platform
                      CppUnitLite2
                      Windows
)

target_link_libraries(CppUnitLite2
                      PUBLIC
                      ${FreetypeStatic}
)
 
target_link_libraries(FilePathTests
                      PUBLIC
                      Common
                      Serialization
                      Support
                      Math
                      Platform
                      CppUnitLite2
                      Windows
)

target_link_libraries(IntersectionTests
                      PUBLIC
                      Common
                      Serialization
                      Math
                      Geometry
                      Meta
                      Platform
                      CppUnitLite2
                      Windows
                      Zilch
)

target_link_libraries(LcpSolvers
                      PUBLIC
                      Common
                      Math
                      Platform
                      CppUnitLite2
                      Windows
)

target_link_libraries(MathTests
                      PUBLIC
                      Common
                      Math
                      Platform
                      CppUnitLite2
                      Windows
)

target_link_libraries(SerializationTests
                      PUBLIC
                      Common
                      Serialization
                      Support
                      Math
                      Meta
                      Platform
                      CppUnitLite2
                      Windows
                      Zilch
)

target_link_libraries(SSETests
                      PUBLIC
                      Common
                      Math
                      Platform
                      CppUnitLite2
                      Windows
)

if(${generate_with_zilch_shader_tests})
    target_link_libraries(ZilchShadersTests
                          PUBLIC
                          Common
                          ZilchShaders
                          Serialization
                          Support
                          Math
                          Geometry
                          Meta
                          Platform
                          Windows
                          Zilch
    )
endif()
################################################################################
# set the output directories for all of our targets
################################################################################
zero_multitarget_output_directories(
    ContainersTests
    CppUnitLite2
    FilePathTests
    IntersectionTests
    LcpSolvers
    MathTests
    SerializationTests
    SSETests
    LIBRARY_DIRECTORY ${unit_test_library_dir}
    RUNTIME_DIRECTORY ${unit_test_binary_dir}
)

if(${generate_with_zilch_shader_tests})
    zero_multitarget_output_directories(
        ZilchShadersTests
        LIBRARY_DIRECTORY ${unit_test_library_dir}
        RUNTIME_DIRECTORY ${unit_test_binary_dir}
    )
endif()
################################################################################
# Specify any additional target options such as pdb locations
################################################################################

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    zero_multitarget_output_settings(
        ContainersTests
        CppUnitLite2
        FilePathTests
        IntersectionTests
        LcpSolvers
        MathTests
        SerializationTests
        SSETests
        CONFIGS ${supported_configs}
        BASEPATH ${unit_tests_out}
        PLATFORM ${platform}
        BITS ${bit}
        TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
        PRECOMPILED_HEADER_NAME ""
        PRECOMPILED_SOURCE_NAME ""
        TARGET_SUBFOLDER
    )

    if(${generate_with_zilch_shader_tests})
        zero_multitarget_output_settings(
            ZilchShadersTests
            CONFIGS ${supported_configs}
            BASEPATH ${unit_tests_out}
            PLATFORM ${platform}
            BITS ${bit}
            TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
            PRECOMPILED_HEADER_NAME "Precompiled.hpp"
            PRECOMPILED_SOURCE_NAME "Precompiled.cpp"
            TARGET_SUBFOLDER
        )
    endif()
endif()

################################################################################
# set flags and definitions
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)

    zero_multitarget_compile_options( 
        ContainersTests
        CppUnitLite2
        FilePathTests
        IntersectionTests
        LcpSolvers
        MathTests
        SerializationTests
        SSETests

        PUBLIC
        -wd"4305"

        PRIVATE
        ${common_flags}
        "-D \"CURL_STATICLIB\"" 
        "-D \"WIN32\"" 
        "-D \"_CRT_SECURE_NO_WARNINGS\"" 
        "-D \"ZeroExportDll\"" 
        -W3
    )

    if(${generate_with_zilch_shader_tests})
        zero_multitarget_compile_options( 
            ZilchShadersTests

            PUBLIC
            -wd"4305"

            PRIVATE
            ${common_flags}
            "-D \"CURL_STATICLIB\"" 
            "-D \"WIN32\"" 
            "-D \"_CRT_SECURE_NO_WARNINGS\"" 
            "-D \"ZeroExportDll\"" 
            -W3
        )
    endif()
    
endif()
################################################################################
# Group source into folders
################################################################################
zero_subfolder_source_group(${zero_core_path} UnitTests ContainersTests  "")
zero_subfolder_source_group(${zero_core_path} UnitTests CppUnitLite2  "")
zero_subfolder_source_group(${zero_core_path} UnitTests FilePathTests  "")
zero_subfolder_source_group(${zero_core_path} UnitTests IntersectionTests  "")
zero_subfolder_source_group(${zero_core_path} UnitTests LcpSolvers  "")
zero_subfolder_source_group(${zero_core_path} UnitTests MathTests  "")
zero_subfolder_source_group(${zero_core_path} UnitTests SerializationTests  "")
zero_subfolder_source_group(${zero_core_path} UnitTests SSETests  "")

if(${generate_with_zilch_shader_tests})
    zero_subfolder_source_group(${zero_core_path} UnitTests ZilchShadersTests  "")
endif()
