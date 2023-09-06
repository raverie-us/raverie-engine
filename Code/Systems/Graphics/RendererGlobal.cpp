// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Zero
{

namespace Z
{
Renderer* gRenderer = nullptr;
}

ZilchDefineExternalBaseType(GraphicsDriverSupport, TypeCopyMode::ReferenceType, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindFieldGetter(mTextureCompression);
  ZilchBindFieldGetter(mMultiTargetBlend);
  ZilchBindFieldGetter(mSamplerObjects);
}

} // namespace Zero
