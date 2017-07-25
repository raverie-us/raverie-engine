/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_OVERLOAD_RESOLVER_HPP
#define ZILCH_OVERLOAD_RESOLVER_HPP

namespace Zilch
{
  // We perform overload checking in three passes
  namespace OverloadPass
  {
    enum Enum
    {
      // Attempt to find a direct overload that works (exact signature match)
      NoImplicitConversion,
      
      // Now attempt to find an overload that works without any code generation
      // For example, if we have an overload that takes an Animal and we pass in a Cat
      // (only syntax conversion required, no actual code generation)
      RawImplicitConversion,

      // Lastly, if we really need to do a proper conversion (like Integer to Real)
      AnyImplicitConversion
    };
  }

  // A class that's responsible for resolving overloads
  class ZeroShared Overload
  {
  public:
    // Resolve an overload between a function call and the list of functions overloaded under the same name
    static bool ResolveAndImplicitConvert
    (
      const FunctionArray* functions,
      Function*& resolvedFunction,
      FunctionCallNode& functionCallNode
    );

    // Report an error based on the overload result
    static void ReportError
    (
      CompilationErrors& errors,
      const CodeLocation& location,
      const FunctionArray* functions,
      const FunctionCallNode& functionCallNode
    );

    // Report an error based on a single delegate type
    static void ReportSingleError
    (
      CompilationErrors& errors,
      const CodeLocation& location,
      const DelegateType* type,
      const FunctionCallNode& functionCallNode
    );

    // Get function call signature string
    static void GetFunctionCallSignatureString(StringBuilder& builder, const FunctionCallNode& functionCallNode);

    // Test a single function against a function call (performs all overload passes)
    // Note: This will modify the function call node if it needs to add implicit casts
    static bool TestCallAndImplicitConvert(DelegateType* delegateType, FunctionCallNode& functionCallNode);

    // Perform a single pass of the overload detection
    // Note: If we failed the first two passes and pass the 'AnyImplicitConversion' test, then we must generate
    // TypeCastNodes to perform implicit casts on the function call's arguments
    static bool TestDelegateTypeVsCall(DelegateType* delegateType, FunctionCallNode& functionCallNode, OverloadPass::Enum pass);

    // Generates any necessary casts for calling the function
    // This should only be called AFTER the 'AnyImplicitConversion' test has passed
    // Note: This function will modify the 'FunctionCallNode'
    static void GenerateImplicitCasts(DelegateType* delegateType, FunctionCallNode& functionCallNode);

    // Detect any ambiguities between one function signature and a list of other function signatures
    //static void DetectAmbiguities(FunctionArray& functions, Function* function);
  };
}

#endif
