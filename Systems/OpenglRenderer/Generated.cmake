################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################
target_sources(OpenglRenderer
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/OpenglRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OpenglRendererStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StreamedVertexBuffer.cpp
#  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/LoadingShader.inl
    ${CMAKE_CURRENT_LIST_DIR}/OpenglRenderer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/OpenglRendererStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/StreamedVertexBuffer.hpp
)
