add_library(Platform)

raverie_setup_library(Platform ${CMAKE_CURRENT_LIST_DIR} TRUE)

target_sources(Platform
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Atomic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DirectoryWatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Intrinsics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Shell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Socket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Thread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ThreadSync.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VirtualFileAndFileSystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/WebRequest.cpp
)

raverie_target_includes(Platform
  PUBLIC
    Common
)
