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
ZilchDefineType(GraphicsDriverSupport, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindFieldGetter(mTextureCompression);
  ZilchBindFieldGetter(mMultiTargetBlend);
  ZilchBindFieldGetter(mSamplerObjects);
}

//**************************************************************************************************
GraphicsDriverSupport::GraphicsDriverSupport()
  : mTextureCompression(false)
  , mMultiTargetBlend(false)
  , mSamplerObjects(false)
  , mIntel(false)
{
}

//**************************************************************************************************
Renderer::Renderer()
  : mBackBufferSafe(true)
{
}

//**************************************************************************************************
Renderer::~Renderer()
{
}

} // namespace Zero
