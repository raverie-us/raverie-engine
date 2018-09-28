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
add_executable(ZilchShadersTests "")

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
set_target_properties(ZilchShadersTests  PROPERTIES FOLDER "UnitTests")

# Zilch Shader unit tests need the working directory to be set to where the unit
# tests are so it can write output files (to be committed) in the same directory.
set_target_properties(ZilchShadersTests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${zero_core_path}/UnitTests/ZilchShaders/")

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
include(ZilchShadersTests/CMakeLists.txt)

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

target_include_directories(ZilchShadersTests 
    PUBLIC 
        ${CurrentDirectory}/ZilchShadersTests
        ${CurrentDirectory}
)

################################################################################
# set the linked libraries for all of our targets
################################################################################
target_link_libraries(ContainersTests
                      PUBLIC
                      Common
                      CppUnitLite2
                      Platform
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
                      CppUnitLite2
                      Platform
)

target_link_libraries(IntersectionTests
                      PUBLIC
                      Common
                      Serialization
                      Geometry
                      Meta
                      CppUnitLite2
                      Platform
                      Zilch
)

target_link_libraries(LcpSolvers
                      PUBLIC
                      Common
                      CppUnitLite2
                      Platform
)

target_link_libraries(MathTests
                      PUBLIC
                      Common
                      Platform
                      CppUnitLite2
)

target_link_libraries(SerializationTests
                      PUBLIC
                      Common
                      Serialization
                      Support
                      Meta
                      Platform
                      CppUnitLite2
                      Zilch
)

target_link_libraries(SSETests
                      PUBLIC
                      Common
                      Platform
                      CppUnitLite2
)

target_link_libraries(ZilchShadersTests
                      PUBLIC
                      Common
                      ZilchShaders
                      SPIRV-Cross
                      SPIRV-Tools
                      SPIRV-Tools-opt
                      Serialization
                      Support
                      Geometry
                      Meta
                      Platform
                      Zilch
                      ${FreetypeStatic}
)
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
    ZilchShadersTests
    LIBRARY_DIRECTORY ${unit_test_library_dir}
    RUNTIME_DIRECTORY ${unit_test_binary_dir}
)

################################################################################
# Specify any additional target options such as pdb locations
################################################################################

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_output_settings(
        ContainersTests
        CppUnitLite2
        FilePathTests
        IntersectionTests
        LcpSolvers
        MathTests
        SerializationTests
        SSETests
        ZilchShadersTests
        CONFIGS ${supported_configs}
        BASEPATH ${unit_tests_out}
        PLATFORM ${platform}
        BITS ${bit}
        TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
        PRECOMPILED_HEADER_NAME ""
        PRECOMPILED_SOURCE_NAME ""
        TARGET_SUBFOLDER
    )
endif()

################################################################################
# set flags and definitions
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))

    zero_multitarget_compile_options( 
        ContainersTests
        CppUnitLite2
        FilePathTests
        IntersectionTests
        LcpSolvers
        MathTests
        SerializationTests
        SSETests
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
zero_subfolder_source_group(${zero_core_path} UnitTests ZilchShadersTests  "")
