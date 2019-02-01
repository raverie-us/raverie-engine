add_executable(WelderEditor)

welder_setup_library(WelderEditor ${CMAKE_CURRENT_LIST_DIR})
welder_use_precompiled_header(WelderEditor ${CMAKE_CURRENT_LIST_DIR})

target_sources(WelderEditor
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Startup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZeroCrashCallbacks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZeroCrashCallbacks.hpp
)

target_link_libraries(WelderEditor
  PUBLIC
    Scintilla
    Libpng
    Assimp
    ZLib
    FreeType
    Opus
    Nvtt
    SpirvHeaders
    SpirvTools
    SpirvCross
    Platform
    Common
    Gameplay
    SpatialPartition
    CodeTranslator
    ZilchShaders
    Serialization
    Support
    Engine
    Geometry
    Widget
    Editor
    ZilchScript
    Networking
    Meta
    Physics
    Sound
    Content
    Replication
    UiWidget
    Startup
    Zilch
    Graphics
)

add_dependencies(WelderEditor
  BrowserSubProcess
)

welder_copy_from_linked_libraries(WelderEditor)
