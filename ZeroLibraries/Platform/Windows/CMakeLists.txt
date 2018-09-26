################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################
include(${CMAKE_CURRENT_LIST_DIR}/Generated.cmake)

target_sources(Platform
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../Curl/WebRequest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/MainLoop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../OpenGL/OpenglRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../OpenGL/OpenglRenderer.hpp
)
