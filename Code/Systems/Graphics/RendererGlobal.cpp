// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

namespace Z
{
Renderer* gRenderer = nullptr;
}

RaverieDefineExternalBaseType(GraphicsDriverSupport, TypeCopyMode::ReferenceType, builder, type)
{
  type->HandleManager = RaverieManagerId(PointerManager);

  RaverieBindFieldGetter(mTextureCompression);
  RaverieBindFieldGetter(mMultiTargetBlend);
  RaverieBindFieldGetter(mSamplerObjects);
}

} // namespace Raverie
