################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################

################################################################################
# Includes for any other functions used in this file
################################################################################
include(${cmake_utilities_dir}/zero_target_precompiled_headers.cmake)
include(${cmake_utilities_dir}/Helper_Functions.cmake)

################################################################################
# Functions
################################################################################

function(set_precompiled_header target listDir)
  if(MSVC)
    target_compile_options(${target} PRIVATE "/FIPrecompiled.hpp")
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/YuPrecompiled.hpp")
    set_source_files_properties(${listDir}/Precompiled.cpp PROPERTIES COMPILE_FLAGS "/YcPrecompiled.hpp")
  else()
    #add_definitions(-include Precompiled.hpp)
  endif()
endfunction()

function(set_source_ignore_precompiled_header source)
  if(MSVC)
    set_source_files_properties(${source} PROPERTIES COMPILE_FLAGS "/Y-")
  else()
  endif()
endfunction()

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
    set(oneValueArgs CONFIGS BASEPATH PLATFORM BITS TOOLSET PRECOMPILED_HEADER_NAME PRECOMPILED_SOURCE_NAME TARGET_SUBFOLDER IGNORE_TARGET CONFIG)
    cmake_parse_arguments(PARSED "" "${oneValueArgs}" "" ${ARGN})

    if("${PARSED_IGNORE_TARGET}" STREQUAL "")
        set(PARSED_IGNORE_TARGET OFF)
    endif()


    set(PARSED_TARGETS ${PARSED_UNPARSED_ARGUMENTS})

    foreach(target ${PARSED_TARGETS})
        # if we were passed a config, seperate our intermediate files by config instead of platform
        if(PARSED_CONFIG)
          set(intOutputDirectory "${PARSED_BASEPATH}/Int/${PARSED_CONFIG}/${PARSED_BITS}${CONFIGS}/${target}")
        else()
          set(intOutputDirectory "${PARSED_BASEPATH}/Int/${PARSED_PLATFORM}/${PARSED_BITS}${CONFIGS}/${target}")
        endif()
        
        set_target_properties(
            ${target}
            PROPERTIES
            CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY $<$<CONFIG:Debug>:${intOutputDirectory}>
            $<$<CONFIG:Debug>:COMPILE_PDB_NAME ${target}${CONFIGS}${PARSED_TOOLSET}>
        )
        # if we were passed values for the precompiled headers, set the target precompiled headers
        if (NOT ("${PARSED_PRECOMPILED_HEADER_NAME}" STREQUAL ""))
            zero_target_precompiled_headers(${target} "${intOutputDirectory}" ${PARSED_PRECOMPILED_HEADER_NAME} ${PARSED_PRECOMPILED_SOURCE_NAME} "${PARSED_TARGET_SUBFOLDER}" ${PARSED_IGNORE_TARGET})
        else()
            message("<><><> Skipped precompiled for target: ${target}\n")
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

#### Copies contents of given directory into the output folder of every passed target
function(zero_multitarget_copy_folders_to_target_output_directories)
    set(oneValueArgs OUTPUT_DIRECTORY)
    set(multiValueArgs FOLDERS_TO_COPY)

    cmake_parse_arguments(PARSED "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(PARSED_TARGETS ${PARSED_UNPARSED_ARGUMENTS})

    foreach(target ${PARSED_TARGETS})
        foreach(folder ${PARSED_FOLDERS_TO_COPY})
            add_custom_command(TARGET ${target} POST_BUILD
                # executes "cmake -E copy_directory
                COMMAND ${CMAKE_COMMAND} -E copy_directory  
                # input folder
                ${folder}
                #output folder
                ${PARSED_OUTPUT_DIRECTORY}/${target}
            )
        endforeach()
    endforeach()

endfunction()
####


#### multitarget version of zip directory, probably the easier one to use
function(multitarget_zip_directory)
    set(oneValueArgs OUTPUT_FILE)
    # empty since we leave targets as "unparsed" for consistancy
    set(multiValueArgs FOLDERS_TO_ZIP)

    cmake_parse_arguments(PARSED "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(PARSED_TARGETS ${PARSED_UNPARSED_ARGUMENTS})

    # assign this zip directory command to each target that was passsed
    foreach(target ${PARSED_TARGETS})
        zip_directory(${target} "${PARSED_FOLDERS_TO_ZIP}" "${PARSED_OUTPUT_FILE}")
    endforeach()

endfunction()