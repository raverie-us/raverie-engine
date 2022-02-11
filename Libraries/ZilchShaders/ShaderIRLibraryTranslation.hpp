// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Registered zilch function that doesn't have an actual implementation. No
// function that uses this should be invoked.
void UnTranslatedBoundFunction(Zilch::Call& call, Zilch::ExceptionReport& report);
// Registered zilch function that doesn't have an implementation but shouldn't
// throw an error. Currently needed for things like FixedArray that can get
// their constructor called via pre-initialization when inspecting other
// properties.
void DummyBoundFunction(Zilch::Call& call, Zilch::ExceptionReport& report);

/// Helper struct to pass around groups of types for generating library
/// translation
struct TypeGroups
{
  ZilchShaderIRType* mVoidType;

  // Index 0 is Real1 (e.g. Real)
  Array<ZilchShaderIRType*> mRealVectorTypes;
  Array<ZilchShaderIRType*> mRealMatrixTypes;

  Array<ZilchShaderIRType*> mIntegerVectorTypes;
  Array<ZilchShaderIRType*> mBooleanVectorTypes;
  ZilchShaderIRType* mQuaternionType;
  // SpirV does not support non-floating point matrix types
  // Array<ZilchShaderIRType*> mIntegerMatrixTypes;

  ZilchShaderIRType* GetMatrixType(int y, int x)
  {
    y -= 2;
    x -= 2;
    return mRealMatrixTypes[x + y * 3];
  }
};

void ResolveSimpleFunctionFromOpType(ZilchSpirVFrontEnd* translator,
                                     Zilch::FunctionCallNode* functionCallNode,
                                     Zilch::MemberAccessNode* memberAccessNode,
                                     OpType opType,
                                     ZilchSpirVFrontEndContext* context);

// A simple helper to resolve a function (assumed to be value types) into
// calling a basic op function.
template <OpType opType>
inline void ResolveSimpleFunction(ZilchSpirVFrontEnd* translator,
                                  Zilch::FunctionCallNode* functionCallNode,
                                  Zilch::MemberAccessNode* memberAccessNode,
                                  ZilchSpirVFrontEndContext* context)
{
  return ResolveSimpleFunctionFromOpType(translator, functionCallNode, memberAccessNode, opType, context);
}

void ResolveVectorTypeCount(ZilchSpirVFrontEnd* translator,
                            Zilch::FunctionCallNode* functionCallNode,
                            Zilch::MemberAccessNode* memberAccessNode,
                            ZilchSpirVFrontEndContext* context);
void ResolvePrimitiveGet(ZilchSpirVFrontEnd* translator,
                         Zilch::FunctionCallNode* functionCallNode,
                         Zilch::MemberAccessNode* memberAccessNode,
                         ZilchSpirVFrontEndContext* context);
void ResolvePrimitiveSet(ZilchSpirVFrontEnd* translator,
                         Zilch::FunctionCallNode* functionCallNode,
                         Zilch::MemberAccessNode* memberAccessNode,
                         ZilchSpirVFrontEndContext* context);

void ResolveVectorGet(ZilchSpirVFrontEnd* translator,
                      Zilch::FunctionCallNode* functionCallNode,
                      Zilch::MemberAccessNode* memberAccessNode,
                      ZilchSpirVFrontEndContext* context);
void ResolveVectorSet(ZilchSpirVFrontEnd* translator,
                      Zilch::FunctionCallNode* functionCallNode,
                      Zilch::MemberAccessNode* memberAccessNode,
                      ZilchSpirVFrontEndContext* context);
void ResolveMatrixGet(ZilchSpirVFrontEnd* translator,
                      Zilch::FunctionCallNode* functionCallNode,
                      Zilch::MemberAccessNode* memberAccessNode,
                      ZilchSpirVFrontEndContext* context);
void ResolveMatrixSet(ZilchSpirVFrontEnd* translator,
                      Zilch::FunctionCallNode* functionCallNode,
                      Zilch::MemberAccessNode* memberAccessNode,
                      ZilchSpirVFrontEndContext* context);
void ResolveStaticBinaryFunctionOp(ZilchSpirVFrontEnd* translator,
                                   Zilch::FunctionCallNode* functionCallNode,
                                   OpType opType,
                                   ZilchSpirVFrontEndContext* context);
void TranslatePrimitiveDefaultConstructor(ZilchSpirVFrontEnd* translator,
                                          Zilch::Type* zilchResultType,
                                          ZilchSpirVFrontEndContext* context);
void TranslatePrimitiveDefaultConstructor(ZilchSpirVFrontEnd* translator,
                                          Zilch::FunctionCallNode* fnCallNode,
                                          Zilch::StaticTypeNode* staticTypeNode,
                                          ZilchSpirVFrontEndContext* context);
void TranslateBackupPrimitiveConstructor(ZilchSpirVFrontEnd* translator,
                                         Zilch::FunctionCallNode* fnCallNode,
                                         Zilch::StaticTypeNode* staticTypeNode,
                                         ZilchSpirVFrontEndContext* context);
void TranslateCompositeDefaultConstructor(ZilchSpirVFrontEnd* translator,
                                          Zilch::Type* zilchResultType,
                                          ZilchSpirVFrontEndContext* context);
void TranslateCompositeDefaultConstructor(ZilchSpirVFrontEnd* translator,
                                          Zilch::FunctionCallNode* fnCallNode,
                                          Zilch::StaticTypeNode* staticTypeNode,
                                          ZilchSpirVFrontEndContext* context);
void TranslateBackupCompositeConstructor(ZilchSpirVFrontEnd* translator,
                                         Zilch::FunctionCallNode* fnCallNode,
                                         Zilch::StaticTypeNode* staticTypeNode,
                                         ZilchSpirVFrontEndContext* context);
void TranslateMatrixDefaultConstructor(ZilchSpirVFrontEnd* translator,
                                       Zilch::Type* zilchResultType,
                                       ZilchSpirVFrontEndContext* context);
void TranslateMatrixDefaultConstructor(ZilchSpirVFrontEnd* translator,
                                       Zilch::FunctionCallNode* fnCallNode,
                                       Zilch::StaticTypeNode* staticTypeNode,
                                       ZilchSpirVFrontEndContext* context);
void TranslateMatrixFullConstructor(ZilchSpirVFrontEnd* translator,
                                    Zilch::FunctionCallNode* fnCallNode,
                                    Zilch::StaticTypeNode* staticTypeNode,
                                    ZilchSpirVFrontEndContext* context);
ZilchShaderIROp* RecursivelyTranslateCompositeSplatConstructor(ZilchSpirVFrontEnd* translator,
                                                               Zilch::FunctionCallNode* fnCallNode,
                                                               Zilch::StaticTypeNode* staticTypeNode,
                                                               ZilchShaderIRType* type,
                                                               ZilchShaderIROp* splatValueOp,
                                                               ZilchSpirVFrontEndContext* context);
void TranslateCompositeSplatConstructor(ZilchSpirVFrontEnd* translator,
                                        Zilch::FunctionCallNode* fnCallNode,
                                        Zilch::StaticTypeNode* staticTypeNode,
                                        ZilchSpirVFrontEndContext* context);
bool IsVectorSwizzle(StringParam memberName);
void ResolveScalarComponentAccess(ZilchSpirVFrontEnd* translator,
                                  Zilch::MemberAccessNode* memberAccessNode,
                                  byte componentName,
                                  ZilchSpirVFrontEndContext* context);
void ResolveScalarSwizzle(ZilchSpirVFrontEnd* translator,
                          Zilch::MemberAccessNode* memberAccessNode,
                          StringParam memberName,
                          ZilchSpirVFrontEndContext* context);
void ScalarBackupFieldResolver(ZilchSpirVFrontEnd* translator,
                               Zilch::MemberAccessNode* memberAccessNode,
                               ZilchSpirVFrontEndContext* context);
void ResolveVectorCopyConstructor(ZilchSpirVFrontEnd* translator,
                                  Zilch::FunctionCallNode* fnCallNode,
                                  Zilch::StaticTypeNode* staticTypeNode,
                                  ZilchSpirVFrontEndContext* context);
void ResolveVectorComponentAccess(ZilchSpirVFrontEnd* translator,
                                  ZilchShaderIROp* selfInstance,
                                  ZilchShaderIRType* componentType,
                                  byte componentName,
                                  ZilchSpirVFrontEndContext* context);
void ResolveVectorComponentAccess(ZilchSpirVFrontEnd* translator,
                                  Zilch::MemberAccessNode* memberAccessNode,
                                  byte componentName,
                                  ZilchSpirVFrontEndContext* context);
void ResolveVectorSwizzle(ZilchSpirVFrontEnd* translator,
                          IZilchShaderIR* selfInstance,
                          ZilchShaderIRType* resultType,
                          StringParam memberName,
                          ZilchSpirVFrontEndContext* context);
void ResolveVectorSwizzle(ZilchSpirVFrontEnd* translator,
                          Zilch::MemberAccessNode* memberAccessNode,
                          StringParam memberName,
                          ZilchSpirVFrontEndContext* context);
void VectorBackupFieldResolver(ZilchSpirVFrontEnd* translator,
                               Zilch::MemberAccessNode* memberAccessNode,
                               ZilchSpirVFrontEndContext* context);
void ResolverVectorSwizzleSetter(ZilchSpirVFrontEnd* translator,
                                 Zilch::MemberAccessNode* memberAccessNode,
                                 ZilchShaderIROp* resultValue,
                                 ZilchSpirVFrontEndContext* context);
void VectorBackupPropertySetter(ZilchSpirVFrontEnd* translator,
                                Zilch::MemberAccessNode* memberAccessNode,
                                ZilchShaderIROp* resultValue,
                                ZilchSpirVFrontEndContext* context);
bool MatrixElementAccessResolver(ZilchSpirVFrontEnd* translator,
                                 Zilch::MemberAccessNode* memberAccessNode,
                                 ZilchSpirVFrontEndContext* context,
                                 Zilch::MatrixUserData& matrixUserData);
void MatrixBackupFieldResolver(ZilchSpirVFrontEnd* translator,
                               Zilch::MemberAccessNode* memberAccessNode,
                               ZilchSpirVFrontEndContext* context);

// Quaternions
void TranslateQuaternionDefaultConstructor(ZilchSpirVFrontEnd* translator,
                                           Zilch::Type* zilchResultType,
                                           ZilchSpirVFrontEndContext* context);
void TranslateQuaternionDefaultConstructor(ZilchSpirVFrontEnd* translator,
                                           Zilch::FunctionCallNode* fnCallNode,
                                           Zilch::StaticTypeNode* staticTypeNode,
                                           ZilchSpirVFrontEndContext* context);
void QuaternionBackupFieldResolver(ZilchSpirVFrontEnd* translator,
                                   Zilch::MemberAccessNode* memberAccessNode,
                                   ZilchSpirVFrontEndContext* context);
void QuaternionBackupPropertySetter(ZilchSpirVFrontEnd* translator,
                                    Zilch::MemberAccessNode* memberAccessNode,
                                    ZilchShaderIROp* resultValue,
                                    ZilchSpirVFrontEndContext* context);
void ResolveQuaternionTypeCount(ZilchSpirVFrontEnd* translator,
                                Zilch::FunctionCallNode* functionCallNode,
                                Zilch::MemberAccessNode* memberAccessNode,
                                ZilchSpirVFrontEndContext* context);
void ResolveQuaternionGet(ZilchSpirVFrontEnd* translator,
                          Zilch::FunctionCallNode* functionCallNode,
                          Zilch::MemberAccessNode* memberAccessNode,
                          ZilchSpirVFrontEndContext* context);
void ResolveQuaternionSet(ZilchSpirVFrontEnd* translator,
                          Zilch::FunctionCallNode* functionCallNode,
                          Zilch::MemberAccessNode* memberAccessNode,
                          ZilchSpirVFrontEndContext* context);
void TranslateQuaternionSplatConstructor(ZilchSpirVFrontEnd* translator,
                                         Zilch::FunctionCallNode* fnCallNode,
                                         Zilch::StaticTypeNode* staticTypeNode,
                                         ZilchSpirVFrontEndContext* context);
void TranslateBackupQuaternionConstructor(ZilchSpirVFrontEnd* translator,
                                          Zilch::FunctionCallNode* fnCallNode,
                                          Zilch::StaticTypeNode* staticTypeNode,
                                          ZilchSpirVFrontEndContext* context);

void ResolveBinaryOp(ZilchSpirVFrontEnd* translator,
                     Zilch::BinaryOperatorNode* binaryOpNode,
                     OpType opType,
                     ZilchSpirVFrontEndContext* context);
void ResolveBinaryOp(ZilchSpirVFrontEnd* translator,
                     Zilch::BinaryOperatorNode* binaryOpNode,
                     OpType opType,
                     IZilchShaderIR* lhs,
                     IZilchShaderIR* rhs,
                     ZilchSpirVFrontEndContext* context);

void ResolveUnaryOperator(ZilchSpirVFrontEnd* translator,
                          Zilch::UnaryOperatorNode* unaryOpNode,
                          OpType opType,
                          ZilchSpirVFrontEndContext* context);

template <OpType opType>
inline void ResolveUnaryOperator(ZilchSpirVFrontEnd* translator,
                                 Zilch::UnaryOperatorNode* unaryOpNode,
                                 ZilchSpirVFrontEndContext* context)
{
  ResolveUnaryOperator(translator, unaryOpNode, opType, context);
}

template <OpType opType>
inline void ResolveBinaryOperator(ZilchSpirVFrontEnd* translator,
                                  Zilch::BinaryOperatorNode* binaryOpNode,
                                  ZilchSpirVFrontEndContext* context)
{
  ResolveBinaryOp(translator, binaryOpNode, opType, context);
}

// Shader intrinsics to write backend specific code by checking the current
// language/version
void ResolveIsLanguage(ZilchSpirVFrontEnd* translator,
                       Zilch::FunctionCallNode* functionCallNode,
                       Zilch::MemberAccessNode* memberAccessNode,
                       ZilchSpirVFrontEndContext* context);
void ResolveIsLanguageMinMaxVersion(ZilchSpirVFrontEnd* translator,
                                    Zilch::FunctionCallNode* functionCallNode,
                                    Zilch::MemberAccessNode* memberAccessNode,
                                    ZilchSpirVFrontEndContext* context);

void ResolveStaticBinaryFunctionOp(ZilchSpirVFrontEnd* translator,
                                   Zilch::FunctionCallNode* functionCallNode,
                                   OpType opType,
                                   ZilchSpirVFrontEndContext* context);
void RegisterArithmeticOps(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types);
void RegisterConversionOps(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types);
void RegisterLogicalOps(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types);
void RegisterBitOps(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types);
void RegisterGlsl450Extensions(ZilchShaderIRLibrary* shaderLibrary,
                               SpirVExtensionLibrary* extLibrary,
                               TypeGroups& types);
void AddGlslExtensionIntrinsicOps(Zilch::LibraryBuilder& builder,
                                  SpirVExtensionLibrary* extLibrary,
                                  Zilch::BoundType* type,
                                  TypeGroups& types);
void RegisterShaderIntrinsics(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary);
void RegisterColorsOps(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types);
void FixedArrayResolver(ZilchSpirVFrontEnd* translator, Zilch::BoundType* zilchFixedArrayType);
void RuntimeArrayResolver(ZilchSpirVFrontEnd* translator, Zilch::BoundType* zilchRuntimeArrayType);
void GeometryStreamInputResolver(ZilchSpirVFrontEnd* translator, Zilch::BoundType* zilchFixedArrayType);
void GeometryStreamOutputResolver(ZilchSpirVFrontEnd* translator, Zilch::BoundType* zilchFixedArrayType);

} // namespace Zero
