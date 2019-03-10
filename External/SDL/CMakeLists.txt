set(SDL_SHARED_ENABLED_BY_DEFAULT OFF)

add_subdirectory(Repo)

add_library(SDL INTERFACE)

target_link_libraries(SDL
  INTERFACE
    SDL2-static
    SDL2main
)
