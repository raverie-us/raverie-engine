################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################

target_sources(SPIRV-Tools
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/bit_stream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bit_vector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/parse_number.cpp
    ${CMAKE_CURRENT_LIST_DIR}/string_utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/timer.cpp
#  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/bitutils.h
    ${CMAKE_CURRENT_LIST_DIR}/bit_stream.h
    ${CMAKE_CURRENT_LIST_DIR}/bit_vector.h
    ${CMAKE_CURRENT_LIST_DIR}/hex_float.h
    ${CMAKE_CURRENT_LIST_DIR}/huffman_codec.h
    ${CMAKE_CURRENT_LIST_DIR}/ilist.h
    ${CMAKE_CURRENT_LIST_DIR}/ilist_node.h
    ${CMAKE_CURRENT_LIST_DIR}/make_unique.h
    ${CMAKE_CURRENT_LIST_DIR}/move_to_front.h
    ${CMAKE_CURRENT_LIST_DIR}/parse_number.h
    ${CMAKE_CURRENT_LIST_DIR}/small_vector.h
    ${CMAKE_CURRENT_LIST_DIR}/string_utils.h
    ${CMAKE_CURRENT_LIST_DIR}/timer.h
)
