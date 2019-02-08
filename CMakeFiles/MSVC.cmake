add_definitions(-DPLATFORM_WINDOWS=1 -DPLATFORM_NAME="Windows")
add_definitions(-DCOMPILER_MICROSOFT=1)

add_definitions(-D_UNICODE -DUNICODE)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SECURE_SCL=0 -D_NO_CVCONST_H)

set(WELDER_C_CXX_FLAGS "\
  -W3\
  -Zc:wchar_t\
  -Zc:inline\
  -Zc:forScope\
  -fp:fast\
  -errorReport:prompt\
  -Gd\
  -Oy-\
  -EHsc\
  -nologo\
  -analyze-\
  -bigobj\
")

set(WELDER_C_CXX_FLAGS_DEBUG "\
  -Zi\
  -Gm\
  -MDd\
  -GS\
  -Od\
")

set(WELDER_C_CXX_FLAGS_RELEASE "\
  -Zi\
  -Gm-\
  -MT\
  -MP\
  -GL\
  -GS-\
  -O2\
  -Oi\
")

set(WELDER_LINKER_FLAGS "/ignore:4099,4221,4075,4251")
set(WELDER_LINKER_FLAGS_RELEASE "/LTCG")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO /SUBSYSTEM:WINDOWS /STACK:8388608")

set(WELDER_C_CXX_EXTERNAL_FLAGS /W0 /wd4267)
