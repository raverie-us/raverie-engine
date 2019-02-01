add_executable(WelderLauncherShell)

welder_setup_library(WelderLauncherShell ${CMAKE_CURRENT_LIST_DIR})
welder_use_precompiled_header(WelderLauncherShell ${CMAKE_CURRENT_LIST_DIR})

target_sources(WelderLauncherShell
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

target_link_libraries(WelderLauncherShell
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

add_dependencies(WelderLauncherShell
  BrowserSubProcess
  WelderLauncher
)
