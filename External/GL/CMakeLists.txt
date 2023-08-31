add_library(GL INTERFACE)

find_package(OpenGL)

target_include_directories(GL
  INTERFACE
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(GL
  INTERFACE
    ${OPENGL_LIBRARIES}
)