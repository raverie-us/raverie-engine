add_subdirectory(ZLib)
add_subdirectory(Libpng)
add_subdirectory(FreeType)
add_subdirectory(Nvtt)
add_subdirectory(Assimp)
add_subdirectory(Opus)
add_subdirectory(Scintilla)
add_subdirectory(SpirvHeaders)
add_subdirectory(SpirvTools)
add_subdirectory(SpirvCross)
add_subdirectory(Glew)

add_subdirectory(Common)
add_subdirectory(Support)
add_subdirectory(Zilch)
add_subdirectory(Serialization)
add_subdirectory(Geometry)
add_subdirectory(SpatialPartition)
add_subdirectory(Meta)
add_subdirectory(Engine)
add_subdirectory(Content)
add_subdirectory(ZilchShaders)
add_subdirectory(Graphics)
add_subdirectory(Replication)
add_subdirectory(Networking)
add_subdirectory(Physics)
add_subdirectory(Sound)
add_subdirectory(UiWidget)
add_subdirectory(Widget)
add_subdirectory(Editor)
add_subdirectory(CodeTranslator)
add_subdirectory(ZilchScript)
add_subdirectory(Gameplay)
add_subdirectory(Startup)

add_subdirectory(BrowserSubProcess)
add_subdirectory(WelderEditor)
add_subdirectory(WelderLauncherShell)
add_subdirectory(WelderLauncher)

#if (${platform} STREQUAL "Emscripten")
#    set_target_properties(
#        ZeroEditor
#        PROPERTIES SUFFIX ".html"
#        LINK_FLAGS "--embed-file \"${zero_build_out}/ZeroEditorFileSystem.zip\"@/FileSystem.zip"
#    )
#    set_target_properties(
#        ZeroLauncher
#        PROPERTIES SUFFIX ".html"
#        LINK_FLAGS "--embed-file \"${zero_build_out}/ZeroLauncherFileSystem.zip\"@/FileSystem.zip"
#    )
#endif()

#################################################################################
## Post build steps
#################################################################################
#add_custom_command(
#    TARGET Engine PRE_BUILD
#    COMMAND ${CMAKE_COMMAND}
#    # options
#    -DZERO_CORE_ROOT="${zero_core_path}"
#    # command
#    -P ${cmake_utilities_dir}/Update_Build_Version_Script.cmake
#)
#
##### ZeroEditor editor
#copy_multiple_shared_objects(
#    ZeroEditor
#    ZeroLauncher
#    OUTPUT_DIRECTORY ${zero_binary_dir}
#    SHARED_OBJECT_LOCATIONS
#    ## setup all dll locations
#    ${CEFSharedPath}
#    ${PortAudioSharedPath}
#    ${SDLSharedPath}
#)
#
#if("${platform}" STREQUAL "Emscripten")
#    message("should be calling zip directory")
#    multitarget_zip_directory(
#        ZeroEditor 
#        FOLDERS_TO_ZIP
#        "Resources/Loading"
#        "Resources/FragmentCore"
#        "Resources/ZeroCore"
#        "Resources/Editor"
#        "Resources/EditorUi"
#        "Resources/UiWidget"
#        "PrebuiltZeroContent"
#        "Data/ColorSchemes"
#        "Data/Fallback"
#        "Data/Licenses"
#        "Data/ZilchFragmentSettings"
#        "Data/*.*"
#        OUTPUT_FILE
#        "BuildOutput/ZeroEditorFileSystem.zip"
#    )
#    multitarget_zip_directory(
#        ZeroLauncher 
#        FOLDERS_TO_ZIP
#        "Resources/Loading"
#        "Resources/FragmentCore"
#        "Resources/ZeroCore"
#        "Resources/ZeroLauncherResources"
#        "PrebuiltZeroContent"
#        "Data/ColorSchemes"
#        "Data/Fallback"
#        "Data/Licenses"
#        "Data/ZilchFragmentSettings"
#        "Data/*.*"
#        OUTPUT_FILE
#        "BuildOutput/ZeroLauncherFileSystem.zip"
#    )
#endif()
#
#
##function(create_build_info aTarget aSourceDir aBuildOutDir aOutputLocation)
#create_build_info(
#    ZeroEditor
#    ${CMAKE_CURRENT_LIST_DIR}/ZeroEditor
#    ${zero_core_path}
#    ${zero_binary_dir}
#    ${zero_binary_dir}
#)
#
##function(CreatePluginDirectory aTarget aZeroCoreDir aBuildOutputDirectory aOS aplatformShortName)
#editor_post_build_step(
#    ZeroEditor
#    ${zero_core_path}
#    ${zero_library_dir}
#    ${zero_binary_dir}
#    ${platform}
#    ${bit}
#)
#
#####
#
##### ZeroLauncherSharedLibrary
## launcher_post_build(aTarget aZeroCoreDirectory aProjectDirectory aBuildOutputDirectory)
#launcher_shared_post_build(
#    ZeroLauncherSharedLibrary 
#    ${zero_core_path}
#    ${CMAKE_CURRENT_LIST_DIR}/ZeroLauncherSharedLibrary
#    ${zero_binary_dir}
#)
#
##### BrowserSubProcess
#add_custom_command(
#    TARGET BrowserSubProcess POST_BUILD
#    # executes "cmake -E copy_directory
#    COMMAND ${CMAKE_COMMAND} -E copy_directory  
#    # input folder
#    ${zero_binary_dir}/BrowserSubProcess                                  
#    #output folder
#    ${zero_binary_dir}/ZeroEditor
#)
#
#####
#
##### ZeroLauncher
#launcher_post_build(
#    ZeroLauncher 
#    ${zero_core_path}
#    ${CMAKE_CURRENT_LIST_DIR}
#    ${zero_binary_dir}
#)
#
#
##### Copy the CEF files for every project that needs them
## first check if we need CEF for this platform, then copy cef files over
#if (${CEF_exists})
#    zero_multitarget_copy_folders_to_target_output_directories(
#        ZeroEditor
#        ZeroLauncher
#        BrowserSubProcess
#        OUTPUT_DIRECTORY ${zero_binary_dir}
#        FOLDERS_TO_COPY 
#        ${aZeroCoreDirectory}/External/CEF/bin
#    )
#endif()
#####
#