#include "../CppUnitLite2.h"
#include <vector>

TEST (MyFirstTest)
{
    int a = 102;
    CHECK_EQUAL (102, a);
}



struct MyFixture
{
    MyFixture() 
    {
        someValue = 2.0f;
        str = "Hello";
    }

    float someValue;
    std::string str;
};




TEST_F (MyFixture, TestCase1)
{
    CHECK_CLOSE (someValue, 2.0f, 0.00001);
    someValue = 13;
}

TEST_F (MyFixture, TestCase2)
{
    CHECK_CLOSE (someValue, 2.0f, 0.00001);
    CHECK_EQUAL (str, "Hello");
}


struct FunctionFixture
{
    FunctionFixture() 
    {
        numberOfFunctionCalls = 0;
    }

    static int numberOfFunctionCalls;
	static void IncrementCalls()
	{
		++numberOfFunctionCalls;
	}
};

int FunctionFixture::numberOfFunctionCalls = 0;

#define GETACTIVETEST()													\
	CppUnitLite::TestResult& result_ = *CppUnitLite::Test::ActiveTest()->GetActiveResult();	\
	const char * m_name = CppUnitLite::Test::ActiveTest()->GetName();			




void TestFunction1()
{	
	GETACTIVETEST();
	
	FunctionFixture::IncrementCalls();

	//TestResult& result_ = FunctionFixture::GetResult();
	//TestResult& result_ = *TestResult::Active();

	//TestResult& result_ = *Test::ActiveTest()->GetActiveResult();
	//const char * m_name = Test::ActiveTest()->GetName();



	CHECK_EQUAL(  1, 1 );


	//std::vector<int> ints;
	//ints.push_back( 1 );
	//ints[5] = 3;


	std::vector<int> ints;
	//ints.push_back( 1 );
	//CHECK_ACTION( ints.back() );

	//ints.back();







}

TEST_F (FunctionFixture, FunctionBasic)
{
	CHECK_EQUAL (FunctionFixture::numberOfFunctionCalls,0);

	TestFunction1();


   	CHECK_EQUAL (FunctionFixture::numberOfFunctionCalls,1);
}