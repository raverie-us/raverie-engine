################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# Set iPhone platform definitions
################################################################################

#define PLATFORM_HARDWARE 1
#define PLATFORM_NAME "iPhone"
add_definitions(-DPLATFORM_IPHONE=1 -DPLATFORM_HARDWARE=1 -DPLATFORM_NAME="iPhone")

set(platform "iPhone")
