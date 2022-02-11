set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_BUILD_DOCS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)

add_subdirectory(Repo)

if(WIN32)
	#set_property(TARGET "assimp" PROPERTY FOLDER "External/Assimp")
	#set_property(TARGET "uninstall" PROPERTY FOLDER "External/Assimp")
	#set_property(TARGET "UpdateAssimpLibsDebugSymbolsAndDLLs" PROPERTY FOLDER "External/Assimp")
endif()