# place to hide useful functions used when defining commands that should
# actually be called

#### Zips up the files in the directory or the list of directories and puts the resulting zip in the given output file
#### used by the proper multitarget version
function(zip_directory aTarget aFoldersToZip aOutputFile)
    # add a command to make sure to clean out he zip first before we create a new one
    add_custom_command(
        TARGET ${aTarget} PRE_LINK
        #command (-f so file not existing is still a success)
        COMMAND ${CMAKE_COMMAND} -E remove -f
        #file to remove
        ${aOutputFile}
        WORKING_DIRECTORY "${zero_core_path}"
    )

    foreach(folder ${aFoldersToZip})
        file(TO_NATIVE_PATH ${folder} folder)
        set(formattedList "${formattedList} ${folder}")
    endforeach()


    message("zip_directory formated list: ${formattedList}")

    add_custom_command(
        TARGET ${aTarget} PRE_LINK    
        COMMAND ${CMAKE_COMMAND} 
        # options
        -DFOLDERS_TO_ZIP=${formattedList}
        -DOUTPUT_FILE=${aOutputFile}
        -DWORKING_DIRECTORY=${zero_core_path}
        # command
        -P ${cmake_utilities_dir}/Zip_Command.cmake

        WORKING_DIRECTORY ${zero_core_path}
        VERBATIM
    )

endfunction()
####
