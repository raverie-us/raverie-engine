// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

void ShaderInputSetValue(ShaderInput& input, AnyParam value)
{
  if (input.mShaderInputType == ShaderInputType::Texture)
  {
    Texture* texture = value.Get<Texture*>(GetOptions::ReturnDefaultOrNull);
    if (texture == nullptr)
      texture = TextureManager::GetDefault();

    *(TextureRenderData**)input.mValue = texture->mRenderData;
  }
  else if (value.IsNotNull())
  {
    ErrorIf(value.StoredType->GetAllocatedSize() > ShaderInput::MaxSize, "Type cannot be stored in ShaderInput.");
    memcpy(input.mValue, value.Dereference(), value.StoredType->GetAllocatedSize());
  }
  else
  {
    memset(input.mValue, 0, ShaderInput::MaxSize);
  }
}

} // namespace Raverie
