################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################
target_sources(ZeroEditor
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Exporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Startup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/WinMain.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZeroCrashCallbacks.cpp
#  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ZeroCrashCallbacks.hpp
)
