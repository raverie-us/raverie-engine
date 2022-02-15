add_executable(WelderEditor)

welder_setup_library(WelderEditor ${CMAKE_CURRENT_LIST_DIR} TRUE)
welder_use_precompiled_header(WelderEditor ${CMAKE_CURRENT_LIST_DIR})

target_sources(WelderEditor
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/VirtualFileSystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GameOrEditorStartup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GameOrEditorStartup.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ZeroCrashCallbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZeroCrashCallbacks.hpp
)

target_link_libraries(WelderEditor
  PUBLIC
    assimp
    CodeTranslator
    Common
    Content
    Editor
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
    ZilchScript
    ZilchShaders
)

welder_copy_from_linked_libraries(WelderEditor)
