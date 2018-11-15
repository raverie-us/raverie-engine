/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Precompiled.hpp"
#include <vld.h>


using namespace Zilch;

Real    Test01();
Integer Test02();
Integer Test03();
Integer Test04();
Integer Test05();
Integer Test06();
Integer Test07();
String  Test08();
Integer Test09();
Integer Test10();
Integer Test11();
Integer Test12();

void DiffTest01(Array<Diff>& diffs);


#define ZilchPrintAndFlush(...) printf(__VA_ARGS__); fflush(stdout)
#define ZilchPauseInDebugger() Zero::Os::DebugBreak()

bool CheckCloseReal(Real p1, Real p2)
{
  return Math::Abs(p1 - p2) < 0.00001f;
}

bool CheckCloseReal2(const Real2& p1, const Real2& p2)
{
  return
    CheckCloseReal(p1.x, p2.x) &&
    CheckCloseReal(p1.y, p2.y);
}

bool CheckCloseReal3(const Real3& p1, const Real3& p2)
{
  return
    CheckCloseReal(p1.x, p2.x) &&
    CheckCloseReal(p1.y, p2.y) &&
    CheckCloseReal(p1.z, p2.z);
}

bool CheckCloseReal4(const Real4& p1, const Real4& p2)
{
  return
    CheckCloseReal(p1.x, p2.x) &&
    CheckCloseReal(p1.y, p2.y) &&
    CheckCloseReal(p1.z, p2.z) &&
    CheckCloseReal(p1.w, p2.w);
}

bool CheckCloseQuaternion(const Quaternion& p1, const Quaternion& p2)
{
  return
    CheckCloseReal(p1.x, p2.x) &&
    CheckCloseReal(p1.y, p2.y) &&
    CheckCloseReal(p1.z, p2.z) &&
    CheckCloseReal(p1.w, p2.w);
}

template <typename T>
bool CheckEqual(const T& p1, const T& p2)
{
  return p1 == p2;
}

template <typename ReturnType, typename EqualiyCheck>
static void UnitTest(String className, String testAndFunctionName, ReturnType (*cppFunction)(), ExecutableState* state, EqualiyCheck checkFn)
{
  ZilchPrintAndFlush("#BEGIN: %s\n", testAndFunctionName.c_str());
  
  // Find the scripting function by name
  BoundType* type = state->Dependencies.FindType(className);
  Function* function = type->FindFunction(testAndFunctionName, Array<Type*>(), ZilchTypeId(ReturnType), FindMemberOptions::Static);

  // Call the C++ function and get it's return value
  ReturnType testResultCpp = cppFunction();
  
  // Call the scripting function and get it's return value
  ExceptionReport report;
  Call call(function, state);
  call.Invoke(report);
  if (report.HasThrownExceptions())
  {
    String cppVal = TestToString(testResultCpp);
    ZilchPrintAndFlush("#FAILED:\n");
    ZilchPrintAndFlush("  Zilch: Exception was thrown\n");
    ZilchPrintAndFlush("    C++: '%s'\n", cppVal.c_str());
    ZilchPauseInDebugger();
  }
  else
  {
    // Call the scripting function and get it's return value
    ReturnType testResultScript = call.Get<ReturnType>(Call::Return);

    if (checkFn(testResultScript, testResultCpp))
    {
      ZilchPrintAndFlush("#SUCCESS\n");
    }
    else
    {
      // Get the string representation of both outputs (expect TestToString to exist for this type)
      String scriptVal = TestToString(testResultScript);
      String cppVal = TestToString(testResultCpp);

      ZilchPrintAndFlush("#FAILED:\n");
      ZilchPrintAndFlush("  Zilch: '%s'\n", scriptVal.c_str());
      ZilchPrintAndFlush("    C++: '%s'\n", cppVal.c_str());
      ZilchPauseInDebugger();
    }
  }

  ZilchPrintAndFlush("#END\n\n");
}

void RunStressTests()
{
  {
    String name = "StressNonTokenizable";
    ZilchPrintAndFlush("#BEGIN: %s\n", name.c_str());

    for (size_t i = 0; i < 1000; ++i)
    {
      String code = GeneratePotentiallyNonTokenizable();

      Module dependencies;
      Project project;
      EventConnect(&project, Events::CompilationError, DefaultErrorCallback);
      project.AddCodeFromString(code, name, nullptr);
      LibraryRef lib = project.Compile(name, dependencies, EvaluationMode::Project);
    }

    ZilchPrintAndFlush("#END\n\n");
  }

  {
    String name = "StressTokenizablePotentiallyNonParsable";
    ZilchPrintAndFlush("#BEGIN: %s\n", name.c_str());

    for (size_t i = 0; i < 50; ++i)
    {
      String code = GenerateTokenizablePotentiallyNonParsable();

      Module dependencies;
      Project project;
      EventConnect(&project, Events::CompilationError, DefaultErrorCallback);
      project.AddCodeFromString(code, name, nullptr);

      Array<UserToken> tokens;
      Array<UserToken> comments;
      if (project.Tokenize(tokens, comments) == false)
      {
        __debugbreak();
      }

      LibraryRef lib = project.Compile(name, dependencies, EvaluationMode::Project);
    }
    
    ZilchPrintAndFlush("#END\n\n");
  }
}

void ZilchCallMe5Times(Call& call, ExceptionReport& report)
{
  Delegate& delegate = call.Get<Delegate&>(0);

  for (size_t i = 0; i < 5; ++i)
  {
    Call childCall(delegate, call.GetState());
    childCall.Invoke(report);
    if (report.HasThrownExceptions())
      return;
  }
}

void RunSanityTests()
{
  // Create a scripting engine
  {
    Module dependencies;

    EventConnect(&Console::Events, Events::ConsoleWrite, DefaultWriteText);

    // Math
    {
      Project project;
      EventConnect(&project, Events::CompilationError, DefaultErrorCallback);
      project.AddCodeFromFile("CustomMath.z", nullptr);
      LibraryRef lib = project.Compile("Math", dependencies, EvaluationMode::Project);
      ErrorIf(lib == nullptr, "Unit test 'Math' library did not compile");
      dependencies.PushBack(lib);
    }

    // Callbacks
    {
      LibraryBuilder builder("Callbacks");

      BoundType* type = builder.AddBoundType("CallBack", TypeCopyMode::ReferenceType, 0);
      ParameterArray parameters;
      DelegateType* parameterType = builder.GetDelegateType(ParameterArray(), ZilchTypeId(void));
      builder.AddBoundFunction(type, "CallMe5Times", ZilchCallMe5Times, OneParameter(parameterType), ZilchTypeId(void), FunctionOptions::Static);

      LibraryRef lib = builder.CreateLibrary();
      ErrorIf(lib == nullptr, "Unit test 'Events' library did not compile");
      dependencies.PushBack(lib);
    }
  
    // Unit Tests
    {
      Project project;
      EventConnect(&project, Events::CompilationError, DefaultErrorCallback);

      project.AddCodeFromFile("Test01.z", nullptr);
      project.AddCodeFromFile("Test02.z", nullptr);
      project.AddCodeFromFile("Test03.z", nullptr);
      project.AddCodeFromFile("Test04.z", nullptr);
      project.AddCodeFromFile("Test05.z", nullptr);
      project.AddCodeFromFile("Test06.z", nullptr);
      project.AddCodeFromFile("Test07.z", nullptr);
      project.AddCodeFromFile("Test08.z", nullptr);
      project.AddCodeFromFile("Test09.z", nullptr);
      project.AddCodeFromFile("Test10.z", nullptr);
      project.AddCodeFromFile("Test11.z", nullptr);
      project.AddCodeFromFile("Test12.z", nullptr);

      LibraryRef lib = project.Compile("UnitTests", dependencies, EvaluationMode::Project);
      ErrorIf(lib == nullptr, "Unit test 'UnitTests' library did not compile");

      if (lib != nullptr)
      {
        dependencies.PushBack(lib);
        //auto html = dependencies.BuildDocumentationHtml();

        ExecutableState* state = dependencies.Link();
        EventConnect(state, Events::UnhandledException, DefaultExceptionCallback);
        //StateCreated(state);
        ErrorIf(state == nullptr, "Unit tests did not link");
        
        UnitTest("UnitTest01", "Test01", Test01, state, CheckEqual<TypeOf(Test01())>);
        UnitTest("UnitTest02", "Test02", Test02, state, CheckEqual<TypeOf(Test02())>);
        UnitTest("UnitTest03", "Test03", Test03, state, CheckEqual<TypeOf(Test03())>);
        UnitTest("UnitTest04", "Test04", Test04, state, CheckEqual<TypeOf(Test04())>);
        UnitTest("UnitTest05", "Test05", Test05, state, CheckEqual<TypeOf(Test05())>);
        UnitTest("UnitTest06", "Test06", Test06, state, CheckEqual<TypeOf(Test06())>);
        UnitTest("UnitTest07", "Test07", Test07, state, CheckEqual<TypeOf(Test07())>);
        UnitTest("UnitTest08", "Test08", Test08, state, CheckEqual<TypeOf(Test08())>);
        UnitTest("UnitTest09", "Test09", Test09, state, CheckEqual<TypeOf(Test09())>);
        UnitTest("UnitTest10", "Test10", Test10, state, CheckEqual<TypeOf(Test10())>);
        UnitTest("UnitTest11", "Test11", Test11, state, CheckEqual<TypeOf(Test11())>);
        UnitTest("UnitTest12", "Test12", Test12, state, CheckEqual<TypeOf(Test12())>);

        delete state;
      }
    }
  }
}


// The error code we look for when testing for errors
static ErrorCode::Enum TestErrorCode   = ErrorCode::Invalid;
static ErrorCode::Enum ActualErrorCode = ErrorCode::Invalid;
static String ActualErrorMessage;

void UnitTestErrorCallback(ErrorEvent* e)
{
  ActualErrorCode = e->ErrorCode;
  ActualErrorMessage = e->GetFormattedMessage(MessageFormat::Zilch);
}

void UnitTestErrorFile(Module& dependencies, ErrorCode::Enum error)
{
  auto errorInfo = ErrorDatabase::GetInstance().GetErrorInfo(error);

  // Get the error test name
  String name = String::Format("%03d %s", error, errorInfo.Name.c_str());

  ZilchPrintAndFlush("#BEGIN: %s\n", name.c_str());

  String fileName = String::Format("ErrorTest%s.z", errorInfo.Name.c_str());

  Project project;
  EventConnect(&project, Events::CompilationError, UnitTestErrorCallback);

  if (FileExists(fileName) == false)
  {
    printf("Not Found %s\n", name.c_str());
    fflush(stdout);
  }
  else
  {
    Status status;
    String text = Project::ReadTextFile(status, fileName);
    if (status.Failed())
    {
      ZilchPrintAndFlush("#FAILED: Unable to open %s\n", fileName.c_str());
      ZilchPauseInDebugger();
    }
    else
    {
      if (text.StartsWith("//SkipTest"))
      {
        ZilchPrintAndFlush("#SKIPPED\n");
      }
      else
      {
        project.AddCodeFromString(text, fileName, nullptr);

        // Reset the test error code
        ActualErrorCode = ErrorCode::Invalid;
        TestErrorCode   = error;

        // This compilation should fail!
        LibraryRef lib = project.Compile(errorInfo.Name, dependencies, EvaluationMode::Project);

        // If the library actually compiled...
        if (lib != nullptr)
        {
          ZilchPrintAndFlush("#FAILED: The test compiled when it should not have\n");
          ZilchPauseInDebugger();
        }
        else
        {
          if (ActualErrorCode != TestErrorCode)
          {
            ZilchPrintAndFlush("#FAILED: Produced a different error then we were expecting\n");
            ZilchPrintAndFlush("  Error: %s\n", ActualErrorMessage.c_str());
            ZilchPauseInDebugger();
          }
        }
      }
    }
  }

  ZilchPrintAndFlush("#END\n\n");
}

void RunErrorTests()
{
  Module dependencies;

  for (size_t i = 0; i < ErrorCode::Count; ++i)
  {
    UnitTestErrorFile(dependencies, (ErrorCode::Enum)i);
  }
}

void EvaluateDiffs(void (*diffFunction)(Array<Diff>& diffs), const char* name)
{
  ZilchPrintAndFlush("#BEGIN: %s\n", name);

  Array<Diff> diffs;
  diffFunction(diffs);

  bool success = true;
  ZilchForEach(Diff& diff, diffs)
  {
    if (diff.first != diff.second)
    {
      success = false;
      break;
    }
  }

  if (success)
  {
    ZilchPrintAndFlush("#SUCCESS\n");
  }
  else
  {
    ZilchPrintAndFlush("#FAILED: One of the tests did not diff\n");
    ZilchPauseInDebugger();
  }

  ZilchPrintAndFlush("#END\n\n");
}

void RunDiffTests()
{
  EvaluateDiffs(DiffTest01, "DiffTest01");
}

int ValueUnitTestConstantResult()
{
  return 98234756;
}

void ValueUnitTest(String testAndFunctionName, ExecutableState* state)
{
  static const String ClassName("AccessorUnitTests");
  UnitTest(ClassName, testAndFunctionName, ValueUnitTestConstantResult, state, CheckEqual<Integer>);
}

Real2 Vec2Abs()
{
  return Real2(2.0f,30.0f);
}

Real3 Vec3Abs()
{
  return Real3(1.5f,8.3f,27.5f);
}

Real4 Vec4Abs()
{
  return Real4(1.5f,8.3f,27.5f,54.32f);
}

Quaternion QuatNormalizeTest1()
{
  //normalize Quat(1,2,3,4)
  Quaternion ret;
  ret.x = 0.1825741858350553711523232609336f;
  ret.y = 0.3651483716701107423046465218672f;
  ret.z = 0.5477225575051661134569697828008f;
  ret.w = 0.73029674334022148460929304373441f;
  return ret;
}

Quaternion QuatInvertTest1()
{
  //normalize Quat(4,4,2,6)
  Quaternion ret;
  ret.x = -0.05555555555555555555555555555556f;
  ret.y = -0.05555555555555555555555555555556f;
  ret.z = -0.02777777777777777777777777777778f;
  ret.w =  0.08333333333333333333333333333333f;
  return ret;
}

Quaternion QuatInvertTest2()
{
  //normalize Quat(1,0,0,0)
  Quaternion ret;
  ret.x = -1;
  ret.y = 0;
  ret.z = 0;
  ret.w = 0;
  return ret;
}

Quaternion QuatInvertTest3()
{
  //normalize Quat(2,-3,5,7)
  Quaternion ret;
  ret.x =  -0.0229885057471264367816091954023f;
  ret.y =  0.03448275862068965517241379310345f;
  ret.z = -0.05747126436781609195402298850575f;
  ret.w =  0.08045977011494252873563218390805f;
  return ret;
}

Quaternion QuatSlerpTest()
{
  //normalize Slerp(Quat(0,0,sqrt(2)/2,sqrt(2)/2), Quat(0,0,01), .75)
  Quaternion ret;
  ret.x = 0.0f;
  ret.y = 0.0f;
  ret.z = 0.19509032201612826784828486847702f;
  ret.w = 0.98078528040323044912618223613423f;
  return ret;
}

Quaternion QuatTransformQuat()
{
  //Quat(-1,-2,-3,-4) * Quat(6,8,10,12)
  Quaternion ret;
  ret.x = -32.0f;
  ret.y = -64.0;
  ret.z = -72.0f;
  ret.w = 4.0f;
  return ret;
}

Real3 QuatTransformVec3()
{
  //Quat(1.0,0.0,0.0,0.0) * Vec3(1.0,1.0,1.0)
  Real3 ret;
  ret.x = 1.0f;
  ret.y =-1.0f;
  ret.z =-1.0f;
  return ret;
}

void RunUnitTests()
{
  //Sha1Builder::RunUnitTests();

  Module dependencies;

  Project project;
  EventConnect(&project, Events::CompilationError, DefaultErrorCallback);
  project.AddCodeFromFile("Quaternion.z", nullptr);
  project.AddCodeFromFile("VectorTests.z", nullptr);
  project.AddCodeFromFile("ValueUnitTests.z", nullptr);

  String name = "MathTests";
  // This compilation should fail!
  LibraryRef lib = project.Compile(name, dependencies, EvaluationMode::Project);

  if(lib == nullptr)
  {
    ZilchPrintAndFlush("#FAILED: Unit test files did not compile\n");
    ZilchPauseInDebugger();
    return;
  }

  dependencies.PushBack(lib);
  ExecutableState* state = dependencies.Link();
  EventConnect(state, Events::UnhandledException, DefaultExceptionCallback);

  UnitTest("VectorTests", "Vec2Abs", Vec2Abs, state, CheckCloseReal2);
  UnitTest("VectorTests", "Vec3Abs", Vec3Abs, state, CheckCloseReal3);
  UnitTest("VectorTests", "Vec4Abs", Vec4Abs, state, CheckCloseReal4);

  UnitTest("QuatTests", "QuatNormalizeTest1", QuatNormalizeTest1, state, CheckCloseQuaternion);
  UnitTest("QuatTests", "QuatInvertTest1",    QuatInvertTest1   , state, CheckCloseQuaternion);
  UnitTest("QuatTests", "QuatInvertTest2",    QuatInvertTest2   , state, CheckCloseQuaternion);
  UnitTest("QuatTests", "QuatInvertTest3",    QuatInvertTest3   , state, CheckCloseQuaternion);
  UnitTest("QuatTests", "QuatSlerpTest",      QuatSlerpTest     , state, CheckCloseQuaternion);
  UnitTest("QuatTests", "QuatTransformQuat",  QuatTransformQuat , state, CheckCloseQuaternion);
  UnitTest("QuatTests", "QuatTransformVec3",  QuatTransformVec3 , state, CheckCloseReal3);

  BoundType* type = state->Dependencies.FindType("AccessorUnitTests");
  for (size_t i = 0; i < type->AllFunctions.Size(); ++i)
  {
    auto testAndFunctionName = String::Format("ValueUnitTest%d", i);
    ValueUnitTest(testAndFunctionName, state);
  }

  delete state;
}

int main()
{
  ZilchSetup setup(SetupFlags::None);

  //RunDiffTests();
  RunStressTests();
  RunUnitTests();
  RunSanityTests();
  RunErrorTests();
  return 0;
}
