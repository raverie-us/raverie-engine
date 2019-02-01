add_library(CEF INTERFACE)

target_include_directories(CEF
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/Includes/
)

add_library(Libcef STATIC IMPORTED GLOBAL)

set_target_properties(Libcef
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/Libraries/Windows_x86/libcef.lib
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/Includes/
)

add_library(LibcefWrapper STATIC IMPORTED GLOBAL)

set_target_properties(LibcefWrapper
  PROPERTIES
    IMPORTED_LOCATION_DEBUG ${CMAKE_CURRENT_LIST_DIR}/Libraries/Windows_x86_Debug/libcef_dll_wrapper.lib
    IMPORTED_LOCATION_RELEASE ${CMAKE_CURRENT_LIST_DIR}/Libraries/Windows_x86_Release/libcef_dll_wrapper.lib
)

target_link_libraries(CEF
  INTERFACE
    Libcef
    LibcefWrapper
)

set(CEFCopyDirectories
  ${CMAKE_CURRENT_LIST_DIR}/Binaries/Windows_x86/
  ${CMAKE_CURRENT_LIST_DIR}/Resources/
  CACHE INTERNAL ""
)
