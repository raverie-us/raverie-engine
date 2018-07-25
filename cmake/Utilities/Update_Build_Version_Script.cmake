################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################

message("Updating BuildVersion.inl file...")

set(ZERO_CORE_ROOT "" CACHE STRING "Root folder of all of Zero source")

set(zeroBuildFolder ${ZERO_CORE_ROOT}/Build)

set(buildVersionIdsFolder ${zeroBuildFolder}/BuildVersionIds)

# file we are going to write to
set(tempFile ${zeroBuildFolder}/BuildVersion.temp)

# check if git exists
execute_process(
    COMMAND git status > nul
    RESULT_VARIABLE procResult
    WORKING_DIRECTORY "${ZERO_CORE_ROOT}"
)

# the file that contains the default info for BuildVersion.txt
set(buildIdBranchFileName ${buildVersionIdsFolder}/default.txt)

# print the ret from git status to assist in any debugging down the road
message("return value from git status: ${procResult}")
#--------------------------------------------------------------------GitPresent
if (${procResult} EQUAL 0)

    # get the revision number
    execute_process(
        COMMAND git rev-list --count HEAD
        RESULT_VARIABLE procResult
        OUTPUT_VARIABLE procOutput
        WORKING_DIRECTORY "${ZERO_CORE_ROOT}"
    )

    set(revisionId "${procOutput}")
    # print the revision id to assist in future debugging
    message("revision ID: ${revisionId}")

    # line that will be inserted into temp file to define ZeroRevisionId
    set(revisionIdDefine "#define ZeroRevisionId ${revisionId}")

    # top of the temp file
    file(WRITE ${tempFile} "#pragma once\n")

    # get the build id file and insert it into the temp file
    file(READ ${buildIdBranchFileName} buildIdFileContent)
    file(APPEND ${tempFile} "${buildIdFileContent}\n")

    # append the revision Id line from above
    file(APPEND ${tempFile} "${revisionIdDefine}\n")

    # string to tell git how we want the info from git log to be formatted
    # look up documentation on get log --pretty for more information
    set(format          "#define ZeroShortChangeSet %h%n")
    set(format "${format}#define ZeroChangeSet %H%n")
    set(format "${format}#define ZeroChangeSetDate \"%cd\"%n")

    # get the change set information from get formatted how we want
    execute_process(
        COMMAND git log -1 --abbrev=12 --date=format:%Y-%m-%d --pretty=format:${format}
        RESULT_VARIABLE procResult
        OUTPUT_VARIABLE procOutput
        WORKING_DIRECTORY "${ZERO_CORE_ROOT}"
    )

    # append the change set information to the temp file
    file(APPEND ${tempFile} "${procOutput}\n")
#---------------------------------------------------------------NoSourceControl
else()
    # otherwise, fall back on having no source control
    file(WRITE "${tempFile}" "#pragma once\n")
    # write default buildId information
    file(READ "${buildIdBranchFileName}" buildIdFileContent)
    file(APPEND "${tempFile}" "${buildIdFileContent}\n")
    
    # We don't have any source control so just hard-code all of this information to empty
    file(APPEND "${tempFile}" "ZeroRevisionId 0\n")
    file(APPEND "${tempFile}" "#define ZeroShortChangeSet 0\n")
    file(APPEND "${tempFile}" "ZeroChangeSet 0 \n")
    file(APPEND "${tempFile}" "ZeroChangeSetDate \"\"\n")

endif()


#------------------------------------------------------------------------Finish
# if the content is different, replace the old file with our new temp file
execute_process(
    # executes "cmake -E copy_if_different
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    # input
    "${tempFile}"
    # output
    "${ZERO_CORE_ROOT}/Systems/Engine/BuildVersion.inl"
)


#REM Delete the temporary file
execute_process(
    # executes "cmake -E copy_if_different
    COMMAND ${CMAKE_COMMAND} -E remove
    # input
    "${tempFile}"
)

