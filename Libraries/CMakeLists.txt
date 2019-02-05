# Libraries.
add_subdirectory(CodeTranslator)
add_subdirectory(Common)
add_subdirectory(Content)
add_subdirectory(Editor)
add_subdirectory(Engine)
add_subdirectory(Gameplay)
add_subdirectory(Geometry)
add_subdirectory(Graphics)
add_subdirectory(Meta)
add_subdirectory(Networking)
add_subdirectory(Physics)
add_subdirectory(Replication)
add_subdirectory(Serialization)
add_subdirectory(Sound)
add_subdirectory(SpatialPartition)
add_subdirectory(Startup)
add_subdirectory(Support)
add_subdirectory(UiWidget)
add_subdirectory(Widget)
add_subdirectory(Zilch)
add_subdirectory(ZilchScript)
add_subdirectory(ZilchShaders)

# Executables.
add_subdirectory(BrowserSubProcess)
add_subdirectory(WelderEditor)
add_subdirectory(WelderLauncher)
add_subdirectory(WelderLauncherShell)

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
