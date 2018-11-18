################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################

target_sources(SPIRV-Tools
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/tools/buildHeaders/header.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/tools/buildHeaders/jsonToSpirv.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/tools/buildHeaders/jsoncpp/dist/jsoncpp.cpp
#  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.0/GLSL.std.450.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.0/OpenCL.std.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.0/spirv.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.0/spirv.hpp
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.1/GLSL.std.450.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.1/OpenCL.std.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.1/spirv.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.1/spirv.hpp
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.2/GLSL.std.450.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.2/OpenCL.std.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.2/spirv.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/1.2/spirv.hpp
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/unified1/GLSL.std.450.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/unified1/OpenCL.std.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/unified1/spirv.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/include/spirv/unified1/spirv.hpp
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/tools/buildHeaders/header.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/tools/buildHeaders/jsonToSpirv.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/tools/buildHeaders/jsoncpp/dist/json/json-forwards.h
    ${CMAKE_CURRENT_LIST_DIR}/SPIRV-Headers/tools/buildHeaders/jsoncpp/dist/json/json.h
)
