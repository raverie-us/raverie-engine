################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################
# get our parent directory
get_filename_component(ParentDirectory ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
# get our current directory
set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

# based on the set platform variable, include the correct definition of the platform target
if ("${platform_library}" STREQUAL "Windows")
    include(Windows.cmake)
elseif ("${platform_library}" STREQUAL "Posix")
    include(Posix.cmake)
elseif ("${platform_library}" STREQUAL "SDL")
    include(SDL.cmake)
elseif ("${platform_library}" STREQUAL "STL")
    include(STL.cmake)
elseif ("${platform_library}" STREQUAL "SDLSTLEmpty")
    include(SDLSTLEmpty.cmake)
# if we are not given a known platform (or empty is selected) try the empty platform
else()
    include(Empty.cmake)
endif()
