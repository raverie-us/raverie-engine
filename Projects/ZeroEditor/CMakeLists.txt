################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################
include(${CMAKE_CURRENT_LIST_DIR}/Generated.cmake)

set(platformSpecificSource)

# get the rc file so we get icons on windows builds
if (${platform} STREQUAL "Windows")
    set(platformSpecificSource
        ${CMAKE_CURRENT_LIST_DIR}/../Win32Shared/ZeroEditor.rc
    )
endif()

target_sources(ZeroEditor
    PRIVATE
    #Win32Shared 
    #${CurrentDirectory}/Win32Shared/Importer.cpp 
    #${CurrentDirectory}/Win32Shared/Importer.hpp 
    ${CurrentDirectory}/Win32Shared/resource.h 
    ${platformSpecificSource}
)
