################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# Set Posix platform definitions (catch-all for any missed POSIX compatible platforms)
################################################################################

#define PLATFORM_POSIX 1
#define PLATFORM_HARDWARE 1
#define PLATFORM_NAME "Posix"
add_definitions(-DPLATFORM_POSIX=1 -DPLATFORM_HARDWARE=1 -DPLATFORM_NAME="Posix")

set(platform "Posix")
