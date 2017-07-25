///////////////////////////////////////////////////////////////////////////////
///
/// \file Diagnostic.hpp
/// Declaration of the basic debug diagnostic functions.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Utility/Typedefs.hpp"
#include "Utility/Standard.hpp"

/*

These macros are only for displaying information to developers. 
For informing a user of errors use the notification system.

Warn/WarnIf: Indicates an problem has occurred but execution will continue
             normally. The problem will be handled locally but may also have 
             to be handled by the caller. By default in a development build this 
             will trigger a debug break but execution can continue.
             Examples: Scripting Errors, File Not Found, Bad Sound Name, etc 

ReturnIf: Operates the same as Warn except that the it will cause the current 
          block to return with the provided parameter if expression if true. 
          Used as a shorthand instead of
          if(a) { Warn(a,"message") return errorCode; }

Error/ErrorIf: Indicates that a problem has occurred and if execution continues 
               it will put the program in a invalid state and most likely crash.
               The equivalent of assert this always triggers a break point expect
               in the final build where the program will crash.

Verify(): Runs the function and generates an error if the function returns non
          zero. When diagnostic disabled the function call will remain but the
          error code will be removed.
*/

#ifdef _MSC_VER
  #define ZERO_DEBUG_BREAK __debugbreak()
#else
  #define ZERO_DEBUG_BREAK
#endif

namespace Zero
{

//---------------------------------------------------------------- Error Signaler
class ZeroShared ErrorSignaler
{
public:

  enum SignalErrorType { Warning, Error, FileError };

  //--------------------------------------------------------------- Error Data
  struct ErrorData
  {
    int Line;
    cstr Expression;
    cstr File;
    cstr Message;
    bool IgnoreFutureAssert;
    SignalErrorType ErrorType;
  };

  //The error handler can display Ui, filter errors, or other processing
  //Return true to debug break.
  typedef bool (*ErrorHandler)(ErrorData& errorData);
  static void SetErrorHandler(ErrorHandler newHandler){activeErrorHandler = newHandler; }
  static ErrorHandler GetErrorHandler() { return activeErrorHandler; }
  
  static bool SignalError(SignalErrorType erroType, cstr exp, cstr file,
                          int line, bool& ignore, cstr msg = 0, ...);
private:
  static ErrorHandler activeErrorHandler;
};

}//namespace Zero


#if !defined(ZERO_ENABLE_ERROR) 
#   if defined(ZeroDebug)
#       define ZERO_ENABLE_ERROR 1
#   else
#       define ZERO_ENABLE_ERROR 0
#   endif
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1600
  #define StaticAssert(name, Expression, error) \
    static_assert(Expression, error)
#else
  #define StaticAssert(name, Expression, error) \
    static int name = (sizeof(char[1 - 2 * !(Expression)]))
#endif

static int gConditionalFalseConstant = 0;

#if ZERO_ENABLE_ERROR

#define UnusedParameter(param) (void)param

#define WarnIf(Expression, ...) \
  do { static bool __ignore = false; if((Expression) && ::Zero::ErrorSignaler::SignalError(::Zero::ErrorSignaler::Warning,#Expression, __FILE__, __LINE__, __ignore,##__VA_ARGS__)) \
  ZERO_DEBUG_BREAK; } while(gConditionalFalseConstant)

#define ErrorIf(Expression, ...) \
  do { static bool __ignore = false; if((Expression) && ::Zero::ErrorSignaler::SignalError(::Zero::ErrorSignaler::Error,#Expression, __FILE__, __LINE__, __ignore,##__VA_ARGS__)) \
  ZERO_DEBUG_BREAK; } while(gConditionalFalseConstant)

#define Assert(Expression, ...) \
  do { static bool __ignore = false; if(!(Expression) && ::Zero::ErrorSignaler::SignalError(::Zero::ErrorSignaler::Error,#Expression, __FILE__, __LINE__, __ignore,##__VA_ARGS__)) \
  ZERO_DEBUG_BREAK; } while(gConditionalFalseConstant)

#define Error(...)\
  do { static bool __ignore = false; if(::Zero::ErrorSignaler::SignalError(::Zero::ErrorSignaler::Error,"", __FILE__, __LINE__, __ignore,##__VA_ARGS__)) \
  ZERO_DEBUG_BREAK; } while(gConditionalFalseConstant)

#define Warn(...)\
  do { static bool __ignore = false; if(::Zero::ErrorSignaler::SignalError(::Zero::ErrorSignaler::Warning,"", __FILE__, __LINE__, __ignore,##__VA_ARGS__)) \
  ZERO_DEBUG_BREAK; } while(gConditionalFalseConstant)

#define FileErrorIf(Expression, file, Line, ...) \
  do { static bool __ignore = false; if((Expression) && ::Zero::ErrorSignaler::SignalError(::Zero::ErrorSignaler::FileError,#Expression, file, Line, __ignore,##__VA_ARGS__)) \
  ZERO_DEBUG_BREAK; } while(gConditionalConstant)

#define Verify(funccall) ErrorIf(funcall != 0);

#else

#define UnusedParameter(param)
#define WarnIf(...) ((void)0)
#define ErrorIf(...) ((void)0)
#define Assert(...) ((void)0)
#define Error(...) ((void)0)
#define Warn(...) ((void)0)
#define FileErrorIf(...) ((void)0)
#define Verify(funccall) funccall

#endif

#define ReturnIf(Expression , whatToReturn, ...) \
  do { if(Expression) {                          \
    WarnIf(Expression, __VA_ARGS__);             \
    return whatToReturn;                         \
  } } while(gConditionalFalseConstant)

#define ReturnFileErrorIf(Expression , whatToReturn , file , Line , ...)  \
  do { if(Expression) {                                                   \
    FileErrorIf(Expression, file , Line , __VA_ARGS__);                   \
    return whatToReturn;                                                  \
  } } while(gConditionalFalseConstant)

/// Asserts value is within [min, max]
#define AssertWithinRange(value, min, max) \
  Assert((min) <= (value) && (value) <= (max))
/// Statically asserts value is within [min, max]
#define StaticAssertWithinRange(name, value, min, max) \
  StaticAssert(name, (min) <= (value) && (value) <= (max), "Value outside range")
