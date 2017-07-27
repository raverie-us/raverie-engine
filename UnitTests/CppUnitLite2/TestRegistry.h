#pragma once

namespace CppUnitLite
{
	const int MAX_TESTS  = 5000; 

	class Test;
	class TestResult;

	class TestRegistry
	{
	public:
		static TestRegistry& Instance();
		static void Destroy();

		void Add (Test* test);
		void Run (TestResult& result);

	private:
		TestRegistry();

		Test* m_tests[MAX_TESTS];
		int m_testCount;
	};
}
