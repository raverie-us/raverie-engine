add_library(Platform)

welder_setup_library(Platform ${CMAKE_CURRENT_LIST_DIR} TRUE)
welder_use_precompiled_header(Platform ${CMAKE_CURRENT_LIST_DIR})

target_sources(Platform
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/ComPort.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/CrashHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Debug.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/DebugSymbolInformation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/DirectoryWatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/ExecutableResource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Git.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Intrinsics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Thread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/ThreadSync.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/VirtualFileAndFileSystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../OpenGL/OpenglRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Posix/Socket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Audio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/ExternalLibrary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Peripherals.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/PlatformStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Resolution.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Shell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../STD/Atomic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../STD/FpControl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../STD/Process.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CallStack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MainLoop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OpenglRendererEmscripten.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/WebRequest.cpp
)

target_compile_definitions(Platform
  PUBLIC
    ZeroPlatformNoShellOpenFile
    ZeroPlatformNoOpenUrl
    ZeroPlatformNoShellOpenApplication
    ZeroPlatformNoSupportsDownloadingFiles
    ZeroPlatformNoDownloadFile
    ZeroPlatformNoClipboardEvents
)

target_link_libraries(Platform
  PUBLIC
    SDL
)

welder_target_includes(Platform
  PUBLIC
    Common
)
