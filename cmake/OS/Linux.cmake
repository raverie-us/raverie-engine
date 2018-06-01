################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# Set Linux platform definitions
################################################################################
#define PLATFORM_LINUX 1
#define PLATFORM_POSIX 1
#define PLATFORM_HARDWARE 1
#define PLATFORM_NAME "Linux"
add_definitions(-DPLATFORM_LINUX=1 -DPLATFORM_POSIX=1 -DPLATFORM_HARDWARE=1 -DPLATFORM_NAME="Linux")

set(platform "Linux")
