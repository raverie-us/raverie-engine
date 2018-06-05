################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################
# get our parent directory
get_filename_component(ParentDirectory ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY))

################################################################################
# define all of the libraries that are in this folder
################################################################################
add_library(ExampleLibrary "")
add_executable(ExampleExecutable "")

################################################################################
# include filelists for each library
################################################################################
include(ExampleLibrary/CMakeLists.txt)

################################################################################
# define include directories for all of our libraries
################################################################################
target_include_directories(ExampleLibrary 
    PUBLIC 
        ${Dependencies_Root}
        ${Source_Root}
        ${ParentDirectory}
        ${ZilchDirectory}
)
################################################################################
# set the output directories for all of our targets
################################################################################
set_target_properties(ExampleLibrary
                      ExampleSecondLibrary
                      PROPERTIES
                      ARCHIVE_OUTPUT_DIRECTORY ${Zero_Library_Dir}
                      LIBRARY_OUTPUT_DIRECTORY ${Zero_Library_Dir}
                      RUNTIME_OUTPUT_DIRECTORY ${Zero_Binary_Dir}
)

################################################################################
# set flags and definitions
################################################################################
################################################################################
# Group source into folders
################################################################################
