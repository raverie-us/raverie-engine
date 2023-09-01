add_library(RendererGL)

welder_setup_library(RendererGL ${CMAKE_CURRENT_LIST_DIR} TRUE)

set(RendererPlatformSource 
${CMAKE_CURRENT_LIST_DIR}/SDLSTDEmpty/OpenglRendererSDL.cpp
)

target_sources(RendererGL
  PRIVATE
    ${RendererPlatformSource}
    ${CMAKE_CURRENT_LIST_DIR}/OpenglRenderer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/OpenglRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RendererGLStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RendererGLStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

welder_target_includes(RendererGL
  PUBLIC
    RendererBase
)
target_link_libraries(RendererGL
  PUBLIC
    GL
    Glew
)
