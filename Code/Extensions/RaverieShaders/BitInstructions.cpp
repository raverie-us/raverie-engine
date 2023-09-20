// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void ResolveIntBitCount(RaverieSpirVFrontEnd* translator,
                        Raverie::FunctionCallNode* functionCallNode,
                        Raverie::MemberAccessNode* memberAccessNode,
                        RaverieSpirVFrontEndContext* context)
{
  RaverieShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  RaverieShaderIROp* baseOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  RaverieShaderIROp* bitCountOp = translator->BuildCurrentBlockIROp(OpType::OpBitCount, resultType, baseOp, context);
  context->PushIRStack(bitCountOp);
}

// Register function callbacks for all bit operations (see Bit Instructions in
// the spir-v spec). Some functions aren't implemented here as raverie doesn't
// have a corresponding function. Everything else should be implemented on the
// ShaderIntrinsics type.
void RegisterBitOps(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, RaverieTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;
  TypeResolvers& mathTypeResolver = shaderLibrary->mTypeResolvers[mathType];
  OperatorResolvers& opResolvers = shaderLibrary->mOperatorResolvers;

  Raverie::BoundType* realType = core.RealType;
  Raverie::BoundType* intType = core.IntegerType;
  Raverie::BoundType* boolType = core.BooleanType;

  // Register ops that are on all integer vector types
  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mIntegerVectorTypes[i];
    String raverieTypeName = raverieType->ToString();

    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "CountBits", raverieTypeName),
                                              ResolveIntBitCount);

    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::BitwiseOr, ResolveBinaryOperator<OpType::OpBitwiseOr>);
    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::BitwiseAnd, ResolveBinaryOperator<OpType::OpBitwiseAnd>);
    opResolvers.RegisterBinaryOpResolver(
        raverieType, raverieType, Raverie::Grammar::BitwiseXor, ResolveBinaryOperator<OpType::OpBitwiseXor>);
    opResolvers.RegisterUnaryOpResolver(raverieType, Raverie::Grammar::BitwiseNot, ResolveUnaryOperator<OpType::OpNot>);
  }
}

} // namespace Raverie
