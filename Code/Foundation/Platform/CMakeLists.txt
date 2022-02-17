if(${WELDER_PLATFORM} STREQUAL "Windows")
  include(Windows/Platform.cmake)
elseif(${WELDER_PLATFORM} STREQUAL "Emscripten")
  include(Emscripten/Platform.cmake)
elseif(${WELDER_PLATFORM} STREQUAL "SDLSTDEmpty")
  include(SDLSTDEmpty/Platform.cmake)
else()
  include(Stub/Platform.cmake)
endif()

if(WIN32)
	set_property(TARGET Platform PROPERTY FOLDER Foundation)
endif()

