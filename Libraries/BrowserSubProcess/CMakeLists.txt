add_executable(BrowserSubProcess)

welder_setup_library(BrowserSubProcess ${CMAKE_CURRENT_LIST_DIR})
welder_use_precompiled_header(BrowserSubProcess ${CMAKE_CURRENT_LIST_DIR})

target_sources(BrowserSubProcess
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

target_link_libraries(BrowserSubProcess
  PUBLIC
    Common
    Platform
)

get_target_property(BrowserSubProcessBinaryDir BrowserSubProcess BINARY_DIR)

set(BrowserSubProcessCopyDirectories
  "$<TARGET_FILE_DIR:BrowserSubProcess>"
  CACHE INTERNAL ""
)
