add_library(Platform)

raverie_setup_library(Platform ${CMAKE_CURRENT_LIST_DIR} TRUE)

target_sources(Platform
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Atomic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Audio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CallStack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ComPort.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CrashHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Debug.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DebugSymbolInformation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DirectoryWatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ExecutableResource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ExternalLibrary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/FpControl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Git.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Intrinsics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MainLoop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Peripherals.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Process.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Resolution.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Shell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Socket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Thread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ThreadSync.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VirtualFileAndFileSystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/WebRequest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PlatformCommunication.cpp
)

raverie_target_includes(Platform
  PUBLIC
    Common
)
