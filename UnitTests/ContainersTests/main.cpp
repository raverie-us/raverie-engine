#include "CppUnitLite2/CppUnitLite2.h"
#include "CppUnitLite2/TestResultStdErr.h"
#include "CppUnitLite2/Win32/TestResultDebugOut.h"
#include "Diagnostic/Diagnostic.hpp"
#include "Memory/Stack.hpp"

#if VLD
#include <vld.h>
#endif

int __cdecl UnitTestReportHook( int reportType, char *message, int *returnValue )
{
  (void)returnValue;
  switch(reportType)
  {
  case _CRT_ASSERT:
    throw CppUnitLite::TestException( __FILE__, 0 , message );
  }
  return 0;
}

bool UnitTestErrorHandler(Zero::ErrorSignaler::ErrorData& errorData) 
{
  throw CppUnitLite::TestException( errorData.File , errorData.Line , errorData.Message );
  return true;
}

int main()
{

  int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(tmpDbgFlag);
  _CrtSetReportHook2( 0 , UnitTestReportHook );


  //CppUnitLite::TestResultStdErr result;
  CppUnitLite::TestResultDebugOut result;

  CppUnitLite::TestRegistry::Instance().Run(result); 
  CppUnitLite::TestRegistry::Destroy();

  Zero::Memory::Shutdown();
  return (result.FailureCount());
}