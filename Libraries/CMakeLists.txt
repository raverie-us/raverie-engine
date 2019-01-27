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
add_library(Common "")
add_library(Content "")
add_library(Editor "")
add_library(Engine "")
add_library(Gameplay "")
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
add_library(UiWidget "")
add_library(Widget "")
add_library(Zilch "")
add_library(ZilchScript "")
add_library(ZilchShaders "")

add_library(Scintilla "")
add_library(Libpng "")
add_library(Assimp "")
add_library(ZLib "")
add_library(FreeType "")
add_library(Opus "")
add_library(Glew "")
add_library(Nvtt "")
add_library(SpirvHeaders "")
add_library(SpirvTools "")
add_library(SpirvCross "")

add_executable(ZeroEditor "")
add_executable(ZeroLauncher "")
add_executable(BrowserSubProcess "")
add_library(ZeroLauncherSharedLibrary SHARED "")

################################################################################
# include filelists for each library
################################################################################
include(CodeTranslator/CMakeLists.txt)
include(Common/CMakeLists.txt)
include(Content/CMakeLists.txt)
include(Editor/CMakeLists.txt)
include(Engine/CMakeLists.txt)
include(Gameplay/CMakeLists.txt)
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
include(UiWidget/CMakeLists.txt)
include(Widget/CMakeLists.txt)
include(Zilch/CMakeLists.txt)
include(ZilchScript/CMakeLists.txt)
include(ZilchShaders/CMakeLists.txt)

include(Scintilla/CMakeLists.txt)
include(Libpng/CMakeLists.txt)
include(Assimp/CMakeLists.txt)
include(ZLib/CMakeLists.txt)
include(FreeType/CMakeLists.txt)
include(Opus/CMakeLists.txt)
include(Glew/CMakeLists.txt)
include(Nvtt/CMakeLists.txt)
include(SpirvHeaders/CMakeLists.txt)
include(SpirvTools/CMakeLists.txt)
include(SpirvCross/CMakeLists.txt)

include(ZeroEditor/CMakeLists.txt)
include(ZeroLauncher/CMakeLists.txt)
include(ZeroLauncherSharedLibrary/CMakeLists.txt)
include(BrowserSubProcess/CMakeLists.txt)

# platform is going to define its own target
add_subdirectory(Platform)
################################################################################
# define include directories for all of our libraries
################################################################################

target_include_directories(CodeTranslator PUBLIC ${CurrentDirectory} ${CurrentDirectory}/CodeTranslator)
target_include_directories(Common PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Common)
target_include_directories(Content PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Content)
target_include_directories(Editor PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Editor)
target_include_directories(Engine PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Engine)
target_include_directories(Gameplay PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Gameplay)
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
target_include_directories(UiWidget PUBLIC ${CurrentDirectory} ${CurrentDirectory}/UiWidget)
target_include_directories(Widget PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Widget)
target_include_directories(Zilch PUBLIC ${CurrentDirectory} ${CurrentDirectory}/Zilch)
target_include_directories(ZilchScript PUBLIC ${CurrentDirectory} ${CurrentDirectory}/ZilchScript)
target_include_directories(ZilchShaders PUBLIC ${CurrentDirectory} ${CurrentDirectory}/ZilchShaders)

target_include_directories(Scintilla
  PUBLIC
    ${CurrentDirectory}/Scintilla/
    ${CurrentDirectory}/Scintilla/Repo/
    ${CurrentDirectory}/Scintilla/Repo/lexlib/
    ${CurrentDirectory}/Scintilla/Repo/include/)

target_include_directories(Libpng
  PUBLIC
    ${CurrentDirectory}/Libpng/
    ${CurrentDirectory}/Libpng/Repo/)

target_include_directories(Assimp
  PUBLIC
    ${CurrentDirectory}/Assimp/
    ${CurrentDirectory}/Assimp/Repo/
    ${CurrentDirectory}/Assimp/Repo/include/
    ${CurrentDirectory}/Assimp/Repo/contrib/rapidjson/include/
    ${CurrentDirectory}/Assimp/Repo/contrib/openddlparser/include/)

target_include_directories(ZLib
  PUBLIC
    ${CurrentDirectory}/ZLib/
    ${CurrentDirectory}/ZLib/Repo/)

target_include_directories(FreeType
  PUBLIC
    ${CurrentDirectory}/FreeType/
    ${CurrentDirectory}/FreeType/Repo/
    ${CurrentDirectory}/FreeType/Repo/include/
    ${CurrentDirectory}/FreeType/Repo/include/freetype/)

target_include_directories(Opus
  PUBLIC
    ${CurrentDirectory}/Opus/
    ${CurrentDirectory}/Opus/Repo/
    ${CurrentDirectory}/Opus/Repo/include/
    ${CurrentDirectory}/Opus/Repo/silk/
    ${CurrentDirectory}/Opus/Repo/silk/float/
    ${CurrentDirectory}/Opus/Repo/celt/)

target_include_directories(Nvtt
  PUBLIC
    ${CurrentDirectory}/Nvtt/
    ${CurrentDirectory}/Nvtt/Repo/src/
    ${CurrentDirectory}/Nvtt/Repo/extern/poshlib/)

target_include_directories(Glew
  PUBLIC
    ${CurrentDirectory}/Glew/
    ${CurrentDirectory}/Glew/Repo/
    ${CurrentDirectory}/Glew/Repo/include/)

target_include_directories(SpirvHeaders
  PUBLIC
    ${CurrentDirectory}/SpirvHeaders/
    ${CurrentDirectory}/SpirvHeaders/Repo/
    ${CurrentDirectory}/SpirvHeaders/Repo/include/)

target_include_directories(SpirvTools
  PUBLIC
    ${CurrentDirectory}/SpirvTools/
    ${CurrentDirectory}/SpirvTools/Build/
    ${CurrentDirectory}/SpirvTools/Repo/
    ${CurrentDirectory}/SpirvTools/Repo/source/
    ${CurrentDirectory}/SpirvTools/Repo/include/)

target_include_directories(SpirvCross
  PUBLIC
    ${CurrentDirectory}/SpirvCross/
    ${CurrentDirectory}/SpirvCross/Repo/
    ${CurrentDirectory}/SpirvCross/Repo/include/)

target_include_directories(ZeroEditor PUBLIC ${CurrentDirectory} ${CurrentDirectory}/ZeroEditor)
target_include_directories(ZeroLauncher PUBLIC ${CurrentDirectory} ${CurrentDirectory}/ZeroLauncher)
target_include_directories(ZeroLauncherSharedLibrary PUBLIC ${CurrentDirectory} ${CurrentDirectory}/ZeroLauncherSharedLibrary)
target_include_directories(BrowserSubProcess PUBLIC ${CurrentDirectory} ${CurrentDirectory}/BrowserSubProcess)

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
target_link_libraries(ZLib
)
target_link_libraries(FreeType
                      PUBLIC
                      ZLib
                      Libpng
)
target_link_libraries(Libpng
                      PUBLIC
                      ZLib
)
target_link_libraries(Assimp
                      PUBLIC
                      ZLib
)
target_link_libraries(Support
                      PUBLIC
                      Common
                      Libpng
                      ZLib
)
target_link_libraries(Content
                      PUBLIC
                      Common
                      Zilch
                      Assimp
                      ZLib
                      Opus
                      Nvtt
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
                      FreeType
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
                      Opus
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
target_link_libraries(CodeTranslator
                      PUBLIC
                      Engine
                      ZilchShaders
)
target_link_libraries(Editor
                      PUBLIC
                      Scintilla
                      Common
                      ZilchShaders
)
target_link_libraries(ZilchShaders
                      PUBLIC
                      Common
                      Geometry
                      Zilch
                      SpirvHeaders
                      SpirvTools
                      SpirvCross
)
target_link_libraries(SpirvTools
                      PUBLIC
                      SpirvHeaders
)
target_link_libraries(SpirvCross
                      PUBLIC
                      SpirvHeaders
)
target_link_libraries(Widget
                      PUBLIC
                      Engine
                      ZilchShaders
)
target_link_libraries(UiWidget
                      PUBLIC
                      Engine
                      ZilchShaders
)
target_link_libraries(Gameplay
                      PUBLIC
                      Common
                      Zilch
                      Engine
                      ZilchShaders
                      ${CEFStatic}
)

target_link_libraries(ZeroEditor
                      PUBLIC
                      Common
                      Gameplay
                      SpatialPartition
                      CodeTranslator
                      ZilchShaders
                      Serialization
                      Support
                      Engine 
                      Geometry 
                      Widget 
                      Editor 
                      ZilchScript 
                      Networking 
                      Meta 
                      Physics 
                      Sound 
                      Platform 
                      Content 
                      Replication 
                      UiWidget
                      Startup
                      Zilch
                      Graphics
                      
                      Scintilla
                      Libpng
                      Assimp
                      ZLib
                      FreeType
                      Opus
                      Nvtt
                      SpirvHeaders
                      SpirvTools
                      SpirvCross
)

target_link_libraries(ZeroLauncher
                      PUBLIC
                      Common
                      Gameplay
                      SpatialPartition 
                      ZilchShaders
                      Serialization 
                      Support 
                      Geometry 
                      Widget 
                      Editor 
                      ZilchScript 
                      Graphics 
                      Networking 
                      Meta 
                      Engine 
                      Physics 
                      Sound 
                      Startup 
                      Platform 
                      Content 
                      Replication 
                      Zilch 
                      UiWidget 
                      CodeTranslator
                      
                      Scintilla
                      Libpng
                      Assimp
                      ZLib
                      FreeType
                      Opus
                      Nvtt
                      SpirvHeaders
                      SpirvTools
                      SpirvCross
) 
target_link_libraries(ZeroLauncherSharedLibrary
                      PUBLIC
                      Common
                      Gameplay
                      SpatialPartition 
                      ZilchShaders
                      Serialization 
                      Support 
                      Geometry 
                      Widget 
                      Editor 
                      ZilchScript 
                      Graphics 
                      Networking 
                      Meta 
                      Engine 
                      Physics 
                      Sound 
                      Startup 
                      Platform 
                      Content 
                      Replication 
                      Zilch 
                      UiWidget 
                      CodeTranslator
                      
                      Scintilla
                      Libpng
                      Assimp
                      ZLib
                      FreeType
                      Opus
                      Nvtt
                      SpirvHeaders
                      SpirvTools
                      SpirvCross
) 

# the launcher is going to need the chrome subprocess to run correctly
add_dependencies(ZeroLauncherSharedLibrary
    BrowserSubProcess
)
add_dependencies(ZeroEditor
    BrowserSubProcess
)
# launcher needs the dll to run
add_dependencies(ZeroLauncher
    ZeroLauncherSharedLibrary
)
target_link_libraries(BrowserSubProcess
                      PUBLIC
                      Common
                      Platform
                      
                      Scintilla
                      Libpng
                      Assimp
                      ZLib
                      FreeType
                      Opus
                      Nvtt
                      SpirvHeaders
                      SpirvTools
                      SpirvCross
                      ${CEFStatic}
)

################################################################################
# Specify any additional target options such as pdb locations
################################################################################
zero_multitarget_output_settings(
    CodeTranslator
    Common
    Content
    Editor
    Engine
    Gameplay
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
    UiWidget
    Widget
    Zilch
    ZilchScript
    ZilchShaders
    
    ZeroEditor
    ZeroLauncher 
    ZeroLauncherSharedLibrary
    BrowserSubProcess
    
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

zero_multitarget_output_settings(
    Scintilla
    Libpng
    Assimp
    ZLib
    FreeType
    Opus
    Glew
    Nvtt
    SpirvHeaders
    SpirvTools
    SpirvCross
    
    CONFIGS ${supported_configs}
    BASEPATH ${zero_build_out}
    PLATFORM ${platform}
    CONFIG ${configuration}
    BITS ${bit}
    TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
    TARGET_SUBFOLDER
)

################################################################################
# set flags and definitions
################################################################################
zero_multitarget_compile_options(
    CodeTranslator
    Common
    Content
    Editor
    Engine
    Gameplay
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
    UiWidget
    Widget
    Zilch
    ZilchScript
    ZilchShaders
    
    Scintilla
    Libpng
    Assimp
    ZLib
    FreeType
    Opus
    Nvtt
    SpirvHeaders
    SpirvTools
    SpirvCross
    
    ZeroEditor
    ZeroLauncher 
    ZeroLauncherSharedLibrary
    BrowserSubProcess
    
    PRIVATE
    ${common_flags}
)

################################################################################
# Set linker flags
################################################################################
# set the correct subsystems for executable targets, and set stack size for the editor
set_target_properties(
    CodeTranslator
    Common
    Content
    Editor
    Engine
    Gameplay
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
    UiWidget
    Widget
    Zilch
    ZilchScript
    ZilchShaders
    
    Scintilla
    Libpng
    Assimp
    ZLib
    FreeType
    Opus
    Nvtt
    SpirvHeaders
    SpirvTools
    SpirvCross
    
    ZeroEditor
    ZeroLauncher 
    ZeroLauncherSharedLibrary
    BrowserSubProcess
    
    PROPERTIES
    LINK_FLAGS "${common_linker_flags}"
    STATIC_LIBRARY_FLAGS "${common_library_flags}"
    STATIC_LIBRARY_FLAGS_RELEASE "/LTCG"
)

if (${platform} STREQUAL "Emscripten")
    set_target_properties(
        ZeroEditor
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS "--embed-file \"${zero_build_out}/ZeroEditorFileSystem.zip\"@/FileSystem.zip"
    )
    set_target_properties(
        ZeroLauncher
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS "--embed-file \"${zero_build_out}/ZeroLauncherFileSystem.zip\"@/FileSystem.zip"
    )
endif()

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

#### ZeroEditor editor
copy_multiple_shared_objects(
    ZeroEditor
    ZeroLauncher
    OUTPUT_DIRECTORY ${zero_binary_dir}
    SHARED_OBJECT_LOCATIONS
    ## setup all dll locations
    ${CEFSharedPath}
    ${PortAudioSharedPath}
    ${SDLSharedPath}
)

if("${platform}" STREQUAL "Emscripten")
    message("should be calling zip directory")
    multitarget_zip_directory(
        ZeroEditor 
        FOLDERS_TO_ZIP
        "Resources/Loading"
        "Resources/FragmentCore"
        "Resources/ZeroCore"
        "Resources/Editor"
        "Resources/EditorUi"
        "Resources/UiWidget"
        "PrebuiltZeroContent"
        "Data/ColorSchemes"
        "Data/Fallback"
        "Data/Licenses"
        "Data/ZilchFragmentSettings"
        "Data/*.*"
        OUTPUT_FILE
        "BuildOutput/ZeroEditorFileSystem.zip"
    )
    multitarget_zip_directory(
        ZeroLauncher 
        FOLDERS_TO_ZIP
        "Resources/Loading"
        "Resources/FragmentCore"
        "Resources/ZeroCore"
        "Resources/ZeroLauncherResources"
        "PrebuiltZeroContent"
        "Data/ColorSchemes"
        "Data/Fallback"
        "Data/Licenses"
        "Data/ZilchFragmentSettings"
        "Data/*.*"
        OUTPUT_FILE
        "BuildOutput/ZeroLauncherFileSystem.zip"
    )
endif()


#function(create_build_info aTarget aSourceDir aBuildOutDir aOutputLocation)
create_build_info(
    ZeroEditor
    ${CurrentDirectory}/ZeroEditor
    ${zero_core_path}
    ${zero_binary_dir}
    ${zero_binary_dir}
)

#function(CreatePluginDirectory aTarget aZeroCoreDir aBuildOutputDirectory aOS aplatformShortName)
editor_post_build_step(
    ZeroEditor
    ${zero_core_path}
    ${zero_library_dir}
    ${zero_binary_dir}
    ${platform}
    ${bit}
)

####

#### ZeroLauncherSharedLibrary
# launcher_post_build(aTarget aZeroCoreDirectory aProjectDirectory aBuildOutputDirectory)
launcher_shared_post_build(
    ZeroLauncherSharedLibrary 
    ${zero_core_path}
    ${CurrentDirectory}/ZeroLauncherSharedLibrary
    ${zero_binary_dir}
)

#### BrowserSubProcess
add_custom_command(
    TARGET BrowserSubProcess POST_BUILD
    # executes "cmake -E copy_directory
    COMMAND ${CMAKE_COMMAND} -E copy_directory  
    # input folder
    ${zero_binary_dir}/BrowserSubProcess                                  
    #output folder
    ${zero_binary_dir}/ZeroEditor
)

####

#### ZeroLauncher
launcher_post_build(
    ZeroLauncher 
    ${zero_core_path}
    ${CurrentDirectory}
    ${zero_binary_dir}
)


#### Copy the CEF files for every project that needs them
# first check if we need CEF for this platform, then copy cef files over
if (${CEF_exists})
    zero_multitarget_copy_folders_to_target_output_directories(
        ZeroEditor
        ZeroLauncher
        BrowserSubProcess
        OUTPUT_DIRECTORY ${zero_binary_dir}
        FOLDERS_TO_COPY 
        ${aZeroCoreDirectory}/External/CEF/bin
    )
endif()
####
