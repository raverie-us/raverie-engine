#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ProxyObject<MaterialBlock>, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

ZilchDefineType(MaterialBlock, builder, type)
{
  ZilchBindDefaultCopyDestructor();

  type->Add(new CogComponentMeta(type));
  type->Add(new MetaSerialization());
}

MaterialBlock::MaterialBlock() :
  mOwner(nullptr)
{
}

void MaterialBlock::Serialize(Serializer& stream)
{
  MetaSerializeProperties(this, stream);
}

IndexRange MaterialBlock::AddShaderInputs(Array<ShaderInput>& shaderInputs)
{
  IndexRange range;

  ZilchShaderGenerator* shaderGenerator = Z::gEngine->has(GraphicsEngine)->mShaderGenerator;

  range.start = shaderInputs.Size();

  BoundType* materialType = ZilchVirtualTypeId(this);
  forRange (Property* metaProperty, materialType->GetProperties())
  {
    ShaderInputType::Enum type = MaterialFactory::GetInstance()->GetShaderInputType(metaProperty->PropertyType);
    Any value = metaProperty->GetValue(this);
    
    ShaderInput shaderInput = shaderGenerator->CreateShaderInput(materialType->Name, metaProperty->Name, type, value);
    if (shaderInput.mShaderInputType != ShaderInputType::Invalid)
      shaderInputs.PushBack(shaderInput);
  }

  range.end = shaderInputs.Size();
  return range;
}

} // namespace Zero
