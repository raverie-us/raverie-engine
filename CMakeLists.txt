cmake_minimum_required(VERSION 3.22.1 FATAL_ERROR)

set(CMAKE_CONFIGURATION_TYPES Debug RelWithDebInfo Release MinSizeRel)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RaverieEditor)

find_program(RAVERIE_CCACHE_PATH ccache)
if(RAVERIE_CCACHE_PATH)
  message(STATUS "Found ccache at ${RAVERIE_CCACHE_PATH}")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${RAVERIE_CCACHE_PATH}")
endif()

project(Raverie)
set(CMAKE_EXECUTABLE_SUFFIX ".wasm")

add_definitions(-DRaverieMsSinceEpoch=${RAVERIE_MS_SINCE_EPOCH})
add_definitions(-DRaverieBranchName="${RAVERIE_BRANCH}")
add_definitions(-DRaverieRevisionId=${RAVERIE_REVISION})
add_definitions(-DRaverieShortChangeSet=${RAVERIE_SHORT_CHANGESET})
add_definitions(-DRaverieChangeSet=${RAVERIE_CHANGESET})
add_definitions(-DRaverieChangeSetDate=${RAVERIE_CHANGESET_DATE})
add_definitions(-DRaverieChangeSetDate=${RAVERIE_CHANGESET_DATE})
add_definitions(-DRaverieMajorVersion=${RAVERIE_MAJOR_VERSION})
add_definitions(-DRaverieMinorVersion=${RAVERIE_MINOR_VERSION})
add_definitions(-DRaveriePatchVersion=${RAVERIE_PATCH_VERSION})
add_definitions(-DRaverieConfigName="${RAVERIE_CONFIG}")

add_definitions(-D_WASI_EMULATED_SIGNAL)
add_definitions(-D_WASI_EMULATED_PROCESS_CLOCKS)

option(RAVERIE_EXCEPTIONS "Enable exception throwing/catching" ON)
if (RAVERIE_EXCEPTIONS)
  add_definitions(-DRaverieExceptions)
endif()

set(RAVERIE_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(RAVERIE_CMAKE_DIR ${RAVERIE_CORE_DIR}/CMakeFiles/)
set(RAVERIE_LIBRARIES_DIR ${RAVERIE_CORE_DIR}/Code/)

function(raverie_setup_library target directory internal)
  target_include_directories(${target} PUBLIC ${directory} ${RAVERIE_LIBRARIES_DIR})
endfunction()

function(raverie_get_all_dependencies outputList TARGET)
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
                raverie_get_all_dependencies(childLinkedLibs ${dependency})
                list(APPEND allLinkedLibs ${dependency} ${childLinkedLibs})
            endif()
        endif()
    endforeach()
    set(visitedTargets ${visitedTargets} PARENT_SCOPE)
    set(${outputList} ${allLinkedLibs} PARENT_SCOPE)
endfunction()

function(raverie_target_includes target)
  set(multiValueArgs PUBLIC)
  cmake_parse_arguments(RAVERIE "" "" "${multiValueArgs}" "${ARGN}")

  foreach(fromTarget ${RAVERIE_PUBLIC})
  target_include_directories(${target} PUBLIC $<TARGET_PROPERTY:${fromTarget},INTERFACE_INCLUDE_DIRECTORIES>)
  target_compile_definitions(${target} PUBLIC $<TARGET_PROPERTY:${fromTarget},INTERFACE_COMPILE_DEFINITIONS>)
  endforeach()
endfunction()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(RAVERIE_C_CXX_FLAGS "\
  -Wno-address-of-packed-member\
  -Wno-empty-body\
  -fexceptions\
  -fwasm-exceptions\
  -frtti\
  -fno-vectorize\
  -fno-slp-vectorize\
  -fno-tree-vectorize\
")

set(RAVERIE_C_CXX_FLAGS_DEBUG "\
  -g\
")

set(RAVERIE_C_CXX_FLAGS_RELWITHDEBINFO "\
  -O3\
  -g\
")

set(RAVERIE_C_CXX_FLAGS_RELEASE "\
  -O3\
")

set(RAVERIE_C_CXX_FLAGS_MINSIZEREL "\
  -Oz\
")

set(RAVERIE_C_CXX_EXTERNAL_FLAGS -Wno-everything)


set(CMAKE_C_FLAGS                             "${CMAKE_C_FLAGS}                             ${RAVERIE_C_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS                           "${CMAKE_CXX_FLAGS}                           ${RAVERIE_C_CXX_FLAGS}")

set(CMAKE_C_FLAGS_DEBUG                       "${CMAKE_C_FLAGS_DEBUG}                       ${RAVERIE_C_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG                     "${CMAKE_CXX_FLAGS_DEBUG}                     ${RAVERIE_C_CXX_FLAGS_DEBUG}")

set(CMAKE_C_FLAGS_RELWITHDEBINFO              "${CMAKE_C_FLAGS_RELWITHDEBINFO}              ${RAVERIE_C_CXX_FLAGS_RELWITHDEBINFO}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO            "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}            ${RAVERIE_C_CXX_FLAGS_RELWITHDEBINFO}")

set(CMAKE_C_FLAGS_RELEASE                     "${CMAKE_C_FLAGS_RELEASE}                     ${RAVERIE_C_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELEASE                   "${CMAKE_CXX_FLAGS_RELEASE}                   ${RAVERIE_C_CXX_FLAGS_RELEASE}")

set(CMAKE_C_FLAGS_MINSIZEREL                  "${CMAKE_C_FLAGS_MINSIZEREL}                  ${RAVERIE_C_CXX_FLAGS_MINSIZEREL}")
set(CMAKE_CXX_FLAGS_MINSIZEREL                "${CMAKE_CXX_FLAGS_MINSIZEREL}                ${RAVERIE_C_CXX_FLAGS_MINSIZEREL}")

set(CMAKE_STATIC_LINKER_FLAGS                 "${CMAKE_STATIC_LINKER_FLAGS}                 ${RAVERIE_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS                 "${CMAKE_SHARED_LINKER_FLAGS}                 ${RAVERIE_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS                 "${CMAKE_MODULE_LINKER_FLAGS}                 ${RAVERIE_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS                    "${CMAKE_EXE_LINKER_FLAGS}                    ${RAVERIE_LINKER_FLAGS}")

set(CMAKE_STATIC_LINKER_FLAGS_DEBUG           "${CMAKE_STATIC_LINKER_FLAGS_DEBUG}           ${RAVERIE_LINKER_FLAGS_DEBUG}")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG           "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}           ${RAVERIE_LINKER_FLAGS_DEBUG}")
set(CMAKE_MODULE_LINKER_FLAGS_DEBUG           "${CMAKE_MODULE_LINKER_FLAGS_DEBUG}           ${RAVERIE_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG              "${CMAKE_EXE_LINKER_FLAGS_DEBUG}              ${RAVERIE_LINKER_FLAGS_DEBUG}")

set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO}  ${RAVERIE_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}  ${RAVERIE_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO  "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO}  ${RAVERIE_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO     "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}     ${RAVERIE_LINKER_FLAGS_RELWITHDEBINFO}")

set(CMAKE_STATIC_LINKER_FLAGS_RELEASE         "${CMAKE_STATIC_LINKER_FLAGS_RELEASE}         ${RAVERIE_LINKER_FLAGS_RELEASE}")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE         "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}         ${RAVERIE_LINKER_FLAGS_RELEASE}")
set(CMAKE_MODULE_LINKER_FLAGS_RELEASE         "${CMAKE_MODULE_LINKER_FLAGS_RELEASE}         ${RAVERIE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE            "${CMAKE_EXE_LINKER_FLAGS_RELEASE}            ${RAVERIE_LINKER_FLAGS_RELEASE}")

set(CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL      "${CMAKE_STATIC_LINKER_FLAGS_MINSIZEREL}      ${RAVERIE_LINKER_FLAGS_MINSIZEREL}")
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL      "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL}      ${RAVERIE_LINKER_FLAGS_MINSIZEREL}")
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL      "${CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL}      ${RAVERIE_LINKER_FLAGS_MINSIZEREL}")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL         "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL}         ${RAVERIE_LINKER_FLAGS_MINSIZEREL}")

add_subdirectory(External)
add_subdirectory(Code)
