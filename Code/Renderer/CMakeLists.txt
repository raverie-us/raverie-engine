add_subdirectory(RendererBase)

add_library(RendererImpl INTERFACE)

add_subdirectory(RendererGL)
target_link_libraries(RendererImpl INTERFACE RendererGL)
