add_library(RendererGL)

welder_setup_library(RendererGL ${CMAKE_CURRENT_LIST_DIR} TRUE)
welder_use_precompiled_header(RendererGL ${CMAKE_CURRENT_LIST_DIR})

#Emscripten
#SDLSTDEmpty
#Windows

if(${WELDER_TARGETOS} STREQUAL "Windows")
  set(RendererPlatformSource 
    ${CMAKE_CURRENT_LIST_DIR}/Windows/OpenglRendererWindows.cpp
  )
elseif(${WELDER_TARGETOS} STREQUAL "Emscripten")
  set(RendererPlatformSource 
    ${CMAKE_CURRENT_LIST_DIR}/Emscripten/OpenglRendererEmscripten.cpp
  )
elseif(${WELDER_TARGETOS} STREQUAL "Linux")
  set(RendererPlatformSource 
    ${CMAKE_CURRENT_LIST_DIR}/SDLSTDEmpty/OpenglRendererSDL.cpp
  )
else()
  message(FATAL_ERROR "No renderer available for '${WELDER_PLATFORM}'.")
endif()

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
    RendererBase
)

if(NOT ${WELDER_TARGETOS} STREQUAL "Windows")
  target_link_libraries(RendererGL
    PUBLIC
      SDL
  )
endif()
