################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################
# get our current directory
set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

################################################################################
# define all of the targets that are in this folder
################################################################################
add_executable(ZeroEditor "")
add_executable(ZeroLauncher "")
add_executable(BrowserSubProcess "")
add_library(ZeroLauncherSharedLibrary SHARED "")

################################################################################
# Explicitly define the folders as they are not organized how we want
################################################################################
set_target_properties(ZeroEditor       PROPERTIES FOLDER "Projects")
set_target_properties(ZeroLauncher     PROPERTIES FOLDER "Projects")
set_target_properties(ZeroLauncherSharedLibrary  PROPERTIES FOLDER "Projects")
set_target_properties(BrowserSubProcess  PROPERTIES FOLDER "Projects")

################################################################################
# include filelists for each library
################################################################################
include(ZeroEditor/CMakeLists.txt)
include(ZeroLauncher/ZeroLauncher/CMakeLists.txt)
include(ZeroLauncher/ZeroLauncherSharedLibrary/CMakeLists.txt)
include(BrowserSubProcess/CMakeLists.txt)

################################################################################
# define include directories for all of our libraries
################################################################################
target_include_directories(ZeroEditor 
    PUBLIC 
        ${Source_Root}
        ${CurrentDirectory}/ZeroEditor 
        ${CurrentDirectory}
)
target_include_directories(ZeroLauncher 
    PUBLIC 
        ${CurrentDirectory}
        ${CurrentDirectory}/ZeroLauncher/ZeroLauncher
        ${CurrentDirectory}/ZeroLauncher/ZeroLauncherSharedLibrary
)
target_include_directories(ZeroLauncherSharedLibrary 
    PUBLIC 
        ${CurrentDirectory}
        ${CurrentDirectory}/ZeroLauncher/ZeroLauncherSharedLibrary
        ${Dependencies_Root}
        ${Source_Root}
)
target_include_directories(BrowserSubProcess
    PUBLIC 
        ${CurrentDirectory}/BrowserSubProcess    
        ${CurrentDirectory}
)

################################################################################
# set the linked libraries for all of our targets
################################################################################
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
                      Dash 
                      AudioEngine 
                      UiWidget
                      Startup
                      Zilch
                      Graphics
                      ${OpusStatic}
                      ${FreetypeStatic}
                      ${ZLibStatic}
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
                      Dash 
                      Zilch 
                      AudioEngine 
                      UiWidget 
                      CodeTranslator
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
                      Dash 
                      Zilch 
                      AudioEngine 
                      UiWidget 
                      CodeTranslator
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
                      ${FreetypeStatic}
                      ${ZLibStatic}
                      ${LibpngStatic}
                      ${CEFStatic}
) 
################################################################################
# set the output directories for all of our targets
################################################################################
zero_multitarget_output_directories(
                      ZeroEditor
                      ZeroLauncher 
                      ZeroLauncherSharedLibrary
                      BrowserSubProcess
                      LIBRARY_DIRECTORY ${zero_library_dir}
                      RUNTIME_DIRECTORY ${zero_binary_dir}
)
# go through and seperate outputs to different folders

#ARCHIVE_OUTPUT_DIRECTORY
#LIBRARY_OUTPUT_DIRECTORY
#RUNTIME_OUTPUT_DIRECTORY

################################################################################
# Specify any additional target options such as pdb locations
################################################################################
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    zero_multitarget_output_settings(
        ZeroEditor
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
        ZeroLauncher 
        ZeroLauncherSharedLibrary
        CONFIGS ${supported_configs}
        BASEPATH "${zero_build_out}"
        PLATFORM ${platform}
        CONFIG ${configuration}
        BITS ${bit}
        TOOLSET ${CMAKE_VS_PLATFORM_TOOLSET}
        PRECOMPILED_HEADER_NAME "Precompiled.hpp"
        PRECOMPILED_SOURCE_NAME "Precompiled.cpp"
        TARGET_SUBFOLDER ZeroLauncher
    )

    zero_multitarget_output_settings(
        BrowserSubProcess
        CONFIGS ${supported_configs}
        BASEPATH ${zero_build_out}
        PLATFORM ${platform}
        CONFIG ${configuration}
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
        ZeroEditor
        ZeroLauncher
        PRIVATE
         
        PUBLIC
        "-D \"_UNICODE\"" 
        "-D \"UNICODE\""
        "-D \"CURL_STATICLIB\" "
        "-D \"WIN32\" "
        "-D \"_CRT_SECURE_NO_WARNINGS\" "
        "-D \"ZeroExportDll\" "
        PRIVATE
        ${common_flags}

    )

    target_compile_options(
        ZeroLauncherSharedLibrary
        PRIVATE
         
        PUBLIC
        PRIVATE
        ${common_flags}
        "-D \"_UNICODE\"" 
        "-D \"UNICODE\""
        "-D \"CURL_STATICLIB\"" 
        "-D \"WIN32\"" 
        "-D \"_WINDOWS\"" 
        "-D \"_USRDLL\"" 
        "-D \"ZeroLauncherSharedLibrary_EXPORTS\"" 
        "-D \"_WINDLL\"" 
        "-D \"_CRT_SECURE_NO_WARNINGS\"" 
        "-D \"ZeroExportDll\"" 
    )

    # have to set flags for BrowserSubProcess seperatly because it has very different settings
    zero_multitarget_compile_options(
        BrowserSubProcess
        PRIVATE
        PUBLIC
        PRIVATE
        $<$<CONFIG:Debug>:-GS>
        $<$<CONFIG:Release>:-MP>
        $<$<CONFIG:Release>:-GS->
        $<$<CONFIG:Release>:-GL>

        -analyze-
        -W3 
        -wd"4302"
        -wd"4305"
        -Zc:wchar_t

        $<$<CONFIG:Debug>:-Zi>
        $<$<CONFIG:Release>:-Zi>
        $<$<CONFIG:Debug>:-Gm>
        $<$<CONFIG:Release>:-Gm->
        $<$<CONFIG:Debug>:-Od>
        $<$<CONFIG:Release>:-O2>

        -Zc:inline 
        -fp:precise 
        -errorReport:prompt 
        -Zc:forScope 
        -GR- 
        -Gd
        $<$<CONFIG:Release>:-arch:SSE2>
        -Oy-
        $<$<CONFIG:Debug>:-MDd>
        $<$<CONFIG:Release>:-Oi>
        $<$<CONFIG:Release>:-MT>
        -EHsc 
        $<$<CONFIG:Release>:-Gy>

        -nologo

        $<$<CONFIG:Release>:-"D \"NDEBUG\"">
        $<$<CONFIG:Release>:-"D \"VLD\"">
    )
    
################################################################################
# Set linker flags
################################################################################

    # set the correct subsystems for executable targets, and set stack size for the editor
    set_target_properties(
        ZeroEditor 
        PROPERTIES 
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /STACK:8388608 /DEBUG ${common_linker_flags}"
    )

    set_target_properties(
        ZeroLauncher 
        ZeroEditor 
        ZeroLauncherSharedLibrary 
        PROPERTIES 
        LINK_FLAGS_RELEASE "/LTCG"
    )
    
    set_target_properties(
        ZeroLauncher 
        ZeroLauncherSharedLibrary 
        BrowserSubProcess
        PROPERTIES 
        LINK_FLAGS "/SUBSYSTEM:WINDOWS /DEBUG ${common_linker_flags}"
    )

else()
    zero_multitarget_compile_options(
        ZeroEditor
        ZeroLauncher
        BrowserSubProcess
        PRIVATE
        PUBLIC
        PRIVATE
        ${common_flags}
    )
    set_target_properties(
        ZeroEditor
        ZeroLauncher
        BrowserSubProcess
        PROPERTIES
        LINK_FLAGS "${common_linker_flags}"
    )
endif()

if (${platform} STREQUAL "Emscripten")
    set_target_properties(
        ZeroEditor
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS "--preload-file \"${zero_build_out}/ZeroEditorFileSystem.zip\"@/FileSystem.zip"
    )
    set_target_properties(
        ZeroLauncher
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS "--preload-file \"${zero_build_out}/ZeroLauncherFileSystem.zip\"@/FileSystem.zip"
    )
endif()

################################################################################
# Group source into folders
################################################################################
zero_subfolder_source_group_ignore_target_folder(${zero_core_path} Projects ZeroEditor "${zero_core_path}/Projects")
zero_subfolder_source_group_ignore_target_folder(${zero_core_path} Projects ZeroLauncher "${zero_core_path}/Projects")
zero_subfolder_source_group(${zero_core_path} Projects/ZeroLauncher ZeroLauncherSharedLibrary "${zero_core_path}/Projects")

zero_subfolder_source_group(${zero_core_path} Projects BrowserSubProcess "")



################################################################################
# Post build steps
################################################################################
#### ZeroEditor editor
copy_multiple_shared_objects(
    ZeroEditor
    ZeroLauncher
    OUTPUT_DIRECTORY ${zero_binary_dir}
    SHARED_OBJECT_LOCATIONS
    ## setup all dll locations
    ${AssimpSharedPath}
    ${CEFSharedPath}
    ${FreetypeSharedPath}
    ${GLEWSharedPath}
    ${MemoryDebuggerSharedPath}
    ${MemoryTrackerSharedPath}
    ${NvttSharedPath}
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
        "Tools/SpriteVersion.txt"
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
        "Tools/SpriteVersion.txt"
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
    ${CurrentDirectory}/ZeroLauncher/ZeroLauncherSharedLibrary
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
    ${CurrentDirectory}/ZeroLauncher/ZeroLauncher
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
