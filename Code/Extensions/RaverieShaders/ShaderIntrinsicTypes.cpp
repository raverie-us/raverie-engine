// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

// Bind some extra types to raverie for simulating shaders
namespace Raverie
{

using namespace Raverie;

template <OpType opType>
Raverie::Function* AddFunction(Raverie::LibraryBuilder& builder, Raverie::BoundType* owner, Raverie::BoundType* returnType, StringParam fnName, ParameterArray& params)
{
  Raverie::Function* raverieFn = builder.AddBoundFunction(owner, fnName, UnTranslatedBoundFunction, params, returnType, Raverie::FunctionOptions::Static);
  raverieFn->UserData = (void*)&ResolveSimpleFunction<opType>;
  return raverieFn;
}

template <OpType opType>
Raverie::Function* AddFunction(Raverie::LibraryBuilder& builder, Raverie::BoundType* owner, Raverie::BoundType* returnType, StringParam fnName)
{
  ParameterArray params;
  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Raverie::Function* AddFunction(Raverie::LibraryBuilder& builder, Raverie::BoundType* owner, Raverie::BoundType* returnType, StringParam fnName, Raverie::BoundType* param0Type, StringParam param0Name)
{
  DelegateParameter param0(param0Type);
  param0.Name = param0Name;
  ParameterArray params;
  params.PushBack(param0);
  return AddFunction<opType>(builder, owner, returnType, fnName, params);
}

template <OpType opType>
Raverie::Function* AddFunction(Raverie::LibraryBuilder& builder, Raverie::BoundType* owner, Raverie::BoundType* returnType, StringParam fnName, Raverie::BoundType* param0Type)
{
  return AddFunction<opType>(builder, owner, returnType, fnName, param0Type, String());
}

template <OpType opType>
Raverie::Function* AddFunction(Raverie::LibraryBuilder& builder,
                               Raverie::BoundType* owner,
                               Raverie::BoundType* returnType,
                               StringParam fnName,
                               Raverie::BoundType* param0Type,
                               StringParam param0Name,
                               Raverie::BoundType* param1Type,
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
Raverie::Function*
AddFunction(Raverie::LibraryBuilder& builder, Raverie::BoundType* owner, Raverie::BoundType* returnType, StringParam fnName, Raverie::BoundType* param0Type, Raverie::BoundType* param1Type)
{
  return AddFunction<opType>(builder, owner, returnType, fnName, param0Type, String(), param1Type, String());
}

template <OpType opType>
Raverie::Function* AddFunction(Raverie::LibraryBuilder& builder,
                               Raverie::BoundType* owner,
                               Raverie::BoundType* returnType,
                               StringParam fnName,
                               Raverie::BoundType* param0Type,
                               StringParam param0Name,
                               Raverie::BoundType* param1Type,
                               StringParam param1Name,
                               Raverie::BoundType* param2Type,
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
Raverie::Function* AddFunction(Raverie::LibraryBuilder& builder,
                               Raverie::BoundType* owner,
                               Raverie::BoundType* returnType,
                               StringParam fnName,
                               Raverie::BoundType* param0Type,
                               Raverie::BoundType* param1Type,
                               Raverie::BoundType* param2Type)
{
  return AddFunction<opType>(builder, owner, returnType, fnName, param0Type, String(), param1Type, String(), param2Type, String());
}

template <OpType opType>
Raverie::Function* AddFunction(Raverie::LibraryBuilder& builder,
                               Raverie::BoundType* owner,
                               Raverie::BoundType* returnType,
                               StringParam fnName,
                               Raverie::BoundType* param0Type,
                               StringParam param0Name,
                               Raverie::BoundType* param1Type,
                               StringParam param1Name,
                               Raverie::BoundType* param2Type,
                               StringParam param2Name,
                               Raverie::BoundType* param3Type,
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
Raverie::Function* AddFunction(Raverie::LibraryBuilder& builder,
                               Raverie::BoundType* owner,
                               Raverie::BoundType* returnType,
                               StringParam fnName,
                               Raverie::BoundType* param0Type,
                               Raverie::BoundType* param1Type,
                               Raverie::BoundType* param2Type,
                               Raverie::BoundType* param3Type)
{
  return AddFunction<opType>(builder, owner, returnType, fnName, param0Type, String(), param1Type, String(), param2Type, String(), param3Type, String());
}

void AddMathOps(Raverie::LibraryBuilder& builder, Raverie::BoundType* type, RaverieTypeGroups& types)
{
  Raverie::BoundType* voidType = RaverieTypeId(void);
  Raverie::BoundType* boolType = types.mBooleanVectorTypes[0];
  Raverie::BoundType* intType = types.mIntegerVectorTypes[0];
  Raverie::BoundType* realType = types.mRealVectorTypes[0];

  for (size_t i = 0; i < types.mBooleanVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mBooleanVectorTypes[i];
    Raverie::BoundType* vectorBoolType = raverieType;

    // Unary
    AddFunction<OpType::OpLogicalNot>(builder, type, raverieType, "LogicalNot", raverieType);
    // Any/all only exists on vector types
    if (i != 0)
    {
      AddFunction<OpType::OpAny>(builder, type, boolType, "Any", raverieType);
      AddFunction<OpType::OpAll>(builder, type, boolType, "All", raverieType);
    }

    // Binary
    AddFunction<OpType::OpLogicalEqual>(builder, type, raverieType, "LogicalEqual", raverieType, raverieType);
    AddFunction<OpType::OpLogicalNotEqual>(builder, type, raverieType, "LogicalNotEqual", raverieType, raverieType);
    AddFunction<OpType::OpLogicalOr>(builder, type, raverieType, "LogicalOr", raverieType, raverieType);
    AddFunction<OpType::OpLogicalAnd>(builder, type, raverieType, "LogicalAnd", raverieType, raverieType);

    AddFunction<OpType::OpSelect>(builder, type, raverieType, "Select", vectorBoolType, "condition", raverieType, "obj1", raverieType, "obj2");
  }

  for (size_t i = 0; i < types.mIntegerVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mIntegerVectorTypes[i];
    Raverie::BoundType* vectorBoolType = types.mBooleanVectorTypes[i];

    // Unary
    AddFunction<OpType::OpBitReverse>(builder, type, raverieType, "BitReverse", raverieType);
    AddFunction<OpType::OpBitCount>(builder, type, raverieType, "BitCount", raverieType);
    AddFunction<OpType::OpNot>(builder, type, raverieType, "Not", raverieType);

    // Binary
    AddFunction<OpType::OpShiftRightLogical>(builder, type, raverieType, "ShiftRightLogical", raverieType, "base", raverieType, "shift");
    AddFunction<OpType::OpShiftRightArithmetic>(builder, type, raverieType, "ShiftRightArithmetic", raverieType, "base", raverieType, "shift");
    AddFunction<OpType::OpShiftLeftLogical>(builder, type, raverieType, "ShiftLeftLogical", raverieType, "base", raverieType, "shift");
    AddFunction<OpType::OpBitwiseOr>(builder, type, raverieType, "BitwiseOr", raverieType, raverieType);
    AddFunction<OpType::OpBitwiseXor>(builder, type, raverieType, "BitwiseXor", raverieType, raverieType);
    AddFunction<OpType::OpBitwiseAnd>(builder, type, raverieType, "BitwiseAnd", raverieType, raverieType);
    AddFunction<OpType::OpIEqual>(builder, type, vectorBoolType, "Equal", raverieType, raverieType);
    AddFunction<OpType::OpINotEqual>(builder, type, vectorBoolType, "NotEqual", raverieType, raverieType);
    AddFunction<OpType::OpSGreaterThan>(builder, type, vectorBoolType, "GreaterThan", raverieType, raverieType);
    AddFunction<OpType::OpSGreaterThanEqual>(builder, type, vectorBoolType, "GreaterThanEqual", raverieType, raverieType);
    AddFunction<OpType::OpSLessThan>(builder, type, vectorBoolType, "LessThan", raverieType, raverieType);
    AddFunction<OpType::OpSLessThanEqual>(builder, type, vectorBoolType, "LessThanEqual", raverieType, raverieType);

    AddFunction<OpType::OpSRem>(builder, type, raverieType, "Remainder", raverieType, raverieType);
    AddFunction<OpType::OpSMod>(builder, type, raverieType, "Mod", raverieType, raverieType);

    // Tri
    AddFunction<OpType::OpSelect>(builder, type, raverieType, "Select", vectorBoolType, "condition", raverieType, "obj1", raverieType, "obj2");
    AddFunction<OpType::OpBitFieldSExtract>(builder, type, raverieType, "BitFieldExtract", raverieType, "base", intType, "offset", intType, "count");

    // Quad
    AddFunction<OpType::OpBitFieldInsert>(builder, type, raverieType, "BitFieldInsert", raverieType, "base", raverieType, "insert", intType, "offset", intType, "count");
  }

  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieType = types.mRealVectorTypes[i];
    Raverie::BoundType* vectorBoolType = types.mBooleanVectorTypes[i];

    // Unary
    AddFunction<OpType::OpDPdx>(builder, type, raverieType, "Ddx", raverieType)->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
    AddFunction<OpType::OpDPdy>(builder, type, raverieType, "Ddy", raverieType)->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
    AddFunction<OpType::OpFwidth>(builder, type, raverieType, "FWidth", raverieType)->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);
    AddFunction<OpType::OpIsNan>(builder, type, vectorBoolType, "IsNan", raverieType);
    AddFunction<OpType::OpIsInf>(builder, type, vectorBoolType, "IsInf", raverieType);

    // Binary
    AddFunction<OpType::OpFRem>(builder, type, raverieType, "Remainder", raverieType, raverieType);
    AddFunction<OpType::OpFMod>(builder, type, raverieType, "Mod", raverieType, raverieType);

    AddFunction<OpType::OpFOrdEqual>(builder, type, vectorBoolType, "OrderedEqual", raverieType, raverieType);
    AddFunction<OpType::OpFOrdNotEqual>(builder, type, vectorBoolType, "OrderedNotEqual", raverieType, raverieType);
    AddFunction<OpType::OpFOrdLessThan>(builder, type, vectorBoolType, "OrderedLessThan", raverieType, raverieType);
    AddFunction<OpType::OpFOrdLessThanEqual>(builder, type, vectorBoolType, "OrderedLessThanEqual", raverieType, raverieType);
    AddFunction<OpType::OpFOrdGreaterThan>(builder, type, vectorBoolType, "OrderedGreaterThan", raverieType, raverieType);
    AddFunction<OpType::OpFOrdGreaterThanEqual>(builder, type, vectorBoolType, "OrderedGreaterThanEqual", raverieType, raverieType);

    // Any/all only exists on vector types
    if (i != 0)
    {
      AddFunction<OpType::OpDot>(builder, type, realType, "Dot", raverieType, raverieType);
      AddFunction<OpType::OpVectorTimesScalar>(builder, type, raverieType, "VectorTimesScalar", raverieType, realType);
    }

    // Not implemented in glsl
    AddFunction<OpType::OpFUnordEqual>(builder, type, vectorBoolType, "UnorderedEqual", raverieType, raverieType);
    AddFunction<OpType::OpFUnordNotEqual>(builder, type, vectorBoolType, "UnorderedNotEqual", raverieType, raverieType);
    AddFunction<OpType::OpFUnordLessThan>(builder, type, vectorBoolType, "UnorderedLessThan", raverieType, raverieType);
    AddFunction<OpType::OpFUnordLessThanEqual>(builder, type, vectorBoolType, "UnorderedLessThanEqual", raverieType, raverieType);
    AddFunction<OpType::OpFUnordGreaterThan>(builder, type, vectorBoolType, "UnorderedGreaterThan", raverieType, raverieType);
    AddFunction<OpType::OpFUnordGreaterThanEqual>(builder, type, vectorBoolType, "UnorderedGreaterThanEqual", raverieType, raverieType);

    // Trinary
    AddFunction<OpType::OpSelect>(builder, type, raverieType, "Select", vectorBoolType, "condition", raverieType, "obj1", raverieType, "obj2");
  }

  for (size_t y = 2; y <= 4; ++y)
  {
    for (size_t x = 2; x <= 4; ++x)
    {
      Raverie::BoundType* raverieType = types.GetMatrixType(y, x);

      // Unary
      AddFunction<OpType::OpTranspose>(builder, type, raverieType, "Transpose", raverieType);

      // Binary
      AddFunction<OpType::OpMatrixTimesScalar>(builder, type, raverieType, "MatrixTimesScalar", raverieType, realType);

      // Ignore for now (have to figure out row/column order stuff)
      // AddFunction<OpType::OpMatrixTimesMatrix>(builder, type, raverieType,
      // "MatrixTimesMatrix", raverieType, raverieType);
    }
  }

  // Conversion
  for (size_t i = 0; i < types.mRealVectorTypes.Size(); ++i)
  {
    Raverie::BoundType* raverieRealType = types.mRealVectorTypes[i];
    Raverie::BoundType* raverieIntType = types.mIntegerVectorTypes[i];
    Raverie::BoundType* raverieBoolType = types.mBooleanVectorTypes[i];

    AddFunction<OpType::OpConvertFToS>(builder, type, raverieIntType, "ConvertFToS", raverieRealType);
    AddFunction<OpType::OpConvertSToF>(builder, type, raverieRealType, "ConvertSToF", raverieIntType);

    AddFunction<OpType::OpBitcast>(builder, type, raverieRealType, "BitCastToReal", raverieIntType);
    AddFunction<OpType::OpBitcast>(builder, type, raverieIntType, "BitCastToInteger", raverieRealType);
  }
}

RaverieDefineType(ShaderIntrinsics, builder, type)
{
  Raverie::BoundType* voidType = RaverieTypeId(void);
  Raverie::BoundType* boolType = RaverieTypeId(bool);
  Raverie::BoundType* intType = RaverieTypeId(int);

  RaverieShaderIRCore& shaderCore = RaverieShaderIRCore::GetInstance();
  RaverieTypeGroups& types = shaderCore.mRaverieTypes;

  // This technically needs to be restricted to pixel fragment types.
  AddFunction<OpType::OpKill>(builder, type, voidType, "Kill")->AddAttribute(SpirVNameSettings::mRequiresPixelAttribute);

  AddMathOps(builder, type, types);
  AddGlslExtensionIntrinsicOps(builder, shaderCore.mGlsl450ExtensionsLibrary, type, types);
  AddImageFunctions(builder, type, types);

  Raverie::ParameterArray parameters = OneParameter(intType, "language");
  Raverie::Function* isLanguageFn = builder.AddBoundFunction(type, "IsLanguage", Raverie::DummyBoundFunction, parameters, boolType, Raverie::FunctionOptions::Static);
  isLanguageFn->UserData = (void*)&ResolveIsLanguage;

  parameters = ThreeParameters(intType, "language", intType, "minVersion", intType, "maxVersion");
  isLanguageFn = builder.AddBoundFunction(type, "IsLanguage", Raverie::DummyBoundFunction, parameters, boolType, Raverie::FunctionOptions::Static);
  isLanguageFn->UserData = (void*)&ResolveIsLanguageMinMaxVersion;
}

RaverieDefineType(GeometryStreamUserData, builder, type)
{
  RaverieBindDefaultCopyDestructor();
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

RaverieDefineType(GeometryFragmentUserData, builder, type)
{
  RaverieBindDefaultCopyDestructor();
}

GeometryFragmentUserData::GeometryFragmentUserData()
{
  mInputStreamType = nullptr;
  mOutputStreamType = nullptr;
}

RaverieShaderIRType* GeometryFragmentUserData::GetInputVertexType()
{
  IRaverieShaderIR* param = mInputStreamType->mParameters[0];
  RaverieShaderIRType* subType = param->As<RaverieShaderIRType>();
  return subType;
}

RaverieShaderIRType* GeometryFragmentUserData::GetOutputVertexType()
{
  IRaverieShaderIR* param = mOutputStreamType->mParameters[0];
  RaverieShaderIRType* subType = param->As<RaverieShaderIRType>();
  return subType;
}

RaverieDefineType(ComputeFragmentUserData, builder, type)
{
  RaverieBindDefaultCopyDestructor();
}

ComputeFragmentUserData::ComputeFragmentUserData()
{
  mLocalSizeX = 1;
  mLocalSizeY = 1;
  mLocalSizeZ = 1;
}

RaverieDefineType(UnsignedInt, builder, type)
{
  RaverieBindDefaultCopyDestructor();
}

} // namespace Raverie
