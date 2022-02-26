add_executable(Game)

welder_setup_library(Game ${CMAKE_CURRENT_LIST_DIR} TRUE)
welder_use_precompiled_header(Game ${CMAKE_CURRENT_LIST_DIR})

target_sources(Game
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/VirtualFileSystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GameStartup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GameStartup.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ZeroCrashCallbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZeroCrashCallbacks.hpp
)

target_link_libraries(Game
  PUBLIC
    assimp
    Common
    Content
    EditorCore
    Engine
    FreeType
    Gameplay
    Geometry
    Graphics
    Libpng
    Meta
    Networking
    Nvtt
    Opus
    Physics
    Platform
    RendererBase
    RendererImpl
#    RendererGL
    Replication
    Scintilla
    Serialization
    Sound
    SpatialPartition
    SpirvCross
    SpirvHeaders
    SpirvTools
    Startup
    Support
    UiWidget
    Widget
    ZLib
    Zilch
    ZilchShaders
)

welder_copy_from_linked_libraries(Game)
