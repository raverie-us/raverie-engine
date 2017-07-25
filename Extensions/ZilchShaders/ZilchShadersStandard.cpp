///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zilch
{

// Make a dummy function for binding FixedArray's functions. They don't need to do anything except be a valid function for calling.
// The only function that should currently call this is the FixedArray's constructor when pre-constructing objects to find default values.
void FixedArrayDummyFunction(Call& call, ExceptionReport& report) {}

BoundType* InstantiateFixedArray(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;
  // Bind the arraytype
  BoundType* arrayType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);

  // Bind all of the array's functions and properties (stubbed out since we're only using this for translation)
  builder.AddBoundConstructor(arrayType, FixedArrayDummyFunction, ParameterArray());
  builder.AddBoundFunction(arrayType, "Add", FixedArrayDummyFunction, OneParameter(templateType), core.VoidType, Zilch::FunctionOptions::None);
  builder.AddBoundFunction(arrayType, "Get", FixedArrayDummyFunction, OneParameter(core.IntegerType, "index"), templateType, Zilch::FunctionOptions::None);
  builder.AddBoundFunction(arrayType, "Set", FixedArrayDummyFunction, TwoParameters(core.IntegerType, "index", templateType, "value"), core.VoidType, Zilch::FunctionOptions::None);
  builder.AddBoundGetterSetter(arrayType, "Count", core.IntegerType, nullptr, FixedArrayDummyFunction, Zilch::MemberOptions::None);

  return arrayType;
}

BoundType* InstantiateGeometryInput(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  // Bind all of the functions and properties (stubbed out since we're only using this for translation)
  builder.AddBoundConstructor(selfType, FixedArrayDummyFunction, ParameterArray());
  builder.AddBoundFunction(selfType, "Get", FixedArrayDummyFunction, OneParameter(core.IntegerType), templateType, Zilch::FunctionOptions::None);
  builder.AddBoundFunction(selfType, "Set", FixedArrayDummyFunction, TwoParameters(core.IntegerType, templateType), core.VoidType, Zilch::FunctionOptions::None);
  builder.AddBoundGetterSetter(selfType, "Count", core.IntegerType, nullptr, FixedArrayDummyFunction, Zilch::MemberOptions::None);

  return selfType;
}

BoundType* InstantiateGeometryOutput(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Core& core = Core::GetInstance();
  Type* templateType = templateTypes[0].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  selfType->CreatableInScript = false;
  // Bind all of the functions and properties (stubbed out since we're only using this for translation)
  builder.AddBoundFunction(selfType, "Append", FixedArrayDummyFunction, TwoParameters(templateType, core.IntegerType), core.VoidType, Zilch::FunctionOptions::None);
  builder.AddBoundFunction(selfType, "Restart", FixedArrayDummyFunction, ParameterArray(), core.VoidType, Zilch::FunctionOptions::None);

  return selfType;
}

BoundType* InstantiateGeometryStreamMover(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData)
{
  Type* fromType = templateTypes[0].TypeValue;
  Type* toType = templateTypes[1].TypeValue;

  BoundType* selfType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ValueType, 0);
  selfType->CreatableInScript = false;
  builder.AddBoundFunction(selfType, "Move", FixedArrayDummyFunction, OneParameter(fromType), toType, Zilch::FunctionOptions::Static);

  return selfType;
}

ZilchDefineStaticLibrary(ShaderIntrinsicsLibrary)
{
  builder.CreatableInScriptDefault = false;
  
  ZilchInitializeType(Shader);

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

    builder.AddTemplateInstantiator("PointInput", InstantiateGeometryInput, templateTypes, nullptr);
    builder.AddTemplateInstantiator("LineInput", InstantiateGeometryInput, templateTypes, nullptr);
    builder.AddTemplateInstantiator("TriangleInput", InstantiateGeometryInput, templateTypes, nullptr);

    builder.AddTemplateInstantiator("PointOutput", InstantiateGeometryOutput, templateTypes, nullptr);
    builder.AddTemplateInstantiator("LineOutput", InstantiateGeometryOutput, templateTypes, nullptr);
    builder.AddTemplateInstantiator("TriangleOutput", InstantiateGeometryOutput, templateTypes, nullptr);
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
