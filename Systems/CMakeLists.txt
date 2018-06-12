################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################
# get our current directory
set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

################################################################################
# define all of the libraries that are in this folder
################################################################################
add_library(Content "")
add_library(Engine "")
add_library(Graphics "")
add_library(Networking "")
add_library(Physics "")
add_library(Sound "")
add_library(Startup "")
add_library(ZilchScript "")

################################################################################
# Explicitly define the folders as they are not organized how we want
################################################################################
set_target_properties(Content        PROPERTIES FOLDER "Systems")
set_target_properties(Engine         PROPERTIES FOLDER "Systems")
set_target_properties(Graphics       PROPERTIES FOLDER "Systems")
set_target_properties(Networking     PROPERTIES FOLDER "Systems")
set_target_properties(Physics        PROPERTIES FOLDER "Systems")
set_target_properties(Sound          PROPERTIES FOLDER "Systems")
set_target_properties(Startup        PROPERTIES FOLDER "Systems")
set_target_properties(ZilchScript    PROPERTIES FOLDER "Systems")


################################################################################
# include filelists for each library
################################################################################
include(Content/CMakeLists.txt)
include(Engine/CMakeLists.txt)
include(Graphics/CMakeLists.txt)
include(Networking/CMakeLists.txt)
include(Physics/CMakeLists.txt)
include(Sound/CMakeLists.txt)
include(Startup/CMakeLists.txt)
include(ZilchScript/CMakeLists.txt)

################################################################################
# define include directories for all of our libraries
################################################################################
target_include_directories(Content 
    PUBLIC 
        ${CurrentDirectory}/Content
        ${CurrentDirectory}
        ${AssimpHeaders}
        ${NvttHeaders}

)
target_include_directories(Engine 
    PUBLIC 
        ${CurrentDirectory}/Engine
        ${CurrentDirectory}
)
target_include_directories(Graphics
    PUBLIC 
        ${CurrentDirectory}
        ${FreetypeHeaders}
)
target_include_directories(Networking 
    PUBLIC 
        ${CurrentDirectory}/Networking
        ${CurrentDirectory}
        ${CurlHeaders}
)
target_include_directories(Physics 
    PUBLIC 
        ${CurrentDirectory}
)
target_include_directories(Sound 
    PUBLIC 
        ${CurrentDirectory}
)
target_include_directories(Startup 
    PUBLIC 
        ${CurrentDirectory}
)
target_include_directories(ZilchScript 
    PUBLIC 
        ${CurrentDirectory}
)
################################################################################
# set the linked libraries for all of our targets
################################################################################
target_link_libraries(Content
                      PUBLIC
                      Common
                      Zilch
                      ${AssimpStatic}
                      ${NvttStatic}
)
target_link_libraries(Engine
                      PUBLIC
                      Common
                      Geometry
                      Zilch
)  
target_link_libraries(Graphics
                      PUBLIC
                      Common
                      Geometry
                      ZilchShaders
                      ${FreetypeStatic}
) 
target_link_libraries(Networking
                      PUBLIC
                      ${CurlStatic}
                      Common
                      Geometry
) 
target_link_libraries(Physics
                      PUBLIC
                      Zilch
) 
target_link_libraries(Sound
                      PUBLIC
                      Common
                      Geometry
) 
target_link_libraries(Startup
                      PUBLIC
                      Common
                      Geometry
                      Graphics
) 
target_link_libraries(ZilchScript
                      PUBLIC
                      Common
                      Geometry
                      Editor
) 
################################################################################
# set the output directories for all of our targets
################################################################################
zero_multitarget_output_directories(
    Content
    Engine
    Graphics
    Networking
    Physics
    Sound
    Startup
    ZilchScript
    LIBRARY_DIRECTORY ${zero_library_dir}
    RUNTIME_DIRECTORY ${zero_binary_dir}
)
################################################################################
# Specify any additional target options such as pdb locations
################################################################################

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_output_settings(
        Content
        Engine
        Graphics
        Physics
        Sound
        Startup
        ZilchScript
        Networking
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
        Content
        Engine
        Graphics
        Networking
        Physics
        Sound
        Startup
        ZilchScript

        PUBLIC
        -wd"4305"

        PRIVATE
        ${common_flags}
        "-D \"CURL_STATICLIB\"" 
        "-D \"WIN32\"" 
        "-D \"_CRT_SECURE_NO_WARNINGS\"" 
        "-D \"ZeroExportDll\"" 
        
    )

    zero_multitarget_compile_options( 
        PUBLIC
        PRIVATE
        "-D \"_UNICODE\"" 
        "-D \"UNICODE\""
    )
    #common_library_flags
    set_target_properties(
        Content
        Engine
        Graphics
        Networking
        Physics
        Sound
        Startup
        PROPERTIES
        STATIC_LIBRARY_FLAGS ${common_library_flags}
        STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
    )
else()
    zero_multitarget_compile_options(
        Content
        Engine
        Graphics
        Networking
        Physics
        Sound
        Startup
        ZilchScript
        PRIVATE
        PUBLIC
        PRIVATE
        ${common_flags}
    )
    set_target_properties(
        Content
        Engine
        Graphics
        Networking
        Physics
        Sound
        Startup
        ZilchScript
        PROPERTIES
        LINK_FLAGS "${common_linker_flags}"
    )
endif()

################################################################################
# Group source into folders
################################################################################
zero_subfolder_source_group(${zero_core_path} Systems Content "")
zero_subfolder_source_group(${zero_core_path} Systems Engine "")
zero_subfolder_source_group(${zero_core_path} Systems Graphics "")
zero_subfolder_source_group(${zero_core_path} Systems Networking "")
zero_subfolder_source_group(${zero_core_path} Systems Physics "")
zero_subfolder_source_group(${zero_core_path} Systems Sound "")
zero_subfolder_source_group(${zero_core_path} Systems Startup "")
zero_subfolder_source_group(${zero_core_path} Systems ZilchScript "")
