################################################################################
# Add any files that need to be manually added in this file.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################
#Adds generated list, do not remove line unless you are sure you want to ignore the generated file.
include(${CMAKE_CURRENT_LIST_DIR}/Generated.cmake)

target_sources(MathTests
    PRIVATE
)
