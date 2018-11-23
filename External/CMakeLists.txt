################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
#
# The way that this is implemented, adding or changing libraries in exising
# dependencies will require a force generation, and the addition of new deps
# will require modification of this file.
################################################################################
# get our parent directory
get_filename_component(ParentDirectory ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# get our current directory
set(CurrentDirectory ${CMAKE_CURRENT_LIST_DIR})

################################################################################
# Specify our list of StaticExternals (use the same name as the folder in External/)
################################################################################
set(AllPossibleStaticExternals
    CEF
    Curl
    GL
    GLEW
    MemoryDebugger
    MemoryTracker
    Nvtt
    Opus
    WinHid
    SDL 
)

# list of shared
set(AllPossibleSharedExternals
    CEF
    GLEW
    MemoryDebugger
    MemoryTracker
    Nvtt
    SDL
)

if(use_spirv_shared_library AND NOT generate_spirv_projects)
  set(AllPossibleSharedExternals ${AllPossibleSharedExternals} SpirVTools)
endif()

# define empty values for all externals so if they don't exist they will just result in an empty string
foreach(possibleStaticLib ${AllPossibleStaticExternals})
    set(${possibleStaticLib}Static "")
    set(${possibleStaticLib}Static "" PARENT_SCOPE)
    set(${possibleStaticLib}Headers "" PARENT_SCOPE)

    #variable to easily check if an external exists
    set(${possibleStaticLib}_exists OFF)
endforeach()

foreach(possibleSharedLib ${AllPossibleSharedExternals})
    set(${possibleSharedLib}Shared "")
    set(${possibleSharedLib}Shared "" PARENT_SCOPE)
    set(${possibleSharedLib}Headers "" PARENT_SCOPE)

    #variable to easily check if an external exists
    set(${possibleSharedLib}_exists OFF)
endforeach()


#endif()
################################################################################
# specify the include directories for each of the dependencies.
# use parent scope so the folders are accessible anywhere
################################################################################

#foreach(externLib ${AllPossibleStaticExternals})
#    set(${externLib}Headers "" PARENT_SCOPE)
#endforeach()

#foreach(externLib ${StaticExternals})
#  set(${externLib}Headers ${CurrentDirectory}/${externLib}/include PARENT_SCOPE)
#  set_target_properties(glfwStatic PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_LIST_DIR}/include)
#endforeach()

################################################################################
# Set path to library folders
################################################################################

# set lib paths
set(CEFPath ${CurrentDirectory}/CEF)
set(CurlPath ${CurrentDirectory}/Curl)
set(GLPath ${CurrentDirectory}/GL)
set(GLEWPath ${CurrentDirectory}/GLEW)
set(MemoryDebuggerPath ${CurrentDirectory}/MemoryDebugger)
set(MemoryTrackerPath ${CurrentDirectory}/MemoryTracker)
set(NvttPath ${CurrentDirectory}/Nvtt)
set(OpusPath ${CurrentDirectory}/Opus)
set(WinHidPath ${CurrentDirectory}/WinHid)
set(SDLPath ${CurrentDirectory}/SDL)
set(SpirVToolsPath ${CurrentDirectory}/SpirVTools)

################################################################################
# Find all libraries from listed dependencies and make them globally available
################################################################################

foreach(libStatic ${StaticExternals})
    #variable to easily check if an external exists
    set(${libStatic}_exists ON)

    # get lists of libraries
    if ("${bit}" STREQUAL "")
        file(GLOB ${libStatic}StaticPath ${${libStatic}Path}/lib/${platform}/*.*)
        file(GLOB ${libStatic}StaticDebugPath ${${libStatic}Path}/lib/${platform}/Debug/*.*)
        file(GLOB ${libStatic}StaticReleasePath ${${libStatic}Path}/lib/${platform}/Release/*.*)
    else()
        file(GLOB ${libStatic}StaticPath ${${libStatic}Path}/lib/${platform}/${bit}/*.*)
        file(GLOB ${libStatic}StaticDebugPath ${${libStatic}Path}/lib/${platform}/${bit}/Debug/*.*)
        file(GLOB ${libStatic}StaticReleasePath ${${libStatic}Path}/lib/${platform}/${bit}/Release/*.*)
    endif()

    if ("${${libStatic}StaticPath}"        STRGREATER "" 
     OR "${${libStatic}StaticDebugPath}"   STRGREATER ""
     OR "${${libStatic}StaticReleasePath}" STRGREATER "")
        #set(${libStatic}Static)
        # COMMON
        foreach(libPath ${${libStatic}StaticPath})
            #get name of library
            get_filename_component(libName ${libPath} NAME_WE)

            
            #create library using that name
            #insert into list of libraries under this name
            add_library(${libName} STATIC IMPORTED GLOBAL) 

            set_target_properties(
                ${libName} 
                PROPERTIES
                IMPORTED_LOCATION ${libPath}
            )

            set_target_properties(
                ${libName}
                PROPERTIES 
                INTERFACE_INCLUDE_DIRECTORIES ${CurrentDirectory}/${libStatic}/include
            )

            set(${libStatic}Static ${${libStatic}Static} ${libName})
            set(${libStatic}Static ${${libStatic}Static} PARENT_SCOPE)
        endforeach()

        # DEBUG
        foreach(libPath ${${libStatic}StaticDebugPath})
            #get name of library
            get_filename_component(libNameDebug ${libPath} NAME_WE)
            set(libNameDebug ${libNameDebug}Debug)

            #create library using that name
            #insert into list of libraries under this name
            add_library(${libNameDebug} STATIC IMPORTED GLOBAL) 

            set_target_properties(
                ${libNameDebug}
                PROPERTIES
                IMPORTED_LOCATION ${libPath}
            )

            set_target_properties(
                ${libNameDebug}
                PROPERTIES 
                INTERFACE_INCLUDE_DIRECTORIES ${CurrentDirectory}/${libStatic}/include
            )

            set(${libStatic}Static ${${libStatic}Static} $<$<CONFIG:Debug>:${libNameDebug}>)
            set(${libStatic}Static ${${libStatic}Static} PARENT_SCOPE)
            set(${libStatic}StaticDebug ${${libStatic}StaticDebug} ${libNameDebug})
            set(${libStatic}StaticDebug ${${libStatic}StaticDebug} PARENT_SCOPE)
        endforeach()

        # RELEASE
        foreach(libPath ${${libStatic}StaticReleasePath})
            #get name of library
            get_filename_component(libNameRelease ${libPath} NAME_WE)
            set(libNameRelease ${libNameRelease}Release)

            #create library using that name
            #insert into list of libraries under this name
            add_library(${libNameRelease} STATIC IMPORTED GLOBAL) 

            set_target_properties(
                ${libNameRelease} 
                PROPERTIES
                IMPORTED_LOCATION ${libPath}
            )

            # add the include directory for the headers
            set_target_properties(
                ${libNameRelease}
                PROPERTIES 
                INTERFACE_INCLUDE_DIRECTORIES ${CurrentDirectory}/${libStatic}/include
            )

            set(${libStatic}Static ${${libStatic}Static} $<$<CONFIG:Release>:${libNameRelease}>)
            set(${libStatic}Static ${${libStatic}Static} PARENT_SCOPE)
            set(${libStatic}StaticRelease ${${libStatic}StaticRelease} ${libNameRelease})
            set(${libStatic}StaticRelease ${${libStatic}StaticRelease} PARENT_SCOPE)
        endforeach()
    else()
        message("THE PATH: ${${libStatic}Path}/lib/${platform}/${bit}/")
        message("${${libStatic}StaticPath}")
        message(FATAL_ERROR "Unable to find any static libraries in the '${libStatic}/lib' folder in the External folder.\
         \nCheck specified path in the CMakeLists.txt file in the 'External' folder of ZeroCore.")
    endif()
endforeach()

foreach(libShared ${SharedExternals})
    # set easy variable for seeing if a lib exists
    set(${libShared}_exists ON)

    # get list of shared libraries
    file(GLOB ${libShared}SharedPath ${${libShared}Path}/bin/*.dll)

    # add the path to parent scope since we are gonna have to move them to executable directory
    set(${libShared}SharedPath ${${libShared}SharedPath} PARENT_SCOPE)

    # make sure we actually found anything
    if ("${${libShared}SharedPath}" STRGREATER "")

        set(${libShared}Shared)

        foreach(libPath ${${libShared}SharedPath})
            get_filename_component(libName ${libPath} NAME_WE)
            set(libName ${libName}Shared)

            add_library(${libName} SHARED IMPORTED GLOBAL)

            set_target_properties(
                ${libName} PROPERTIES
                IMPORTED_LOCATION ${libPath}
            )

            # add the include directory for the headers
            set_target_properties(
                ${libName}
                PROPERTIES 
                INTERFACE_INCLUDE_DIRECTORIES ${CurrentDirectory}/${libShared}/include
            )


            # set the libraries that can be added by targets
            set(${libShared}Shared ${${libShared}Shared} ${libName})
            set(${libShared}Shared ${libShared}Shared PARENT_SCOPE)

        endforeach()

    else()
        message("DLL PATH: ${${libShared}Path}/bin/*.dll")
        message("${${libShared}SharedPath}")
        message(FATAL_ERROR "Unable to find any shared libraries in the '${libShared}/bin' folder in the External folder.\
        \n Check specified path in the CMakeLists.txt file in the 'External' folder of ZeroCore.")
    endif()
endforeach()

if (${platform} STREQUAL Windows)
    # set any platform libraries here
    set(PlatformSoundLibrary avrt.lib PARENT_SCOPE)
endif()
