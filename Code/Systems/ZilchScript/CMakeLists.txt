add_library(ZilchScript)

raverie_setup_library(ZilchScript ${CMAKE_CURRENT_LIST_DIR} TRUE)

target_sources(ZilchScript
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchScript.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchScript.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchScriptStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchScriptStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchZero.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ZilchZero.hpp
)

raverie_target_includes(ZilchScript
  PUBLIC
    Common
    Geometry
    Engine
)
