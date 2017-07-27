#include "TestResultDebugOut.h"
#include "../Failure.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <windows.h>

namespace CppUnitLite
{
	void TestResultDebugOut::StartTests ()
	{
		OutputDebugString("\n\nRunning unit tests...\n\n");
    printf("\n\nRunning unit tests...\n\n");
	}


	void TestResultDebugOut::AddFailure (const Failure & failure) 
	{
		TestResult::AddFailure(failure);

		std::ostringstream oss;
		oss << failure;
		OutputDebugString(oss.str().c_str());
    printf(oss.str().c_str());
	}

	void TestResultDebugOut::EndTests () 
	{
		TestResult::EndTests();

    std::ostringstream oss;

    oss << std::endl;

    if (m_testCount == 0)
    {
      oss << "NO TESTS RUN" << std::endl;
    }
    else
    {
      oss << "  TIME TAKEN: " << std::setprecision(3) << m_secondsElapsed << " seconds" << std::endl;
      oss << "   TESTS RUN: " << m_testCount << std::endl;
      oss << "      PASSED: " << m_testCount - m_failureCount << std::endl;

        // Only print failure if we actually fail.
      if (m_failureCount != 0)
      {
        oss << "      FAILED: " << m_failureCount << std::endl;
      }

      float testGrade  = static_cast<float>(m_testCount - m_failureCount);
            testGrade /= static_cast<float>(m_testCount);
            testGrade *= 100.0f;

      oss << "     OVERALL: " << testGrade << "%";
    }
// 		std::ostringstream oss;
// 		oss << m_testCount << " tests run" << std::endl;
// 		if (m_failureCount > 0)
// 			oss << "****** There were " << m_failureCount << " failures." << std::endl;
// 		else
// 			oss << "There were no test failures." << std::endl;
// 
// 		oss << "Test time: " << std::setprecision(3) << m_secondsElapsed << " seconds." << std::endl;

		OutputDebugString(oss.str().c_str());
		OutputDebugString("\n");
    
    printf(oss.str().c_str());
    printf("\n");
	}
}
