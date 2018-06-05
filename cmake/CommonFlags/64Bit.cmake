################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# 64 bit platform definitions
################################################################################
#define PLATFORM_64 1
#define PLATFORM_BITS "64"
add_definitions(-DPLATFORM_64=1 -DPLATFORM_BITS="64")

set(bit 64)

message(FATAL_ERROR "64 bit builds are currently not supported")