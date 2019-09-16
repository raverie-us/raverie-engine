cmake_minimum_required(VERSION 3.12.1 FATAL_ERROR)

set(CMAKE_CONFIGURATION_TYPES Debug RelWithDebInfo Release MinSizeRel)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT WelderEditor)

find_program(WELDER_IWYU_PATH NAMES include-what-you-use iwyu)
if(WELDER_IWYU_PATH)
  message(STATUS "Found iwyu at ${WELDER_IWYU_PATH}")
endif()

find_program(WELDER_CCACHE_PATH ccache)
if(WELDER_CCACHE_PATH)
  message(STATUS "Found ccache at ${WELDER_CCACHE_PATH}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${WELDER_CCACHE_PATH}")
endif()

project(Welder)

add_definitions(-DWelderRevisionId=${WELDER_REVISION})
add_definitions(-DWelderShortChangeSet=${WELDER_SHORT_CHANGESET})
add_definitions(-DWelderChangeSet=${WELDER_CHANGESET})
add_definitions(-DWelderChangeSetDate=${WELDER_CHANGESET_DATE})
add_definitions(-DWelderArchitectureName="${WELDER_ARCHITECTURE}")
add_definitions(-DWelderHostOs${WELDER_HOSTOS}=1)
add_definitions(-DWelderHostOsName="${WELDER_HOSTOS}")
add_definitions(-DWelderTargetOs${WELDER_TARGETOS}=1)
add_definitions(-DWelderTargetOsName="${WELDER_TARGETOS}")
add_definitions(-DWelderPlatform${WELDER_PLATFORM}=1)
add_definitions(-DWelderPlatformName="${WELDER_PLATFORM}")
add_definitions(-DWelderToolchain${WELDER_TOOLCHAIN}=1)
add_definitions(-DWelderToolchainName="${WELDER_TOOLCHAIN}")

set(WELDER_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WELDER_CMAKE_DIR ${WELDER_CORE_DIR}/CMakeFiles/)
set(WELDER_TOOLCHAIN_DIR ${WELDER_CMAKE_DIR}/Toolchain/)
set(WELDER_TARGETOS_DIR ${WELDER_CMAKE_DIR}/TargetOs/)
set(WELDER_LIBRARIES_DIR ${WELDER_CORE_DIR}/Libraries/)
set(WELDER_PLATFORM_DATA_DIR ${WELDER_CORE_DIR}/PlatformData/${WELDER_TOOLCHAIN})
set(WELDER_VIRTUAL_FILE_SYSTEM_ZIP ${CMAKE_BINARY_DIR}/FileSystem.zip)

add_custom_target(VirtualFileSystemZip
  COMMAND ${CMAKE_COMMAND} -E remove ${WELDER_VIRTUAL_FILE_SYSTEM_ZIP}
  COMMAND 7z a -tzip -mx=9 -mfb=128 -mpass=10 ${WELDER_VIRTUAL_FILE_SYSTEM_ZIP}
    "Resources"
    "Data"
    "Build/PrebuiltContent/ZeroVersion${WELDER_REVISION}-${WELDER_CHANGESET}"
    ".welder"
  #COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${WELDER_VIRTUAL_FILE_SYSTEM_ZIP}" --format=zip
  WORKING_DIRECTORY ${WELDER_CORE_DIR}
)

function(welder_setup_library target directory internal)
  target_include_directories(${target} PUBLIC ${directory} ${WELDER_LIBRARIES_DIR})
  if (${internal})
    #set_property(TARGET ${target} PROPERTY C_INCLUDE_WHAT_YOU_USE ${WELDER_IWYU_PATH})
    #set_property(TARGET ${target} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${WELDER_IWYU_PATH})
  endif()
endfunction()

function(welder_get_all_dependencies outputList TARGET)
    list(APPEND visitedTargets ${TARGET})
    get_target_property(dependencies ${TARGET} INTERFACE_LINK_LIBRARIES)
    get_target_property(targetType ${TARGET} TYPE)
    if (NOT targetType STREQUAL "INTERFACE_LIBRARY")
      get_target_property(manualDependencies ${TARGET} MANUALLY_ADDED_DEPENDENCIES)
      list(APPEND dependencies ${manualDependencies})
    endif()
    set(allLinkedLibs "")
    foreach(dependency ${dependencies})
        if (TARGET ${dependency})
            list(FIND visitedTargets ${dependency} VISITED)
            if (${VISITED} EQUAL -1)
                welder_get_all_dependencies(childLinkedLibs ${dependency})
                list(APPEND allLinkedLibs ${dependency} ${childLinkedLibs})
            endif()
        endif()
    endforeach()
    set(visitedTargets ${visitedTargets} PARENT_SCOPE)
    set(${outputList} ${allLinkedLibs} PARENT_SCOPE)
endfunction()

function(welder_copy_from_linked_libraries target)
  welder_get_all_dependencies(dependencies ${target})
  foreach(dependency ${dependencies})
    set(directory "${${dependency}CopyDirectories}")
    if(directory)
      add_custom_command(TARGET ${target} POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${directory} $<TARGET_FILE_DIR:${target}>)
    endif()
  endforeach()
endfunction()

function(welder_target_includes target)
  set(multiValueArgs PUBLIC)
  cmake_parse_arguments(WELDER "" "" "${multiValueArgs}" "${ARGN}")

  foreach(fromTarget ${WELDER_PUBLIC})
  target_include_directories(${target} PUBLIC $<TARGET_PROPERTY:${fromTarget},INTERFACE_INCLUDE_DIRECTORIES>)
  target_compile_definitions(${target} PUBLIC $<TARGET_PROPERTY:${fromTarget},INTERFACE_COMPILE_DEFINITIONS>)
  endforeach()
endfunction()

#Emscripten
#MSVC
include(${WELDER_TOOLCHAIN_DIR}/${WELDER_TOOLCHAIN}.cmake)
include(${WELDER_TARGETOS_DIR}/${WELDER_TARGETOS}.cmake)

set(CMAKE_C_FLAGS                             "${CMAKE_C_FLAGS}                             ${WELDER_C_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS                           "${CMAKE_CXX_FLAGS}                           ${WELDER_C_CXX_FLAGS}")

set(CMAKE_C_FLAGS_DEBUG                       "${CMAKE_C_FLAGS_DEBUG}                       ${WELDER_C_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG                     "${CMAKE_CXX_FLAGS_DEBUG}                     ${WELDER_C_CXX_FLAGS_DEBUG}")

set(CMAKE_C_FLAGS_RELWITHDEBINFO              "${CMAKE_C_FLAGS_RELWITHDEBINFO}              ${WELDER_C_CXX_FLAGS_RELWITHDEBINFO}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO            "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}            ${WELDER_C_CXX_FLAGS_RELWITHDEBINFO}")

set(CMAKE_C_FLAGS_RELEASE                     "${CMAKE_C_FLAGS_RELEASE}                     ${WELDER_C_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELEASE                   "${CMAKE_CXX_FLAGS_RELEASE}                   ${WELDER_C_CXX_FLAGS_RELEASE}")

set(CMAKE_C_FLAGS_MINSIZEREL                  "${CMAKE_C_FLAGS_MINSIZEREL}                  ${WELDER_C_CXX_FLAGS_MINSIZEREL}")
set(CMAKE_CXX_FLAGS_MINSIZEREL                "${CMAKE_CXX_FLAGS_MINSIZEREL}                ${WELDER_C_CXX_FLAGS_MINSIZEREL}")

set(CMAKE_STATIC_LINKER_FLAGS                 "${CMAKE_STATIC_LINKER_FLAGS}                 ${WELDER_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS                 "${CMAKE_SHARED_LINKER_FLAGS}                 ${WELDER_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS                 "${CMAKE_MODULE_LINKER_FLAGS}                 ${WELDER_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS                    "${CMAKE_EXE_LINKER_FLAGS}                    ${WELDER_LINKER_FLAGS}")

set(CMAKE_STATIC_LINKER_FLAGS_DEBUG           "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}           ${WELDER_LINKER_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG           "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}           ${WELDER_LINKER_FLAGS_DEBUG}")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG           "${CMAKE_MODULE_LINKER_FLAGS_DEBUG}           ${WELDER_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG              "${CMAKE_EXE_LINKER_FLAGS_DEBUG}              ${WELDER_LINKER_FLAGS_DEBUG}")

set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO}  ${WELDER_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}  ${WELDER_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO}  ${WELDER_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO     "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}     ${WELDER_LINKER_FLAGS_RELWITHDEBINFO}")

set(CMAKE_STATIC_LINKER_FLAGS_RELEASE         "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}         ${WELDER_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE         "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}         ${WELDER_LINKER_FLAGS_RELEASE}")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE         "${CMAKE_MODULE_LINKER_FLAGS_RELEASE}         ${WELDER_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE            "${CMAKE_EXE_LINKER_FLAGS_RELEASE}            ${WELDER_LINKER_FLAGS_RELEASE}")

set(CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL      "${CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL}      ${WELDER_LINKER_FLAGS_MINSIZEREL}")
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL      "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL}      ${WELDER_LINKER_FLAGS_MINSIZEREL}")
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL      "${CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL}      ${WELDER_LINKER_FLAGS_MINSIZEREL}")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL         "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL}         ${WELDER_LINKER_FLAGS_MINSIZEREL}")

#Windows
#Emscripten
#SDLSTDEmpty
#Stub
add_subdirectory(Libraries/Platform/${WELDER_PLATFORM})

add_subdirectory(External)
add_subdirectory(Libraries)
