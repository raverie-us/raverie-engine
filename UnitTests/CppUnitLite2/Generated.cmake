################################################################################
# Generated using Joshua T. Fisher's 'CMake Builder'.
# Link: https://github.com/playmer/CmakeBuilder 
################################################################################
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
set(gen_Linux_file_list
    ${CMAKE_CURRENT_LIST_DIR}/Linux/SignalHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Linux/SignalHandler.h
)
endif()
if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
set(gen_Windows_file_list
    ${CMAKE_CURRENT_LIST_DIR}/Win32/TestResultDebugOut.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Win32/TestResultDebugOut.h
)
endif()
target_sources(CppUnitLite2
  PRIVATE
    ${gen_Linux_file_list}
    ${gen_Windows_file_list}
    ${CMAKE_CURRENT_LIST_DIR}/CppUnitLite2.h
    ${CMAKE_CURRENT_LIST_DIR}/ExceptionHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ExceptionHandler.h
    ${CMAKE_CURRENT_LIST_DIR}/Failure.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Failure.h
    ${CMAKE_CURRENT_LIST_DIR}/Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Test.h
    ${CMAKE_CURRENT_LIST_DIR}/TestException.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TestException.h
    ${CMAKE_CURRENT_LIST_DIR}/TestMacros.h
    ${CMAKE_CURRENT_LIST_DIR}/TestRegistry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TestRegistry.h
    ${CMAKE_CURRENT_LIST_DIR}/TestResult.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TestResult.h
    ${CMAKE_CURRENT_LIST_DIR}/TestResultStdErr.cpp
    ${CMAKE_CURRENT_LIST_DIR}/TestResultStdErr.h
    ${CMAKE_CURRENT_LIST_DIR}/Test/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Test/MockTestResult.h
    ${CMAKE_CURRENT_LIST_DIR}/Test/SampleTests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Test/TestExceptionHandling.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Test/TestFixtures.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Test/TestMacros.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Test/TestTestCase.cpp
)
unset(gen_Linux_file_list)
unset(gen_Windows_file_list)
