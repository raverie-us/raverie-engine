add_library(SpirvCross)

# This cmake option 'RaverieExceptions' needs to be setup somewhere for this to work
if (NOT RAVERIE_EXCEPTIONS)
  add_compile_definitions(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
endif()

raverie_setup_library(SpirvCross ${CMAKE_CURRENT_LIST_DIR} FALSE)

target_sources(SpirvCross
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Repo/GLSL.std.450.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/spirv_cross/barrier.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/spirv_cross/external_interface.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/spirv_cross/image.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/spirv_cross/internal_interface.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/spirv_cross/sampler.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/spirv_cross/thread_group.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_cfg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_cfg.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_common.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_cpp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_cpp.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_cross.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_cross.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_cross_parsed_ir.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_cross_parsed_ir.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_cross_util.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_cross_util.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_glsl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_glsl.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_hlsl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_hlsl.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_msl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_msl.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_parser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_parser.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_reflect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/spirv_reflect.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

target_include_directories(SpirvCross
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Repo/
    ${CMAKE_CURRENT_LIST_DIR}/Repo/include/
)
