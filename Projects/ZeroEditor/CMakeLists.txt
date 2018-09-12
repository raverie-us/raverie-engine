################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################
include(${CMAKE_CURRENT_LIST_DIR}/Generated.cmake)

set(platformSpecificSource)

# get the rc file so we get icons on windows builds
if (${platform} STREQUAL "Windows")
    set(platformSpecificSource
        ${zero_platform_data}/ZeroEditor/ZeroEditor.rc
        ${zero_platform_data}/ZeroEditor/Resource.h
    )
endif()

target_sources(ZeroEditor
    PRIVATE
    ${platformSpecificSource}
)
