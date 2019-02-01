add_definitions(-DPLATFORM_WINDOWS=1 -DPLATFORM_HARDWARE=1 -DPLATFORM_BITS="32" -DPLATFORM_NAME="Windows")
add_definitions(-DCOMPILER_MICROSOFT=1)

add_definitions(-D_UNICODE -DUNICODE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SECURE_SCL=0 -D_NO_CVCONST_H)

set(WELDER_COMPILE_OPTIONS
    -W3
    -WX
    # Ignore truncation warnings.
    -wd"4302"
    # Libpng uses fopen and other 'deprecated' functions.
    -wd"4996"
    # FreeType compares signed/unsigned numbers.
    -wd"4018"
    # FreeType has unreferenced labels.
    -wd"4102"
    # Opus converts from double to float implicitly.
    -wd"4244"
    # Assimp uses this in base initializer.
    -wd"4355"
    -Zc:wchar_t
    -Zc:inline
    -Zc:forScope
    -fp:fast
    -errorReport:prompt
    -Gd
    -Oy-
    -EHsc
    -nologo
    -analyze-
    $<$<CONFIG:Debug>:-Zi>
    $<$<CONFIG:Debug>:-Gm>
    $<$<CONFIG:Debug>:-MDd>
    $<$<CONFIG:Debug>:-GS>
    $<$<CONFIG:Debug>:-Od>
    $<$<CONFIG:Release>:-Zi>
    $<$<CONFIG:Release>:-Gm->
    $<$<CONFIG:Release>:-MT>
    $<$<CONFIG:Release>:-MP>
    $<$<CONFIG:Release>:-GL>
    $<$<CONFIG:Release>:-GS->
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-Oi>
    $<$<CONFIG:Release>:-arch:SSE2>
)

set(WELDER_STATIC_LIBRARY_FLAGS "/ignore:4099,4221,4075,4251")
set(WELDER_STATIC_LIBRARY_RELEASE "/LTCG")
set(WELDER_LINK_FLAGS "${WELDER_STATIC_LIBRARY_FLAGS} /SAFESEH:NO /SUBSYSTEM:WINDOWS /STACK:8388608")
