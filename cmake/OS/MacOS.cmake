################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# Set MacOS platform definitions
################################################################################

#define PLATFORM_MAC 1
#define PLATFORM_POSIX 1
#define PLATFORM_HARDWARE 1
#define PLATFORM_NAME "MacOs"
add_definitions(-DPLATFORM_MAC=1 -DPLATFORM_POSIX=1 -DPLATFORM_HARDWARE=1 -DPLATFORM_NAME="MacOs")

set(platform "MacOS")

