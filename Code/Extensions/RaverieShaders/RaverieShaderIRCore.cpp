// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "RaverieShaderIRCore.hpp"
#include "ShaderIRLibraryTranslation.hpp"

namespace Raverie
{

RaverieShaderIRCore* RaverieShaderIRCore::mInstance = nullptr;

void RaverieShaderIRCore::InitializeInstance()
{
  ReturnIf(mInstance != nullptr, , "Can't initialize a static library more than once");
  mInstance = new RaverieShaderIRCore();
}

void RaverieShaderIRCore::Destroy()
{
  delete mInstance;
  mInstance = nullptr;
}

RaverieShaderIRCore& RaverieShaderIRCore::GetInstance()
{
  ErrorIf(mInstance == nullptr, "Attempted to get an uninitialized singleton static library");

  return *mInstance;
}

RaverieShaderIRCore::RaverieShaderIRCore()
{
  mGlsl450ExtensionsLibrary = nullptr;

  Raverie::Core& core = Raverie::Core::GetInstance();
  mRaverieTypes.mVoidType = core.VoidType;

  mRaverieTypes.mRealVectorTypes.PushBack(core.RealType);
  mRaverieTypes.mRealVectorTypes.PushBack(core.Real2Type);
  mRaverieTypes.mRealVectorTypes.PushBack(core.Real3Type);
  mRaverieTypes.mRealVectorTypes.PushBack(core.Real4Type);

  // Make all of the matrix types
  for (size_t y = 2; y <= 4; ++y)
  {
    for (size_t x = 2; x <= 4; ++x)
    {
      Raverie::BoundType* basisType = mRaverieTypes.mRealVectorTypes[x - 1];
      String matrixName = BuildString("Real", ToString(y), "x", ToString(x));
      Raverie::BoundType* raverieMatrixType = core.GetLibrary()->BoundTypes.FindValue(matrixName, nullptr);
      mRaverieTypes.mRealMatrixTypes.PushBack(raverieMatrixType);
    }
  }

  mRaverieTypes.mIntegerVectorTypes.PushBack(core.IntegerType);
  mRaverieTypes.mIntegerVectorTypes.PushBack(core.Integer2Type);
  mRaverieTypes.mIntegerVectorTypes.PushBack(core.Integer3Type);
  mRaverieTypes.mIntegerVectorTypes.PushBack(core.Integer4Type);

  mRaverieTypes.mBooleanVectorTypes.PushBack(core.BooleanType);
  mRaverieTypes.mBooleanVectorTypes.PushBack(core.Boolean2Type);
  mRaverieTypes.mBooleanVectorTypes.PushBack(core.Boolean3Type);
  mRaverieTypes.mBooleanVectorTypes.PushBack(core.Boolean4Type);

  mRaverieTypes.mQuaternionType = core.QuaternionType;
}

void RaverieShaderIRCore::Parse(RaverieSpirVFrontEnd* translator)
{
  RaverieShaderIRLibrary* shaderLibrary = new RaverieShaderIRLibrary();
  shaderLibrary->mRaverieLibrary = Raverie::Core::GetInstance().GetLibrary();
  mLibraryRef = shaderLibrary;
  translator->mLibrary = shaderLibrary;

  Raverie::Core& core = Raverie::Core::GetInstance();
  Raverie::BoundType* mathType = core.MathType;
  translator->MakeStructType(shaderLibrary, core.MathType->Name, core.MathType, spv::StorageClass::StorageClassGeneric);

  Raverie::BoundType* raverieIntType = core.IntegerType;
  String intTypeName = raverieIntType->ToString();

  ShaderTypeGroups& types = mShaderTypes;
  MakeMathTypes(translator, shaderLibrary, types);

  // Add all static/instance functions for primitive types
  RegisterPrimitiveFunctions(translator, shaderLibrary, types, types.mRealVectorTypes[0]);
  RegisterPrimitiveFunctions(translator, shaderLibrary, types, types.mIntegerVectorTypes[0]);
  RegisterPrimitiveFunctions(translator, shaderLibrary, types, types.mBooleanVectorTypes[0]);

  // Add all static/instance functions for vector types
  RegisterVectorFunctions(translator, shaderLibrary, types, types.mRealVectorTypes);
  RegisterVectorFunctions(translator, shaderLibrary, types, types.mIntegerVectorTypes);
  RegisterVectorFunctions(translator, shaderLibrary, types, types.mBooleanVectorTypes);

  // Add all static/instance functions for matrix types (only float matrices
  // exist)
  RegisterMatrixFunctions(translator, shaderLibrary, types, types.mRealMatrixTypes);
  // Also the quaternion type
  RegisterQuaternionFunctions(translator, shaderLibrary, types, types.mQuaternionType);

  // Add a bunch of math ops (a lot on the math class)
  RegisterArithmeticOps(translator, shaderLibrary, mRaverieTypes);
  RegisterConversionOps(translator, shaderLibrary, mRaverieTypes);
  RegisterLogicalOps(translator, shaderLibrary, mRaverieTypes);
  RegisterBitOps(translator, shaderLibrary, mRaverieTypes);
  RegisterColorsOps(translator, shaderLibrary, mRaverieTypes);

  // Add special extension functions such as Matrix.Determinant and Sin
  mGlsl450ExtensionsLibrary = new SpirVExtensionLibrary();
  shaderLibrary->mExtensionLibraries.PushBack(mGlsl450ExtensionsLibrary);
  RegisterGlsl450Extensions(shaderLibrary, mGlsl450ExtensionsLibrary, mRaverieTypes);
  shaderLibrary->mTranslated = true;
}

RaverieShaderIRLibraryRef RaverieShaderIRCore::GetLibrary()
{
  return mLibraryRef;
}

void RaverieShaderIRCore::MakeMathTypes(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types)
{
  Raverie::Core& core = Raverie::Core::GetInstance();

  types.mVoidType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Void, 0, nullptr, core.VoidType, false);

  RaverieShaderIRType* floatType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Float, 1, nullptr, core.RealType);
  RaverieShaderIRType* float2Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 2, floatType, core.Real2Type);
  RaverieShaderIRType* float3Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 3, floatType, core.Real3Type);
  RaverieShaderIRType* float4Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 4, floatType, core.Real4Type);

  types.mRealVectorTypes.PushBack(floatType);
  types.mRealVectorTypes.PushBack(float2Type);
  types.mRealVectorTypes.PushBack(float3Type);
  types.mRealVectorTypes.PushBack(float4Type);

  // Make all of the matrix types
  for (size_t y = 2; y <= 4; ++y)
  {
    for (size_t x = 2; x <= 4; ++x)
    {
      RaverieShaderIRType* basisType = types.mRealVectorTypes[x - 1];
      String matrixName = BuildString("Real", ToString(y), "x", ToString(x));
      Raverie::BoundType* raverieMatrixType = core.GetLibrary()->BoundTypes.FindValue(matrixName, nullptr);
      RaverieShaderIRType* shaderMatrixType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Matrix, y, basisType, raverieMatrixType);
      types.mRealMatrixTypes.PushBack(shaderMatrixType);
    }
  }

  RaverieShaderIRType* intType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Int, 1, nullptr, core.IntegerType);
  RaverieShaderIRType* int2Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 2, intType, core.Integer2Type);
  RaverieShaderIRType* int3Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 3, intType, core.Integer3Type);
  RaverieShaderIRType* int4Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 4, intType, core.Integer4Type);
  types.mIntegerVectorTypes.PushBack(intType);
  types.mIntegerVectorTypes.PushBack(int2Type);
  types.mIntegerVectorTypes.PushBack(int3Type);
  types.mIntegerVectorTypes.PushBack(int4Type);

  RaverieShaderIRType* boolType = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Bool, 1, nullptr, core.BooleanType);
  RaverieShaderIRType* bool2Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 2, boolType, core.Boolean2Type);
  RaverieShaderIRType* bool3Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 3, boolType, core.Boolean3Type);
  RaverieShaderIRType* bool4Type = translator->MakeCoreType(shaderLibrary, ShaderIRTypeBaseType::Vector, 4, boolType, core.Boolean4Type);
  types.mBooleanVectorTypes.PushBack(boolType);
  types.mBooleanVectorTypes.PushBack(bool2Type);
  types.mBooleanVectorTypes.PushBack(bool3Type);
  types.mBooleanVectorTypes.PushBack(bool4Type);

  // Make quaternion a struct type. Ideally quaternion would just be a vec4
  // type, but it's illegal to declare multiple vec4 types. This causes a lot of
  // complications in translating core functionality.
  Raverie::BoundType* raverieQuaternion = core.QuaternionType;
  String quaternionTypeName = raverieQuaternion->ToString();
  RaverieShaderIRType* quaternionType = translator->MakeStructType(shaderLibrary, quaternionTypeName, core.QuaternionType, spv::StorageClassFunction);
  quaternionType->mDebugResultName = quaternionType->mName = quaternionTypeName;
  quaternionType->AddMember(float4Type, "Data");
  types.mQuaternionType = quaternionType;
}

void RaverieShaderIRCore::RegisterPrimitiveFunctions(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types, RaverieShaderIRType* shaderType)
{
  Raverie::BoundType* intType = types.mIntegerVectorTypes[0]->mRaverieType;
  Raverie::BoundType* raverieType = shaderType->mRaverieType;
  String raverieTypeName = raverieType->ToString();
  String intTypeName = intType->ToString();

  TypeResolvers& primitiveTypeResolvers = shaderLibrary->mTypeResolvers[raverieType];
  primitiveTypeResolvers.mDefaultConstructorResolver = &TranslatePrimitiveDefaultConstructor;
  primitiveTypeResolvers.mBackupConstructorResolver = &TranslateBackupPrimitiveConstructor;
  primitiveTypeResolvers.RegisterBackupFieldResolver(&ScalarBackupFieldResolver);
  primitiveTypeResolvers.RegisterFunctionResolver(GetStaticProperty(raverieType, "Count")->Get, ResolveVectorTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(GetInstanceProperty(raverieType, "Count")->Get, ResolveVectorTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(raverieType, Raverie::OperatorGet, intTypeName), ResolvePrimitiveGet);
  primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(raverieType, Raverie::OperatorSet, intTypeName, raverieTypeName), ResolvePrimitiveSet);
}

void RaverieShaderIRCore::RegisterVectorFunctions(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types, Array<RaverieShaderIRType*>& vectorTypes)
{
  Raverie::BoundType* intType = types.mIntegerVectorTypes[0]->mRaverieType;
  String intTypeName = intType->ToString();

  for (size_t i = 1; i < vectorTypes.Size(); ++i)
  {
    RaverieShaderIRType* shaderType = vectorTypes[i];
    Raverie::BoundType* raverieType = shaderType->mRaverieType;
    String raverieTypeName = raverieType->ToString();
    RaverieShaderIRType* componentType = GetComponentType(shaderType);
    Raverie::BoundType* raverieComponentType = componentType->mRaverieType;
    String raverieComponentTypeName = raverieComponentType->ToString();

    TypeResolvers& primitiveTypeResolvers = shaderLibrary->mTypeResolvers[raverieType];
    primitiveTypeResolvers.mDefaultConstructorResolver = &TranslateCompositeDefaultConstructor;
    primitiveTypeResolvers.RegisterBackupFieldResolver(&VectorBackupFieldResolver);
    primitiveTypeResolvers.RegisterConstructorResolver(raverieType->GetDefaultConstructor(), TranslateCompositeDefaultConstructor);
    primitiveTypeResolvers.RegisterConstructorResolver(GetConstructor(raverieType, raverieComponentTypeName), TranslateCompositeSplatConstructor);
    Raverie::Function* copyConstructor = GetConstructor(raverieType, raverieTypeName);
    if (copyConstructor != nullptr)
      primitiveTypeResolvers.RegisterConstructorResolver(copyConstructor, ResolveVectorCopyConstructor);
    primitiveTypeResolvers.RegisterFunctionResolver(GetStaticProperty(raverieType, "Count")->Get, ResolveVectorTypeCount);
    primitiveTypeResolvers.RegisterFunctionResolver(GetInstanceProperty(raverieType, "Count")->Get, ResolveVectorTypeCount);
    primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(raverieType, Raverie::OperatorGet, intTypeName), ResolveVectorGet);
    primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(raverieType, Raverie::OperatorSet, intTypeName, raverieComponentTypeName), ResolveVectorSet);
    primitiveTypeResolvers.RegisterBackupSetterResolver(VectorBackupPropertySetter);
    primitiveTypeResolvers.mBackupConstructorResolver = &TranslateBackupCompositeConstructor;
  }
}

void RaverieShaderIRCore::RegisterMatrixFunctions(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types, Array<RaverieShaderIRType*>& matrixTypes)
{
  Raverie::BoundType* intType = types.mIntegerVectorTypes[0]->mRaverieType;
  String intTypeName = intType->ToString();

  for (size_t i = 0; i < matrixTypes.Size(); ++i)
  {
    RaverieShaderIRType* shaderType = matrixTypes[i];
    Raverie::BoundType* raverieType = shaderType->mRaverieType;
    // Get the component (vector row) type
    RaverieShaderIRType* componentType = GetComponentType(shaderType);
    Raverie::BoundType* raverieComponentType = componentType->mRaverieType;
    String raverieComponentTypeName = raverieComponentType->ToString();
    // Get the scalar type of the matrix
    RaverieShaderIRType* scalarType = GetComponentType(componentType);
    Raverie::BoundType* raverieScalarType = scalarType->mRaverieType;

    TypeResolvers& matrixTypeResolver = shaderLibrary->mTypeResolvers[raverieType];
    // Constructors
    matrixTypeResolver.mDefaultConstructorResolver = &TranslateMatrixDefaultConstructor;
    matrixTypeResolver.RegisterConstructorResolver(raverieType->GetDefaultConstructor(), TranslateMatrixDefaultConstructor);
    matrixTypeResolver.RegisterConstructorResolver(GetConstructor(raverieType, raverieScalarType->ToString()), TranslateCompositeSplatConstructor);
    // Constructor for each scalar entry in the matrix
    Array<String> constructorParams;
    for (size_t i = 0; i < shaderType->mComponents * componentType->mComponents; ++i)
      constructorParams.PushBack(raverieScalarType->ToString());
    matrixTypeResolver.RegisterConstructorResolver(GetConstructor(raverieType, constructorParams), TranslateMatrixFullConstructor);

    // Fields (M00, etc...)
    matrixTypeResolver.RegisterBackupFieldResolver(&MatrixBackupFieldResolver);

    matrixTypeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(raverieType, Raverie::OperatorGet, intTypeName), ResolveMatrixGet);
    matrixTypeResolver.RegisterFunctionResolver(GetMemberOverloadedFunction(raverieType, Raverie::OperatorSet, intTypeName, raverieComponentTypeName), ResolveMatrixSet);
  }
}

void RaverieShaderIRCore::RegisterQuaternionFunctions(RaverieSpirVFrontEnd* translator, RaverieShaderIRLibrary* shaderLibrary, ShaderTypeGroups& types, RaverieShaderIRType* quaternionType)
{
  Raverie::BoundType* intType = types.mIntegerVectorTypes[0]->mRaverieType;
  String intTypeName = intType->ToString();

  Raverie::BoundType* raverieType = quaternionType->mRaverieType;
  String raverieTypeName = raverieType->ToString();
  // While quaternion's component type is technically vec4, all operations
  // behave as if it's real
  RaverieShaderIRType* componentType = types.mRealVectorTypes[0];
  Raverie::BoundType* raverieComponentType = componentType->mRaverieType;
  String raverieComponentTypeName = raverieComponentType->ToString();

  TypeResolvers& primitiveTypeResolvers = shaderLibrary->mTypeResolvers[raverieType];
  primitiveTypeResolvers.mDefaultConstructorResolver = &TranslateQuaternionDefaultConstructor;
  primitiveTypeResolvers.RegisterBackupFieldResolver(&QuaternionBackupFieldResolver);
  primitiveTypeResolvers.RegisterConstructorResolver(raverieType->GetDefaultConstructor(), TranslateQuaternionDefaultConstructor);
  primitiveTypeResolvers.RegisterConstructorResolver(GetConstructor(raverieType, raverieComponentTypeName), TranslateQuaternionSplatConstructor);
  primitiveTypeResolvers.RegisterFunctionResolver(GetStaticProperty(raverieType, "Count")->Get, ResolveQuaternionTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(GetInstanceProperty(raverieType, "Count")->Get, ResolveQuaternionTypeCount);
  primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(raverieType, Raverie::OperatorGet, intTypeName), ResolveQuaternionGet);
  primitiveTypeResolvers.RegisterFunctionResolver(GetMemberOverloadedFunction(raverieType, Raverie::OperatorSet, intTypeName, raverieComponentTypeName), ResolveQuaternionSet);
  primitiveTypeResolvers.RegisterBackupSetterResolver(QuaternionBackupPropertySetter);
  primitiveTypeResolvers.mBackupConstructorResolver = &TranslateBackupQuaternionConstructor;
}

} // namespace Raverie
