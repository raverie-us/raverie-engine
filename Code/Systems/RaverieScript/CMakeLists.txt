add_library(RaverieScript)

raverie_setup_library(RaverieScript ${CMAKE_CURRENT_LIST_DIR} TRUE)

target_sources(RaverieScript
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieScript.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieScript.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieScriptStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieScriptStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieBase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RaverieBase.hpp
)

raverie_target_includes(RaverieScript
  PUBLIC
    Common
    Geometry
    Engine
)
