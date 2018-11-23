################################################################################
# Author: Joshua Shlemmer
# Copyright 2017, DigiPen Institute of Technology
# MSVC flags (These also work when generating with LLVN_vs2014)
################################################################################

set(common_flags     
    $<$<CONFIG:Debug>:-GS>
    $<$<CONFIG:Release>:-MP>
    $<$<CONFIG:Release>:-GS->
    $<$<CONFIG:Release>:-GL>
    -analyze-
    -W3 
    -wd"4302"
    -wd"4996"
    -Zc:wchar_t
    $<$<CONFIG:Debug>:-Zi>
    $<$<CONFIG:Release>:-Zi>
    $<$<CONFIG:Debug>:-Gm>
    $<$<CONFIG:Release>:-Gm->
    $<$<CONFIG:Debug>:-Od>
    $<$<CONFIG:Release>:-O2>
    -Zc:inline 
    -fp:fast 
    -errorReport:prompt 
    -WX 
    -Zc:forScope
    -Gd
    $<$<CONFIG:Release>:-arch:SSE2>
    -Oy-
    $<$<CONFIG:Debug>:-MDd>
    $<$<CONFIG:Release>:-Oi>
    $<$<CONFIG:Release>:-MT>
    -EHsc 
    -nologo
    $<$<CONFIG:Debug>:-DDEBUG>
    $<$<CONFIG:Release>:-DNDEBUG>
    "-D \"_CRT_SECURE_NO_WARNINGS\"" 
)

set(common_library_flags "/ignore:4099,4221,4075")
set(common_linker_flags "${common_library_flags} /SAFESEH:NO /SUBSYSTEM:WINDOWS /STACK:8388608")