add_library(RendererBase)

raverie_setup_library(RendererBase ${CMAKE_CURRENT_LIST_DIR} TRUE)

target_sources(RendererBase
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Renderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Renderer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RendererEnumerations.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RendererBaseStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RendererBaseStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

raverie_target_includes(RendererBase
  PUBLIC
    Common
)
