add_library(SDL INTERFACE)

target_include_directories(SDL
  INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/Includes/
)

add_library(SDL2 STATIC IMPORTED GLOBAL)

set_target_properties(SDL2
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/Libraries/Windows_x86/SDL2.lib
)

add_library(SDL2main STATIC IMPORTED GLOBAL)

set_target_properties(SDL2main
  PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/Libraries/Windows_x86/SDL2main.Lib
)

target_link_libraries(SDL
  INTERFACE
    SDL2
    SDL2main
)

set(SDLCopyDirectories
  ${CMAKE_CURRENT_LIST_DIR}/Binaries/Windows_x86/
  CACHE INTERNAL ""
)
