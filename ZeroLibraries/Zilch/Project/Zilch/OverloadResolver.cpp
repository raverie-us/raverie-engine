/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  bool Overload::ResolveAndImplicitConvert
  (
    const FunctionArray* functions,
    Function*& resolvedFunction,
    FunctionCallNode& functionCallNode
  )
  {
    // Loop through all the overload passes
    for (size_t passIndex = OverloadPass::NoImplicitConversion; passIndex <= OverloadPass::AnyImplicitConversion; ++passIndex)
    {
      // Get the current pass as an enum
      OverloadPass::Enum pass = (OverloadPass::Enum)passIndex;

      // Loop through all the function overloads that we have to choose from
      for (size_t i = 0; i < functions->Size(); ++i)
      {
        // Get the current function
        Function* function = (*functions)[i];

        // Check the current function type against the call
        if (TestDelegateTypeVsCall(function->FunctionType, functionCallNode, pass))
        {
          // If we're on the last pass, then we need to generate implicit conversion code
          if (pass == OverloadPass::AnyImplicitConversion)
            GenerateImplicitCasts(function->FunctionType, functionCallNode);

          // Set the resolved function, and return a success
          resolvedFunction = function;
          return true;
        }
      }
    }

    // Since we got here, the overload was not resolved
    return false;
  }

  //***************************************************************************
  void Overload::GetFunctionCallSignatureString(StringBuilder& builder, const FunctionCallNode& functionCallNode)
  {
    // Add on the beginning call parentheses
    builder.Append("(");

    // Store the number of arguments we need to print
    size_t count = functionCallNode.Arguments.Size();

    // Loop through all the arguments
    for (size_t i = 0; i < count; ++i)
    {
      // If the call uses named arguments...
      if (functionCallNode.IsNamed)
      {
        // Write the name of the argument
        builder.Append(functionCallNode.ArgumentNames[i]);
        builder.Append(" : ");
      }

      // Grab the current type
      Type* type = functionCallNode.Arguments[i]->ResultType;

      // Always make sure the type is valid / resolved
      if (type != nullptr)
      {
        // Write the type of the argument
        builder.Append(type->ToString());
      }
      else
      {
        Error("Attempting to print a signature where one of the argument types is invalid!");
      }

      // If we're not at the end
      if (i != count - 1)
      {
        // Add commas to separate out the arguments
        builder.Append(", ");
      }
    }

    // Add on the ending call parentheses
    builder.Append(")");
  }

  //***************************************************************************
  void Overload::ReportSingleError
  (
    CompilationErrors& errors,
    const CodeLocation& location,
    const DelegateType* type,
    const FunctionCallNode& functionCallNode
  )
  {
    // Get the calling signature
    StringBuilder call;
    call.Append("\n\n  delegate");
    GetFunctionCallSignatureString(call, functionCallNode);
    call.Append("\n\n");

    // Build a string to enumerate all possible overloads
    StringBuilder options;
    options.Append("\n\n  delegate");
    type->BuildSignatureString(options, functionCallNode.IsNamed);

    // Now report the error
    return errors.Raise(location, ErrorCode::UnableToResolveFunction,
      "delegate",
      call.ToString().c_str(),
      options.ToString().c_str());
  }

  //***************************************************************************
  void Overload::ReportError
  (
    CompilationErrors& errors,
    const CodeLocation& location,
    const FunctionArray* functions,
    const FunctionCallNode& functionCallNode
  )
  {
    // Error checking
    ErrorIf(functions == nullptr || functions->Empty(),
      "We cannot report overloading errors when no functions were provided");

    // Get the name of one of the functions (we know there is at least one function in this array)
    String name = (*functions)[0]->Name;

    // Get the calling signature
    StringBuilder call;
    call.Append("\n\n  ");
    call.Append(name);
    GetFunctionCallSignatureString(call, functionCallNode);
    call.Append("\n\n");

    // Build a string to enumerate all possible overloads
    StringBuilder options;
    options.Append("\n");

    // Loop through all the function choices we have
    for (size_t i = 0; i < functions->Size(); ++i)
    {
      // Get the current function
      Function* function = (*functions)[i];

      // Add each function signature to the output
      options.Append("\n  ");
      options.Append(name);
      function->FunctionType->BuildSignatureString(options, functionCallNode.IsNamed);
    }

    // Now report the error
    return errors.Raise(location, ErrorCode::UnableToResolveFunction,
      name.c_str(),
      call.ToString().c_str(),
      options.ToString().c_str());
  }

  //***************************************************************************
  bool Overload::TestCallAndImplicitConvert(DelegateType* delegateType, FunctionCallNode& functionCallNode)
  {
    // Perform each pass one at a time (early out as soon as we get a hit)
    if (TestDelegateTypeVsCall(delegateType, functionCallNode, OverloadPass::NoImplicitConversion))
      return true;
    if (TestDelegateTypeVsCall(delegateType, functionCallNode, OverloadPass::RawImplicitConversion))
      return true;
    if (TestDelegateTypeVsCall(delegateType, functionCallNode, OverloadPass::AnyImplicitConversion))
    {
      // In the last phase, we need to actually generate implicit casts (TypeCastNodes)
      GenerateImplicitCasts(delegateType, functionCallNode);
      return true;
    }

    // If we got here, the signature did not match at all
    return false;
  }

  //***************************************************************************
  void Overload::GenerateImplicitCasts(DelegateType* delegateType, FunctionCallNode& functionCallNode)
  {
    // Grab the list of arguments and parameters for convenience
    NodeList<ExpressionNode>& arguments = functionCallNode.Arguments;
    ParameterArray& parameters = delegateType->Parameters;

    // Get the number of arguments
    size_t argumentCount = functionCallNode.Arguments.Size();
    ErrorIf(parameters.Size() != argumentCount, "We should have already verified that that the parameter count matched");

    // First check to see that the overload has all the same argument names
    for (size_t i = 0; i < argumentCount; ++i)
    {
      // Get the current delegate parameter and expression argument
      const DelegateParameter& delegateParameter = parameters[i];
      ExpressionNode* argument = arguments[i];

      // Store the types in a more human readable format
      Type* fromType = argument->ResultType;
      Type* toType = delegateParameter.ParameterType;

      // Generate the implicit conversion
      Syntaxer::ImplicitConvertAfterWalkAndIo(arguments[i], toType);
    }
  }

  //***************************************************************************
  bool Overload::TestDelegateTypeVsCall(DelegateType* delegateType, FunctionCallNode& functionCallNode, OverloadPass::Enum pass)
  {
    // Get the number of arguments
    size_t argumentCount = functionCallNode.Arguments.Size();

    // Get the arguments for the function call
    NodeList<ExpressionNode>::range arguments = functionCallNode.Arguments.All();

    // First, check that the call has the same number of arguments as the type's parameters
    // NOTE: This is important that we do this first, since the positional check below
    // test is there are no parameters (and assumes both have the same number)
    if (delegateType->Parameters.Size() != argumentCount)
      return false;

    // First check to see that the overload has all the same argument names
    for (size_t i = 0; i < delegateType->Parameters.Size(); ++i)
    {
      // Get the current delegate parameter
      const DelegateParameter& delegateParameter = delegateType->Parameters[i];

      // If we are calling with named arguments and the current parameter has a name
      if (functionCallNode.IsNamed && delegateParameter.Name.Empty() == false)
      {
        if (functionCallNode.ArgumentNames[i] != delegateParameter.Name)
        {
          // The overload did not match...
          return false;
        }
      }

      // Grab the current argument
      ExpressionNode* argument = arguments.Front();

      // We really need to make sure that all argument types are resolved
      ReturnIf
      (
        argument->ResultType == nullptr,
        false,
        "Failed to find a type for a given argument"
      );

      // Store the types in a more human readable format
      Type* fromType = argument->ResultType;
      Type* toType = delegateParameter.ParameterType;

      // Figure out what type of cast this is...
      Shared& shared = Shared::GetInstance();
      switch (pass)
      {
        case OverloadPass::NoImplicitConversion:
        {
          // In this phase, the types must match *exactly*
          if (Type::IsSame(fromType, toType) == false)
            return false;
          break;
        }

        case OverloadPass::RawImplicitConversion:
        {
          // In this phase the types can be different but must be implicitly raw convertable
          // Ex: Conversion from NullType to Animal (does no work because NullType is also a handle)
          CastOperator cast = shared.GetCastOperator(fromType, toType);
          if (cast.IsValid == false || cast.CanBeImplicit == false || cast.RequiresCodeGeneration)
            return false;
          break;
        }

        case OverloadPass::AnyImplicitConversion:
        {
          // In this phase the types can be different but must be implicitly convertable (code generation may occur)
          // Ex: Conversion from an Integer to a Real
          CastOperator cast = shared.GetCastOperator(fromType, toType);
          if (cast.IsValid == false || cast.CanBeImplicit == false)
            return false;
          break;
        }

        default:
          Error("Invalid overload pass!");
          break;
      }

      // Move the argument forward
      arguments.PopFront();
    }

    // We must have matched if we got here!
    return true;
  }

  ////***************************************************************************
  //void Overload::DetectAmbiguities(FunctionArray& functions, Function* function)
  //{
  //}
}
