#pragma once

#include "ExceptionHandler.h"
#include "TestException.h"
#include "TestMacros.h"
#include "TestResult.h"
 

#define __CONVERT_TO_STR__( x ) #x
#define __TO_STR__( x ) __CONVERT_TO_STR__( x )
#define __MAKE_WARNING__ __FILE__ "(" __TO_STR__( __LINE__ ) ") : warning: "
#define StubbedTest( x ) \
  __pragma(message(__MAKE_WARNING__ #x " hasn't been implemented yet."))

// #define DONT_CHECK_ASSERTS     1

namespace CppUnitLite
{
	class Test
	{
	public:
		Test (const char * testName,const char * filename, int linenumber);
		virtual ~Test();
		virtual void Run (TestResult& result );
		TestResult * GetActiveResult(){ return m_activeTestResult; }
		const char * GetName(){ return m_name; }

		static Test* ActiveTest();

	protected:
		virtual void RunTest (TestResult& result) = 0;    
		const char * m_name;
		const char * m_filename;
		const int m_linenumber;
		TestResult * m_activeTestResult;

	private:
		//Tests can not be copied
		Test(const Test &);
		Test& operator=(const Test &);
		static Test * ActiveTestInstance;
	};

	class TestRegistrar
	{
	public:
		TestRegistrar(Test * test);   
	};
}

#define TEST(test_name)																		\
	class test_name##Test : public CppUnitLite::Test										\
    {																						\
        public:																				\
            test_name##Test () : Test (#test_name "Test", __FILE__, __LINE__){}				\
        protected:																			\
		virtual void RunTest (CppUnitLite::TestResult& result_);							\
    } test_name##Instance;																	\
    CppUnitLite::TestRegistrar test_name##_registrar (&test_name##Instance);				\
    void test_name##Test::RunTest (CppUnitLite::TestResult& result_)


// Test with fixture
#define TEST_F(fixture, test_name)															\
    struct fixture##test_name : public fixture {											\
        fixture##test_name(const char * name_) : m_name(name_) {}							\
        void test_name(CppUnitLite::TestResult& result_);									\
        const char * m_name;																\
    };																						\
	class fixture##test_name##Test : public CppUnitLite::Test								\
    {																						\
        public:																				\
            fixture##test_name##Test ()														\
                : Test (#test_name "Test", __FILE__, __LINE__) {}							\
        protected:																			\
            virtual void RunTest (CppUnitLite::TestResult& result_);						\
    } fixture##test_name##Instance;															\
    CppUnitLite::TestRegistrar fixture##test_name##_registrar (&fixture##test_name##Instance);\
    void fixture##test_name##Test::RunTest (CppUnitLite::TestResult& result_) {				\
        fixture##test_name mt(m_name);														\
        mt.test_name(result_);																\
    }																						\
    void fixture ## test_name::test_name(CppUnitLite::TestResult& result_)


// Test with fixture with construction parameters
#define TEST_FP(fixture, fixture_construction, test_name)									\
    struct fixture##test_name : public fixture {											\
        fixture##test_name(const char * name_) : fixture_construction, m_name(name_) {}		\
        void test_name(CppUnitLite::TestResult& result_);									\
        const char * m_name;																\
    };																						\
	class fixture##test_name##Test : public CppUnitLite::Test								\
    {																						\
        public:																				\
            fixture##test_name##Test ()														\
                : Test (#test_name "Test", __FILE__, __LINE__) {}							\
        protected:																			\
            virtual void RunTest (CppUnitLite::TestResult& result_);						\
    } fixture##test_name##Instance;															\
    CppUnitLite::TestRegistrar fixture##test_name##_registrar (&fixture##test_name##Instance);\
    void fixture##test_name##Test::RunTest (CppUnitLite::TestResult& result_) {				\
        fixture##test_name mt(m_name);														\
        mt.test_name(result_);																\
    }																						\
    void fixture ## test_name::test_name(CppUnitLite::TestResult& result_)
    
    

#define CHECK(condition)																\
    do {																				\
        try {																			\
            if (!(condition))															\
				result_.AddFailure (CppUnitLite::Failure (#condition, m_name, __FILE__, __LINE__));	\
        } catch( const CppUnitLite::TestException& Edge ) {												\
		CppUnitLite::ExceptionHandler::Handle(result_, Edge, m_name, __FILE__, __LINE__);			\
        } catch(...) {																	\
            CppUnitLite::ExceptionHandler::Handle(result_, #condition, m_name, __FILE__, __LINE__);	\
        }																				\
    } while (0)

#define GETACTIVETEST()													\
	CppUnitLite::TestResult& result_ = *CppUnitLite::Test::ActiveTest()->GetActiveResult();	\
	const char * m_name = CppUnitLite::Test::ActiveTest()->GetName();			

#define CHECK_EQUAL(expected,actual)														\
	do																						\
	{																						\
		CppUnitLite::CheckEqual( result_, expected, actual, __FILE__, __LINE__, m_name );	\
	} while( 0 )																			\

#define CHECK_CLOSE(expected,actual,kEpsilon)														\
	do {																							\
		CppUnitLite::CheckClose( result_, expected, actual, kEpsilon, __FILE__, __LINE__, m_name );	\
	} while(0)


#define CHECK_STRING_EQUAL(expected,actual)	                                              \
  do																				                                              \
  {																					                                              \
  CppUnitLite::CheckStringEqual( result_, expected, actual, __FILE__, __LINE__, m_name );	\
} while( 0 )		

#define CHECK_ARRAY_EQUAL(expected,actual,count)														\
	do {																										\
		CppUnitLite::CheckArrayEqual( result_, expected, actual, count, __FILE__, __LINE__, m_name );	\
	} while(0)

#define CHECK_ARRAY_CLOSE(expected,actual,count,kEpsilon)														\
	do {																										\
		CppUnitLite::CheckArrayClose( result_, expected, actual, count, kEpsilon, __FILE__, __LINE__, m_name );	\
	} while(0)

#define CHECK_ARRAY2D_CLOSE(expected,actual,rows,columns,kEpsilon)															\
	do {																													\
		CppUnitLite::CheckArrayClose2D( result_, expected, actual, rows, columns, kEpsilon, __FILE__, __LINE__, m_name );	\
	} while(0)

#define CHECK_MATRIX_EQUAL(expected,actual,dim)															\
	do {																													\
		CppUnitLite::CheckMatrixEqual( result_, expected, actual, dim, __FILE__, __LINE__, m_name );	\
	} while(0)

#define CHECK_MATRIX_CLOSE(expected,actual,dim,kEpsilon)														\
	do {																										\
		CppUnitLite::CheckMatrixClose( result_, expected, actual, dim, kEpsilon, __FILE__, __LINE__, m_name );	\
	} while(0)

#define	CHECK_ACTION(action)																	\
	do { 																						\
		try	{ 																					\
			action;																				\
		} catch(const TestException& Edge) {														\
			CppUnitLite::ExceptionHandler::Handle(result_, Edge, m_name, __FILE__, __LINE__); 					\
		} 																						\
		catch (...) {																			\
			CppUnitLite::ExceptionHandler::Handle(result_, #action, m_name, __FILE__, __LINE__);				\
		}																						\
	} while (0)

///Added to help me identify which tests weren't done yet.
#define MARK_AS_UNIMPLEMENTED()                                               \
  
//result_.AddFailure(CppUnitLite::Failure(NULL, m_name, __FILE__, __LINE__))


///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 2 and compares
///values exactly
#define CHECK_VEC2(expected,actual)\
  CHECK_ARRAY_EQUAL(expected,actual,2)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 2
#define CHECK_VEC2_CLOSE(expected,actual,kEpsilon)\
  CHECK_ARRAY_CLOSE(expected,actual,2,kEpsilon)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 3 and compares
///values exactly
#define CHECK_VEC3(expected,actual)\
  CHECK_ARRAY_EQUAL(expected,actual,3)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 3
#define CHECK_VEC3_CLOSE(expected,actual,kEpsilon)\
  CHECK_ARRAY_CLOSE(expected,actual,3,kEpsilon)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 4 and compares
///values exactly
#define CHECK_VEC4(expected,actual)\
  CHECK_ARRAY_EQUAL(expected,actual,4)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 4
#define CHECK_VEC4_CLOSE(expected,actual,kEpsilon)\
  CHECK_ARRAY_CLOSE(expected,actual,4,kEpsilon)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 4 and compares
///values exactly
#define CHECK_QUAT(expected,actual)\
  CHECK_ARRAY_EQUAL(expected,actual,4)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 4
#define CHECK_QUAT_CLOSE(expected,actual,kEpsilon)\
  CHECK_ARRAY_CLOSE(expected,actual,4,kEpsilon)

///Specialized CHECK_ARRAY_EQUAL that assumes square matrices of dimension 2 and 
///compares values exactly
#define CHECK_MAT2(expected,actual)\
  CHECK_MATRIX_EQUAL(expected,actual,2)

///Specialized CHECK_ARRAY_EQUAL that assumes square matrices of dimension 3 and 
///compares values exactly
#define CHECK_MAT3(expected,actual)\
  CHECK_MATRIX_EQUAL(expected,actual,3)

///Specialized CHECK_ARRAY_EQUAL that assumes square matrices of dimension 4 and 
///compares values exactly
#define CHECK_MAT4(expected,actual)\
  CHECK_MATRIX_EQUAL(expected,actual,4)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 4
#define CHECK_MAT2_CLOSE(expected,actual,kEpsilon)\
  CHECK_MATRIX_CLOSE(expected,actual,2,kEpsilon)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 9
#define CHECK_MAT3_CLOSE(expected,actual,kEpsilon)\
  CHECK_MATRIX_CLOSE(expected,actual,3,kEpsilon)

///Specialized CHECK_ARRAY_CLOSE that assumes arrays of size 16
#define CHECK_MAT4_CLOSE(expected,actual,kEpsilon)\
  CHECK_MATRIX_CLOSE(expected,actual,4,kEpsilon)

#if defined DONT_CHECK_ASSERTS

#define	CHECK_ASSERT(action)																	\
	do { 																						\
		&result_;																				\
	} while (0)

#else

#define	CHECK_ASSERT(action)																	\
	do { 																						\
		bool bExceptionCaught =	false; 															\
		try	{ 																					\
			action;																				\
		} catch(const TestException& ) {														\
			bExceptionCaught = true; 															\
		} 																						\
		catch (...) {																			\
			CppUnitLite::ExceptionHandler::Handle(result_, #action, m_name, __FILE__, __LINE__);				\
		}																						\
		if (!bExceptionCaught) 																	\
			result_.AddFailure (Failure	("No exception detected", m_name, __FILE__,	__LINE__));	\
	} while (0)

#endif 
