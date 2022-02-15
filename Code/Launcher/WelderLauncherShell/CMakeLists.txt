add_executable(WelderLauncherShell)

welder_setup_library(WelderLauncherShell ${CMAKE_CURRENT_LIST_DIR} TRUE)
welder_use_precompiled_header(WelderLauncherShell ${CMAKE_CURRENT_LIST_DIR})

target_sources(WelderLauncherShell
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/VirtualFileSystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

target_link_libraries(WelderLauncherShell
  PUBLIC
    Common
    Platform
    ZLib
    Support
)

add_dependencies(WelderLauncherShell
  WelderLauncher
)

welder_copy_from_linked_libraries(WelderLauncherShell)
