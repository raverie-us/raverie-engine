################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################

# Should be run after all link targets are defined, and all sources are added.
function(zero_target_precompiled_headers aTarget aSubFolder aIgnoreTarget)

  # Currently the only Platforms use this (Windows, Empty, etc.).
  get_target_property(sourceDir ${aTarget} SOURCE_DIR)
  if ("${aSubFolder}" STREQUAL "")
    if (aIgnoreTarget)
      set(sourceDir "${sourceDir}")
    else()
      set(sourceDir "${sourceDir}/${aTarget}")
    endif()
  else()
    if (aIgnoreTarget)
      set(sourceDir "${sourceDir}/${aSubFolder}")
    else()
      set(sourceDir "${sourceDir}/${aSubFolder}/${aTarget}")
    endif()
  endif()

  set(pathToSource "${sourceDir}/Precompiled.cpp")

  get_target_property(targetSources ${aTarget} SOURCES)

  if (MSVC)
    foreach (targetSource ${targetSources})
      # If this is a C++ unit (cpp file).
      if(${targetSource} MATCHES \\.\(cpp|cxx|cc\)$)
        # If it is the Precompiled.cpp:
        if(${targetSource} STREQUAL ${pathToSource})
          set_source_files_properties(${targetSource} PROPERTIES COMPILE_FLAGS "/YcPrecompiled.hpp")
        else()
          set_source_files_properties(${targetSource} PROPERTIES COMPILE_FLAGS "/YuPrecompiled.hpp")
        endif()
      endif()
    endforeach()

    target_compile_options(${aTarget} PRIVATE "/YuPrecompiled.hpp")
  endif()

endfunction()
