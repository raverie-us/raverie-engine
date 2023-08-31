add_subdirectory(RendererBase)

add_library(RendererImpl INTERFACE)

# TODO(trevor): We will want to fully support this, but as WASM external calls
#add_subdirectory(RendererGL)
#target_link_libraries(RendererImpl INTERFACE RendererGL)

add_subdirectory(RendererEmpty)
target_link_libraries(RendererImpl INTERFACE RendererEmpty)
