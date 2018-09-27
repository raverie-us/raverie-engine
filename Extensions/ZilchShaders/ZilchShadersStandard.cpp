///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "ShaderErrors.hpp"

namespace Zilch
{

BoundType* InstantiateFixedArray(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;
  // Bind the arraytype
  BoundType* arrayType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);

  // Bind all of the array's functions and properties (stubbed out since we're only using this for translation)
  builder.AddBoundConstructor(arrayType, Zero::DummyBoundFunction, ParameterArray());
  builder.AddBoundFunction(arrayType, "Add", Zero::DummyBoundFunction, OneParameter(templateType), core.VoidType, Zilch::FunctionOptions::None);
  builder.AddBoundFunction(arrayType, "Get", Zero::DummyBoundFunction, OneParameter(core.IntegerType, "index"), templateType, Zilch::FunctionOptions::None);
  builder.AddBoundFunction(arrayType, "Set", Zero::DummyBoundFunction, TwoParameters(core.IntegerType, "index", templateType, "value"), core.VoidType, Zilch::FunctionOptions::None);
  builder.AddBoundGetterSetter(arrayType, "Count", core.IntegerType, nullptr, Zero::DummyBoundFunction, Zilch::MemberOptions::None);

  return arrayType;
}

BoundType* InstantiateGeometryInput(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  // Bind all of the functions and properties (stubbed out since we're only using this for translation)
  builder.AddBoundConstructor(selfType, Zero::UnTranslatedBoundFunction, ParameterArray());
  builder.AddBoundFunction(selfType, "Get", Zero::UnTranslatedBoundFunction, OneParameter(core.IntegerType), templateType, Zilch::FunctionOptions::None);
  builder.AddBoundFunction(selfType, "Set", Zero::UnTranslatedBoundFunction, TwoParameters(core.IntegerType, templateType), core.VoidType, Zilch::FunctionOptions::None);
  builder.AddBoundGetterSetter(selfType, "Count", core.IntegerType, nullptr, Zero::UnTranslatedBoundFunction, Zilch::MemberOptions::None);

  Zilch::HandleOf<GeometryStreamUserData> handle = ZilchAllocate(GeometryStreamUserData);
  handle->Set((spv::ExecutionMode)(int)userData);
  selfType->Add(*handle);

  return selfType;
}

BoundType* InstantiateGeometryOutput(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  selfType->CreatableInScript = true;
  // Bind all of the functions and properties (stubbed out since we're only using this for translation)
  builder.AddBoundFunction(selfType, "Append", Zero::UnTranslatedBoundFunction, TwoParameters(templateType, core.IntegerType), core.VoidType, Zilch::FunctionOptions::None);
  builder.AddBoundFunction(selfType, "Restart", Zero::UnTranslatedBoundFunction, ParameterArray(), core.VoidType, Zilch::FunctionOptions::None);

  Zilch::HandleOf<GeometryStreamUserData> handle = ZilchAllocate(GeometryStreamUserData);
  handle->Set((spv::ExecutionMode)(int)userData);
  selfType->Add(*handle);

  return selfType;
}

BoundType* InstantiateGeometryStreamMover(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Type* fromType = templateTypes[0].TypeValue;
  Type* toType = templateTypes[1].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  selfType->CreatableInScript = false;
  builder.AddBoundFunction(selfType, "Move", Zero::UnTranslatedBoundFunction, OneParameter(fromType), toType, Zilch::FunctionOptions::Static);

  return selfType;
}

ZilchDefineStaticLibrary(ShaderIntrinsicsLibrary)
{
  builder.CreatableInScriptDefault = false;
  
  // BoundType Components
  ZilchInitializeType(GeometryStreamUserData);
  ZilchInitializeType(GeometryFragmentUserData);

  // @Nate: These have to be uncommented for new shaders
  ZilchInitializeType(ShaderIntrinsics);
  ZilchInitializeType(Sampler);
  ZilchInitializeType(Image2d);
  ZilchInitializeType(DepthImage2d);
  ZilchInitializeType(ImageCube);
  ZilchInitializeType(SampledImage2d);
  ZilchInitializeType(SampledDepthImage2d);
  ZilchInitializeType(SampledImageCube);

  // Bind the fixed array type instantiator (creates the different arrays when instantiated)
  {
    Array<Zilch::TemplateParameter> templateTypes;
    TemplateParameter& typeParam = templateTypes.PushBack();
    typeParam.Name = "Type";
    typeParam.Type = ConstantType::Type;
    TemplateParameter& sizeParam = templateTypes.PushBack();
    sizeParam.Name = "Size";
    sizeParam.Type = ConstantType::Integer;
    builder.AddTemplateInstantiator("FixedArray", InstantiateFixedArray, templateTypes, nullptr);
  }

  // Create the geometry shader input/output types
  {
    Array<Zilch::TemplateParameter> templateTypes;
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
    Array<Zilch::TemplateParameter> templateTypes;
    TemplateParameter& fromTypeParam = templateTypes.PushBack();
    fromTypeParam.Name = "FromType";
    fromTypeParam.Type = ConstantType::Type;
    TemplateParameter& toTypeParam = templateTypes.PushBack();
    toTypeParam.Name = "ToType";
    toTypeParam.Type = ConstantType::Type;

    builder.AddTemplateInstantiator("GeometryStreamMover", InstantiateGeometryStreamMover, templateTypes, nullptr);
  }
}

}//namespace Zilch

namespace Zero
{

//-------------------------------------------------------------------ShaderSettingsLibrary
ZilchDefineStaticLibrary(ShaderSettingsLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(TranslationErrorEvent);
  ZilchInitializeType(ValidationErrorEvent);
  ZilchInitializeType(SpecializationConstantEvent);
}

}//namespace Zero
