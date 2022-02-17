add_subdirectory(RendererBase)

#Stub
#Emscripten
#SDLSTDEmpty
#Windows

add_library(RendererImpl INTERFACE)

if(${WELDER_PLATFORM} STREQUAL "Windows" OR ${WELDER_PLATFORM} STREQUAL "Emscripten" OR ${WELDER_PLATFORM} STREQUAL "SDLSTDEmpty")
  add_subdirectory(RendererGL)
  target_link_libraries(RendererImpl INTERFACE RendererGL)
elseif(${WELDER_PLATFORM} STREQUAL "Stub")
  add_subdirectory(RendererEmpty)
  target_link_libraries(RendererImpl INTERFACE RendererEmpty)
else()
  message(FATAL_ERROR "No renderer available for target '${WELDER_TARGETOS}'.")
endif()
