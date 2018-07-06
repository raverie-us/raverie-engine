################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################
include(${CMAKE_CURRENT_LIST_DIR}/Generated.cmake)

set(manualAdditions "")

if ("${CMAKE_GENERATOR}" MATCHES "Visual Studio*")
    set(manualAdditions "${CMAKE_CURRENT_LIST_DIR}/ZeroVisualizers.natvis")
endif()

target_sources(Common
PRIVATE
    ${manualAdditions}
)
