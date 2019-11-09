if(NOT EMSCRIPTEN)
  add_subdirectory(Repo)

  add_library(Libgit2 INTERFACE)

  target_link_libraries(Libgit2
    INTERFACE
      git2
  )
endif()
