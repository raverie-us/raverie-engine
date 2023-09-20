// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "GLSL.std.450.h"

namespace Raverie
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

RaverieShaderIROp* MakeBasicExtensionFunction(
    RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, int extensionOpId, RaverieShaderExtensionImport* importLibraryIR, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);
  RaverieShaderIRConstantLiteral* instructionLiteral = translator->GetOrCreateConstantLiteral(extensionOpId);
  RaverieShaderIROp* extensionOp = translator->BuildIROpNoBlockAdd(OpType::OpExtInst, resultType, context);
  extensionOp->mArguments.PushBack(importLibraryIR);
  extensionOp->mArguments.PushBack(instructionLiteral);
  return extensionOp;
}

void WriteExtensionFunctionArguments(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode*& node, RaverieShaderIROp* functionCallOp, RaverieSpirVFrontEndContext* context)
{
  // @JoshD: Writing arguments for an extension function is slightly different than
  // writing arguments for a regular function. It would be nice to refactor this later...
  for (size_t i = 0; i < node->Arguments.Size(); ++i)
  {
    IRaverieShaderIR* argument = translator->WalkAndGetResult(node->Arguments[i], context);
    translator->WriteFunctionCallArgument(argument, functionCallOp, nullptr, context);
  }

  // Write any postamble copies (these should never happen)
  translator->WriteFunctionCallPostamble(context);

  // If there is a return then we have to push the result onto a stack so any assignments can get the value
  Raverie::Type* returnType = node->ResultType;
  if (returnType != RaverieTypeId(void))
  {
    context->PushIRStack(functionCallOp);
  }
}

template <int extensionOpId>
void BasicExtensionFunction(RaverieSpirVFrontEnd* translator,
                            Raverie::FunctionCallNode* functionCallNode,
                            Raverie::MemberAccessNode* memberAccessNode,
                            RaverieShaderExtensionImport* importLibraryIR,
                            RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* extensionOp = MakeBasicExtensionFunction(translator, functionCallNode, extensionOpId, importLibraryIR, context);

  // Write the remaining function arguments
  WriteExtensionFunctionArguments(translator, functionCallNode, extensionOp, context);

  BasicBlock* currentBlock = context->GetCurrentBlock();
  currentBlock->mLines.PushBack(extensionOp);
}

void ResolveGlslExtensionFunction(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  ExtensionLibraryUserData& userData = memberAccessNode->AccessedFunction->ComplexUserData.ReadObject<ExtensionLibraryUserData>(0);
  RaverieShaderExtensionImport* importOp = translator->mLibrary->FindExtensionLibraryImport(userData.mExtensionLibrary);
  RaverieShaderIROp* extensionOp = MakeBasicExtensionFunction(translator, functionCallNode, userData.mOpCode, importOp, context);

  // Write the remaining function arguments
  WriteExtensionFunctionArguments(translator, functionCallNode, extensionOp, context);

  BasicBlock* currentBlock = context->GetCurrentBlock();
  currentBlock->mLines.PushBack(extensionOp);
}

// SpirV FSign returns a float but in raverie it returns an int.
void GenerateFSign(RaverieSpirVFrontEnd* translator,
                   Raverie::FunctionCallNode* functionCallNode,
                   Raverie::MemberAccessNode* memberAccessNode,
                   RaverieShaderExtensionImport* importLibraryIR,
                   RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();

  RaverieShaderIRType* intResultType = translator->FindType(functionCallNode->ResultType, functionCallNode);
  RaverieShaderIRType* realResultType = translator->FindType(functionCallNode->Arguments[0]->ResultType, functionCallNode);
  // Convert the FSign instruction with the float type
  RaverieShaderIRConstantLiteral* instructionLiteral = translator->GetOrCreateConstantLiteral((int)GLSLstd450FSign);
  RaverieShaderIROp* extensionOp = translator->BuildIROpNoBlockAdd(OpType::OpExtInst, realResultType, context);
  extensionOp->mArguments.PushBack(importLibraryIR);
  extensionOp->mArguments.PushBack(instructionLiteral);

  // Write the remaining function arguments
  WriteExtensionFunctionArguments(translator, functionCallNode, extensionOp, context);
  // Pop the extension op off the stack and write the instruction to the current
  // block
  context->PopIRStack();
  currentBlock->mLines.PushBack(extensionOp);

  // Now write out the conversion from float to int so the types match raverie
  RaverieShaderIROp* intSignOp = translator->BuildIROp(currentBlock, OpType::OpConvertFToS, intResultType, extensionOp, context);
  context->PushIRStack(intSignOp);
}

void GenerateAngleAndTrigFunctions(SpirVExtensionLibrary* extLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mRealVectorTypes[i];
    String raverieTypeName = raverieType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ToRadians", raverieTypeName), BasicExtensionFunction<GLSLstd450Radians>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ToDegrees", raverieTypeName), BasicExtensionFunction<GLSLstd450Degrees>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Cos", raverieTypeName), BasicExtensionFunction<GLSLstd450Cos>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sin", raverieTypeName), BasicExtensionFunction<GLSLstd450Sin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Tan", raverieTypeName), BasicExtensionFunction<GLSLstd450Tan>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ACos", raverieTypeName), BasicExtensionFunction<GLSLstd450Acos>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ASin", raverieTypeName), BasicExtensionFunction<GLSLstd450Asin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ATan", raverieTypeName), BasicExtensionFunction<GLSLstd450Atan>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ATan2", raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450Atan2>);

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Cosh", raverieTypeName), BasicExtensionFunction<GLSLstd450Cosh>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sinh", raverieTypeName), BasicExtensionFunction<GLSLstd450Sinh>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Tanh", raverieTypeName), BasicExtensionFunction<GLSLstd450Tanh>);
  }
}

void GenerateExponentialFunctions(SpirVExtensionLibrary* extLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mRealVectorTypes[i];
    String raverieTypeName = raverieType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Pow", raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450Pow>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Log", raverieTypeName), BasicExtensionFunction<GLSLstd450Log>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Exp", raverieTypeName), BasicExtensionFunction<GLSLstd450Exp>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Log2", raverieTypeName), BasicExtensionFunction<GLSLstd450Log2>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Exp2", raverieTypeName), BasicExtensionFunction<GLSLstd450Exp2>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sqrt", raverieTypeName), BasicExtensionFunction<GLSLstd450Sqrt>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "RSqrt", raverieTypeName), BasicExtensionFunction<GLSLstd450InverseSqrt>);
  }
}

void GenerateCommonFloatFunctions(SpirVExtensionLibrary* extLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mRealVectorTypes[i];
    String raverieTypeName = raverieType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Abs", raverieTypeName), BasicExtensionFunction<GLSLstd450FAbs>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sign", raverieTypeName), GenerateFSign);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Floor", raverieTypeName), BasicExtensionFunction<GLSLstd450Floor>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Ceil", raverieTypeName), BasicExtensionFunction<GLSLstd450Ceil>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Frac", raverieTypeName), BasicExtensionFunction<GLSLstd450Fract>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Truncate", raverieTypeName), BasicExtensionFunction<GLSLstd450Trunc>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Round", raverieTypeName), BasicExtensionFunction<GLSLstd450Round>);

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Min", raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450FMin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Max", raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450FMax>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Clamp", raverieTypeName, raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450FClamp>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Lerp", raverieTypeName, raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450FMix>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Step", raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450Step>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "SmoothStep", raverieTypeName, raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450SmoothStep>);
  }
}

void GenerateCommonIntFunctions(SpirVExtensionLibrary* extLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;

  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mIntegerVectorTypes[i];
    String raverieTypeName = raverieType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Abs", raverieTypeName), BasicExtensionFunction<GLSLstd450SAbs>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Clamp", raverieTypeName, raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450SClamp>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Min", raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450SMin>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Max", raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450SMax>);

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Sign", raverieTypeName), BasicExtensionFunction<GLSLstd450SSign>);
  }
}

void GenerateGeometricFloatFunctions(SpirVExtensionLibrary* extLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;

  String realName = core.RealType->ToString();
  String real2Name = core.Real2Type->ToString();
  String real3Name = core.Real3Type->ToString();

  for (size_t i = 1; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mRealVectorTypes[i];
    String raverieTypeName = raverieType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Distance", raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450Distance>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Length", raverieTypeName), BasicExtensionFunction<GLSLstd450Length>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Normalize", raverieTypeName), BasicExtensionFunction<GLSLstd450Normalize>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "ReflectAcrossPlane", raverieTypeName, raverieTypeName), BasicExtensionFunction<GLSLstd450Reflect>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Refract", raverieTypeName, raverieTypeName, realName), BasicExtensionFunction<GLSLstd450Refract>);
  }

  extLibrary->CreateExtInst(GetStaticFunction(mathType, "Cross", real3Name, real3Name), BasicExtensionFunction<GLSLstd450Cross>);
}

void CreateFloatMatrixFunctions(SpirVExtensionLibrary* extLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;

  for (size_t i = 2; i <= 4; ++i)
  {
    Raverie::BoundType* raverieType = types.GetMatrixType(i, i);
    String raverieTypeName = raverieType->Name;

    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Invert", raverieTypeName), BasicExtensionFunction<GLSLstd450MatrixInverse>);
    extLibrary->CreateExtInst(GetStaticFunction(mathType, "Determinant", raverieTypeName), BasicExtensionFunction<GLSLstd450Determinant>);
  }
}

// Registers callback functions for all of the glsl 450 extension library
// instructions that exist in raverie
void RegisterGlsl450Extensions(RaverieShaderIRLibrary* shaderLibrary, SpirVExtensionLibrary* extLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;

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
void AddGlslIntrinsic(Raverie::LibraryBuilder& builder,
                      Raverie::BoundType* type,
                      SpirVExtensionLibrary* extLibrary,
                      int glslOpId,
                      StringParam fnName,
                      const Raverie::ParameterArray& parameters,
                      Raverie::BoundType* returnType)
{
  Raverie::Function* fn = builder.AddBoundFunction(type, fnName, UnTranslatedBoundFunction, parameters, returnType, Raverie::FunctionOptions::Static);
  fn->UserData = (void*)&ResolveGlslExtensionFunction;
  fn->ComplexUserData.WriteObject(ExtensionLibraryUserData(glslOpId, extLibrary));
}

/// Adds all relevant glsl extension operations to the ShaderIntrinsics type,
/// including non-supported instructions in raverie.
void AddGlslExtensionIntrinsicOps(Raverie::LibraryBuilder& builder, SpirVExtensionLibrary* extLibrary, Raverie::BoundType* type, RaverieTypeGroups& types)
{
  Raverie::BoundType* realType = types.mRealVectorTypes[0];
  Raverie::BoundType* real3Type = types.mRealVectorTypes[2];

  // Reals
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mRealVectorTypes[i];

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Round, "Round", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450RoundEven, "RoundEven", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Trunc, "Trunc", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FAbs, "FAbs", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FSign, "FSign", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Floor, "Floor", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Ceil, "Ceil", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Fract, "Fract", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Radians, "Radians", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Degrees, "Degrees", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Sin, "Sin", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Cos, "Cos", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Tan, "Tan", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Sinh, "Sinh", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Cosh, "Cosh", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Tanh, "Tanh", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Asin, "ASin", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Acos, "ACos", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Atan, "ATan", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Asinh, "ASinh", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Acosh, "ACosh", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Atanh, "ATanh", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Atan2, "ATan2", TwoParameters(raverieType, "y", raverieType, "x"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Pow, "Pow", TwoParameters(raverieType, "base", raverieType, "exp"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Exp, "Exp", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Log, "Log", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Exp2, "Exp2", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Log2, "Log2", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Sqrt, "Sqrt", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450InverseSqrt, "InverseSqrt", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FMin, "FMin", TwoParameters(raverieType, raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FMax, "FMax", TwoParameters(raverieType, raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FClamp, "FClamp", ThreeParameters(raverieType, "value", raverieType, "minValue", raverieType, "maxValue"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FMix, "FMix", ThreeParameters(raverieType, "start", raverieType, "end", raverieType, "t"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Step, "Step", TwoParameters(raverieType, "y", raverieType, "x"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SmoothStep, "SmoothStep", ThreeParameters(raverieType, "start", raverieType, "end", raverieType, "t"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Fma, "Fma", ThreeParameters(raverieType, "a", raverieType, "b", raverieType, "c"), raverieType);

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Length, "Length", OneParameter(raverieType), realType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Distance, "Distance", TwoParameters(raverieType, raverieType), realType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Normalize, "Normalize", OneParameter(raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FaceForward, "FaceForward", ThreeParameters(raverieType, "n", raverieType, "i", raverieType, "nRef"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Reflect, "Reflect", TwoParameters(raverieType, "i", raverieType, "n"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Refract, "Refract", ThreeParameters(raverieType, "i", raverieType, "n", realType, "eta"), raverieType);

    // Causes SpirV-Cross exceptions
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450NMin, "NMin",
    // TwoParameters(raverieType, raverieType), raverieType);
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450NMax, "NMax",
    // TwoParameters(raverieType, raverieType), raverieType);
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450NClamp, "NClamp",
    // ThreeParameters(raverieType, "value", raverieType, "minValue", raverieType,
    // "maxValue"), raverieType);

    // Requires pointer types
    // AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Modf, "Modf",
    // TwoParameters(raverieType, "value"), raverieType);
  }

  // Integer
  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mIntegerVectorTypes[i];

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SAbs, "SAbs", OneParameter(raverieType, "value"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SSign, "SSign", OneParameter(raverieType, "value"), raverieType);

    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SMin, "SMin", TwoParameters(raverieType, raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SMax, "SMax", TwoParameters(raverieType, raverieType), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450SClamp, "SClamp", ThreeParameters(raverieType, "value", raverieType, "minValue", raverieType, "maxValue"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FindILsb, "FindLeastSignificantBit", OneParameter(raverieType, "value"), raverieType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450FindSMsb, "FindMostSignificantBit", OneParameter(raverieType, "value"), raverieType);
  }

  // Matrices
  for (size_t i = 2; i <= 4; ++i)
  {
    Raverie::BoundType* raverieType = types.GetMatrixType(i, i);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Determinant, "Determinant", OneParameter(raverieType), realType);
    AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450MatrixInverse, "MatrixInverse", OneParameter(raverieType), raverieType);
  }

  AddGlslIntrinsic(builder, type, extLibrary, GLSLstd450Cross, "Cross", TwoParameters(real3Type, real3Type), real3Type);
}

} // namespace Raverie
