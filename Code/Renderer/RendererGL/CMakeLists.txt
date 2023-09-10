add_library(RendererGL)

raverie_setup_library(RendererGL ${CMAKE_CURRENT_LIST_DIR} TRUE)

set(RendererPlatformSource 
)

target_sources(RendererGL
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/OpenglRenderer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/OpenglRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RendererGLStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RendererGLStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

raverie_target_includes(RendererGL
  PUBLIC
    RendererBase
)
