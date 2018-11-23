add_definitions(-DGLEW_STATIC)

target_sources(Glew
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Repo/src/glew.c
)
