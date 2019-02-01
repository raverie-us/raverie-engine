add_library(WinHid STATIC IMPORTED GLOBAL)

set_target_properties(WinHid
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/Libraries/Windows_x86/hid.lib
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/Includes/Windows/
)
