################################################################################
# Author: Joshua T. Fisher and Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
################################################################################

function(zero_source_group aRoot aTarget aSharedPath) 
  get_target_property(targetBinaryDir ${aTarget} BINARY_DIR)
  get_target_property(targetSourceDir ${aTarget} SOURCE_DIR)
  get_target_property(targetSources ${aTarget} SOURCES)

  # This will determine if the given files are in a folder or not and separate 
  # them on that alone. 
  foreach(aFile ${targetSources}) 
    set(fromSharedPath "")
    file(TO_CMAKE_PATH ${aFile} resultFile) 
    get_filename_component(nameComponent ${resultFile} NAME) 

    set(fullPath ${aRoot}/${nameComponent})

    if(IS_ABSOLUTE ${aFile})
      # do this check only if filepath contains root, otherwise compare shared path
      string(FIND ${aFile} ${aRoot} oStringCompare)

      if (NOT (${oStringCompare} EQUAL -1))
        # It's only safe to call RELATIVE_PATH if the path begins with our "root" dir.
        string(FIND "${aFile}" "${targetSourceDir}" checkIfRelativeToSourceDir)
        string(FIND "${aFile}" "${targetBinaryDir}" checkIfRelativeToBinaryDir)

        if ("${checkIfRelativeToSourceDir}" EQUAL 0)
          file(RELATIVE_PATH relativePath ${targetSourceDir} ${aFile})
        elseif ("${checkIfRelativeToBinaryDir}" EQUAL 0)
          file(RELATIVE_PATH relativePath ${targetBinaryDir} ${aFile})
          set(fullPath ${targetBinaryDir}/${nameComponent})
        endif()
      # we are compared to the passed in shared path
      else()
        if("${aSharedPath}" STREQUAL "")
          message(SEND_ERROR "File with absolute path without matching root passed without a shared path. File Path: ${aFile}\n Root: ${aRoot}\n")
        else()
          set(relativePath ${aSharedPath}/${aTarget}/${nameComponent})
        endif()

      endif()
    else()
      set(relativePath ${aFile})
    endif()

    if(EXISTS ${fullPath})
      set(notInAFolder ${notInAFolder} ${relativePath}) 
    else()
      set(inAFolder ${inAFolder} ${relativePath}) 
    endif() 
  endforeach() 

  # We use a no space prefix with files from folders, otherwise the filters  
  # don't get made. 
  source_group(TREE ${aRoot}
               PREFIX "" 
               FILES ${inAFolder}) 

  # We use a one space prefix with files not in folders, otherwise the files 
  # are put into "Source Files" and "Header Files" filters. 
  source_group(TREE ${aRoot}  
               PREFIX " " 
               FILES ${notInAFolder}) 
endfunction() 

function(zero_subfolder_source_group aRoot restOfPath aTarget aSharedPath)
  set(pathToUse ${aRoot}/${restOfPath}/${aTarget})
  zero_source_group(${pathToUse} ${aTarget} "${aSharedPath}")
  unset(pathToUse)
endfunction()