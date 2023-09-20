// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Registered raverie function that doesn't have an actual implementation. No
// function that uses this should be invoked.
void UnTranslatedBoundFunction(Raverie::Call& call, Raverie::ExceptionReport& report);
// Registered raverie function that doesn't have an implementation but shouldn't
// throw an error. Currently needed for things like FixedArray that can get
// their constructor called via pre-initialization when inspecting other
// properties.
void DummyBoundFunction(Raverie::Call& call, Raverie::ExceptionReport& report);

/// Helper struct to pass around groups of types for generating library
/// translation
struct RaverieTypeGroups
{
  Raverie::BoundType* mVoidType;

  // Index 0 is Real1 (e.g. Real)
  Array<Raverie::BoundType*> mRealVectorTypes;
  Array<Raverie::BoundType*> mRealMatrixTypes;

  Array<Raverie::BoundType*> mIntegerVectorTypes;
  Array<Raverie::BoundType*> mBooleanVectorTypes;
  Raverie::BoundType* mQuaternionType;
  // SpirV does not support non-floating point matrix types
  // Array<RaverieShaderIRType*> mIntegerMatrixTypes;

  Raverie::BoundType* GetMatrixType(int y, int x)
  {
    y -= 2;
    x -= 2;
    return mRealMatrixTypes[x + y * 3];
  }
};

/// Helper struct to pass around groups of types for generating library translation
struct ShaderTypeGroups
{
  RaverieShaderIRType* mVoidType;

  // Index 0 is Real1 (e.g. Real)
  Array<RaverieShaderIRType*> mRealVectorTypes;
  Array<RaverieShaderIRType*> mRealMatrixTypes;

  Array<RaverieShaderIRType*> mIntegerVectorTypes;
  Array<RaverieShaderIRType*> mBooleanVectorTypes;
  RaverieShaderIRType* mQuaternionType;
  // SpirV does not support non-floating point matrix types
  // Array<RaverieShaderIRType*> mIntegerMatrixTypes;

  RaverieShaderIRType* GetMatrixType(int y, int x)
  {
    y -= 2;
    x -= 2;
    return mRealMatrixTypes[x + y * 3];
  }
};

void ResolveSimpleFunctionFromOpType(RaverieSpirVFrontEnd* translator,
                                     Raverie::FunctionCallNode* functionCallNode,
                                     Raverie::MemberAccessNode* memberAccessNode,
                                     OpType opType,
                                     RaverieSpirVFrontEndContext* context);

// A simple helper to resolve a function (assumed to be value types) into
// calling a basic op function.
template <OpType opType>
inline void ResolveSimpleFunction(RaverieSpirVFrontEnd* translator,
                                  Raverie::FunctionCallNode* functionCallNode,
                                  Raverie::MemberAccessNode* memberAccessNode,
                                  RaverieSpirVFrontEndContext* context)
{
  return ResolveSimpleFunctionFromOpType(translator, functionCallNode, memberAccessNode, opType, context);
}

void ResolveVectorTypeCount(RaverieSpirVFrontEnd* translator,
                            Raverie::FunctionCallNode* functionCallNode,
                            Raverie::MemberAccessNode* memberAccessNode,
                            RaverieSpirVFrontEndContext* context);
void ResolvePrimitiveGet(RaverieSpirVFrontEnd* translator,
                         Raverie::FunctionCallNode* functionCallNode,
                         Raverie::MemberAccessNode* memberAccessNode,
                         RaverieSpirVFrontEndContext* context);
void ResolvePrimitiveSet(RaverieSpirVFrontEnd* translator,
                         Raverie::FunctionCallNode* functionCallNode,
                         Raverie::MemberAccessNode* memberAccessNode,
                         RaverieSpirVFrontEndContext* context);

void ResolveVectorGet(RaverieSpirVFrontEnd* translator,
                      Raverie::FunctionCallNode* functionCallNode,
                      Raverie::MemberAccessNode* memberAccessNode,
                      RaverieSpirVFrontEndContext* context);
void ResolveVectorSet(RaverieSpirVFrontEnd* translator,
                      Raverie::FunctionCallNode* functionCallNode,
                      Raverie::MemberAccessNode* memberAccessNode,
                      RaverieSpirVFrontEndContext* context);
void ResolveMatrixGet(RaverieSpirVFrontEnd* translator,
                      Raverie::FunctionCallNode* functionCallNode,
                      Raverie::MemberAccessNode* memberAccessNode,
                      RaverieSpirVFrontEndContext* context);
void ResolveMatrixSet(RaverieSpirVFrontEnd* translator,
                      Raverie::FunctionCallNode* functionCallNode,
                      Raverie::MemberAccessNode* memberAccessNode,
                      RaverieSpirVFrontEndContext* context);
void ResolveStaticBinaryFunctionOp(RaverieSpirVFrontEnd* translator,
                                   Raverie::FunctionCallNode* functionCallNode,
                                   OpType opType,
                                   RaverieSpirVFrontEndContext* context);
void TranslatePrimitiveDefaultConstructor(RaverieSpirVFrontEnd* translator,
                                          Raverie::Type* raverieResultType,
                                          RaverieSpirVFrontEndContext* context);
void TranslatePrimitiveDefaultConstructor(RaverieSpirVFrontEnd* translator,
                                          Raverie::FunctionCallNode* fnCallNode,
                                          Raverie::StaticTypeNode* staticTypeNode,
                                          RaverieSpirVFrontEndContext* context);
void TranslateBackupPrimitiveConstructor(RaverieSpirVFrontEnd* translator,
                                         Raverie::FunctionCallNode* fnCallNode,
                                         Raverie::StaticTypeNode* staticTypeNode,
                                         RaverieSpirVFrontEndContext* context);
void TranslateCompositeDefaultConstructor(RaverieSpirVFrontEnd* translator,
                                          Raverie::Type* raverieResultType,
                                          RaverieSpirVFrontEndContext* context);
void TranslateCompositeDefaultConstructor(RaverieSpirVFrontEnd* translator,
                                          Raverie::FunctionCallNode* fnCallNode,
                                          Raverie::StaticTypeNode* staticTypeNode,
                                          RaverieSpirVFrontEndContext* context);
void TranslateBackupCompositeConstructor(RaverieSpirVFrontEnd* translator,
                                         Raverie::FunctionCallNode* fnCallNode,
                                         Raverie::StaticTypeNode* staticTypeNode,
                                         RaverieSpirVFrontEndContext* context);
void TranslateMatrixDefaultConstructor(RaverieSpirVFrontEnd* translator,
                                       Raverie::Type* raverieResultType,
                                       RaverieSpirVFrontEndContext* context);
void TranslateMatrixDefaultConstructor(RaverieSpirVFrontEnd* translator,
                                       Raverie::FunctionCallNode* fnCallNode,
                                       Raverie::StaticTypeNode* staticTypeNode,
                                       RaverieSpirVFrontEndContext* context);
void TranslateMatrixFullConstructor(RaverieSpirVFrontEnd* translator,
                                    Raverie::FunctionCallNode* fnCallNode,
                                    Raverie::StaticTypeNode* staticTypeNode,
                                    RaverieSpirVFrontEndContext* context);
RaverieShaderIROp* RecursivelyTranslateCompositeSplatConstructor(RaverieSpirVFrontEnd* translator,
                                                               Raverie::FunctionCallNode* fnCallNode,
                                                               Raverie::StaticTypeNode* staticTypeNode,
                                                               RaverieShaderIRType* type,
                                                               RaverieShaderIROp* splatValueOp,
                                                               RaverieSpirVFrontEndContext* context);
void TranslateCompositeSplatConstructor(RaverieSpirVFrontEnd* translator,
                                        Raverie::FunctionCallNode* fnCallNode,
                                        Raverie::StaticTypeNode* staticTypeNode,
                                        RaverieSpirVFrontEndContext* context);
bool IsVectorSwizzle(StringParam memberName);
void ResolveScalarComponentAccess(RaverieSpirVFrontEnd* translator,
                                  Raverie::MemberAccessNode* memberAccessNode,
                                  byte componentName,
                                  RaverieSpirVFrontEndContext* context);
void ResolveScalarSwizzle(RaverieSpirVFrontEnd* translator,
                          Raverie::MemberAccessNode* memberAccessNode,
                          StringParam memberName,
                          RaverieSpirVFrontEndContext* context);
void ScalarBackupFieldResolver(RaverieSpirVFrontEnd* translator,
                               Raverie::MemberAccessNode* memberAccessNode,
                               RaverieSpirVFrontEndContext* context);
void ResolveVectorCopyConstructor(RaverieSpirVFrontEnd* translator,
                                  Raverie::FunctionCallNode* fnCallNode,
                                  Raverie::StaticTypeNode* staticTypeNode,
                                  RaverieSpirVFrontEndContext* context);
void ResolveVectorComponentAccess(RaverieSpirVFrontEnd* translator,
                                  RaverieShaderIROp* selfInstance,
                                  RaverieShaderIRType* componentType,
                                  byte componentName,
                                  RaverieSpirVFrontEndContext* context);
void ResolveVectorComponentAccess(RaverieSpirVFrontEnd* translator,
                                  Raverie::MemberAccessNode* memberAccessNode,
                                  byte componentName,
                                  RaverieSpirVFrontEndContext* context);
void ResolveVectorSwizzle(RaverieSpirVFrontEnd* translator,
                          IRaverieShaderIR* selfInstance,
                          RaverieShaderIRType* resultType,
                          StringParam memberName,
                          RaverieSpirVFrontEndContext* context);
void ResolveVectorSwizzle(RaverieSpirVFrontEnd* translator,
                          Raverie::MemberAccessNode* memberAccessNode,
                          StringParam memberName,
                          RaverieSpirVFrontEndContext* context);
void VectorBackupFieldResolver(RaverieSpirVFrontEnd* translator,
                               Raverie::MemberAccessNode* memberAccessNode,
                               RaverieSpirVFrontEndContext* context);
void ResolverVectorSwizzleSetter(RaverieSpirVFrontEnd* translator,
                                 Raverie::MemberAccessNode* memberAccessNode,
                                 RaverieShaderIROp* resultValue,
                                 RaverieSpirVFrontEndContext* context);
void VectorBackupPropertySetter(RaverieSpirVFrontEnd* translator,
                                Raverie::MemberAccessNode* memberAccessNode,
                                RaverieShaderIROp* resultValue,
                                RaverieSpirVFrontEndContext* context);
bool MatrixElementAccessResolver(RaverieSpirVFrontEnd* translator,
                                 Raverie::MemberAccessNode* memberAccessNode,
                                 RaverieSpirVFrontEndContext* context,
                                 Raverie::MatrixUserData& matrixUserData);
void MatrixBackupFieldResolver(RaverieSpirVFrontEnd* translator,
                               Raverie::MemberAccessNode* memberAccessNode,
                               RaverieSpirVFrontEndContext* context);

// Quaternions
void TranslateQuaternionDefaultConstructor(RaverieSpirVFrontEnd* translator,
                                           Raverie::Type* raverieResultType,
                                           RaverieSpirVFrontEndContext* context);
void TranslateQuaternionDefaultConstructor(RaverieSpirVFrontEnd* translator,
                                           Raverie::FunctionCallNode* fnCallNode,
                                           Raverie::StaticTypeNode* staticTypeNode,
                                           RaverieSpirVFrontEndContext* context);
void QuaternionBackupFieldResolver(RaverieSpirVFrontEnd* translator,
                                   Raverie::MemberAccessNode* memberAccessNode,
                                   RaverieSpirVFrontEndContext* context);
void QuaternionBackupPropertySetter(RaverieSpirVFrontEnd* translator,
                                    Raverie::MemberAccessNode* memberAccessNode,
                                    RaverieShaderIROp* resultValue,
                                    RaverieSpirVFrontEndContext* context);
void ResolveQuaternionTypeCount(RaverieSpirVFrontEnd* translator,
                                Raverie::FunctionCallNode* functionCallNode,
                                Raverie::MemberAccessNode* memberAccessNode,
                                RaverieSpirVFrontEndContext* context);
void ResolveQuaternionGet(RaverieSpirVFrontEnd* translator,
                          Raverie::FunctionCallNode* functionCallNode,
                          Raverie::MemberAccessNode* memberAccessNode,
                          RaverieSpirVFrontEndContext* context);
void ResolveQuaternionSet(RaverieSpirVFrontEnd* translator,
                          Raverie::FunctionCallNode* functionCallNode,
                          Raverie::MemberAccessNode* memberAccessNode,
                          RaverieSpirVFrontEndContext* context);
void TranslateQuaternionSplatConstructor(RaverieSpirVFrontEnd* translator,
                                         Raverie::FunctionCallNode* fnCallNode,
                                         Raverie::StaticTypeNode* staticTypeNode,
                                         RaverieSpirVFrontEndContext* context);
void TranslateBackupQuaternionConstructor(RaverieSpirVFrontEnd* translator,
                                          Raverie::FunctionCallNode* fnCallNode,
                                          Raverie::StaticTypeNode* staticTypeNode,
                                          RaverieSpirVFrontEndContext* context);

void ResolveBinaryOp(RaverieSpirVFrontEnd* translator,
                     Raverie::BinaryOperatorNode* binaryOpNode,
                     OpType opType,
                     RaverieSpirVFrontEndContext* context);
void ResolveBinaryOp(RaverieSpirVFrontEnd* translator,
                     Raverie::BinaryOperatorNode* binaryOpNode,
                     OpType opType,
                     IRaverieShaderIR* lhs,
                     IRaverieShaderIR* rhs,
                     RaverieSpirVFrontEndContext* context);

void ResolveUnaryOperator(RaverieSpirVFrontEnd* translator,
                          Raverie::UnaryOperatorNode* unaryOpNode,
                          OpType opType,
                          RaverieSpirVFrontEndContext* context);

template <OpType opType>
inline void ResolveUnaryOperator(RaverieSpirVFrontEnd* translator,
                                 Raverie::UnaryOperatorNode* unaryOpNode,
                                 RaverieSpirVFrontEndContext* context)
{
  ResolveUnaryOperator(translator, unaryOpNode, opType, context);
}

template <OpType opType>
inline void ResolveBinaryOperator(RaverieSpirVFrontEnd* translator,
                                  Raverie::BinaryOperatorNode* binaryOpNode,
                                  RaverieSpirVFrontEndContext* context)
{
  ResolveBinaryOp(translator, binaryOpNode, opType, context);
}

// Shader intrinsics to write backend specific code by checking the current
// language/version
void ResolveIsLanguage(RaverieSpirVFrontEnd* translator,
                       Raverie::FunctionCallNode* functionCallNode,
                       Raverie::MemberAccessNode* memberAccessNode,
                       RaverieSpirVFrontEndContext* context);
void ResolveIsLanguageMinMaxVersion(RaverieSpirVFrontEnd* translator,
                                    Raverie::FunctionCallNode* functionCallNode,
                                    Raverie::MemberAccessNode* memberAccessNode,
                                    RaverieSpirVFrontEndContext* context);

void ResolveStaticBinaryFunctionOp(RaverieSpirVFrontEnd* translator,
                                   Raverie::FunctionCallNode* functionCallNode,
                                   OpType opType,
                                   RaverieSpirVFrontEndContext* context);
void RegisterArithmeticOps(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, RaverieTypeGroups& types);
void RegisterConversionOps(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, RaverieTypeGroups& types);
void RegisterLogicalOps(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, RaverieTypeGroups& types);
void RegisterBitOps(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, RaverieTypeGroups& types);
void RegisterGlsl450Extensions(RaverieShaderIRLibrary* shaderLibrary,
                               SpirVExtensionLibrary* extLibrary,
                               RaverieTypeGroups& types);
void AddGlslExtensionIntrinsicOps(Raverie::LibraryBuilder& builder,
                                  SpirVExtensionLibrary* extLibrary,
                                  Raverie::BoundType* type,
                                  RaverieTypeGroups& types);
void RegisterShaderIntrinsics(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary);
void RegisterColorsOps(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, RaverieTypeGroups& types);
void FixedArrayResolver(RaverieSpirVFrontEnd* translator, Raverie::BoundType* raverieFixedArrayType);
void RuntimeArrayResolver(RaverieSpirVFrontEnd* translator, Raverie::BoundType* raverieRuntimeArrayType);
void GeometryStreamInputResolver(RaverieSpirVFrontEnd* translator, Raverie::BoundType* raverieFixedArrayType);
void GeometryStreamOutputResolver(RaverieSpirVFrontEnd* translator, Raverie::BoundType* raverieFixedArrayType);

} // namespace Raverie
