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
add_library(CodeTranslator "")
add_library(Editor "")
add_library(Gameplay "")
add_library(UiWidget "")
add_library(Widget "")
add_library(ZilchShaders "")

################################################################################
# Explicitly define the folders as they are not organized how we want
################################################################################
set_target_properties(CodeTranslator PROPERTIES FOLDER "Extensions")
set_target_properties(Editor         PROPERTIES FOLDER "Extensions")
set_target_properties(Gameplay       PROPERTIES FOLDER "Extensions")
set_target_properties(UiWidget       PROPERTIES FOLDER "Extensions")
set_target_properties(Widget         PROPERTIES FOLDER "Extensions")
set_target_properties(ZilchShaders   PROPERTIES FOLDER "Extensions")

################################################################################
# include filelists for each library
################################################################################
include(CodeTranslator/CMakeLists.txt)
include(Editor/CMakeLists.txt)
include(Gameplay/CMakeLists.txt)
include(UiWidget/CMakeLists.txt)
include(Widget/CMakeLists.txt)
include(ZilchShaders/CMakeLists.txt)

################################################################################
# define include directories for all of our libraries
################################################################################
target_include_directories(CodeTranslator 
    PUBLIC 
        ${CurrentDirectory}

)
target_include_directories(Editor 
    PUBLIC 
        ${CurrentDirectory}/../ZeroLibraries/Zilch/Project    
        ${CurrentDirectory}/Widget
        ${CurrentDirectory}/Editor
        ${CurrentDirectory}/../Systems
        ${CurrentDirectory}/../ZeroLibraries
        ${CurrentDirectory}
)
target_include_directories(Gameplay
    PUBLIC 
        ${CurrentDirectory}
        ${CEFHeaders}

)

target_include_directories(UiWidget 
    PUBLIC 
        ${CurrentDirectory}

)
target_include_directories(Widget
    PUBLIC 
        ${CurrentDirectory}

)
target_include_directories(ZilchShaders
    PUBLIC 
        ${CurrentDirectory}
)

################################################################################
# set the linked libraries for all of our targets
################################################################################
target_link_libraries(CodeTranslator
                      PUBLIC
                      Engine
)
target_link_libraries(Editor
                      PUBLIC
                      Common
) 
target_link_libraries(ZilchShaders
                      PUBLIC
                      Common
                      Geometry
                      Zilch
) 
target_link_libraries(Widget
                      PUBLIC
                      Engine
) 
target_link_libraries(UiWidget
                      PUBLIC
                      Engine
) 
target_link_libraries(Gameplay
                      PUBLIC
                      Common
                      Zilch
                      Engine
                      ${CEFStatic}
)

################################################################################
# set the output directories for all of our targets
################################################################################
zero_multitarget_output_directories(CodeTranslator
                      Editor
                      Gameplay 
                      UiWidget 
                      Widget
                      ZilchShaders
                      LIBRARY_DIRECTORY ${zero_library_dir}
                      RUNTIME_DIRECTORY ${zero_binary_dir}
)

################################################################################
# Specify any additional target options such as pdb locations
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_output_settings(
        CodeTranslator
        Editor
        Gameplay 
        UiWidget 
        Widget
        ZilchShaders
        CONFIGS ${supported_configs}
        BASEPATH ${zero_build_out}
        PLATFORM ${platform}
        BITS ${bit}
        TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
        PRECOMPILED_HEADER_NAME "Precompiled.hpp"
        PRECOMPILED_SOURCE_NAME "Precompiled.cpp"
        TARGET_SUBFOLDER
    )
endif()

################################################################################
# set flags and definitions
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_compile_options(  
        CodeTranslator
        Editor
        Gameplay
        UiWidget
        Widget
        ZilchShaders
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
            ${common_flags}
    )
endif()

################################################################################
# Set linker flags
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    # set the correct subsystems for executable targets, and set stack size for the editor
    set_target_properties(
        CodeTranslator
        Editor
        Gameplay
        UiWidget
        Widget
        ZilchShaders
        PROPERTIES 
        STATIC_LIBRARY_FLAGS "${common_library_flags}"
        STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
    )
endif()

################################################################################
# Group source into folders
################################################################################
zero_subfolder_source_group(${zero_core_path} Extensions CodeTranslator "")
zero_subfolder_source_group(${zero_core_path} Extensions Editor "")
zero_subfolder_source_group(${zero_core_path} Extensions Gameplay "")
zero_subfolder_source_group(${zero_core_path} Extensions UiWidget "")
zero_subfolder_source_group(${zero_core_path} Extensions Widget "")
zero_subfolder_source_group(${zero_core_path} Extensions ZilchShaders "")