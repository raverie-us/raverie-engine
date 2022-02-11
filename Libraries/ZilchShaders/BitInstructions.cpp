// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void ResolveIntBitCount(ZilchSpirVFrontEnd* translator,
                        Zilch::FunctionCallNode* functionCallNode,
                        Zilch::MemberAccessNode* memberAccessNode,
                        ZilchSpirVFrontEndContext* context)
{
  ZilchShaderIRType* resultType = translator->FindType(functionCallNode->ResultType, functionCallNode);

  ZilchShaderIROp* baseOp = translator->WalkAndGetValueTypeResult(functionCallNode->Arguments[0], context);
  ZilchShaderIROp* bitCountOp = translator->BuildCurrentBlockIROp(OpType::OpBitCount, resultType, baseOp, context);
  context->PushIRStack(bitCountOp);
}

// Register function callbacks for all bit operations (see Bit Instructions in
// the spir-v spec). Some functions aren't implemented here as zilch doesn't
// have a corresponding function. Everything else should be implemented on the
// ShaderIntrinsics type.
void RegisterBitOps(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, ZilchTypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* mathType = core.MathType;
  TypeResolvers& mathTypeResolver = shaderLibrary->mTypeResolvers[mathType];
  OperatorResolvers& opResolvers = shaderLibrary->mOperatorResolvers;

  Zilch::BoundType* realType = core.RealType;
  Zilch::BoundType* intType = core.IntegerType;
  Zilch::BoundType* boolType = core.BooleanType;

  // Register ops that are on all integer vector types
  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mIntegerVectorTypes[i];
    String zilchTypeName = zilchType->ToString();

    mathTypeResolver.RegisterFunctionResolver(GetStaticFunction(mathType, "CountBits", zilchTypeName),
                                              ResolveIntBitCount);

    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::BitwiseOr, ResolveBinaryOperator<OpType::OpBitwiseOr>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::BitwiseAnd, ResolveBinaryOperator<OpType::OpBitwiseAnd>);
    opResolvers.RegisterBinaryOpResolver(
        zilchType, zilchType, Zilch::Grammar::BitwiseXor, ResolveBinaryOperator<OpType::OpBitwiseXor>);
    opResolvers.RegisterUnaryOpResolver(zilchType, Zilch::Grammar::BitwiseNot, ResolveUnaryOperator<OpType::OpNot>);
  }
}

} // namespace Zero
