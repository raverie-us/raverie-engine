add_library(RendererEmpty)

raverie_setup_library(RendererEmpty ${CMAKE_CURRENT_LIST_DIR} TRUE)

target_sources(RendererEmpty
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Renderer.cpp
)

raverie_target_includes(RendererEmpty
  PUBLIC
    RendererBase
)
