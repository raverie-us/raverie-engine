add_library(Startup)

raverie_setup_library(Startup ${CMAKE_CURRENT_LIST_DIR} TRUE)

target_sources(Startup
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Startup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Startup.hpp
    ${CMAKE_CURRENT_LIST_DIR}/StartupStandard.hpp
)

raverie_target_includes(Startup
  PUBLIC
    Common
    Geometry
    Graphics
)
