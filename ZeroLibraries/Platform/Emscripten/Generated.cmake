################################################################################
# Author: Dane Curbow
# Copyright 2018, DigiPen Institute of Technology
################################################################################
target_sources(Platform
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Browser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CallStack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MainLoop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/WebRequest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
)

add_definitions(-DZeroPlatformNoShellOpenFile)