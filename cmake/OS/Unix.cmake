################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# Set Unix platform definitions
################################################################################

#define PLATFORM_Unix 1
#define PLATFORM_POSIX 1
#define PLATFORM_HARDWARE 1
#define PLATFORM_NAME "Unix"
add_definitions(-DPLATFORM_UNIX=1 -DPLATFORM_POSIX=1 -DPLATFORM_HARDWARE=1 -DPLATFORM_NAME="Unix")

set(platform "Unix")
