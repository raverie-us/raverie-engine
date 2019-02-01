add_library(CodeTranslator)

welder_setup_library(CodeTranslator ${CMAKE_CURRENT_LIST_DIR})
welder_use_precompiled_header(CodeTranslator ${CMAKE_CURRENT_LIST_DIR})

target_sources(CodeTranslator
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/CodeSplitWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CodeSplitWindow.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CodeTranslatorCommands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CodeTranslatorCommands.hpp
    ${CMAKE_CURRENT_LIST_DIR}/CodeTranslatorStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderTranslationDebugHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ShaderTranslationDebugHelper.hpp
)

target_link_libraries(CodeTranslator
  PUBLIC
    Engine
    ZilchShaders
)
