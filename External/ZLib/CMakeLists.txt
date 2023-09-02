add_library(ZLib)

raverie_setup_library(ZLib ${CMAKE_CURRENT_LIST_DIR} FALSE)

target_sources(ZLib
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Repo/adler32.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/compress.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/crc32.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/crc32.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/deflate.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/deflate.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/gzclose.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/gzguts.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/gzlib.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/gzread.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/gzwrite.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/infback.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/inffast.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/inffast.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/inffixed.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/inflate.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/inflate.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/inftrees.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/inftrees.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/trees.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/trees.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/uncompr.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/zconf.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/zlib.h
    ${CMAKE_CURRENT_LIST_DIR}/Repo/zutil.c
    ${CMAKE_CURRENT_LIST_DIR}/Repo/zutil.h
)

target_include_directories(ZLib
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Repo/
)

if(WIN32)
	set_property(TARGET "ZLib" PROPERTY FOLDER "External")
endif()
