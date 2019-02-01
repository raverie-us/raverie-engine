add_executable(BrowserSubProcess)

welder_setup_library(BrowserSubProcess ${CMAKE_CURRENT_LIST_DIR})
welder_use_precompiled_header(BrowserSubProcess ${CMAKE_CURRENT_LIST_DIR})

target_sources(BrowserSubProcess
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

target_link_libraries(BrowserSubProcess
  PUBLIC
    #Scintilla
    #Libpng
    #Assimp
    #ZLib
    #FreeType
    #Opus
    #Nvtt
    #SpirvHeaders
    #SpirvTools
    #SpirvCross
    Common
    Platform
)
