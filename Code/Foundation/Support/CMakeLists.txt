add_library(Support)

welder_setup_library(Support ${CMAKE_CURRENT_LIST_DIR} TRUE)
welder_use_precompiled_header(Support ${CMAKE_CURRENT_LIST_DIR})

target_sources(Support
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Archive.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Archive.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ChunkReader.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ChunkWriter.hpp
    ${CMAKE_CURRENT_LIST_DIR}/FileConsoleListener.hpp
    ${CMAKE_CURRENT_LIST_DIR}/FileSupport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/FileSupport.hpp
    ${CMAKE_CURRENT_LIST_DIR}/HdrSupport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HdrSupport.hpp
    ${CMAKE_CURRENT_LIST_DIR}/ImageSupport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ImageSupport.hpp
    ${CMAKE_CURRENT_LIST_DIR}/NameValidation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NameValidation.hpp
    ${CMAKE_CURRENT_LIST_DIR}/PartialMatch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PartialMatch.hpp
    ${CMAKE_CURRENT_LIST_DIR}/PngSupport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PngSupport.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Precompiled.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Profiler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Profiler.hpp
    ${CMAKE_CURRENT_LIST_DIR}/StringReplacement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StringReplacement.hpp
    ${CMAKE_CURRENT_LIST_DIR}/SupportStandard.hpp
    ${CMAKE_CURRENT_LIST_DIR}/Urls.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Urls.hpp
)

welder_target_includes(Support
  PUBLIC
    Common
    RendererBase
    Libpng
    ZLib
)

target_link_libraries(Support
  PUBLIC
    Common
    RendererBase
    Zilch
    Platform
    Libpng
    ZLib
)
