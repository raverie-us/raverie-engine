# Most of these helper "functions" are macros because I don't know how to return values from a function

# Iterates through a directory and gets all subdirectory names
macro(BuildSubDirectoryList result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
	  LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
endmacro()

# Finds a library at a specific relative path and adds it to a list
# This is a weird function because it has to take a lot of extra stuff I'd like to eventually remove
macro(AddLibraryToList libraryName libraryRelativePath variableName listName)
	find_library(${variableName} ${libraryName} PATHS ${libraryRelativePath} NO_DEFAULT_PATH)
	list(APPEND ${listName} ${${variableName}})
endmacro()

# Same as above but it is used to find Dlls
macro(AddDllToList dllName dllRelativePath variableName listName)
	find_file(${variableName} ${dllName} PATHS ${dllRelativePath} NO_DEFAULT_PATH)
	list(APPEND ${listName} ${${variableName}})
endmacro()
