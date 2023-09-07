add_library(Exceptions)

raverie_setup_library(Exceptions ${CMAKE_CURRENT_LIST_DIR} FALSE)

target_compile_definitions(Exceptions
  PUBLIC
  __USING_WASM_EXCEPTIONS__
  __ELF__
)

target_sources(Exceptions
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Repo/system/lib/libcxxabi/src/cxa_exception.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/system/lib/libcxxabi/src/cxa_personality.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Repo/system/lib/libunwind/src/Unwind-wasm.c
)

target_include_directories(Exceptions
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Repo/system/lib/libcxx/src/
    ${CMAKE_CURRENT_LIST_DIR}/Repo/system/lib/libunwind/include/
)
