// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void DummyDefaultConstructorResolver(RaverieSpirVFrontEnd* translator,
                                     Raverie::Type* resultType,
                                     RaverieSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyConstructorCallResolver(RaverieSpirVFrontEnd* translator,
                                  Raverie::FunctionCallNode* fnCallNode,
                                  Raverie::StaticTypeNode* staticTypeNode,
                                  RaverieSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyMemberAccessResolver(RaverieSpirVFrontEnd* translator,
                               Raverie::MemberAccessNode* memberAccessNode,
                               RaverieSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyMemberFunctionResolver(RaverieSpirVFrontEnd* translator,
                                 Raverie::FunctionCallNode* functionCallNode,
                                 Raverie::MemberAccessNode* memberAccessNode,
                                 RaverieSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyMemberPropertySetterResolver(RaverieSpirVFrontEnd* translator,
                                       Raverie::MemberAccessNode* memberAccessNode,
                                       RaverieShaderIROp* resultValue,
                                       RaverieSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyBinaryOpResolver(RaverieSpirVFrontEnd* translator,
                           Raverie::BinaryOperatorNode* binaryOpNode,
                           RaverieSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyUnaryOpResolver(RaverieSpirVFrontEnd* translator,
                          Raverie::UnaryOperatorNode* binaryOpNode,
                          RaverieSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyTypeCastResolver(RaverieSpirVFrontEnd* translator,
                           Raverie::TypeCastNode* binaryOpNode,
                           RaverieSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

void DummyTemplateTypeReslover(RaverieSpirVFrontEnd* translator, Raverie::BoundType* boundType)
{
  Error("This should not be called");
}

void DummyExpressionInitializerResolver(RaverieSpirVFrontEnd* translator,
                                        Raverie::ExpressionInitializerNode*& node,
                                        RaverieSpirVFrontEndContext* context)
{
  Error("This should not be called");
}

} // namespace Raverie
