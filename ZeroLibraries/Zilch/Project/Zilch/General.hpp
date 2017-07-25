/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_GENERAL_HPP
#define ZILCH_GENERAL_HPP

namespace Zilch
{
  // Defines
  #ifndef ZilchLoop
    #define ZilchLoop for (;;)
  #endif

  // This define is purely intended to namespace usage within macros
  #define ZZ ::Zilch
  #define ZE ::Zero

  // Helper macros for stringifiying previous #defines
  #define ZilchStr(Argument) #Argument
  #define ZilchStringify(Argument) ZilchStr(Argument)

  // Don't allow copying of a type
  #define ZilchNoCopy(type)       \
    private:                      \
    type& operator=(const type&); \
    type(const type&);

  // Don't allow copying of a type
  #define ZilchNoDefaultConstructor(type) \
    private:                              \
    type();
  
  // Don't allow destruction of a type
  #define ZilchNoDestructor(type) \
    private:                      \
    ~type();

  // Don't allow instantiations of this type
  #define ZilchNoInstantiations(type) \
    private:                          \
    type();                           \
    ~type();                          \
    type& operator=(const type&);     \
    type(const type&);

  // Helper macros
  #define ZilchStringDeref(text) #text
  #define ZilchStringize(text) ZilchStringDeref(text)

  // Shows a todo message
  #ifndef ZilchTodo
    #define ZilchTodo(text) __pragma(message(__FILE__ "(" ZilchStringize(__LINE__) ") : Todo: " text))
  #endif

  // MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
  // MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
  // MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
  // MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
  // MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
  // MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio 2003)
  // MSVC++ 7.0  _MSC_VER == 1300
  // MSVC++ 6.0  _MSC_VER == 1200
  // MSVC++ 5.0  _MSC_VER == 1100

  
  // Thread local storage is supported by everything but clang on Windows. Also not supported on some special platforms
  #if !(defined(__clang__) && (defined(_WIN32) || defined(WIN32)))
    #define ZilchSupportsThreadLocalStorage
  #endif

  // If we're running 0x features in gcc, or the C++ version is defined, or we're in VS2010 or later...
  #if defined(__GXX_EXPERIMENTAL_CXX0X__) ||  __cplusplus >= 201103L || _MSC_VER >= 1600
    #define ZilchSupportsDecltypeAuto
    #define ZilchSupportsNullptr
    #define ZilchSupportsStaticAssert
  #endif

  // This just changes how we do auto, theoretically the same (it's all wrapped in a macro anyways...)
  //#define ZilchUseDeclTypeForAuto


  // *************** Compiler Specific Warnings ***************

  // If we're on Clang or Gcc...
  #if defined(__clang__) || defined(__GNUC__)
    #if defined(__clang__)
      // Ignore unknown pragma warnings...
      #pragma clang diagnostic ignored "-Wunknown-pragmas"
    #else
      // Ignore unknown pragma warnings...
      #pragma GCC diagnostic ignored "-Wpragmas"
    #endif

    // We have many valid switch statements that don't handle all values
    #pragma GCC diagnostic ignored "-Wswitch"

    // We use offsetof, which is technically compiler specific and undefined, but works on all supported platforms
    #pragma GCC diagnostic ignored "-Winvalid-offsetof"

    // This should be investigated for a better method (generally it's for shoving values into void* user data)
    #pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

    // When we force inline some functions, gcc has a very complicated way of determining if a function can be inlined or not
    // If it decides the function does not get inlined, and the function was actually implemented in the cpp (not the header, link time code-gen)
    // then it complains about the function being unused (which it's absolutely not, seems like a gcc bug)
    #pragma GCC diagnostic ignored "-Wunused-function"

    // We declare typedefs that may be useful for people using Zilch, but we don't use them ourselves
    #pragma GCC diagnostic ignored "-Wunused-local-typedefs"

  #endif

  // If we're on the Microsoft compiler (MSVC)...
  #ifdef _MSC_VER
    // We don't want to get warned about security
    #ifndef _CRT_SECURE_NO_WARNINGS
      #define _CRT_SECURE_NO_WARNINGS
    #endif

    // We don't consider unreferenced parameters to be an error
    #pragma warning(disable : 4100)

    // Unfortunately, local variable initialized but not referenced is actually a very
    // useful warning, however there are cases where we only initialize variables to be
    // used in debug checks, which this will complain / error in release
    // There are also other cases where we explicitly invoke destructors for class binding
    // and if the destructor is trivial, it complains that the self parameter is not used
    #pragma warning(disable : 4189)

    // We get a warning about an outer structure being padded due to inner members having alignment specifications
    // An example of this is using jmp_buf anywhere (such as in PerFrameData) especially with 64-bit compatability warnings
    #pragma warning(disable : 4324)

    // We really don't care about C++ initializing POD types to zero
    #pragma warning(disable : 4345)

    // Disable the 'this' in base initializer, since we use it to track owners
    #pragma warning(disable : 4355)

    // Ignore the warning about a non-standard extension for 'override'
    // It really is way too useful (both for documentation and error checking)
    #pragma warning(disable : 4481)

    // This is a warning about unused functions being removed, but there is a serious issue with VS2010
    // where it can end up removing a virtual [thunk] function and then it warns you about it
    // (seems to be when combined with SFINAE testing for the virtual member)
    #pragma warning(disable : 4505)

    // Disable the execption handling warning. We don't use exceptions in Zilch
    // because many console and embedded systems do not support them)
    #pragma warning(disable : 4530)

    // For virtual function binding we need to ignore warnings about no constructors
    // being generated, as well as warnings about a class not being able to be instantiated
    #pragma warning(disable : 4510)
    #pragma warning(disable : 4610)

    // Disable "Interaction between '_setjmp' and C++ object destruction is non-portable"
    // We use setjmp / longjmp for exception handling, and we make sure to properly handle
    // destruction of objects
    #pragma warning(disable : 4611)

    // Disable warnings about deprecated function calls (primarily WIN32)
    // It's nice to know these are deprecated, but we'll deal with them when they become actual errors
    #pragma warning(disable : 4996)

    // Static analysis complains about passing a 'char' into isspace and similar functions
    // For some weird reason it requires casting the char to an 'unsigned char', even though isspace is defined to take an int
    // This is well formed C and C++ code, static analysis is complaining about it
    #pragma warning(disable : 6330)

    // Static analysis also complains about the use of alloca, which has a well defined meaning
    // It recommends the usage of _malloca, a completely non-standard MSVC only function which functions EXACTLY the same in Release mode
    #pragma warning(disable : 6255)

    // Disabling the local declaration hiding, for now (this one is the most valid, but we have valid cases for it)
    #pragma warning(disable : 6246)

    // Static analysis also complains about 'Dereferencing NULL pointer' for every single instance we use in-place new, which is simply not correct
    #pragma warning(disable : 6011)

    // Comparing a constant to another constant (not caught by the W4 via templates) is considered bad by static analysis, even though
    // it completly makes sense and will be optimized out if it is in fact a constant
    #pragma warning(disable : 6326)

    // This particular warning complains about the dereferencing a null pointer because it implied
    // that a variable could be null (assigned to another variable, check that variable)
    // The entire warning is fundamentally flawed:
    //  Player* foundPlayer = nullptr;
    //  if (SomeCondition)
    //  {
    //    Player* globalPlayer = GrabGlobalPlayer();
    //    if (globalPlayer->IsAlive()) <--- Here it complains that 'globalPlayer' could be null, simply because of the below line
    //      foundPlayer = globalPlayer;
    //  }
    //  if (foundPlayer != nullptr) <--- It sees that 'foundPlayer' is being checked for null, and because foundPlayer
    //    ...                            is assigned from nearestPlayer, then it thinks 'globalPlayer' could be null
    #pragma warning(disable : 28182)

    // Standard examples (such as setting thread name) from Microsoft's own documentation cause the static analysis to complain
    // about __try and __catch (about continuing from the exception possibly causing an infinite loop, which it does not)
    #pragma warning(disable : 6312)
    #pragma warning(disable : 6322)

    // Disable a warning about doing well defined operations on bools
    #pragma warning(disable : 6323)

    ZilchTodo("These must be fixed / examined");
    #pragma warning(disable : 6201)
    #pragma warning(disable : 6031)

    // These static analysis warnings are literally happening inside Microsoft's own 'wspiapi.h'
    #pragma warning(disable : 6101)
    #pragma warning(disable : 6102)
    #pragma warning(disable : 6387)
    #pragma warning(disable : 6386)
    #pragma warning(disable : 28196)

    // We don't care about using deprecated Windows code, we'll change it later if we care
    #pragma warning(disable : 28159)

  #endif


  // *************** Compiler Specific Macros ***************

  // If we're on the Microsoft compiler...
  #ifdef _MSC_VER
    
    // When we want to declare a thread local variable
    // This must be used in the declaration (after static or extern if applicable)
    // and also in the cpp file before anything
    // Thread locals cannot be initialized
    #define ZilchThreadLocal __declspec(thread)

    // On some compilers we need to define static thread locals (as members of a class)
    // This macro will remove any of its contents for compilers that don't need it
    #define ZilchDefineStaticThreadLocal(StaticVariableDefinition) StaticVariableDefinition
    
    // Atttempts to trigger a breakpoint in the debugger
    #define ZilchDebugBreak() __debugbreak()

    // Converts a string to a double integer
    #define ZilchStringToDoubleInteger(CStr, Base) (long long)_strtoui64(CStr, nullptr, Base)

    // Forces a function to be inlined for optimization purposes
    #define ZilchForceInline inline __forceinline

    // Forces a function to not be inlined (only used to debug, and unfortunately to fix true linker bugs)
    #define ZilchNoInline __declspec(noinline)

  #else
    #ifndef override
      // We don't have access to the 'override' keyword
      #define override
    #endif
    
    // When we want to declare a thread local variable
    // This must be used in the declaration (after static or extern if applicable)
    // and also in the cpp file before anything
    // Thread locals cannot be initialized
    #ifdef ZilchSupportsThreadLocalStorage
      #define ZilchThreadLocal __thread
    #else
      #define ZilchThreadLocal
    #endif

    // On some compilers we still need to define static thread locals (as members of a class)
    // This macro will remove any of its contents for compilers that don't need it
    #define ZilchDefineStaticThreadLocal(StaticVariableDefinition)

    // Atttempts to trigger a breakpoint in the debugger
    #define ZilchDebugBreak()

    // Converts a string to a double integer
    #define ZilchStringToDoubleInteger(CStr, Base) (long long)strtoull(CStr, nullptr, Base)

    // Forces a function to be inlined for optimization purposes
    // There seems to be issues with __attribute__((always_inline)) wit GCC linking
    #define ZilchForceInline inline

    // Forces a function to not be inlined (only used to debug, and unfortunately to fix true linker bugs)
    #define ZilchNoInline

  #endif

  #ifdef ZilchSupportsDecltypeAuto
    // Most modern compilers support decltype...
    #define ZilchTypeOf(Expression) decltype(Expression)

    #ifdef ZilchUseDeclTypeForAuto

      template <typename T>
      class StripRef
      {
      public:
        typedef T Type;
      };

      template <typename T>
      class StripRef<T&>
      {
      public:
        typedef T Type;
      };

      // We can use decltype for auto, but the big issue is that normally auto requires you to specify
      // if it's a reference or not (&). The decltype always infers the reference, so we have to strip it
      #define ZilchAutoVal(VariableName, Expression) StripRef<decltype(Expression)>::Type VariableName = Expression;
      #define ZilchAutoRef(VariableName, Expression)          decltype(Expression)&       VariableName = Expression;

    #else
      // Infer the type of an expression and store within a variable
      // Used for some template situations where determining the expression's type is tedious
      #define ZilchAutoVal(VariableName, Expression) auto  VariableName = Expression;
      #define ZilchAutoRef(VariableName, Expression) auto& VariableName = Expression;
    #endif

  #else
    // A few really really bad/archaic compilers don't (hopefully they have typeof!)
    #define ZilchTypeOf(Expression) typeof(Expression)

    // Infer the type of an expression and store within a variable
    // Used for some template situations where determining the expression's type is tedious
    #define ZilchAutoVal(VariableName, Expression) typeof(Expression)  VariableName = Expression;
    #define ZilchAutoRef(VariableName, Expression) typeof(Expression)& VariableName = Expression;
  #endif

  // Visual Studio 2010's (and maybe other compilers) decltype has an issue with getting the type
  // of the address of a function template instantiation (decltype(Lerp<Real>) for instance).
  // This template and macro are a work around for doing this.
  template <typename T>
  T ZilchFunctionPointerPassThrough(T);
  #define ZilchTypeOfFunctionPointer(FunctionPointer)  ZilchTypeOf(ZilchFunctionPointerPassThrough(FunctionPointer))

  // If we support static assert (otherwise there's an alternative not as clear way)
  #ifdef ZilchSupportsStaticAssert
    #define ZilchStaticAssert(ConstantExpression, StringMessage, OldMessageName)  \
      static_assert(ConstantExpression, StringMessage)
  #else
    #define ZilchStaticAssert(ConstantExpression, StringMessage, OldMessageName)  \
      static const int OldMessageName = (sizeof(char[1 - 2 * !(ConstantExpression)]))
  #endif

  // Macro for figuring out the size of a fixed C-array
  #define ZilchCArrayCount(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

  // A class we use when we're either debugging or refactoring
  // This class attempts to act as a placeholder for any other class
  template <typename ToEmulate>
  class DebugPlaceholder
  {
  public:

    DebugPlaceholder()
    {
    }

    template <typename T>
    DebugPlaceholder(const T&)
    {
    }

    operator ToEmulate()
    {
      return ToEmulate();
    }

    template <typename T>
    T& operator=(T& value)
    {
      return value;
    }

    template <typename T>
    const T& operator=(const T& value)
    {
      return value;
    }

    template <typename T>
    bool operator==(const T&) const
    {
      return false;
    }

    template <typename T>
    bool operator!=(const T&) const
    {
      return false;
    }
  };
}

#endif
