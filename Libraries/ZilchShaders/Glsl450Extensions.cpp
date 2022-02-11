// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "GLSL.std.450.h"

namespace Zero
{

// Helper data used to translate glsl intrinsic functions
struct ExtensionLibraryUserData
{
  ExtensionLibraryUserData()
  {
    mOpCode = 0;
    mExtensionLibrary = nullptr;
  }
  ExtensionLibraryUserData(int opCode, SpirVExtensionLibrary* extensionLibrary)
  {
    mOpCode = opCode;
    mExtensionLibrary = extensionLibrary;
  }
  // The op-code to call.
  int mOpCode;
  // The extension library required for this op-code
  SpirVExtensionLibrary* mExtensionLibrary;
};

ZilchShaderIROp* MakeBasicExtensionFunction(ZilchSpirVFrontEnd* translator,
                                            Zilch::FunctionCallNode* functionCallNode,
                                            int extensionOpId,
                                            ZilchShaderExtensionImport* importLibraryIR,
                                            ZilchSpirVFrontEndContext* context)
{
  ZilchShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);
  ZilchShaderIRConstantLiteral* instructionLiteral = translator->GetOrCreateConstantLiteral(extensionOpId);
  ZilchShaderIROp* extensionOp = translator->BuildIROpNoBlockAdd(OpType::OpExtInst, resultType, context);
  extensionOp->mArguments.PushBack(importLibraryIR);
  extensionOp->mArguments.PushBack(instructionLiteral);
  return extensionOp;
}

void WriteExtensionFunctionArguments(ZilchSpirVFrontEnd* translator,
                                     Zilch::FunctionCallNode*& node,
                                     ZilchShaderIROp* functionCallOp,
                                     ZilchSpirVFrontEndContext* context)
{
  // @JoshD: Writing arguments for an extension function is slightly different than
  // writing arguments for a regular function. It would be nice to refactor this later...
  for (size_t i = 0; i < node->Arguments.Size(); ++i)
  {
    IZilchShaderIR* argument = translator->WalkAndGetResult(node->Arguments[i], context);
    translator->WriteFunctionCallArgument(argument, functionCallOp, nullptr, context);
  }

  // Write any postamble copies (these should never happen)
  translator->WriteFunctionCallPostamble(context);

  // If there is a return then we have to push the result onto a stack so any assignments can get the value
  Zilch::Type* returnType = node->ResultType;
  if (returnType != ZilchTypeId(void))
  {
    context->PushIRStack(functionCallOp);
  }
}

template <int extensionOpId>
void BasicExtensionFunction(ZilchSpirVFrontEnd* translator,
                            Zilch::FunctionCallNode* functionCallNode,
                            Zilch::MemberAccessNode* memberAccessNode,
                            ZilchShaderExtensionImport* importLibraryIR,
                            ZilchSpirVFrontEndContext* context)
{
  ZilchShaderIROp* extensionOp =
      MakeBasicExtensionFunction(translator, functionCallNode, extensionOpId, importLibraryIR, context);

  // Write the remaining function arguments
  WriteExtensionFunctionArguments(translator, functionCallNode, extensionOp, context);

  BasicBlock* currentBlock = context->GetCurrentBlock();
  currentBlock->mLines.PushBack(extensionOp);
}

void ResolveGlslExtensionFunction(ZilchSpirVFrontEnd* translator,
                                  Zilch::FunctionCallNode* functionCallNode,
                                  Zilch::MemberAccessNode* memberAccessNode,
                                  ZilchSpirVFrontEndContext* context)
{
  ExtensionLibraryUserData& userData =
      memberAccessNode->AccessedFunction->ComplexUserData.ReadObject<ExtensionLibraryUserData>(0);
  ZilchShaderExtensionImport* importOp = translator->mLibrary->FindExtensionLibraryImport(userData.mExtensionLibrary);
  ZilchShaderIROp* extensionOp =
      MakeBasicExtensionFunction(translator, functionCallNode, userData.mOpCode, importOp, context);

  // Write the remaining function arguments
  WriteExtensionFunctionArguments(translator, functionCallNode, extensionOp, context);

  BasicBlock* currentBlock = context->GetCurrentBlock();
  currentBlock->mLines.PushBack(extensionOp);
}

// SpirV FSign returns a float but in zilch it returns an int.
void GenerateFSign(ZilchSpirVFrontEnd* translator,
                   Zilch::FunctionCallNode* functionCallNode,
                   Zilch::MemberAccessNode* memberAccessNode,
                   ZilchShaderExtensionImport* importLibraryIR,
                   ZilchSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  ZilchShaderIRType* intResultType = translator->FindType(functionCallNode->ResultType, functionCallNode);
  ZilchShaderIRType* realResultType =
      translator->FindType(functionCallNode->Arguments[0]->ResultType, functionCallNode);
  // Convert the FSign instruction with the float type
  ZilchShaderIRConstantLiteral* instructionLiteral = translator->GetOrCreateConstantLiteral((int)GLSLstd450FSign);
  ZilchShaderIROp* extensionOp = translator->BuildIROpNoBlockAdd(OpType::OpExtInst, realResultType, context);
  extensionOp->mArguments.PushBack(importLibraryIR);
  extensionOp->mArguments.PushBack(instructionLiteral);

  // Write the remaining function arguments
  WriteExtensionFunctionArguments(translator, functionCallNode, extensionOp, context);
  // Pop the extension op off the stack and write the instruction to the current
  // block
  context->PopIRStack();
  currentBlock->mLines.PushBack(extensionOp);

  // Now write out the conversion from float to int so the types match zilch
  ZilchShaderIROp* intSignOp =
      translator->BuildIROp(currentBlock, OpType::OpConvertFToS, intResultType, extensionOp, context);
  context->PushIRStack(intSignOp);
}

void GenerateAngleAndTrigFunctions(SpirVExtensionLibrary* extLibrary, ZilchTypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mRealVectorTypes[i];
    String zilchTypeName = zilchType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ToRadians", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Radians>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ToDegrees", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Degrees>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Cos", zilchTypeName), BasicExtensionFunction<GLSLstd450Cos>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sin", zilchTypeName), BasicExtensionFunction<GLSLstd450Sin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Tan", zilchTypeName), BasicExtensionFunction<GLSLstd450Tan>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ACos", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Acos>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ASin", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Asin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ATan", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Atan>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ATan2", zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Atan2>);

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Cosh", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Cosh>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sinh", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Sinh>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Tanh", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Tanh>);
  }
}

void GenerateExponentialFunctions(SpirVExtensionLibrary* extLibrary, ZilchTypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mRealVectorTypes[i];
    String zilchTypeName = zilchType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Pow", zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Pow>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Log", zilchTypeName), BasicExtensionFunction<GLSLstd450Log>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Exp", zilchTypeName), BasicExtensionFunction<GLSLstd450Exp>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Log2", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Log2>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Exp2", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Exp2>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sqrt", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Sqrt>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "RSqrt", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450InverseSqrt>);
  }
}

void GenerateCommonFloatFunctions(SpirVExtensionLibrary* extLibrary, ZilchTypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mRealVectorTypes[i];
    String zilchTypeName = zilchType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Abs", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450FAbs>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sign", zilchTypeName), GenerateFSign);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Floor", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Floor>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Ceil", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Ceil>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Frac", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Fract>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Truncate", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Trunc>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Round", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Round>);

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Min", zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450FMin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Max", zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450FMax>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Clamp", zilchTypeName, zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450FClamp>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Lerp", zilchTypeName, zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450FMix>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Step", zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Step>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "SmoothStep", zilchTypeName, zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450SmoothStep>);
  }
}

void GenerateCommonIntFunctions(SpirVExtensionLibrary* extLibrary, ZilchTypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mIntegerVectorTypes[i];
    String zilchTypeName = zilchType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Abs", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450SAbs>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Clamp", zilchTypeName, zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450SClamp>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Min", zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450SMin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Max", zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450SMax>);

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sign", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450SSign>);
  }
}

void GenerateGeometricFloatFunctions(SpirVExtensionLibrary* extLibrary, ZilchTypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* mathType = core.MathType;

  String realName = core.RealType->ToString();
  String real2Name = core.Real2Type->ToString();
  String real3Name = core.Real3Type->ToString();

  for (size_t i = 1; i < types.mRealVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mRealVectorTypes[i];
    String zilchTypeName = zilchType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Distance", zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Distance>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Length", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Length>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Normalize", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Normalize>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ReflectAcrossPlane", zilchTypeName, zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Reflect>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Refract", zilchTypeName, zilchTypeName, realName),
                              BasicExtensionFunction<GLSLstd450Refract>);
  }

  extLibrary->CreateExtInst(GetStaticFunction(mathType, "Cross", real3Name, real3Name),
                            BasicExtensionFunction<GLSLstd450Cross>);
}

void CreateFloatMatrixFunctions(SpirVExtensionLibrary* extLibrary, ZilchTypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* mathType = core.MathType;

  for (size_t i = 2; i <= 4; ++i)
  {
    Zilch::BoundType* zilchType = types.GetMatrixType(i, i);
    String zilchTypeName = zilchType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Invert", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450MatrixInverse>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Determinant", zilchTypeName),
                              BasicExtensionFunction<GLSLstd450Determinant>);
  }
}

// Registers callback functions for all of the glsl 450 extension library
// instructions that exist in zilch
void RegisterGlsl450Extensions(ZilchShaderIRLibrary* shaderLibrary,
                               SpirVExtensionLibrary* extLibrary,
                               ZilchTypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* mathType = core.MathType;

  extLibrary->mName = "GLSL.std.450";
  extLibrary->mOwningLibrary = shaderLibrary;

  GenerateAngleAndTrigFunctions(extLibrary, types);
  GenerateExponentialFunctions(extLibrary, types);
  GenerateCommonIntFunctions(extLibrary, types);
  GenerateCommonFloatFunctions(extLibrary, types);
  GenerateGeometricFloatFunctions(extLibrary, types);
  CreateFloatMatrixFunctions(extLibrary, types);
}

// Simple helper to look up a zich function via name and type and then map it to
// a glsl extension instruction
void AddGlslIntrinsic(Zilch::LibraryBuilder& builder,
                      Zilch::BoundType* type,
                      SpirVExtensionLibrary* extLibrary,
                      int glslOpId,
                      StringParam fnName,
                      const Zilch::ParameterArray& parameters,
                      Zilch::BoundType* returnType)
{
  Zilch::Function* fn = builder.AddBoundFunction(
      type, fnName, UnTranslatedBoundFunction, parameters, returnType, Zilch::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveGlslExtensionFunction;
  fn->ComplexUserData.WriteObject(ExtensionLibraryUserData(glslOpId, extLibrary));
}

/// Adds all relevant glsl extension operations to the ShaderIntrinsics type,
/// including non-supported instructions in zilch.
void AddGlslExtensionIntrinsicOps(Zilch::LibraryBuilder& builder,
                                  SpirVExtensionLibrary* extLibrary,
                                  Zilch::BoundType* type,
                                  ZilchTypeGroups& types)
{
  Zilch::BoundType* realType = types.mRealVectorTypes[0];
  Zilch::BoundType* real3Type = types.mRealVectorTypes[2];

  // Reals
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mRealVectorTypes[i];

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Round, "Round", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450RoundEven, "RoundEven", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Trunc, "Trunc", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FAbs, "FAbs", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FSign, "FSign", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Floor, "Floor", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Ceil, "Ceil", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Fract, "Fract", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Radians, "Radians", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Degrees, "Degrees", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Sin, "Sin", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Cos, "Cos", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Tan, "Tan", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Sinh, "Sinh", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Cosh, "Cosh", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Tanh, "Tanh", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Asin, "ASin", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Acos, "ACos", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Atan, "ATan", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Asinh, "ASinh", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Acosh, "ACosh", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Atanh, "ATanh", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450Atan2, "ATan2", TwoParameters(zilchType, "y", zilchType, "x"), zilchType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450Pow, "Pow", TwoParameters(zilchType, "base", zilchType, "exp"), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Exp, "Exp", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Log, "Log", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Exp2, "Exp2", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Log2, "Log2", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Sqrt, "Sqrt", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450InverseSqrt, "InverseSqrt", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FMin, "FMin", TwoParameters(zilchType, zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FMax, "FMax", TwoParameters(zilchType, zilchType), zilchType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450FClamp,
                     "FClamp",
                     ThreeParameters(zilchType, "value", zilchType, "minValue", zilchType, "maxValue"),
                     zilchType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450FMix,
                     "FMix",
                     ThreeParameters(zilchType, "start", zilchType, "end", zilchType, "t"),
                     zilchType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450Step, "Step", TwoParameters(zilchType, "y", zilchType, "x"), zilchType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450SmoothStep,
                     "SmoothStep",
                     ThreeParameters(zilchType, "start", zilchType, "end", zilchType, "t"),
                     zilchType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450Fma,
                     "Fma",
                     ThreeParameters(zilchType, "a", zilchType, "b", zilchType, "c"),
                     zilchType);

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Length, "Length", OneParameter(zilchType), realType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450Distance, "Distance", TwoParameters(zilchType, zilchType), realType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Normalize, "Normalize", OneParameter(zilchType), zilchType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450FaceForward,
                     "FaceForward",
                     ThreeParameters(zilchType, "n", zilchType, "i", zilchType, "nRef"),
                     zilchType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450Reflect,
                     "Reflect",
                     TwoParameters(zilchType, "i", zilchType, "n"),
                     zilchType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450Refract,
                     "Refract",
                     ThreeParameters(zilchType, "i", zilchType, "n", realType, "eta"),
                     zilchType);

    // Causes SpirV-Cross exceptions
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450NMin, "NMin",
    // TwoParameters(zilchType, zilchType), zilchType);
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450NMax, "NMax",
    // TwoParameters(zilchType, zilchType), zilchType);
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450NClamp, "NClamp",
    // ThreeParameters(zilchType, "value", zilchType, "minValue", zilchType,
    // "maxValue"), zilchType);

    // Requires pointer types
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Modf, "Modf",
    // TwoParameters(zilchType, "value"), zilchType);
  }

  // Integer
  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mIntegerVectorTypes[i];

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SAbs, "SAbs", OneParameter(zilchType, "value"), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SSign, "SSign", OneParameter(zilchType, "value"), zilchType);

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SMin, "SMin", TwoParameters(zilchType, zilchType), zilchType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SMax, "SMax", TwoParameters(zilchType, zilchType), zilchType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450SClamp,
                     "SClamp",
                     ThreeParameters(zilchType, "value", zilchType, "minValue", zilchType, "maxValue"),
                     zilchType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450FindILsb,
                     "FindLeastSignificantBit",
                     OneParameter(zilchType, "value"),
                     zilchType);
    AddGlslIntrinsic(builder,
                     type,
                     extLibrary,
                     GLSLstd450FindSMsb,
                     "FindMostSignificantBit",
                     OneParameter(zilchType, "value"),
                     zilchType);
  }

  // Matrices
  for (size_t i = 2; i <= 4; ++i)
  {
    Zilch::BoundType* zilchType = types.GetMatrixType(i, i);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450Determinant, "Determinant", OneParameter(zilchType), realType);
    AddGlslIntrinsic(
        builder, type, extLibrary, GLSLstd450MatrixInverse, "MatrixInverse", OneParameter(zilchType), zilchType);
  }

  AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Cross, "Cross", TwoParameters(real3Type, real3Type), real3Type);
}

} // namespace Zero
