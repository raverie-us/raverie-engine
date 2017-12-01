///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void DummyDefaultConstructorResolver(ZilchSpirVFrontEnd* translator, Zilch::Type* resultType, ZilchSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyConstructorCallResolver(ZilchSpirVFrontEnd* translator, Zilch::FunctionCallNode* fnCallNode, Zilch::StaticTypeNode* staticTypeNode, ZilchSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyMemberAccessResolver(ZilchSpirVFrontEnd* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyMemberFunctionResolver(ZilchSpirVFrontEnd* translator, Zilch::FunctionCallNode* functionCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyMemberPropertySetterResolver(ZilchSpirVFrontEnd* translator, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderIROp* resultValue, ZilchSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyBinaryOpResolver(ZilchSpirVFrontEnd* translator, Zilch::BinaryOperatorNode* binaryOpNode, ZilchSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyUnaryOpResolver(ZilchSpirVFrontEnd* translator, Zilch::UnaryOperatorNode* binaryOpNode, ZilchSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyTypeCastResolver(ZilchSpirVFrontEnd* translator, Zilch::TypeCastNode* binaryOpNode, ZilchSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyTemplateTypeReslover(ZilchSpirVFrontEnd* translator, Zilch::BoundType* boundType)
{
  Error("This should not be called");
}

void DummyExpressionInitializerResolver(ZilchSpirVFrontEnd* translator, Zilch::ExpressionInitializerNode*& node, ZilchSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

}//namespace Zero
