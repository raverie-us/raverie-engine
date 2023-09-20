// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void ResolveOpBitcast(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  IRaverieShaderIR* operand = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  IRaverieShaderIR* operation = translator->BuildCurrentBlockIROp(OpType::OpBitcast, resultType, operand, context);
  context->PushIRStack(operation);
}

template <OpType opType>
void ResolveOpCast(RaverieSpirVFrontEnd* translator, Raverie::TypeCastNode* node, RaverieSpirVFrontEndContext* context)
{
  translator->PerformTypeCast(node, opType, context);
}

void ResolveFromBoolCast(RaverieSpirVFrontEnd* translator, Raverie::TypeCastNode* node, IRaverieShaderIR* zero, IRaverieShaderIR* one, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIROp* condition = translator->WalkAndGetValueTypeResult(node->Operand, context);
  RaverieShaderIRType* destType = translator->FindType(node->ResultType, node);
  BasicBlock* currentBlock = context->GetCurrentBlock();
  IRaverieShaderIR* operation = translator->GenerateFromBoolCast(currentBlock, condition, destType, zero, one, context);
  context->PushIRStack(operation);
}

void ResolveBoolToIntCast(RaverieSpirVFrontEnd* translator, Raverie::TypeCastNode* node, RaverieSpirVFrontEndContext* context)
{
  IRaverieShaderIR* one = translator->GetIntegerConstant(1, context);
  IRaverieShaderIR* zero = translator->GetIntegerConstant(0, context);
  ResolveFromBoolCast(translator, node, zero, one, context);
}

void ResolveBoolToRealCast(RaverieSpirVFrontEnd* translator, Raverie::TypeCastNode* node, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* realType = translator->FindType(RaverieTypeId(float), node, context);
  IRaverieShaderIR* one = translator->GetConstant(realType, 1.0f, context);
  IRaverieShaderIR* zero = translator->GetConstant(realType, 0.0f, context);
  ResolveFromBoolCast(translator, node, zero, one, context);
}

void ResolveToBoolCast(RaverieSpirVFrontEnd* translator, Raverie::TypeCastNode* node, OpType op, IRaverieShaderIR* zero, RaverieSpirVFrontEndContext* context)
{
  BasicBlock* currentBlock = context->GetCurrentBlock();
  RaverieShaderIROp* condition = translator->WalkAndGetValueTypeResult(node->Operand, context);
  RaverieShaderIRType* destType = translator->FindType(node->ResultType, node);
  IRaverieShaderIR* operation = translator->GenerateToBoolCast(currentBlock, op, condition, destType, zero, context);
  context->PushIRStack(operation);
}

void ResolveIntToBoolCast(RaverieSpirVFrontEnd* translator, Raverie::TypeCastNode* node, RaverieSpirVFrontEndContext* context)
{
  IRaverieShaderIR* zero = translator->GetIntegerConstant(0, context);
  ResolveToBoolCast(translator, node, OpType::OpINotEqual, zero, context);
}

void ResolveRealToBoolCast(RaverieSpirVFrontEnd* translator, Raverie::TypeCastNode* node, RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* realType = translator->FindType(RaverieTypeId(float), node, context);
  IRaverieShaderIR* zero = translator->GetConstant(realType, 0.0f, context);
  ResolveToBoolCast(translator, node, OpType::OpFOrdNotEqual, zero, context);
}

// Register function callbacks for all conversion operations (see Conversion
// Instructions in the spir-v spec). Some functions aren't implemented here as
// raverie doesn't have a corresponding function. Everything else should be
// implemented on the ShaderIntrinsics type.
void RegisterConversionOps(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  OperatorResolvers& opResolvers = shaderLibrary->mOperatorResolvers;

  // Iterate over all dimensions of vector types (including scalar) to build
  // all supported conversions. Note: Bool conversions are not explicit
  // instructions in spir-v and must be generated from multiple instructions.
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* floatType = types.mRealVectorTypes[i];
    Raverie::BoundType* intType = types.mIntegerVectorTypes[i];
    Raverie::BoundType* boolType = types.mBooleanVectorTypes[i];

    opResolvers.RegisterTypeCastOpResolver(floatType, intType, ResolveOpCast<OpType::OpConvertFToS>);
    opResolvers.RegisterTypeCastOpResolver(intType, floatType, ResolveOpCast<OpType::OpConvertSToF>);

    opResolvers.RegisterTypeCastOpResolver(boolType, intType, ResolveBoolToIntCast);
    opResolvers.RegisterTypeCastOpResolver(intType, boolType, ResolveIntToBoolCast);

    opResolvers.RegisterTypeCastOpResolver(boolType, floatType, ResolveBoolToRealCast);
    opResolvers.RegisterTypeCastOpResolver(floatType, boolType, ResolveRealToBoolCast);
  }

  // Reinterpret cast is only supported between int and real (scalar) in raverie.
  Raverie::BoundType* realType = core.RealType;
  Raverie::BoundType* intType = core.IntegerType;
  // Register the re-interpret cast functions
  TypeResolvers& realTypeResolver = shaderLibrary->mTypeResolvers[realType];
  realTypeResolver.RegisterFunctionResolver(GetStaticFunction(realType, "Reinterpret", intType->ToString()), ResolveOpBitcast);
  TypeResolvers& intTypeResolver = shaderLibrary->mTypeResolvers[intType];
  intTypeResolver.RegisterFunctionResolver(GetStaticFunction(intType, "Reinterpret", realType->ToString()), ResolveOpBitcast);
}

} // namespace Raverie
