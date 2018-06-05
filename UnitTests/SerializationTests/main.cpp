#include "CppUnitLite2/CppUnitLite2.h"
#include "CppUnitLite2/TestResultStdErr.h"
#include "CppUnitLite2/Win32/TestResultDebugOut.h"

#include "Geometry/GeometryStandard.hpp"
#include "Serialization/SerializationStandard.hpp"

#include "Platform/Windows/Windows.hpp"
//#include "Windows.hpp"

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

class VisualStudioConsoleListener : public Zero::ConsoleListener
{
  void Print(Zero::FilterType filterType, cstr message)
  {
    OutputDebugStringA(message);
  }
};


int main()
{
	Zero::ErrorSignaler::SetErrorHandler(UnitTestErrorHandler);

  VisualStudioConsoleListener vs;
  Zero::Console::Add(&vs);


	int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpDbgFlag);
	_CrtSetReportHook2( 0 , UnitTestReportHook );


	//CppUnitLite::TestResultStdErr result;
	CppUnitLite::TestResultDebugOut result;

	CppUnitLite::TestRegistry::Instance().Run(result); 
	CppUnitLite::TestRegistry::Destroy();

	Zero::Memory::Root::PrintAll();
	Zero::Memory::Root::Shutdown();



	return (result.FailureCount());
}