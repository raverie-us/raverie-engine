#pragma once

#include "TestResult.h"

namespace CppUnitLite
{
	class TestResultStdErr : public TestResult
	{
	public:
		virtual void AddFailure (const Failure & failure);
		virtual void EndTests ();
	};
}
