################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################

# Should be run after all link targets are defined, and all sources are added.
function(zero_target_precompiled_headers aTarget aIntPath aHeaderName aSourceName aSubFolder aIgnoreTarget)

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

  set(pathToSource "${sourceDir}/${aSourceName}")

  set(precompiledObj "${aIntPath}/${aTarget}.pch")

  get_target_property(targetSources ${aTarget} SOURCES)

  foreach (targetSource ${targetSources})
    # if this is a cpp
    if(${targetSource} MATCHES \\.\(cpp|cxx|cc\)$)
      # if it is the precompiled cpp
      if(${targetSource} STREQUAL ${pathToSource})
        set_source_files_properties(${targetSource} PROPERTIES COMPILE_FLAGS
        "/Yc\"${aHeaderName}\" /Fp\"${precompiledObj}\"")
        set_source_files_properties(${targetSource} PROPERTIES OBJECT_OUTPUTS "${precompiledObj}")
        # if it is every other cpp
      else()
        set_source_files_properties(${targetSource} PROPERTIES OBJECT_DEPENDS "${precompiledObj}")
        set_source_files_properties(${targetSource} PROPERTIES COMPILE_FLAGS
                                "/Yu\"${aHeaderName}\" /Fp\"${precompiledObj}\"")
      endif()
    endif()
  endforeach()

  if (${MSVC} OR (CMAKE_GENERATOR_TOOLSET STREQUAL "LLVM-vs2014"))
    target_compile_options(${aTarget} PRIVATE #"/Yc${aHeaderName}"
                                              "/Yu\"${aHeaderName}\""
                                              "/Fp\"${precompiledObj}\"")
    message("Precompiled header added for target: ${aTarget}")                                    
  else()
    message(SEND_ERROR "zero_target_precompiled_headers doesn't currently support anything but MSVC.")
  endif()


  set_property(GLOBAL PROPERTY "${aTarget}_Precompiled_Headers_Enabled" TRUE)

endfunction()
