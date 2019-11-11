if(NOT EMSCRIPTEN)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
  set(BUILD_CLAR OFF CACHE BOOL "")

  add_subdirectory(Repo)

  add_library(Libgit2 INTERFACE)

  target_link_libraries(Libgit2
    INTERFACE
      git2
  )
endif()
