add_library(Startup)

welder_setup_library(Startup ${CMAKE_CURRENT_LIST_DIR})
welder_use_precompiled_header(Startup ${CMAKE_CURRENT_LIST_DIR})

target_sources(Startup
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Startup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Startup.hpp
    ${CMAKE_CURRENT_LIST_DIR}/StartupStandard.hpp
)

welder_target_includes(Startup
  PUBLIC
    Common
    Geometry
    Graphics
)