#include "Precompiled.hpp"

namespace Zero
{

void ShaderInput::SetValue(AnyParam value)
{
  if (mShaderInputType == ShaderInputType::Texture)
  {
    Texture* texture = value.Get<Texture*>(GetOptions::ReturnDefaultOrNull);
    if (texture == nullptr)
      texture = TextureManager::GetDefault();

    *(TextureRenderData**)mValue = texture->mRenderData;
  }
  else if (value.IsNotNull())
  {
    ErrorIf(value.StoredType->GetAllocatedSize() > MaxSize, "Type cannot be stored in ShaderInput.");
    memcpy(mValue, value.Dereference(), value.StoredType->GetAllocatedSize());
  }
  else
  {
    memset(mValue, 0, MaxSize);
  }
}

} // namespace Zero
