add_compile_options(${WELDER_C_CXX_EXTERNAL_FLAGS})

# OS dependent static/source libraries.
add_subdirectory(Curl)
add_subdirectory(GL)
add_subdirectory(SDL)

# Cross platform source libraries.
add_subdirectory(Assimp)
add_subdirectory(FreeType)
add_subdirectory(Glew)
add_subdirectory(Libgit2)
add_subdirectory(Libpng)
add_subdirectory(Nvtt)
add_subdirectory(Opus)
add_subdirectory(Scintilla)
add_subdirectory(SpirvCross)
add_subdirectory(SpirvHeaders)
add_subdirectory(SpirvTools)
add_subdirectory(ZLib)
