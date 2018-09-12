message("ZIP COMMAND FILE WAS CALLED CORRECTLY")

set(FOLDERS_TO_ZIP "" CACHE STRING "List of folders that are to be zipped.")
set(OUTPUT_FILE "" CACHE STRING "Zip file to output everything to.")
set(WORKING_DIRECTORY "" CACHE STRING "Where to run the zip command from")
message("\n\nFOLDERS_TO_ZIP ${FOLDERS_TO_ZIP}")
message("OUTPUT_FILE ${OUTPUT_FILE}")
message("WORKING_DIRECTORY ${WORKING_DIRECTORY}\n\n")

string(REPLACE " " ";" foldersToZipCorrectedList "${FOLDERS_TO_ZIP}")

file(TO_NATIVE_PATH ${OUTPUT_FILE} fixedOutputPath)

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
execute_process(
        # command
        COMMAND ${CMAKE_COMMAND} -E tar 
        "cvf"
        # output
        ${fixedOutputPath}
        --format=zip
        # input
        ${FOLDERS_TO_ZIP}
        RESULT_VARIABLE res_var
        WORKING_DIRECTORY "${WORKING_DIRECTORY}"
    )
else()
    message("7zip command: ${sevenZip} a -tzip -mx=9 -mfb=128 -mpass=10 ${OUTPUT_FILE} ${foldersToZipCorrectedList}")
    execute_process(
        # command
        COMMAND ${sevenZip} a -tzip -mx=9 -mfb=128 -mpass=10 ${fixedOutputPath} ${foldersToZipCorrectedList}
        RESULT_VARIABLE res_var
        WORKING_DIRECTORY "${WORKING_DIRECTORY}"
    )
endif()

message("Exit code from zip command: ${res_var}, will be non-zero even if only warnings present.")

message("Going back to linker step for current target...")

# yes I know this is useless but it is here just to remind me that this is
# called like a script, and is not part of a list.
return()
