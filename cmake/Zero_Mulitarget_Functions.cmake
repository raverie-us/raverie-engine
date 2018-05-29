################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################

################################################################################
# Includes for any other functions used in this file
################################################################################
include(${cmake_include}/zero_target_precompiled_headers.cmake)


################################################################################
# Functions
################################################################################

#### Sets the given arguments on every passed target
function(zero_multitarget_compile_options)
    set(oneValueArgs )
    set(multiValueArgs PUBLIC PRIVATE INTERFACE)

    cmake_parse_arguments(PARSED "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(PARSED_TARGETS ${PARSED_UNPARSED_ARGUMENTS})

    foreach(target ${PARSED_TARGETS})
        foreach(option ${multiValueArgs})
            target_compile_options(${target} ${option} ${PARSED_${option}})
        endforeach()
    endforeach()
endfunction()
####


#### Sets the output directories for intermediate files
function(zero_multitarget_output_settings)

    # set arguments
    set(oneValueArgs CONFIGS BASEPATH PLATFORM BITS TOOLSET PRECOMPILED_HEADER_NAME PRECOMPILED_SOURCE_NAME TARGET_SUBFOLDER)
    cmake_parse_arguments(PARSED "" "${oneValueArgs}" "" ${ARGN})

    set(PARSED_TARGETS ${PARSED_UNPARSED_ARGUMENTS})

    foreach(target ${PARSED_TARGETS})
        set(intOutputDirectory ${PARSED_BASEPATH}/Int/${PARSED_PLATFORM}/${PARSED_BITS}${CONFIGS}/${target})
        set_target_properties(${target}
        PROPERTIES
        $<$<CONFIG:Debug>:CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${intOutputDirectory}>
        $<$<CONFIG:Debug>:COMPILE_PDB_NAME ${target}${CONFIGS}${PARSED_TOOLSET}>
        )
        # if we were passed values for the precompiled headers, set the target precompiled headers
        if (NOT ("${PARSED_PRECOMPILED_HEADER_NAME}" STREQUAL ""))
            zero_target_precompiled_headers(${target} ${intOutputDirectory} ${PARSED_PRECOMPILED_HEADER_NAME} ${PARSED_PRECOMPILED_SOURCE_NAME} "${PARSED_TARGET_SUBFOLDER}")
        else()
            #message("<><><> Skipped precompiled for target: ${target}\n")
        endif()

    endforeach()
endfunction()
####

#### Sets the output directories for intermediate files
function(zero_multitarget_output_directories)
    set(oneValueArgs LIBRARY_DIRECTORY RUNTIME_DIRECTORY)
    # empty since we leave targets as "unparsed" for consistancy
    set(multiValueArgs )

    cmake_parse_arguments(PARSED "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(PARSED_TARGETS ${PARSED_UNPARSED_ARGUMENTS})
    foreach(target ${PARSED_TARGETS})
        set_target_properties(${target}
                              PROPERTIES
                              ARCHIVE_OUTPUT_DIRECTORY ${PARSED_LIBRARY_DIRECTORY}/${target}
                              LIBRARY_OUTPUT_DIRECTORY ${PARSED_LIBRARY_DIRECTORY}/${target}
                              RUNTIME_OUTPUT_DIRECTORY ${PARSED_RUNTIME_DIRECTORY}/${target}
        )
    endforeach()
endfunction()
####