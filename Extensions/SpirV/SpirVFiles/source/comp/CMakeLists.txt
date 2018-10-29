################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################

target_sources(SPIRV-Tools
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/markv_codec.cpp
    ${CMAKE_CURRENT_LIST_DIR}/markv_decoder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/markv_encoder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/move_to_front.cpp
#  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/bit_stream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/markv.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bit_stream.h
    ${CMAKE_CURRENT_LIST_DIR}/huffman_codec.h
    ${CMAKE_CURRENT_LIST_DIR}/markv.h
    ${CMAKE_CURRENT_LIST_DIR}/markv_codec.h
    ${CMAKE_CURRENT_LIST_DIR}/markv_decoder.h
    ${CMAKE_CURRENT_LIST_DIR}/markv_encoder.h
    ${CMAKE_CURRENT_LIST_DIR}/markv_logger.h
    ${CMAKE_CURRENT_LIST_DIR}/markv_model.h
    ${CMAKE_CURRENT_LIST_DIR}/move_to_front.h
)
