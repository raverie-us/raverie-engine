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
add_library(Common "") 
add_library(Content "")
add_library(Engine "")
add_library(Geometry "")
add_library(Graphics "")
add_library(Meta "")
add_library(Networking "")
add_library(Physics "")
add_library(Replication "")
add_library(Serialization "")
add_library(Sound "")
add_library(SpatialPartition  "")
add_library(Startup "")
add_library(Support "")
add_library(Zilch "")
add_library(ZilchScript "")

################################################################################
# include filelists for each library
################################################################################
include(Common/CMakeLists.txt)
include(Content/CMakeLists.txt)
include(Engine/CMakeLists.txt)
include(Geometry/CMakeLists.txt)
include(Graphics/CMakeLists.txt)
include(Meta/CMakeLists.txt)
include(Networking/CMakeLists.txt)
include(Physics/CMakeLists.txt)
include(Replication/CMakeLists.txt)
include(Serialization/CMakeLists.txt)
include(Sound/CMakeLists.txt)
include(SpatialPartition/CMakeLists.txt)
include(Startup/CMakeLists.txt)
include(Support/CMakeLists.txt)
include(Zilch/CMakeLists.txt)
include(ZilchScript/CMakeLists.txt)

# platform is going to define its own target
add_subdirectory(Platform)
################################################################################
# define include directories for all of our libraries
################################################################################

target_include_directories(Common PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Common)
target_include_directories(Content PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Content)
target_include_directories(Engine PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Engine)
target_include_directories(Geometry PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Geometry)
target_include_directories(Graphics PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Graphics)
target_include_directories(Meta PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Meta)
target_include_directories(Networking PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Networking)
target_include_directories(Physics PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Physics)
target_include_directories(Replication PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Replication)
target_include_directories(Serialization PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Serialization)
target_include_directories(Sound PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Sound)
target_include_directories(SpatialPartition PUBLIC ${CurrentDirectory} ${CurrentDirectory}/SpatialPartition)
target_include_directories(Startup PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Startup)
target_include_directories(Support PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Support)
target_include_directories(Zilch PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Zilch)
target_include_directories(ZilchScript PUBLIC ${CurrentDirectory} ${CurrentDirectory}/ZilchScript)

################################################################################
# set the linked libraries for all of our targets
################################################################################
target_link_libraries(Replication
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
target_link_libraries(Content
                      PUBLIC
                      Common
                      Zilch
                      ${AssimpStatic}
                      ${NvttStatic}
                      ${OpusStatic}
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
                      ${OpusStatic}
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
    Common
    Content
    Engine
    Geometry
    Graphics
    Meta
    Networking
    Physics
    Replication
    Serialization
    Sound
    SpatialPartition
    Startup
    Support
    Zilch
    ZilchScript
    LIBRARY_DIRECTORY ${zero_library_dir}
    RUNTIME_DIRECTORY ${zero_binary_dir}
)

################################################################################
# Specify any additional target options such as pdb locations
################################################################################
zero_multitarget_output_settings(
    Common
    Content
    Engine
    Geometry
    Graphics
    Meta
    Networking
    Physics
    Replication
    Serialization
    Sound
    SpatialPartition
    Startup
    Support
    Zilch
    ZilchScript
    CONFIGS ${supported_configs}
    BASEPATH ${zero_build_out}
    PLATFORM ${platform}
    CONFIG ${configuration}
    BITS ${bit}
    TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
    PRECOMPILED_HEADER_NAME "Precompiled.hpp"
    PRECOMPILED_SOURCE_NAME "Precompiled.cpp"
    TARGET_SUBFOLDER
)

################################################################################
# set flags and definitions
################################################################################
zero_multitarget_compile_options(
    Common
    Content
    Engine
    Geometry
    Graphics
    Meta
    Networking
    Physics
    Replication
    Serialization
    Sound
    SpatialPartition
    Startup
    Support
    Zilch
    ZilchScript
    PRIVATE
    ${common_flags}
)

################################################################################
# Set linker flags
################################################################################
# set the correct subsystems for executable targets, and set stack size for the editor
set_target_properties(
    Common
    Content
    Engine
    Geometry
    Graphics
    Meta
    Networking
    Physics
    Replication
    Serialization
    Sound
    SpatialPartition
    Startup
    Support
    Zilch
    ZilchScript
    PROPERTIES
    LINK_FLAGS "${common_linker_flags}"
    STATIC_LIBRARY_FLAGS "${common_library_flags}"
    STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
)

################################################################################
# Group source into folders
################################################################################
zero_subfolder_source_group(${zero_core_path} Libraries Common "")
zero_subfolder_source_group(${zero_core_path} Libraries Content "")
zero_subfolder_source_group(${zero_core_path} Libraries Engine "")
zero_subfolder_source_group(${zero_core_path} Libraries Geometry "")
zero_subfolder_source_group(${zero_core_path} Libraries Graphics "")
zero_subfolder_source_group(${zero_core_path} Libraries Meta "")
zero_subfolder_source_group(${zero_core_path} Libraries Networking "")
zero_subfolder_source_group(${zero_core_path} Libraries Physics "")
zero_subfolder_source_group(${zero_core_path} Libraries Replication "")
zero_subfolder_source_group(${zero_core_path} Libraries Serialization "")
zero_subfolder_source_group(${zero_core_path} Libraries Sound "")
zero_subfolder_source_group(${zero_core_path} Libraries SpatialPartition "")
zero_subfolder_source_group(${zero_core_path} Libraries Startup "")
zero_subfolder_source_group(${zero_core_path} Libraries Support "")
zero_subfolder_source_group(${zero_core_path} Libraries Zilch "")
zero_subfolder_source_group(${zero_core_path} Libraries ZilchScript "")

################################################################################
# Post build steps
################################################################################
add_custom_command(
    TARGET Engine PRE_BUILD    
    COMMAND ${CMAKE_COMMAND} 
    # options
    -DZERO_CORE_ROOT="${zero_core_path}"
    # command
    -P ${cmake_utilities_dir}/Update_Build_Version_Script.cmake
)
