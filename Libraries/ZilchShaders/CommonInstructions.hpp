// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

void DummyDefaultConstructorResolver(ZilchSpirVFrontEnd* translator,
                                     Zilch::Type* resultType,
                                     ZilchSpirVFrontEndContext* context);
void DummyConstructorCallResolver(ZilchSpirVFrontEnd* translator,
                                  Zilch::FunctionCallNode* fnCallNode,
                                  Zilch::StaticTypeNode* staticTypeNode,
                                  ZilchSpirVFrontEndContext* context);
void DummyMemberAccessResolver(ZilchSpirVFrontEnd* translator,
                               Zilch::MemberAccessNode* memberAccessNode,
                               ZilchSpirVFrontEndContext* context);
void DummyMemberFunctionResolver(ZilchSpirVFrontEnd* translator,
                                 Zilch::FunctionCallNode* functionCallNode,
                                 Zilch::MemberAccessNode* memberAccessNode,
                                 ZilchSpirVFrontEndContext* context);
void DummyMemberPropertySetterResolver(ZilchSpirVFrontEnd* translator,
                                       Zilch::MemberAccessNode* memberAccessNode,
                                       ZilchShaderIROp* resultValue,
                                       ZilchSpirVFrontEndContext* context);
void DummyBinaryOpResolver(ZilchSpirVFrontEnd* translator,
                           Zilch::BinaryOperatorNode* binaryOpNode,
                           ZilchSpirVFrontEndContext* context);
void DummyUnaryOpResolver(ZilchSpirVFrontEnd* translator,
                          Zilch::UnaryOperatorNode* binaryOpNode,
                          ZilchSpirVFrontEndContext* context);
void DummyTypeCastResolver(ZilchSpirVFrontEnd* translator,
                           Zilch::TypeCastNode* binaryOpNode,
                           ZilchSpirVFrontEndContext* context);
void DummyTemplateTypeReslover(ZilchSpirVFrontEnd* translator, Zilch::BoundType* boundType);
void DummyExpressionInitializerResolver(ZilchSpirVFrontEnd* translator,
                                        Zilch::ExpressionInitializerNode*& node,
                                        ZilchSpirVFrontEndContext* context);

} // namespace Zero
