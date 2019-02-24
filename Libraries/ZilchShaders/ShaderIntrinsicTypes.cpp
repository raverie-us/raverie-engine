// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

// Bind some extra types to zilch for simulating shaders
namespace Zilch
{

using namespace Zero;

template <OpType opType>
Zilch::Function* AddFunction(Zilch::LibraryBuilder& builder,
                             Zilch::BoundType* owner,
                             Zilch::BoundType* returnType,
                             StringParam fnName,
                             ParameterArray& params)
{
  Zilch::Function* zilchFn = builder.AddBoundFunction(
      owner, fnName, UnTranslatedBoundFunction, params, returnType, Zilch::FunctionOptions::Static);
  zilchFn->UserData = (void*)&ResolveSimpleFunction<opType>;
  return zilchFn;
}

template <OpType opType>
Zilch::Function*
AddFunction(Zilch::LibraryBuilder& builder, Zilch::BoundType* owner, Zilch::BoundType* returnType, StringParam fnName)
{
  ParameterArray params;
  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Zilch::Function* AddFunction(Zilch::LibraryBuilder& builder,
                             Zilch::BoundType* owner,
                             Zilch::BoundType* returnType,
                             StringParam fnName,
                             Zilch::BoundType* param0Type,
                             StringParam param0Name)
{
  DelegateParameter param0(param0Type);
  param0.Name = param0Name;
  ParameterArray params;
  params.PushBack(param0);
  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Zilch::Function* AddFunction(Zilch::LibraryBuilder& builder,
                             Zilch::BoundType* owner,
                             Zilch::BoundType* returnType,
                             StringParam fnName,
                             Zilch::BoundType* param0Type)
{
  return AddFunction<opType>(builder, owner, returnType, fnName, param0Type, String());
}

template <OpType opType>
Zilch::Function* AddFunction(Zilch::LibraryBuilder& builder,
                             Zilch::BoundType* owner,
                             Zilch::BoundType* returnType,
                             StringParam fnName,
                             Zilch::BoundType* param0Type,
                             StringParam param0Name,
                             Zilch::BoundType* param1Type,
                             StringParam param1Name)
{
  DelegateParameter param0(param0Type);
  param0.Name = param0Name;
  DelegateParameter param1(param1Type);
  param1.Name = param1Name;

  ParameterArray params;
  params.PushBack(param0);
  params.PushBack(param1);
  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Zilch::Function* AddFunction(Zilch::LibraryBuilder& builder,
                             Zilch::BoundType* owner,
                             Zilch::BoundType* returnType,
                             StringParam fnName,
                             Zilch::BoundType* param0Type,
                             Zilch::BoundType* param1Type)
{
  return AddFunction<opType>(builder, owner, returnType, fnName, param0Type, String(), param1Type, String());
}

template <OpType opType>
Zilch::Function* AddFunction(Zilch::LibraryBuilder& builder,
                             Zilch::BoundType* owner,
                             Zilch::BoundType* returnType,
                             StringParam fnName,
                             Zilch::BoundType* param0Type,
                             StringParam param0Name,
                             Zilch::BoundType* param1Type,
                             StringParam param1Name,
                             Zilch::BoundType* param2Type,
                             StringParam param2Name)
{
  DelegateParameter param0(param0Type);
  param0.Name = param0Name;
  DelegateParameter param1(param1Type);
  param1.Name = param1Name;
  DelegateParameter param2(param2Type);
  param2.Name = param2Name;

  ParameterArray params;
  params.PushBack(param0);
  params.PushBack(param1);
  params.PushBack(param2);
  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Zilch::Function* AddFunction(Zilch::LibraryBuilder& builder,
                             Zilch::BoundType* owner,
                             Zilch::BoundType* returnType,
                             StringParam fnName,
                             Zilch::BoundType* param0Type,
                             Zilch::BoundType* param1Type,
                             Zilch::BoundType* param2Type)
{
  return AddFunction<opType>(
      builder, owner, returnType, fnName, param0Type, String(), param1Type, String(), param2Type, String());
}

template <OpType opType>
Zilch::Function* AddFunction(Zilch::LibraryBuilder& builder,
                             Zilch::BoundType* owner,
                             Zilch::BoundType* returnType,
                             StringParam fnName,
                             Zilch::BoundType* param0Type,
                             StringParam param0Name,
                             Zilch::BoundType* param1Type,
                             StringParam param1Name,
                             Zilch::BoundType* param2Type,
                             StringParam param2Name,
                             Zilch::BoundType* param3Type,
                             StringParam param3Name)
{
  DelegateParameter param0(param0Type);
  param0.Name = param0Name;
  DelegateParameter param1(param1Type);
  param1.Name = param1Name;
  DelegateParameter param2(param2Type);
  param2.Name = param2Name;
  DelegateParameter param3(param3Type);
  param3.Name = param3Name;

  ParameterArray params;
  params.PushBack(param0);
  params.PushBack(param1);
  params.PushBack(param2);
  params.PushBack(param3);

  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Zilch::Function* AddFunction(Zilch::LibraryBuilder& builder,
                             Zilch::BoundType* owner,
                             Zilch::BoundType* returnType,
                             StringParam fnName,
                             Zilch::BoundType* param0Type,
                             Zilch::BoundType* param1Type,
                             Zilch::BoundType* param2Type,
                             Zilch::BoundType* param3Type)
{
  return AddFunction<opType>(builder,
                             owner,
                             returnType,
                             fnName,
                             param0Type,
                             String(),
                             param1Type,
                             String(),
                             param2Type,
                             String(),
                             param3Type,
                             String());
}

void AddMathOps(Zilch::LibraryBuilder& builder, Zilch::BoundType* type, TypeGroups& types)
{
  Zilch::BoundType* voidType = ZilchTypeId(void);
  Zilch::BoundType* boolType = types.mBooleanVectorTypes[0]->mZilchType;
  Zilch::BoundType* intType = types.mIntegerVectorTypes[0]->mZilchType;
  Zilch::BoundType* realType = types.mRealVectorTypes[0]->mZilchType;

  for (size_t i = 0; i < types.mBooleanVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mBooleanVectorTypes[i]->mZilchType;
    Zilch::BoundType* vectorBoolType = zilchType;

    // Unary
    AddFunction<OpType::OpLogicalNot>(builder, type, zilchType, "LogicalNot", zilchType);
    // Any/all only exists on vector types
    if (i != 0)
    {
      AddFunction<OpType::OpAny>(builder, type, boolType, "Any", zilchType);
      AddFunction<OpType::OpAll>(builder, type, boolType, "All", zilchType);
    }

    // Binary
    AddFunction<OpType::OpLogicalEqual>(builder, type, zilchType, "LogicalEqual", zilchType, zilchType);
    AddFunction<OpType::OpLogicalNotEqual>(builder, type, zilchType, "LogicalNotEqual", zilchType, zilchType);
    AddFunction<OpType::OpLogicalOr>(builder, type, zilchType, "LogicalOr", zilchType, zilchType);
    AddFunction<OpType::OpLogicalAnd>(builder, type, zilchType, "LogicalAnd", zilchType, zilchType);

    AddFunction<OpType::OpSelect>(
        builder, type, zilchType, "Select", vectorBoolType, "condition", zilchType, "obj1", zilchType, "obj2");
  }

  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mIntegerVectorTypes[i]->mZilchType;
    Zilch::BoundType* vectorBoolType = types.mBooleanVectorTypes[i]->mZilchType;

    // Unary
    AddFunction<OpType::OpBitReverse>(builder, type, zilchType, "BitReverse", zilchType);
    AddFunction<OpType::OpBitCount>(builder, type, zilchType, "BitCount", zilchType);
    AddFunction<OpType::OpNot>(builder, type, zilchType, "Not", zilchType);

    // Binary
    AddFunction<OpType::OpShiftRightLogical>(
        builder, type, zilchType, "ShiftRightLogical", zilchType, "base", zilchType, "shift");
    AddFunction<OpType::OpShiftRightArithmetic>(
        builder, type, zilchType, "ShiftRightArithmetic", zilchType, "base", zilchType, "shift");
    AddFunction<OpType::OpShiftLeftLogical>(
        builder, type, zilchType, "ShiftLeftLogical", zilchType, "base", zilchType, "shift");
    AddFunction<OpType::OpBitwiseOr>(builder, type, zilchType, "BitwiseOr", zilchType, zilchType);
    AddFunction<OpType::OpBitwiseXor>(builder, type, zilchType, "BitwiseXor", zilchType, zilchType);
    AddFunction<OpType::OpBitwiseAnd>(builder, type, zilchType, "BitwiseAnd", zilchType, zilchType);
    AddFunction<OpType::OpIEqual>(builder, type, vectorBoolType, "Equal", zilchType, zilchType);
    AddFunction<OpType::OpINotEqual>(builder, type, vectorBoolType, "NotEqual", zilchType, zilchType);
    AddFunction<OpType::OpSGreaterThan>(builder, type, vectorBoolType, "GreaterThan", zilchType, zilchType);
    AddFunction<OpType::OpSGreaterThanEqual>(builder, type, vectorBoolType, "GreaterThanEqual", zilchType, zilchType);
    AddFunction<OpType::OpSLessThan>(builder, type, vectorBoolType, "LessThan", zilchType, zilchType);
    AddFunction<OpType::OpSLessThanEqual>(builder, type, vectorBoolType, "LessThanEqual", zilchType, zilchType);

    AddFunction<OpType::OpSRem>(builder, type, zilchType, "Remainder", zilchType, zilchType);
    AddFunction<OpType::OpSMod>(builder, type, zilchType, "Mod", zilchType, zilchType);

    // Tri
    AddFunction<OpType::OpSelect>(
        builder, type, zilchType, "Select", vectorBoolType, "condition", zilchType, "obj1", zilchType, "obj2");
    AddFunction<OpType::OpBitFieldSExtract>(
        builder, type, zilchType, "BitFieldExtract", zilchType, "base", intType, "offset", intType, "count");

    // Quad
    AddFunction<OpType::OpBitFieldInsert>(builder,
                                          type,
                                          zilchType,
                                          "BitFieldInsert",
                                          zilchType,
                                          "base",
                                          zilchType,
                                          "insert",
                                          intType,
                                          "offset",
                                          intType,
                                          "count");
  }

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchType = types.mRealVectorTypes[i]->mZilchType;
    Zilch::BoundType* vectorBoolType = types.mBooleanVectorTypes[i]->mZilchType;

    // Unary
    AddFunction<OpType::OpDPdx>(builder, type, zilchType, "Ddx", zilchType)
        ->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
    AddFunction<OpType::OpDPdy>(builder, type, zilchType, "Ddy", zilchType)
        ->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
    AddFunction<OpType::OpFwidth>(builder, type, zilchType, "FWidth", zilchType)
        ->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
    AddFunction<OpType::OpIsNan>(builder, type, vectorBoolType, "IsNan", zilchType);
    AddFunction<OpType::OpIsInf>(builder, type, vectorBoolType, "IsInf", zilchType);

    // Binary
    AddFunction<OpType::OpFRem>(builder, type, zilchType, "Remainder", zilchType, zilchType);
    AddFunction<OpType::OpFMod>(builder, type, zilchType, "Mod", zilchType, zilchType);

    AddFunction<OpType::OpFOrdEqual>(builder, type, vectorBoolType, "OrderedEqual", zilchType, zilchType);
    AddFunction<OpType::OpFOrdNotEqual>(builder, type, vectorBoolType, "OrderedNotEqual", zilchType, zilchType);
    AddFunction<OpType::OpFOrdLessThan>(builder, type, vectorBoolType, "OrderedLessThan", zilchType, zilchType);
    AddFunction<OpType::OpFOrdLessThanEqual>(
        builder, type, vectorBoolType, "OrderedLessThanEqual", zilchType, zilchType);
    AddFunction<OpType::OpFOrdGreaterThan>(builder, type, vectorBoolType, "OrderedGreaterThan", zilchType, zilchType);
    AddFunction<OpType::OpFOrdGreaterThanEqual>(
        builder, type, vectorBoolType, "OrderedGreaterThanEqual", zilchType, zilchType);

    // Any/all only exists on vector types
    if (i != 0)
    {
      AddFunction<OpType::OpDot>(builder, type, realType, "Dot", zilchType, zilchType);
      AddFunction<OpType::OpVectorTimesScalar>(builder, type, zilchType, "VectorTimesScalar", zilchType, realType);
    }

    // Not implemented in glsl
    AddFunction<OpType::OpFUnordEqual>(builder, type, vectorBoolType, "UnorderedEqual", zilchType, zilchType);
    AddFunction<OpType::OpFUnordNotEqual>(builder, type, vectorBoolType, "UnorderedNotEqual", zilchType, zilchType);
    AddFunction<OpType::OpFUnordLessThan>(builder, type, vectorBoolType, "UnorderedLessThan", zilchType, zilchType);
    AddFunction<OpType::OpFUnordLessThanEqual>(
        builder, type, vectorBoolType, "UnorderedLessThanEqual", zilchType, zilchType);
    AddFunction<OpType::OpFUnordGreaterThan>(
        builder, type, vectorBoolType, "UnorderedGreaterThan", zilchType, zilchType);
    AddFunction<OpType::OpFUnordGreaterThanEqual>(
        builder, type, vectorBoolType, "UnorderedGreaterThanEqual", zilchType, zilchType);

    // Trinary
    AddFunction<OpType::OpSelect>(
        builder, type, zilchType, "Select", vectorBoolType, "condition", zilchType, "obj1", zilchType, "obj2");
  }

  for (size_t y = 2; y <= 4; ++y)
  {
    for (size_t x = 2; x <= 4; ++x)
    {
      ZilchShaderIRType* shaderType = types.GetMatrixType(y, x);
      Zilch::BoundType* zilchType = shaderType->mZilchType;

      // Unary
      AddFunction<OpType::OpTranspose>(builder, type, zilchType, "Transpose", zilchType);

      // Binary
      AddFunction<OpType::OpMatrixTimesScalar>(builder, type, zilchType, "MatrixTimesScalar", zilchType, realType);

      // Ignore for now (have to figure out row/column order stuff)
      // AddFunction<OpType::OpMatrixTimesMatrix>(builder, type, zilchType,
      // "MatrixTimesMatrix", zilchType, zilchType);
    }
  }

  // Conversion
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Zilch::BoundType* zilchRealType = types.mRealVectorTypes[i]->mZilchType;
    Zilch::BoundType* zilchIntType = types.mIntegerVectorTypes[i]->mZilchType;
    Zilch::BoundType* zilchBoolType = types.mBooleanVectorTypes[i]->mZilchType;

    AddFunction<OpType::OpConvertFToS>(builder, type, zilchIntType, "ConvertFToS", zilchRealType);
    AddFunction<OpType::OpConvertSToF>(builder, type, zilchRealType, "ConvertSToF", zilchIntType);

    AddFunction<OpType::OpBitcast>(builder, type, zilchRealType, "BitCastToReal", zilchIntType);
    AddFunction<OpType::OpBitcast>(builder, type, zilchIntType, "BitCastToInteger", zilchRealType);
  }
}

ZilchDefineType(ShaderIntrinsics, builder, type)
{
  Zilch::BoundType* voidType = ZilchTypeId(void);
  Zilch::BoundType* boolType = ZilchTypeId(bool);
  Zilch::BoundType* intType = ZilchTypeId(int);

  ZilchShaderIRCore& shaderCore = ZilchShaderIRCore::GetInstance();
  TypeGroups& types = shaderCore.mTypes;

  // This technically needs to be restricted to pixel fragment types.
  AddFunction<OpType::OpKill>(builder, type, voidType, "Kill")
      ->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  AddMathOps(builder, type, types);
  AddGlslExtensionIntrinsicOps(builder, shaderCore.mGlsl450ExtensionsLibrary, type, types);
  AddImageFunctions(builder, type, types);

  Zilch::ParameterArray parameters = OneParameter(intType, "language");
  Zilch::Function* isLanguageFn = builder.AddBoundFunction(
      type, "IsLanguage", Zero::DummyBoundFunction, parameters, boolType, Zilch::FunctionOptions::Static);
  isLanguageFn->UserData = (void*)&ResolveIsLanguage;

  parameters = ThreeParameters(intType, "language", intType, "minVersion", intType, "maxVersion");
  isLanguageFn = builder.AddBoundFunction(
      type, "IsLanguage", Zero::DummyBoundFunction, parameters, boolType, Zilch::FunctionOptions::Static);
  isLanguageFn->UserData = (void*)&ResolveIsLanguageMinMaxVersion;
}

ZilchDefineType(GeometryStreamUserData, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

void GeometryStreamUserData::Set(spv::ExecutionMode executionMode)
{
  mExecutionMode = executionMode;
  // Use the execution mode to compute the size of this stream and the
  // input/output mode
  if (executionMode == spv::ExecutionModeInputPoints)
  {
    mInput = true;
    mSize = 1;
  }
  else if (executionMode == spv::ExecutionModeInputLines)
  {
    mInput = true;
    mSize = 2;
  }
  else if (executionMode == spv::ExecutionModeTriangles)
  {
    mInput = true;
    mSize = 3;
  }
  else if (executionMode == spv::ExecutionModeOutputPoints)
  {
    mInput = false;
    mSize = 1;
  }
  else if (executionMode == spv::ExecutionModeOutputLineStrip)
  {
    mInput = false;
    mSize = 2;
  }
  else if (executionMode == spv::ExecutionModeOutputTriangleStrip)
  {
    mInput = false;
    mSize = 3;
  }
}

ZilchDefineType(GeometryFragmentUserData, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

GeometryFragmentUserData::GeometryFragmentUserData()
{
  mInputStreamType = nullptr;
  mOutputStreamType = nullptr;
}

ZilchShaderIRType* GeometryFragmentUserData::GetInputVertexType()
{
  IZilchShaderIR* param = mInputStreamType->mParameters[0];
  ZilchShaderIRType* subType = param->As<ZilchShaderIRType>();
  return subType;
}

ZilchShaderIRType* GeometryFragmentUserData::GetOutputVertexType()
{
  IZilchShaderIR* param = mOutputStreamType->mParameters[0];
  ZilchShaderIRType* subType = param->As<ZilchShaderIRType>();
  return subType;
}

ZilchDefineType(ComputeFragmentUserData, builder, type)
{
  ZilchBindDefaultCopyDestructor();
}

ComputeFragmentUserData::ComputeFragmentUserData()
{
  mLocalSizeX = 1;
  mLocalSizeY = 1;
  mLocalSizeZ = 1;
}

} // namespace Zilch
