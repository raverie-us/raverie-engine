add_library(ZilchScript)

welder_setup_library(ZilchScript ${CMAKE_CURRENT_LIST_DIR} TRUE)
welder_use_precompiled_header(ZilchScript ${CMAKE_CURRENT_LIST_DIR})

target_sources(ZilchScript
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchPlugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchPlugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchScript.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchScript.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchScriptStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchScriptStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchZero.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchZero.hpp
)

welder_target_includes(ZilchScript
  PUBLIC
    Common
    Geometry
    Editor
)
