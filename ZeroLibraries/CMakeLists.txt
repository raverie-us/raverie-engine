################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################
# get our parent directory
get_filename_component(ParentDirectory ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# get our current directory
set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

################################################################################
# define all of the libraries that are in this folder
################################################################################
add_library(AudioEngine "")
add_library(Common "") 
add_library(Dash "")
add_library(Geometry "")
add_library(Meta "")
#add_library(Posix "")
add_library(Serialization "")
add_library(SpatialPartition  "")
add_library(Support "")
add_library(Zilch "")

################################################################################
# Explicitly define the folders as they are not organized how we want
################################################################################
set_target_properties(AudioEngine      PROPERTIES FOLDER "ZeroLibraries")
set_target_properties(Common           PROPERTIES FOLDER "ZeroLibraries")
set_target_properties(Dash             PROPERTIES FOLDER "ZeroLibraries")
set_target_properties(Geometry         PROPERTIES FOLDER "ZeroLibraries")
set_target_properties(Meta             PROPERTIES FOLDER "ZeroLibraries")
set_target_properties(Serialization    PROPERTIES FOLDER "ZeroLibraries")
set_target_properties(SpatialPartition PROPERTIES FOLDER "ZeroLibraries")
set_target_properties(Support          PROPERTIES FOLDER "ZeroLibraries")
set_target_properties(Zilch            PROPERTIES FOLDER "ZeroLibraries")

################################################################################
# include filelists for each library
################################################################################
include(AudioEngine/CMakeLists.txt)
include(Common/CMakeLists.txt)
include(Dash/CMakeLists.txt)
include(Geometry/CMakeLists.txt)
include(Meta/CMakeLists.txt)
include(Serialization/CMakeLists.txt)
include(SpatialPartition/CMakeLists.txt)
include(Support/CMakeLists.txt)
include(Zilch/CMakeLists.txt)

# platform is going to define its own target
add_subdirectory(Platform)
################################################################################
# define include directories for all of our libraries
################################################################################
target_include_directories(AudioEngine
    PUBLIC
        ${CurrentDirectory}/AudioEngine 
        ${CurrentDirectory}
        ${ParentDirectory}
        ${OpusHeaders}
)
target_include_directories(Common
    PUBLIC 
        ${CurrentDirectory}/Common
        ${CurrentDirectory}
)
target_include_directories(Dash
    PUBLIC
        ${CurrentDirectory}/Dash 
        ${CurrentDirectory}
)
target_include_directories(Geometry
    PUBLIC
        ${CurrentDirectory}/Geometry 
        ${CurrentDirectory}
)
target_include_directories(Meta
    PUBLIC
        ${CurrentDirectory}/Meta 
        ${CurrentDirectory}
)
target_include_directories(Serialization
    PUBLIC
        ${CurrentDirectory}/Serialization 
        ${CurrentDirectory}
)
target_include_directories(SpatialPartition
    PUBLIC
        ${CurrentDirectory}/SpatialPartition 
        ${CurrentDirectory}
)
target_include_directories(Support
    PUBLIC
        ${CurrentDirectory}/Support
        ${ZLibHeaders}
        ${LibpngHeaders}
        ${CurrentDirectory}
)
target_include_directories(Zilch
    PUBLIC
        ${CurrentDirectory}/Zilch/Project
        ${CurrentDirectory}/Zilch/Project/Zilch
        ${VLD_INCLUDE_DIRS}
        ${CurrentDirectory}
)

################################################################################
# set the linked libraries for all of our targets
################################################################################
target_link_libraries(AudioEngine
                      PUBLIC
                      ${OpusStatic}
                      Common 
                      ${PlatformSoundLibrary}
) 
target_link_libraries(Dash
                      PUBLIC
                      Common
                      Zilch                      
                      Geometry
) 
target_link_libraries(Geometry
                      PUBLIC
                      Common
                      Zilch
) 
target_link_libraries(Zilch
                      PUBLIC
                      Common
                      ${VLD_LIBRARIES}
)
target_link_libraries(Meta
                      PUBLIC
                      Zilch
)
target_link_libraries(Serialization
                      PUBLIC
                      Common
                      Geometry
)
target_link_libraries(SpatialPartition
                      PUBLIC
                      Zilch
)
target_link_libraries(Support
                      PUBLIC
                      Common
                      ${ZLibStatic}
                      ${LibpngStatic}
)


################################################################################
# set the output directories for all of our targets
################################################################################
zero_multitarget_output_directories(
    AudioEngine
    Common
    Dash
    Geometry
    Meta
    Serialization
    SpatialPartition
    Support
    Zilch
    LIBRARY_DIRECTORY ${zero_library_dir}
    RUNTIME_DIRECTORY ${zero_binary_dir}
)

################################################################################
# Specify any additional target options such as pdb locations
################################################################################

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_output_settings(
        Common 
        Dash 
        Geometry
        Meta 
        #Posix 
        Platform
        Serialization
        SpatialPartition
        Support
        CONFIGS ${supported_configs}
        BASEPATH ${zero_build_out}
        PLATFORM ${platform}
        BITS ${bit}
        TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
        PRECOMPILED_HEADER_NAME "Precompiled.hpp"
        PRECOMPILED_SOURCE_NAME "Precompiled.cpp"
        TARGET_SUBFOLDER
    )

    zero_multitarget_output_settings(
        AudioEngine 
        CONFIGS ${supported_configs}
        BASEPATH ${zero_build_out}
        PLATFORM ${platform}
        BITS ${bit}
        TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
        PRECOMPILED_HEADER_NAME "Precompiled.h"
        PRECOMPILED_SOURCE_NAME "Precompiled.cpp"
        TARGET_SUBFOLDER
    )
    zero_multitarget_output_settings(
        Zilch 
        CONFIGS ${supported_configs}
        BASEPATH ${zero_build_out}
        PLATFORM ${platform}
        BITS ${bit}
        TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
        PRECOMPILED_HEADER_NAME "Zilch.hpp"
        PRECOMPILED_SOURCE_NAME "Zilch.cpp"
        TARGET_SUBFOLDER "Zilch/Project"
    )
endif()

################################################################################
# set flags and definitions
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_compile_options(  
        Common 
        Dash 
        Geometry
        Meta 
        Serialization
        SpatialPartition
        Support
        
        PRIVATE
         
        PUBLIC
            -GS -analyze-  -Zc:wchar_t
            
        PRIVATE
            -W3 -wd"4302"
            ${common_flags}
    )
    if (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014")
        zero_multitarget_compile_options(  
            AudioEngine 
            PRIVATE

            PUBLIC
                -GS -analyze-  -Zc:wchar_t

            PRIVATE
            $<$<CONFIG:Debug>:-GS>
            $<$<CONFIG:Release>:-GS->
            $<$<CONFIG:Release>:-GL>
            -analyze-
            -W3 
            -wd"4302"
            -Zc:wchar_t
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
            -DDEBUG  
            "-D \"_CRT_SECURE_NO_WARNINGS\"" 
            "-Wno-unused-command-line-argument"
            "-Wno-undefined-inline"
            -FIPrecompiled.h
        )
        zero_multitarget_compile_options(  
            Zilch 
            PRIVATE

            PUBLIC
                -GS -analyze-  -Zc:wchar_t

            PRIVATE
            $<$<CONFIG:Debug>:-GS>
            $<$<CONFIG:Release>:-GS->
            $<$<CONFIG:Release>:-GL>
            -analyze-
            -W3 
            -wd"4302"
            -Zc:wchar_t
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
            -DDEBUG  
            "-D \"_CRT_SECURE_NO_WARNINGS\"" 
            "-Wno-unused-command-line-argument"
            "-Wno-undefined-inline"
            -FIZilch.hpp
        )
    else()
        zero_multitarget_compile_options(  
            Zilch 
            AudioEngine
            PRIVATE
             
            PUBLIC
                -GS -analyze-  -Zc:wchar_t
                
            PRIVATE
                -W3 -wd"4302"
                ${common_flags}
        )
    endif()
else()
    zero_multitarget_compile_options( 
        AudioEngine
        Common
        Dash
        Geometry
        Meta
        Serialization
        SpatialPartition
        Support
        Zilch
        PRIVATE
        PUBLIC
        PRIVATE
        ${common_flags}
    )
endif()

    
################################################################################
# Set linker flags
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    # set the correct subsystems for executable targets, and set stack size for the editor
    set_target_properties(
        AudioEngine
        Common
        Dash
        Geometry
        Meta
        Serialization
        SpatialPartition
        Support
        Zilch 
        PROPERTIES 
        STATIC_LIBRARY_FLAGS "${common_library_flags}"
        STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
    )
endif()
################################################################################
# Group source into folders
################################################################################
zero_subfolder_source_group(${zero_core_path} ZeroLibraries AudioEngine "")
zero_subfolder_source_group(${zero_core_path} ZeroLibraries Common "")
zero_subfolder_source_group(${zero_core_path} ZeroLibraries Dash "")
zero_subfolder_source_group(${zero_core_path} ZeroLibraries Geometry "")
zero_subfolder_source_group(${zero_core_path} ZeroLibraries Meta "")
zero_subfolder_source_group(${zero_core_path} ZeroLibraries Serialization "")
zero_subfolder_source_group(${zero_core_path} ZeroLibraries SpatialPartition "")
zero_subfolder_source_group(${zero_core_path} ZeroLibraries Support "")
zero_subfolder_source_group(${zero_core_path} ZeroLibraries Zilch "")