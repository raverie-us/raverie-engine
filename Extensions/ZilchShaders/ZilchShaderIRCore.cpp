///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "ZilchShaderIRCore.hpp"
#include "ShaderIRLibraryTranslation.hpp"

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderIRCore
ZilchShaderIRCore* ZilchShaderIRCore::mInstance = nullptr;

void ZilchShaderIRCore::InitializeInstance()
{
  ReturnIf(mInstance != nullptr, ,
    "Can't initialize a static library more than once");
  mInstance = new ZilchShaderIRCore();
}

void ZilchShaderIRCore::Destroy()
{
  delete mInstance;
  mInstance = nullptr;
}

ZilchShaderIRCore& ZilchShaderIRCore::GetInstance()
{
  ErrorIf(mInstance == nullptr,
    "Attempted to get an uninitialized singleton static library");

  return *mInstance;
}

ZilchShaderIRCore::ZilchShaderIRCore()
{
  mGlsl450ExtensionsLibrary = nullptr;
}

void ZilchShaderIRCore::Parse(ZilchSpirVFrontEnd* translator)
{
  ZilchShaderIRLibrary* shaderLibrary = new ZilchShaderIRLibrary();
  shaderLibrary->mZilchLibrary = Zilch::Core::GetInstance().GetLibrary();
  mLibraryRef = shaderLibrary;
  translator->mLibrary = shaderLibrary;

  Zilch::Core& core = Zilch::Core::GetInstance();
  Zilch::BoundType* mathType = core.MathType;
  translator->MakeStructType(shaderLibrary, core.MathType->Name, core.MathType, spv::StorageClass::StorageClassGeneric);

  Zilch::BoundType* zilchIntType = core.IntegerType;
  String intTypeName = zilchIntType->ToString();

  TypeGroups& types = mTypes;
  MakeMathTypes(translator, shaderLibrary, types);

  // Add all static/instance functions for primitive types
  RegisterPrimitiveFunctions(translator, shaderLibrary, types, types.mRealVectorTypes[0]);
  RegisterPrimitiveFunctions(translator, shaderLibrary, types, types.mIntegerVectorTypes[0]);
  RegisterPrimitiveFunctions(translator, shaderLibrary, types, types.mBooleanVectorTypes[0]);

  // Add all static/instance functions for vector types
  RegisterVectorFunctions(translator, shaderLibrary, types, types.mRealVectorTypes);
  RegisterVectorFunctions(translator, shaderLibrary, types, types.mIntegerVectorTypes);
  RegisterVectorFunctions(translator, shaderLibrary, types, types.mBooleanVectorTypes);

  // Add all static/instance functions for matrix types (only float matrices exist)
  RegisterMatrixFunctions(translator, shaderLibrary, types, types.mRealMatrixTypes);
  // Also the quaternion type
  RegisterQuaternionFunctions(translator, shaderLibrary, types, types.mQuaternionType);

  // Add a bunch of math ops (a lot on the math class)
  RegisterArithmeticOps(translator, shaderLibrary, types);
  RegisterConversionOps(translator, shaderLibrary, types);
  RegisterLogicalOps(translator, shaderLibrary, types);
  RegisterBitOps(translator, shaderLibrary, types);
  RegisterColorsOps(translator, shaderLibrary, types);

  // Add special extension functions such as Matrix.Determinant and Sin
  mGlsl450ExtensionsLibrary = new SpirVExtensionLibrary();
  shaderLibrary->mExtensionLibraries.PushBack(mGlsl450ExtensionsLibrary);
  RegisterGlsl450Extensions(shaderLibrary, mGlsl450ExtensionsLibrary, types);
  shaderLibrary->mTranslated = true;
}

ZilchShaderIRLibraryRef ZilchShaderIRCore::GetLibrary()
{
  return mLibraryRef;
}

void ZilchShaderIRCore::MakeMathTypes(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types)
{
  Zilch::Core& core = Zilch::Core::GetInstance();

  types.mVoidType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Void, 0, nullptr, core.VoidType, false);

  ZilchShaderIRType* floatType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Float, 1, nullptr, core.RealType);
  ZilchShaderIRType* float2Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 2, floatType, core.Real2Type);
  ZilchShaderIRType* float3Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 3, floatType, core.Real3Type);
  ZilchShaderIRType* float4Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 4, floatType, core.Real4Type);

  types.mRealVectorTypes.PushBack(floatType);
  types.mRealVectorTypes.PushBack(float2Type);
  types.mRealVectorTypes.PushBack(float3Type);
  types.mRealVectorTypes.PushBack(float4Type);

  // Make all of the matrix types
  for(size_t y = 2; y <= 4; ++y)
  {
    for(size_t x = 2; x <= 4; ++x)
    {
      ZilchShaderIRType* basisType = types.mRealVectorTypes[x - 1];
      String matrixName = BuildString("Real", ToString(y), "x", ToString(x));
      Zilch::BoundType* zilchMatrixType = core.GetLibrary()->BoundTypes.FindValue(matrixName, nullptr);
      ZilchShaderIRType* shaderMatrixType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Matrix, y, basisType, zilchMatrixType);
      types.mRealMatrixTypes.PushBack(shaderMatrixType);
    }
  }

  ZilchShaderIRType* intType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Int, 1, nullptr, core.IntegerType);
  ZilchShaderIRType* int2Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 2, intType, core.Integer2Type);
  ZilchShaderIRType* int3Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 3, intType, core.Integer3Type);
  ZilchShaderIRType* int4Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 4, intType, core.Integer4Type);
  types.mIntegerVectorTypes.PushBack(intType);
  types.mIntegerVectorTypes.PushBack(int2Type);
  types.mIntegerVectorTypes.PushBack(int3Type);
  types.mIntegerVectorTypes.PushBack(int4Type);

  ZilchShaderIRType* boolType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Bool, 1, nullptr, core.BooleanType);
  ZilchShaderIRType* bool2Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 2, boolType, core.Boolean2Type);
  ZilchShaderIRType* bool3Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 3, boolType, core.Boolean3Type);
  ZilchShaderIRType* bool4Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 4, boolType, core.Boolean4Type);
  types.mBooleanVectorTypes.PushBack(boolType);
  types.mBooleanVectorTypes.PushBack(bool2Type);
  types.mBooleanVectorTypes.PushBack(bool3Type);
  types.mBooleanVectorTypes.PushBack(bool4Type);

  // Make quaternion a struct type. Ideally quaternion would just be a vec4 type, but it's illegal to declare
  // multiple vec4 types. This causes a lot of complications in translating core functionality.
  Zilch::BoundType* zilchQuaternion = core.QuaternionType;
  String quaternionTypeName = zilchQuaternion->ToString();
  ZilchShaderIRType* quaternionType = translator->MakeStructType(shaderLibrary, quaternionTypeName, core.QuaternionType, spv::StorageClassFunction);
  quaternionType->mDebugResultName = quaternionType->mName = quaternionTypeName;
  quaternionType->AddMember(float4Type, "Data");
  types.mQuaternionType = quaternionType;
}

void ZilchShaderIRCore::RegisterPrimitiveFunctions(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types, ZilchShaderIRType* shaderType)
{
  Zilch::BoundType* intType = types.mIntegerVectorTypes[0]->mZilchType;
  Zilch::BoundType* zilchType = shaderType->mZilchType;
  String zilchTypeName = zilchType->ToString();
  String intTypeName = intType->ToString();

  TypeResolvers& primitiveTypeResolvers = shaderLibrary->mTypeResolvers[zilchType];
  primitiveTypeResolvers.mDefaultConstructorResolver = &TranslatePrimitiveDefaultConstructor;
  primitiveTypeResolvers.mBackupConstructorResolver = &TranslateBackupPrimitiveConstructor;
  primitiveTypeResolvers.RegisterBackupFieldResolver(&ScalarBackupFieldResolver);
  primitiveTypeResolvers.RegisterFunctionResolver(GetStaticProperty(zilchType, "Count")->Get, ResolveVectorTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(GetInstanceProperty(zilchType, "Count")->Get, ResolveVectorTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchType, Zilch::OperatorGet, intTypeName), ResolvePrimitiveGet);
  primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchType, Zilch::OperatorSet, intTypeName, zilchTypeName), ResolvePrimitiveSet);
}

void ZilchShaderIRCore::RegisterVectorFunctions(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types, Array<ZilchShaderIRType*>& vectorTypes)
{
  Zilch::BoundType* intType = types.mIntegerVectorTypes[0]->mZilchType;
  String intTypeName = intType->ToString();

  for(size_t i = 1; i < vectorTypes.Size(); ++i)
  {
    ZilchShaderIRType* shaderType = vectorTypes[i];
    Zilch::BoundType* zilchType = shaderType->mZilchType;
    String zilchTypeName = zilchType->ToString();
    ZilchShaderIRType* componentType = GetComponentType(shaderType);
    Zilch::BoundType* zilchComponentType = componentType->mZilchType;
    String zilchComponentTypeName = zilchComponentType->ToString();

    TypeResolvers& primitiveTypeResolvers = shaderLibrary->mTypeResolvers[zilchType];
    primitiveTypeResolvers.mDefaultConstructorResolver = &TranslateCompositeDefaultConstructor;
    primitiveTypeResolvers.RegisterBackupFieldResolver(&VectorBackupFieldResolver);
    primitiveTypeResolvers.RegisterConstructorResolver(zilchType->GetDefaultConstructor(), TranslateCompositeDefaultConstructor);
    primitiveTypeResolvers.RegisterConstructorResolver(GetConstructor(zilchType, zilchComponentTypeName), TranslateCompositeSplatConstructor);
    primitiveTypeResolvers.RegisterFunctionResolver(GetStaticProperty(zilchType, "Count")->Get, ResolveVectorTypeCount);
    primitiveTypeResolvers.RegisterFunctionResolver(GetInstanceProperty(zilchType, "Count")->Get, ResolveVectorTypeCount);
    primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchType, Zilch::OperatorGet, intTypeName), ResolveVectorGet);
    primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchType, Zilch::OperatorSet, intTypeName, zilchComponentTypeName), ResolveVectorSet);
    primitiveTypeResolvers.RegisterBackupSetterResolver(VectorBackupPropertySetter);
    primitiveTypeResolvers.mBackupConstructorResolver = &TranslateBackupCompositeConstructor;
  }
}


void ZilchShaderIRCore::RegisterMatrixFunctions(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types, Array<ZilchShaderIRType*>& matrixTypes)
{
  Zilch::BoundType* intType = types.mIntegerVectorTypes[0]->mZilchType;
  String intTypeName = intType->ToString();

  for(size_t i = 0; i < matrixTypes.Size(); ++i)
  {
    ZilchShaderIRType* shaderType = matrixTypes[i];
    Zilch::BoundType* zilchType = shaderType->mZilchType;
    // Get the component (vector row) type
    ZilchShaderIRType* componentType = GetComponentType(shaderType);
    Zilch::BoundType* zilchComponentType = componentType->mZilchType;
    String zilchComponentTypeName = zilchComponentType->ToString();
    // Get the scalar type of the matrix
    ZilchShaderIRType* scalarType = GetComponentType(componentType);
    Zilch::BoundType* zilchScalarType = scalarType->mZilchType;

    TypeResolvers& matrixTypeResolver = shaderLibrary->mTypeResolvers[zilchType];
    // Constructors
    matrixTypeResolver.mDefaultConstructorResolver = &TranslateMatrixDefaultConstructor;
    matrixTypeResolver.RegisterConstructorResolver(zilchType->GetDefaultConstructor(), TranslateMatrixDefaultConstructor);
    matrixTypeResolver.RegisterConstructorResolver(GetConstructor(zilchType, zilchScalarType->ToString()), TranslateCompositeSplatConstructor);
    // Constructor for each scalar entry in the matrix
    Array<String> constructorParams;
    for(size_t i = 0; i < shaderType->mComponents * componentType->mComponents; ++i)
      constructorParams.PushBack(zilchScalarType->ToString());
    matrixTypeResolver.RegisterConstructorResolver(GetConstructor(zilchType, constructorParams), TranslateMatrixFullConstructor);

    // Fields (M00, etc...)
    matrixTypeResolver.RegisterBackupFieldResolver(&MatrixBackupFieldResolver);

    matrixTypeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchType, Zilch::OperatorGet, intTypeName), ResolveMatrixGet);
    matrixTypeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchType, Zilch::OperatorSet, intTypeName, zilchComponentTypeName), ResolveMatrixSet);
  }
}

void ZilchShaderIRCore::RegisterQuaternionFunctions(ZilchSpirVFrontEnd* translator, ZilchShaderIRLibrary* shaderLibrary, TypeGroups& types, ZilchShaderIRType* quaternionType)
{
  Zilch::BoundType* intType = types.mIntegerVectorTypes[0]->mZilchType;
  String intTypeName = intType->ToString();

  Zilch::BoundType* zilchType = quaternionType->mZilchType;
  String zilchTypeName = zilchType->ToString();
  // While quaternion's component type is technically vec4, all operations behave as if it's real
  ZilchShaderIRType* componentType = types.mRealVectorTypes[0];
  Zilch::BoundType* zilchComponentType = componentType->mZilchType;
  String zilchComponentTypeName = zilchComponentType->ToString();

  TypeResolvers& primitiveTypeResolvers = shaderLibrary->mTypeResolvers[zilchType];
  primitiveTypeResolvers.mDefaultConstructorResolver = &TranslateQuaternionDefaultConstructor;
  primitiveTypeResolvers.RegisterBackupFieldResolver(&QuaternionBackupFieldResolver);
  primitiveTypeResolvers.RegisterConstructorResolver(zilchType->GetDefaultConstructor(), TranslateQuaternionDefaultConstructor);
  primitiveTypeResolvers.RegisterConstructorResolver(GetConstructor(zilchType, zilchComponentTypeName), TranslateQuaternionSplatConstructor);
  primitiveTypeResolvers.RegisterFunctionResolver(GetStaticProperty(zilchType, "Count")->Get, ResolveQuaternionTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(GetInstanceProperty(zilchType, "Count")->Get, ResolveQuaternionTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchType, Zilch::OperatorGet, intTypeName), ResolveQuaternionGet);
  primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(zilchType, Zilch::OperatorSet, intTypeName, zilchComponentTypeName), ResolveQuaternionSet);
  primitiveTypeResolvers.RegisterBackupSetterResolver(QuaternionBackupPropertySetter);
  primitiveTypeResolvers.mBackupConstructorResolver = &TranslateBackupQuaternionConstructor;
}

}//namespace Zero
