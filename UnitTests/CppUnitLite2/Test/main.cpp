#include "../CppUnitLite2.h"
#include "../TestResultStdErr.h"

#include "../Win32\TestResultDebugOut.h"

int __cdecl myReportHook( int reportType, char *message, int *returnValue )
{
    (void)returnValue;
    switch(reportType)
    {
    case _CRT_ASSERT:
		throw CppUnitLite::TestException( __FILE__, 0 , message );
    }
    return 0;
}


int main()
{    
	_CrtSetReportHook2( 0 , myReportHook );

	//CppUnitLite::TestResultStdErr result;
	CppUnitLite::TestResultDebugOut result;

	CppUnitLite::TestRegistry::Instance().Run(result);
    return (result.FailureCount());
}

