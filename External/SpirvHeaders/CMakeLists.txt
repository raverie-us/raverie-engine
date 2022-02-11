add_library(SpirvHeaders)

welder_setup_library(SpirvHeaders ${CMAKE_CURRENT_LIST_DIR} FALSE)

target_sources(SpirvHeaders
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/spirv/unified1/spirv.h
    ${CMAKE_CURRENT_LIST_DIR}/Symbols.cpp
)

target_include_directories(SpirvHeaders
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/
)
