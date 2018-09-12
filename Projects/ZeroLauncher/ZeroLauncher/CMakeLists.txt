################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################
include(${CMAKE_CURRENT_LIST_DIR}/Generated.cmake)

set(platformSpecificSource)

# get the rc file so we get icons on windows builds
if (${platform} STREQUAL "Windows")
    set(platformSpecificSource
    ${zero_platform_data}/ZeroLauncher/ZeroLauncher.rc
    ${zero_platform_data}/ZeroLauncher/Resource.h
    )
endif()

target_sources(ZeroLauncher
    PRIVATE
    ${platformSpecificSource}
)
