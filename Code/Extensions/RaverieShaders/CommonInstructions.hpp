// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

void DummyDefaultConstructorResolver(RaverieSpirVFrontEnd* translator, Raverie::Type* resultType, RaverieSpirVFrontEndContext* context);
void DummyConstructorCallResolver(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* fnCallNode, Raverie::StaticTypeNode* staticTypeNode, RaverieSpirVFrontEndContext* context);
void DummyMemberAccessResolver(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context);
void DummyMemberFunctionResolver(RaverieSpirVFrontEnd* translator, Raverie::FunctionCallNode* functionCallNode, Raverie::MemberAccessNode* memberAccessNode, RaverieSpirVFrontEndContext* context);
void DummyMemberPropertySetterResolver(RaverieSpirVFrontEnd* translator, Raverie::MemberAccessNode* memberAccessNode, RaverieShaderIROp* resultValue, RaverieSpirVFrontEndContext* context);
void DummyBinaryOpResolver(RaverieSpirVFrontEnd* translator, Raverie::BinaryOperatorNode* binaryOpNode, RaverieSpirVFrontEndContext* context);
void DummyUnaryOpResolver(RaverieSpirVFrontEnd* translator, Raverie::UnaryOperatorNode* binaryOpNode, RaverieSpirVFrontEndContext* context);
void DummyTypeCastResolver(RaverieSpirVFrontEnd* translator, Raverie::TypeCastNode* binaryOpNode, RaverieSpirVFrontEndContext* context);
void DummyTemplateTypeReslover(RaverieSpirVFrontEnd* translator, Raverie::BoundType* boundType);
void DummyExpressionInitializerResolver(RaverieSpirVFrontEnd* translator, Raverie::ExpressionInitializerNode*& node, RaverieSpirVFrontEndContext* context);

} // namespace Raverie
