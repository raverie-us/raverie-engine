cmake_minimum_required(VERSION 3.9.1 FATAL_ERROR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)
add_definitions(-D$<$<CONFIG:Debug>:DEBUG>$<$<CONFIG:Release>:NDEBUG>$<$<CONFIG:RelWithDebInfo>:NDEBUG>)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT WelderEditor)

project(Welder)

set(WELDER_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WELDER_CMAKE_DIR ${WELDER_CORE_DIR}/CMakeFiles/)
set(WELDER_LIBRARIES_DIR ${WELDER_CORE_DIR}/Libraries/)
set(WELDER_PLATFORM_DATA_DIR ${WELDER_CORE_PATH}/PlatformData/${platform})

function(welder_setup_library target directory)
  target_include_directories(${target} PUBLIC ${directory} ${WELDER_LIBRARIES_DIR})
  set_target_properties(
    ${target}
    PROPERTIES
    STATIC_LIBRARY_FLAGS "${WELDER_STATIC_LIBRARY_FLAGS}"
    STATIC_LIBRARY_FLAGS_RELEASE "${WELDER_STATIC_LIBRARY_RELEASE}"
    LINK_FLAGS "${WELDER_LINK_FLAGS}"
  )
  target_compile_options(${target} PUBLIC "${WELDER_COMPILE_OPTIONS}")
endfunction()

function(welder_use_precompiled_header target directory)
  if(MSVC)
    target_compile_options(${target} PRIVATE "/FIPrecompiled.hpp")
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/YuPrecompiled.hpp")
    set_source_files_properties(${directory}/Precompiled.cpp PROPERTIES COMPILE_FLAGS "/YcPrecompiled.hpp")
  else()
    #add_definitions(-include Precompiled.hpp)
  endif()
endfunction()

function(welder_source_ignore_precompiled_header source)
  if(MSVC)
    set_source_files_properties(${source} PROPERTIES COMPILE_FLAGS "/Y-")
  else()
  endif()
endfunction()

function(welder_get_all_link_libraries outputList TARGET)
    list(APPEND visitedTargets ${TARGET})
    get_target_property(linkedLibs ${TARGET} INTERFACE_LINK_LIBRARIES)
    set(allLinkedLibs "")
    foreach(linkedLib ${linkedLibs})
        if (TARGET ${linkedLib})
            list(FIND visitedTargets ${linkedLib} VISITED)
            if (${VISITED} EQUAL -1)
                welder_get_all_link_libraries(childLinkedLibs ${linkedLib})
                list(APPEND allLinkedLibs ${linkedLib} ${childLinkedLibs})
            endif()
        endif()
    endforeach()
    set(visitedTargets ${visitedTargets} PARENT_SCOPE)
    set(${outputList} ${allLinkedLibs} PARENT_SCOPE)
endfunction()

function(welder_copy_from_linked_libraries target)
  welder_get_all_link_libraries(libraries ${target})
  foreach(library ${libraries})
    set(directory "${${library}CopyDirectories}")
    if(directory)
      add_custom_command(TARGET ${target} POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${directory} $<TARGET_FILE_DIR:${target}>)
    endif()
  endforeach()
endfunction()

if (EMSCRIPTEN)
  include(${WELDER_CMAKE_DIR}/Clang_Emscripten_Emscripten.cmake)
else()
  include(${WELDER_CMAKE_DIR}/MSVC_Windows_Windows.cmake)
endif()

add_subdirectory(External)
add_subdirectory(Libraries)
