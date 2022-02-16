add_library(Platform)

welder_setup_library(Platform ${CMAKE_CURRENT_LIST_DIR} TRUE)
welder_use_precompiled_header(Platform ${CMAKE_CURRENT_LIST_DIR})

target_sources(Platform
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Atomic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Audio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/CallStack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/ComPort.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/CrashHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Debug.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/DebugSymbolInformation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/DirectoryWatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/ExecutableResource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/ExternalLibrary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/FpControl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Git.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Intrinsics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/MainLoop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Peripherals.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/PlatformStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Process.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Resolution.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Shell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Socket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Thread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/ThreadSync.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/VirtualFileAndFileSystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/WebRequest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

welder_target_includes(Platform
  PUBLIC
    Common
)
