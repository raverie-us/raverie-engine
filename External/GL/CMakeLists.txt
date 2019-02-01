add_library(GL INTERFACE)

target_include_directories(GL
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/Includes/Windows/
)

add_library(GlU32 STATIC IMPORTED GLOBAL)

set_target_properties(GlU32
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/Libraries/Windows_x86/GlU32.Lib
)

add_library(OpenGL32 STATIC IMPORTED GLOBAL)

set_target_properties(OpenGL32
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/Libraries/Windows_x86/OpenGL32.Lib
)

target_link_libraries(GL
  INTERFACE
    GlU32
    OpenGL32
)
