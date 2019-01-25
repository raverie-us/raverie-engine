target_sources(SpirvHeaders
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/spirv/unified1/spirv.h
)

set_target_properties(SpirvHeaders
  PROPERTIES
  LINKER_LANGUAGE CXX
)