// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "ShaderErrors.hpp"

namespace Raverie
{

BoundType* InstantiateFixedArray(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;
  // Bind the arraytype
  BoundType* arrayType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);

  // Bind all of the array's functions and properties (stubbed out since we're
  // only using this for translation)
  builder.AddBoundConstructor(arrayType, Raverie::DummyBoundFunction, ParameterArray());
  builder.AddBoundFunction(arrayType, "Add", Raverie::DummyBoundFunction, OneParameter(templateType), core.VoidType, Raverie::FunctionOptions::None);
  builder.AddBoundFunction(arrayType, "Get", Raverie::DummyBoundFunction, OneParameter(core.IntegerType, "index"), templateType, Raverie::FunctionOptions::None);
  builder.AddBoundFunction(arrayType, "Set", Raverie::DummyBoundFunction, TwoParameters(core.IntegerType, "index", templateType, "value"), core.VoidType, Raverie::FunctionOptions::None);
  builder.AddBoundGetterSetter(arrayType, "Count", core.IntegerType, nullptr, Raverie::DummyBoundFunction, Raverie::MemberOptions::None);

  return arrayType;
}

BoundType* InstantiateRuntimeArray(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;
  // Bind the arraytype
  BoundType* arrayType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  Attribute* storageAttribute = arrayType->AddAttribute(Raverie::SpirVNameSettings::mStorageClassAttribute);
  storageAttribute->AddParameter(spv::StorageClassStorageBuffer);
  arrayType->AddAttribute(Raverie::SpirVNameSettings::mNonCopyableAttributeName);

  // Bind all of the array's functions and properties (stubbed out since we're
  // only using this for translation)
  builder.AddBoundFunction(arrayType, "Get", Raverie::DummyBoundFunction, OneParameter(core.IntegerType, "index"), templateType, Raverie::FunctionOptions::None);
  builder.AddBoundFunction(arrayType, "Set", Raverie::DummyBoundFunction, TwoParameters(core.IntegerType, "index", templateType, "value"), core.VoidType, Raverie::FunctionOptions::None);
  builder.AddBoundGetterSetter(arrayType, "Count", core.IntegerType, nullptr, Raverie::DummyBoundFunction, Raverie::MemberOptions::None);

  return arrayType;
}

BoundType* InstantiateGeometryInput(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  // Bind all of the functions and properties (stubbed out since we're only
  // using this for translation)
  builder.AddBoundConstructor(selfType, Raverie::UnTranslatedBoundFunction, ParameterArray());
  builder.AddBoundFunction(selfType, "Get", Raverie::UnTranslatedBoundFunction, OneParameter(core.IntegerType), templateType, Raverie::FunctionOptions::None);
  builder.AddBoundFunction(selfType, "Set", Raverie::UnTranslatedBoundFunction, TwoParameters(core.IntegerType, templateType), core.VoidType, Raverie::FunctionOptions::None);
  builder.AddBoundGetterSetter(selfType, "Count", core.IntegerType, nullptr, Raverie::UnTranslatedBoundFunction, Raverie::MemberOptions::None);

  Raverie::HandleOf<GeometryStreamUserData> handle = RaverieAllocate(GeometryStreamUserData);
  handle->Set((spv::ExecutionMode)(uintptr_t)userData);
  selfType->Add(*handle);

  return selfType;
}

BoundType* InstantiateGeometryOutput(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  selfType->CreatableInScript = true;
  // Bind all of the functions and properties (stubbed out since we're only
  // using this for translation)
  builder.AddBoundFunction(selfType, "Append", Raverie::UnTranslatedBoundFunction, TwoParameters(templateType, core.IntegerType), core.VoidType, Raverie::FunctionOptions::None);
  builder.AddBoundFunction(selfType, "Restart", Raverie::UnTranslatedBoundFunction, ParameterArray(), core.VoidType, Raverie::FunctionOptions::None);

  Raverie::HandleOf<GeometryStreamUserData> handle = RaverieAllocate(GeometryStreamUserData);
  handle->Set((spv::ExecutionMode)(uintptr_t)userData);
  selfType->Add(*handle);

  return selfType;
}

BoundType* InstantiateGeometryStreamMover(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Type* fromType = templateTypes[0].TypeValue;
  Type* toType = templateTypes[1].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  selfType->CreatableInScript = false;
  builder.AddBoundFunction(selfType, "Move", Raverie::UnTranslatedBoundFunction, OneParameter(fromType), toType, Raverie::FunctionOptions::Static);

  return selfType;
}

RaverieDefineStaticLibrary(ShaderIntrinsicsLibrary)
{
  builder.CreatableInScriptDefault = false;

  RaverieInitializeType(UnsignedInt);

  // BoundType Components
  RaverieInitializeType(GeometryStreamUserData);
  RaverieInitializeType(GeometryFragmentUserData);
  RaverieInitializeType(ComputeFragmentUserData);

  // @Nate: These have to be uncommented for new shaders
  RaverieInitializeType(ShaderIntrinsics);
  RaverieInitializeType(Sampler);
  RaverieInitializeType(Image2d);
  RaverieInitializeType(StorageImage2d);
  RaverieInitializeType(DepthImage2d);
  RaverieInitializeType(ImageCube);
  RaverieInitializeType(SampledImage2d);
  RaverieInitializeType(SampledDepthImage2d);
  RaverieInitializeType(SampledImageCube);

  // Bind the fixed array type instantiator (creates the different arrays when
  // instantiated)
  {
    Array<Raverie::TemplateParameter> templateTypes;
    TemplateParameter& typeParam = templateTypes.PushBack();
    typeParam.Name = "Type";
    typeParam.Type = ConstantType::Type;
    TemplateParameter& sizeParam = templateTypes.PushBack();
    sizeParam.Name = "Size";
    sizeParam.Type = ConstantType::Integer;
    builder.AddTemplateInstantiator("FixedArray", InstantiateFixedArray, templateTypes, nullptr);
  }

  {
    Raverie::Array<Raverie::Constant> fixedArrayTemplateParams;
    fixedArrayTemplateParams.PushBack(RaverieTypeId(Raverie::Real4x4));
    fixedArrayTemplateParams.PushBack(80);

    Raverie::InstantiatedTemplate templateData = builder.InstantiateTemplate("FixedArray", fixedArrayTemplateParams, LibraryArray(RaverieInit, builder.BuiltLibrary));
    Raverie::BoundType* boneTransformsType = templateData.Type;
  }

  // Bind the runtime array type instantiator (creates the different arrays when
  // instantiated)
  {
    String runtimeArrayTypeName = Raverie::SpirVNameSettings::mRuntimeArrayTypeName;
    Array<Raverie::TemplateParameter> templateTypes;
    TemplateParameter& typeParam = templateTypes.PushBack();
    typeParam.Name = "Type";
    typeParam.Type = ConstantType::Type;
    builder.AddTemplateInstantiator(runtimeArrayTypeName, InstantiateRuntimeArray, templateTypes, nullptr);
  }

  // Create the geometry shader input/output types
  {
    Array<Raverie::TemplateParameter> templateTypes;
    TemplateParameter& typeParam = templateTypes.PushBack();
    typeParam.Name = "Type";
    typeParam.Type = ConstantType::Type;

    builder.AddTemplateInstantiator("PointInput", InstantiateGeometryInput, templateTypes, (int*)spv::ExecutionModeInputPoints);
    builder.AddTemplateInstantiator("LineInput", InstantiateGeometryInput, templateTypes, (int*)spv::ExecutionModeInputLines);
    builder.AddTemplateInstantiator("TriangleInput", InstantiateGeometryInput, templateTypes, (int*)spv::ExecutionModeTriangles);

    builder.AddTemplateInstantiator("PointOutput", InstantiateGeometryOutput, templateTypes, (int*)spv::ExecutionModeOutputPoints);
    builder.AddTemplateInstantiator("LineOutput", InstantiateGeometryOutput, templateTypes, (int*)spv::ExecutionModeOutputLineStrip);
    builder.AddTemplateInstantiator("TriangleOutput", InstantiateGeometryOutput, templateTypes, (int*)spv::ExecutionModeOutputTriangleStrip);
  }

  // Create the GeometryStreamMoverType
  {
    Array<Raverie::TemplateParameter> templateTypes;
    TemplateParameter& fromTypeParam = templateTypes.PushBack();
    fromTypeParam.Name = "FromType";
    fromTypeParam.Type = ConstantType::Type;
    TemplateParameter& toTypeParam = templateTypes.PushBack();
    toTypeParam.Name = "ToType";
    toTypeParam.Type = ConstantType::Type;

    builder.AddTemplateInstantiator("GeometryStreamMover", InstantiateGeometryStreamMover, templateTypes, nullptr);
  }
}

} // namespace Raverie

namespace Raverie
{

RaverieDefineStaticLibrary(ShaderSettingsLibrary)
{
  builder.CreatableInScriptDefault = false;

  RaverieInitializeType(TranslationErrorEvent);
  RaverieInitializeType(ValidationErrorEvent);
  RaverieInitializeType(SpecializationConstantEvent);
}

} // namespace Raverie
