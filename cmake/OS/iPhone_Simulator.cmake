################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# Set iPhoneSimulator platform definitions
################################################################################

#define PLATFORM_IPHONE 1
#define PLATFORM_VIRTUAL 1
#define PLATFORM_NAME "iPhoneSimulator"
add_definitions(-DPLATFORM_IPHONE=1 -DPLATFORM_VIRTUAL=1 -DPLATFORM_NAME="iPhoneSimulator")

set(platform "iPhoneSimulator")
