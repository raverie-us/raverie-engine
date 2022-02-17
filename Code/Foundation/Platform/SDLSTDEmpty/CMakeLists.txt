add_library(Platform)

welder_setup_library(Platform ${CMAKE_CURRENT_LIST_DIR} TRUE)
welder_use_precompiled_header(Platform ${CMAKE_CURRENT_LIST_DIR})

target_compile_definitions(Platform
  PUBLIC
    main=SDL_main
)

target_sources(Platform
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../Curl/WebRequest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/CallStack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/ComPort.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/CrashHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Debug.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/DebugSymbolInformation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/DirectoryWatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/ExecutableResource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Intrinsics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/MainLoop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Empty/Socket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../Libgit2/Git.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Audio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/ExternalLibrary.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/File.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Keys.inl
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/MouseButtons.inl
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Peripherals.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/PlatformStandard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Resolution.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Shell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Thread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/ThreadSync.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Timer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../SDL/Utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../STD/Atomic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../STD/FileSystem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../STD/FpControl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../STD/Process.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
)

welder_target_includes(Platform
  PUBLIC
    Common
)

target_link_libraries(Platform
  PUBLIC
    Curl
    Libgit2
    SDL
)

if (WELDER_TARGETOS STREQUAL "Linux")
  target_link_libraries(Platform
    PUBLIC
      stdc++fs
      dl
  )
endif()
