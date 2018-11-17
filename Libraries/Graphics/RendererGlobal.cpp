// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#include "Precompiled.hpp"

namespace Zero
{

namespace Z
{
  Renderer* gRenderer;
}

//**************************************************************************************************
ZilchDefineExternalBaseType(GraphicsDriverSupport, TypeCopyMode::ReferenceType, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindFieldGetter(mTextureCompression);
  ZilchBindFieldGetter(mMultiTargetBlend);
  ZilchBindFieldGetter(mSamplerObjects);
}

} // namespace Zero
