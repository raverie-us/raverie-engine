add_library(RendererEmpty)

welder_setup_library(RendererEmpty ${CMAKE_CURRENT_LIST_DIR} TRUE)

target_sources(RendererEmpty
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Renderer.cpp
)

welder_target_includes(RendererEmpty
  PUBLIC
    RendererBase
)
