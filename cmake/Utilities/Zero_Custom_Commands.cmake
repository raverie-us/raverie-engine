################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################

#### Post build step that copies list of dlls  to the given output directory after target is build
function(copy_multiple_dlls)
set(oneValueArgs OUTPUT_DIRECTORY)
set(multiValueArgs DLL_LOCATIONS)
cmake_parse_arguments(PARSED "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

set(PARSED_TARGETS ${PARSED_UNPARSED_ARGUMENTS})
foreach(target ${PARSED_TARGETS})
    foreach(dll ${PARSED_DLL_LOCATIONS})
        add_custom_command(TARGET ${target} POST_BUILD   # Adds a post-build event to MyTest
            COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
            ${dll}                                         # <--this is in-file
            ${PARSED_OUTPUT_DIRECTORY}/${target}           # <--this is out-file path
        )                 
    endforeach()
endforeach()
endfunction()
####

#### Post build command for creating build info file
function(create_build_info aTarget aProjDir aSourceDir aBuildOutDir aOutputLocation)
    add_custom_command(TARGET ${aTarget} POST_BUILD
        COMMAND CALL "\"${aProjDir}/CreateBuildInfo.cmd\"" ${aTarget} "\"${aSourceDir}\"" "\"${aBuildOutDir}/${aTarget}\"" > "\"${aOutputLocation}/${aTarget}/BuildInfo.data\""
    )
endfunction()
####

#### Post build command for creating build info file
function(create_project_build_info aTarget aOutputLocation)
    add_custom_command(TARGET ${aTarget} POST_BUILD
        COMMAND "CreateBuildInfo.cmd ${aTarget} > \"${aOutputLocation}/BuildInfo.data\""
    )
endfunction()
####

function (copy_cef_bin_post_build aTarget aZeroCoreDirectory aBuildOutputDirectory)
    add_custom_command(TARGET ${aTarget} POST_BUILD
    # executes "cmake -E copy_directory
    COMMAND ${CMAKE_COMMAND} -E copy_directory  
    # input folder
    ${aZeroCoreDirectory}/External/CEF/bin
    #output folder
    ${aBuildOutputDirectory}/${aTarget}
    )
endfunction()

#### Add set of required post build steps for plugin support
function(editor_post_build_step aTarget aZeroCoreDirectory aLibOutputDirectory aBuildOutputDirectory aOS aPlatformShortName)
    set(pluginDir "${aZeroCoreDirectory}/Data/ZilchCustomPluginShared/${aOS}-${aPlatformShortName}")

    add_custom_command(TARGET ${aTarget} POST_BUILD
        # executes "cmake -E make_directory
        COMMAND ${CMAKE_COMMAND} -E make_directory  
        # directory to make
        "\"${pluginDir}\""
    )

    if ("${platform}" STREQUAL "Windows")
    # copy the stub lib fileg
    add_custom_command(TARGET ${aTarget} POST_BUILD
        # executes "cmake -E copy_if_different
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        # input file
        ${aLibOutputDirectory}/${aTarget}/${aTarget}.lib
        #output file
        ${pluginDir}/${aTarget}.lib
    )

    # copy the error dialog
    add_custom_command(TARGET ${aTarget} POST_BUILD
        # executes "cmake -E copy_if_different
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        # input file
        ${aZeroCoreDirectory}/Projects/Win32Shared/ErrorDialog.exe
        #output file
        ${aBuildOutputDirectory}/${aTarget}/ErrorDialog.exe
    )
    endif()

    # copy the configuration file
    add_custom_command(TARGET ${aTarget} POST_BUILD
        # executes "cmake -E copy_if_different
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  
        # input file
        ${aZeroCoreDirectory}/Data/Configuration.data
        #output file
        ${aBuildOutputDirectory}/${aTarget}/Configuration.data
    )

    add_custom_command(TARGET ${aTarget} POST_BUILD
        # executes "cmake -E copy_if_different
        COMMAND ${CMAKE_COMMAND} -E copy_directory  
        # input folder
        ${aZeroCoreDirectory}/External/CEF/bin
        #output folder
        ${aBuildOutputDirectory}/${aTarget}
    )

    copy_cef_bin_post_build(${aTarget} ${aZeroCoreDirectory} ${aBuildOutputDirectory})

endfunction()
####

#### Copies processor dependencies 
function(processor_post_build)
    # parse arguments
    set(oneValueArgs ZERO_CORE_DIR PROCESSOR_OUTPUT_LOCATION CONFIGURATION)
    set(multiValueArgs DLL_LOCATIONS)

    cmake_parse_arguments(PARSED "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    set(PARSED_TARGETS ${PARSED_UNPARSED_ARGUMENTS})
    
    # directory where the processor will be copied to
    set(toolDir "${PARSED_ZERO_CORE_DIR}/Tools/${PARSED_TARGETS}/${PARSED_CONFIGURATION}")

    foreach(aTarget ${PARSED_TARGETS})
        # create the tool directory
        add_custom_command(
            TARGET ${aTarget} POST_BUILD
            # executes "cmake -E make_directory
            COMMAND ${CMAKE_COMMAND} -E make_directory  
            # directory to make
            "\"${toolDir}\""
        )
        # copy all the dlls to the tool directory
        foreach(aDll ${PARSED_DLL_LOCATIONS})
            # copy the dll to the output directory
            add_custom_command(
                TARGET ${aTarget} POST_BUILD
                # executes "cmake -E copy_if_different
                COMMAND ${CMAKE_COMMAND} -E copy_if_different  
                # input file
                ${aDll}                                         
                #output file
                ${PARSED_PROCESSOR_OUTPUT_LOCATION}/${aTarget}
            )
            # copy the dll to the tool directory
            add_custom_command(
                TARGET ${aTarget} POST_BUILD
                # executes "cmake -E copy_if_different
                COMMAND ${CMAKE_COMMAND} -E copy_if_different  
                # input file
                ${aDll}                                         
                #output file
                ${toolDir}
            )
        endforeach()

        # copy the tools executable
        add_custom_command(TARGET ${aTarget} POST_BUILD
            # executes "cmake -E copy_if_different
            COMMAND ${CMAKE_COMMAND} -E copy_if_different  
            # input file
            ${PARSED_PROCESSOR_OUTPUT_LOCATION}/${aTarget}/${aTarget}.exe
            #output file
            ${toolDir}
        )
    endforeach()

endfunction()
####

#### Moves CEF binaries into the bin folder and runs Launcher postbuild script
function(copy_launcher_files aTarget aZeroCoreDirectory aBuildOutputDirectory)
    copy_cef_bin_post_build(${aTarget} ${aZeroCoreDirectory} ${aBuildOutputDirectory})

    # copy the configuration file
    add_custom_command(
        TARGET ${aTarget} POST_BUILD
    # executes "cmake -E copy_if_different
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  
    # input file
    ${aZeroCoreDirectory}/Data/Configuration.data
    #output file
    ${aBuildOutputDirectory}/${aTarget}/Configuration.data
    )

    add_custom_command(
        TARGET ${aTarget} POST_BUILD
    # executes "cmake -E copy_if_different
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  
    # input file
    ${aZeroCoreDirectory}/Data/DefaultLauncherConfiguration.data
    #output file
    ${aBuildOutputDirectory}/${aTarget}/DefaultLauncherConfiguration.data
    )
endfunction()

#### Copies launcher files to output directory and calls the postbuild script for the shared launcher
function(launcher_shared_post_build aTarget aZeroCoreDirectory aProjectDirectory aBuildOutputDirectory)
    copy_launcher_files(${aTarget} ${aZeroCoreDirectory} ${aBuildOutputDirectory})

    add_custom_command(
        TARGET ${aTarget} POST_BUILD
        COMMAND CALL "\"${aProjectDirectory}/PostBuild.cmd\"" "\"${aBuildOutputDirectory}\""
    )
endfunction()
####

#### Copies launcher files to output directory then calls the create_build_info custom command for ZeroLauncher
function(launcher_post_build aTarget aZeroCoreDirectory aProjectDirectory aBuildOutputDirectory)
    copy_launcher_files(${aTarget} ${aZeroCoreDirectory} ${aBuildOutputDirectory})

    create_build_info(
        ${aTarget}
        ${CurrentDirectory}/${aTarget}/${aTarget}
        ${aZeroCoreDirectory}
        ${aBuildOutputDirectory}
        ${aBuildOutputDirectory}
    )
endfunction()
####


#### Zips up the files in the directory or the list of directories and puts the resulting zip in the given output file
function(zip_directory aTarget aFoldersToZip aOutputFile)
    # add a command to make sure to clean out he zip first before we create a new one
    add_custom_command(
        TARGET ${aTarget} PRE_LINK
        #command (-f so file not existing is still a success)
        COMMAND ${CMAKE_COMMAND} -E remove -f
        #file to remove
        ${aOutputFile}
    )

    # check for 7zip in path
    find_program(sevenZipLocation 
    NAMES 7z 7z.exe 7za
    HINTS CMAKE_SYSTEM_PROGRAM_PATH
    PATHS CMAKE_SYSTEM_PROGRAM_PATH
     )

     message("===7zip location: ${sevenZipLocation}\n")

     get_filename_component(sevenZip "${sevenZipLocation}" NAME)

    # if we couldn't find 7zip in the path, just use the cmake command for zipping
    if (("${sevenZip}" STREQUAL "sevenZip-NOTFOUND") OR ("${sevenZip}" STREQUAL ""))
        add_custom_command(
            TARGET ${aTarget} PRE_LINK 
            # command
            COMMAND ${CMAKE_COMMAND} -E tar 
            "cvf"
            # output
            "${aOutputFile}"
            --format=zip
            # input
            ${aFoldersToZip}
        )
    else()
        add_custom_command(
            TARGET ${aTarget} PRE_LINK 
            # command
            COMMAND ${sevenZip} a -tzip -mx=9 -mfb=128 -mpass=10 ${aOutputFile} "${aFoldersToZip}"
        )
    endif()
endfunction()
####
