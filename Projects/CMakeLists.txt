################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################
# get our current directory
set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

################################################################################
# define all of the targets that are in this folder
################################################################################
add_executable(GeometryProcessor "")
add_executable(ImageProcessor "")
add_executable(ZeroEditor "")
add_executable(ZeroLauncher "")
add_executable(BrowserSubProcess "")
add_library(ZeroLauncherSharedLibrary SHARED "")

################################################################################
# Explicitly define the folders as they are not organized how we want
################################################################################
set_target_properties(GeometryProcessor PROPERTIES FOLDER "Projects")
set_target_properties(ImageProcessor    PROPERTIES FOLDER "Projects")
set_target_properties(ZeroEditor       PROPERTIES FOLDER "Projects")
set_target_properties(ZeroLauncher     PROPERTIES FOLDER "Projects")
set_target_properties(ZeroLauncherSharedLibrary  PROPERTIES FOLDER "Projects")
set_target_properties(BrowserSubProcess  PROPERTIES FOLDER "Projects")

################################################################################
# include filelists for each library
################################################################################
include(GeometryProcessor/CMakeLists.txt)
include(ImageProcessor/CMakeLists.txt)
include(ZeroEditor/CMakeLists.txt)
include(ZeroLauncher/ZeroLauncher/CMakeLists.txt)
include(ZeroLauncher/ZeroLauncherSharedLibrary/CMakeLists.txt)
include(BrowserSubProcess/CMakeLists.txt)

################################################################################
# define include directories for all of our libraries
################################################################################
target_include_directories(GeometryProcessor 
    PUBLIC 
        ${Dependencies_Root}
        ${Source_Root}
        ${PortAudioHeaders}
        ${AssimpHeaders}
)
target_include_directories(ImageProcessor 
    PUBLIC 
        ${Dependencies_Root}
        ${Source_Root}
        ${PortAudioHeaders}
        ${OpusHeaders}
        ${NvttHeaders}
)
target_include_directories(ZeroEditor 
    PUBLIC 
        ${Source_Root}
        ${CurlHeaders}
        ${PortAudioHeaders}
        ${OpusHeaders}
        ${FreetypeHeaders}
        ${WinHidHeaders}
        ${CurrentDirectory}/ZeroEditor 
        ${CurrentDirectory}
)
target_include_directories(ZeroLauncher 
    PUBLIC 
        ${CurrentDirectory}
        ${CurrentDirectory}/ZeroLauncher/ZeroLauncher
        ${CurrentDirectory}/ZeroLauncher/ZeroLauncherSharedLibrary
        ${ZLibHeaders}
        ${CurlHeaders}
)
target_include_directories(ZeroLauncherSharedLibrary 
    PUBLIC 
        ${CurrentDirectory}
        ${CurrentDirectory}/ZeroLauncher/ZeroLauncherSharedLibrary
        ${Dependencies_Root}
        ${Source_Root}
        ${CurlHeaders}
)
target_include_directories(BrowserSubProcess
    PUBLIC 
        ${CurrentDirectory}/BrowserSubProcess    
        ${CurrentDirectory}
        ${CEFHeaders}
)

################################################################################
# set the linked libraries for all of our targets
################################################################################
target_link_libraries(GeometryProcessor
                      PUBLIC
                      Common
                      SpatialPartition
                      Serialization
                      Support
                      Math
                      Geometry
                      Meta
                      Engine
                      Windows
                      Content
                      Zilch
                      AudioEngine
                      ${AssimpStatic}
                      ${PortAudioStatic}
                      ${FreetypeStatic}
) 
target_link_libraries(ImageProcessor
                      PUBLIC
                      Common
                      SpatialPartition 
                      Serialization 
                      Support 
                      Math
                      Geometry
                      Meta
                      Engine
                      Windows
                      Content
                      Zilch
                      AudioEngine
                      ${PortAudioStatic}
                      ${OpusStatic}
                      ${NvttStatic}
                      ${FreetypeStatic}
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
                      Math 
                      Geometry 
                      Widget 
                      Editor 
                      ZilchScript 
                      Networking 
                      Meta 
                      Physics 
                      Sound 
                      Windows 
                      Content 
                      Dash 
                      AudioEngine 
                      UiWidget
                      Startup
                      Zilch
                      Graphics
                      ${CurlStatic}
                      ${PortAudioStatic}
                      ${OpusStatic}
                      ${FreetypeStatic}
                      ${ZLibStatic}
)

# Zero is going to run the processor tools on startup so they have to build first
add_dependencies(ZeroEditor
    ImageProcessor
    GeometryProcessor
)

target_link_libraries(ZeroLauncher
                      PUBLIC
                      Common
                      Gameplay
                      SpatialPartition 
                      ZilchShaders
                      Serialization 
                      Support 
                      Math 
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
                      Windows 
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
                      Math 
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
                      Windows 
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
# launcher needs the dll to run
add_dependencies(ZeroLauncher
    ZeroLauncherSharedLibrary
)
target_link_libraries(BrowserSubProcess
                      PUBLIC
                      ${CEFStatic}
) 
################################################################################
# set the output directories for all of our targets
################################################################################
zero_multitarget_output_directories(GeometryProcessor
                      ImageProcessor
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
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    zero_multitarget_output_settings(
        GeometryProcessor
        ImageProcessor
        ZeroEditor
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
        ZeroLauncher 
        ZeroLauncherSharedLibrary
        CONFIGS ${supported_configs}
        BASEPATH ${zero_build_out}
        PLATFORM ${platform}
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

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)

    zero_multitarget_compile_options(
        ImageProcessor
        GeometryProcessor
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

        $<$<CONFIG:Debug>:-ZI>
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
    
    set_target_properties(
        ImageProcessor 
        GeometryProcessor 
        PROPERTIES 
        LINK_FLAGS "/SUBSYSTEM:CONSOLE /DEBUG ${common_linker_flags}"
        LINK_FLAGS_RELEASE "/LTCG"
    )
    

endif()

################################################################################
# Group source into folders
################################################################################
zero_subfolder_source_group(${zero_core_path} Projects GeometryProcessor "")
zero_subfolder_source_group(${zero_core_path} Projects ImageProcessor "")
zero_subfolder_source_group(${zero_core_path} Projects ZeroEditor "${zero_core_path}/Projects")
zero_subfolder_source_group(${zero_core_path} Projects/ZeroLauncher ZeroLauncher "${zero_core_path}/Projects")
zero_subfolder_source_group(${zero_core_path} Projects/ZeroLauncher ZeroLauncherSharedLibrary "${zero_core_path}/Projects")
zero_subfolder_source_group(${zero_core_path} Projects BrowserSubProcess "")



################################################################################
# Post build step to put all the dlls next to the executable
################################################################################
#### ZeroEditor editor
copy_multiple_dlls(
    ZeroEditor
    ZeroLauncher
    OUTPUT_DIRECTORY ${zero_binary_dir}
    DLL_LOCATIONS
    ## setup all dll locations
    ${AssimpSharedPath}
    ${CEFSharedPath}
    ${FreetypeSharedPath}
    ${GLEWSharedPath}
    ${MemoryDebuggerSharedPath}
    ${MemoryTrackerSharedPath}
    ${NvttSharedPath}
    ${PortAudioSharedPath}
)

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

#### Geometry Processor
processor_post_build(
    GeometryProcessor
    ZERO_CORE_DIR ${zero_core_path}
    PROCESSOR_OUTPUT_LOCATION ${zero_binary_dir}
    CONFIGURATION ${supported_configs}
    DLL_LOCATIONS ${AssimpSharedPath}
)
####

#### Image Processor
processor_post_build(
    ImageProcessor
    ZERO_CORE_DIR ${zero_core_path}
    PROCESSOR_OUTPUT_LOCATION ${zero_binary_dir}
    CONFIGURATION ${supported_configs}
    DLL_LOCATIONS ${NvttSharedPath}
)
####

#### ZeroLauncherSharedLibrary
# launcher_post_build(aTarget aZeroCoreDirectory aProjectDirectory aBuildOutputDirectory)
launcher_post_build(
    ZeroLauncherSharedLibrary 
    ${zero_core_path}
    ${CurrentDirectory}/ZeroLauncher/ZeroLauncherSharedLibrary
    ${zero_binary_dir}
)

#copy_cef_bin_post_build(aTarget aZeroCoreDirectory aBuildOutputDirectory)
copy_cef_bin_post_build(
    BrowserSubProcess
    ${zero_core_path}
    ${zero_binary_dir}
)
####

#### ZeroLauncher
create_build_info(
    ZeroLauncher
    ${CurrentDirectory}/ZeroLauncher/ZeroLauncher
    ${zero_core_path}
    ${zero_binary_dir}
    ${zero_binary_dir}
)

copy_cef_bin_post_build(
    ZeroLauncher
    ${zero_core_path}
    ${zero_binary_dir}
)
####
