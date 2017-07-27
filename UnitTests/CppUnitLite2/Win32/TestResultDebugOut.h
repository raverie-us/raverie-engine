#pragma once

#include "../TestResult.h"

namespace CppUnitLite
{
	class TestResultDebugOut : public TestResult
	{
	public:
		virtual void StartTests ();
		virtual void AddFailure (const Failure & failure);
		virtual void EndTests ();
	};
}
