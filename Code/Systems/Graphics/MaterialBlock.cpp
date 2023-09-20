// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineTemplateType(ProxyObject<MaterialBlock>, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

RaverieDefineType(MaterialBlock, builder, type)
{
  type->CreatableInScript = true;
  RaverieBindDefaultCopyDestructor();

  type->Add(new CogComponentMeta(type));
  type->Add(new MetaSerialization());
}

MaterialBlock::MaterialBlock() : mOwner(nullptr)
{
  mAligned = 0;
}

void MaterialBlock::Serialize(Serializer& stream)
{
  MetaSerializeProperties(this, stream);
}

void MaterialBlock::MarkAsModified()
{
  if (mOwner != nullptr)
    mOwner->ResourceModified();
}

IndexRange MaterialBlock::AddShaderInputs(Array<ShaderInput>& shaderInputs)
{
  IndexRange range;

  RaverieShaderGenerator* shaderGenerator = Z::gEngine->has(GraphicsEngine)->mShaderGenerator;

  range.start = shaderInputs.Size();

  BoundType* materialType = RaverieVirtualTypeId(this);
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

// Helper function for fragment properties.
static byte* GetFragmentMemberPointer(Call& call, MaterialBlock* materialBlock)
{
  byte* fragmentMemory = (byte*)materialBlock + sizeof(MaterialBlock);
  size_t memberOffset = (size_t)call.GetFunction()->UserData;
  return fragmentMemory + memberOffset;
}

void FragmentConstructor(Call& call, ExceptionReport& report)
{
  // Get the allocated memory.
  byte* memory = call.GetHandle(Call::This).Dereference();

  // Initialize base class.
  MaterialBlock* materialBlock = new (memory) MaterialBlock;

  // Get default values stored on BoundType.
  ByteBufferBlock& defaultMemory = materialBlock->RaverieGetDerivedType()->ComplexUserData.ReadObject<ByteBufferBlock>(0);

  // Initialize derived class.
  byte* fragmentMemory = memory + sizeof(MaterialBlock);
  memcpy(fragmentMemory, defaultMemory.GetBegin(), defaultMemory.Size());
}

void FragmentDestructor(Call& call, ExceptionReport& report)
{
  // All fragment data is pod, nothing to destruct.

  // Destruct base class.
  MaterialBlock* materialBlock = call.Get<MaterialBlock*>(Call::This);
  materialBlock->~MaterialBlock();
}

void FragmentGetter(Call& call, ExceptionReport& report)
{
  // Get pointer to the property.
  MaterialBlock* materialBlock = call.Get<MaterialBlock*>(Call::This);
  byte* memberPtr = GetFragmentMemberPointer(call, materialBlock);

  // Get the type's size off of the return type so that we don't need to store
  // it.
  size_t returnSize = call.GetFunction()->FunctionType->Return->GetCopyableSize();
  // Copy member to return value.
  memcpy(call.GetReturnUnchecked(), memberPtr, returnSize);
  call.MarkReturnAsSet();
}

void FragmentSetter(Call& call, ExceptionReport& report)
{
  // Get pointer to the property.
  MaterialBlock* materialBlock = call.Get<MaterialBlock*>(Call::This);
  byte* memberPtr = GetFragmentMemberPointer(call, materialBlock);

  // Get the type's size off of the parameter type so that we don't need to
  // store it.
  size_t memberSize = call.GetFunction()->FunctionType->Parameters[0].ParameterType->GetCopyableSize();
  // Copy parameter value to member.
  memcpy(memberPtr, call.GetParameterUnchecked(0), memberSize);

  // If this fragment belongs to a Material, this will tell it that its shader
  // inputs have changed.
  materialBlock->MarkAsModified();
}

void FragmentTextureGetter(Call& call, ExceptionReport& report)
{
  // Get pointer to the property.
  MaterialBlock* materialBlock = call.Get<MaterialBlock*>(Call::This);
  byte* memberPtr = GetFragmentMemberPointer(call, materialBlock);

  // Lookup Texture from stored ID.
  u64 textureId = *(u64*)(memberPtr);
  Texture* texture = TextureManager::FindOrNull(textureId);
  // Set the return value.
  call.Set<Texture*>(Call::Return, texture);
}

void FragmentTextureSetter(Call& call, ExceptionReport& report)
{
  // Get pointer to the property.
  MaterialBlock* materialBlock = call.Get<MaterialBlock*>(Call::This);
  byte* memberPtr = GetFragmentMemberPointer(call, materialBlock);

  // Get the Texture from the parameter.
  Texture* texture = call.Get<Texture*>(0);
  // Set the member to the Texture ID.
  if (texture != nullptr)
    *(u64*)memberPtr = (u64)texture->mResourceId;
  else
    *(u64*)memberPtr = 0;

  // If this fragment belongs to a Material, this will tell it that its shader
  // inputs have changed.
  materialBlock->MarkAsModified();
}

} // namespace Raverie
