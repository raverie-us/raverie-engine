################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# Set Linux platform definitions
################################################################################
#define PLATFORM_EMSCRIPTEN 1
#define PLATFORM_HARDWARE 1
#define PLATFORM_NAME "Emscripten"
add_definitions(-DPLATFORM_EMSCRIPTEN=1 -DPLATFORM_HARDWARE=1 -DPLATFORM_NAME="Emscripten")

set(platform "Emscripten")
set(platform_library "Emscripten")

set(StaticExternals 
Assimp 
Freetype 
Libpng 
Nvtt 
Opus 
)

# list of shared
set(SharedExternals
Assimp
Freetype
Nvtt
)
