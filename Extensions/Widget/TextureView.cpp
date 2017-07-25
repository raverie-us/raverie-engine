///////////////////////////////////////////////////////////////////////////////
///
/// \file TextureView.cpp
/// Implementation of Texture View Composites.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

class SkyboxDrag : public MouseManipulation
{
public:
  TextureView* mView;

  //****************************************************************************
  SkyboxDrag(MouseEvent* e, TextureView* view) : MouseManipulation(e->GetMouse(), view->GetParent())
  {
    mView = view;
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* event) override
  {
    mView->mSkyboxInput += Vec4(event->Movement.x, event->Movement.y, 0.0f, 0.0f);
    mView->mSkyboxInput.y = Math::Clamp(mView->mSkyboxInput.y, -330.0f, 330.0f);
    mView->mTimeSinceLastDrag = 0.0f;
  }

  //****************************************************************************
  void OnMouseScroll(MouseEvent* event) override
  {
    mView->mSkyboxInput.z += event->Scroll.y;
    mView->mSkyboxInput.z = Math::Clamp(mView->mSkyboxInput.z, 0.0f, 10.0f);
    event->Handled = true;
  }
};

//----------------------------------------------------------------- Texture View
ZilchDefineType(TextureView, builder, type)
{
}

//******************************************************************************
TextureView::TextureView(Composite* composite)
  : Widget(composite)
{
  mTexture = nullptr;
  mUv0 = Vec2(0, 0);
  mUv1 = Vec2(1, 1);
  mSkyboxInput = Vec4::cZero;
  mTimeSinceLastDrag = Math::PositiveMax();

  ConnectThisTo(this, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(this, Events::MouseScroll, OnMouseScroll);
}

//******************************************************************************
void TextureView::SetTexture(StringParam name)
{
  mTexture = TextureManager::FindOrNull(name);
}

//******************************************************************************
void TextureView::SetTexture(Texture* texture)
{
  mTexture = texture;
}

//******************************************************************************
void TextureView::OnLeftMouseDown(MouseEvent* e)
{
  if(Texture* texture = mTexture)
  {
    if(texture->mType == TextureType::TextureCube)
      new SkyboxDrag(e, this);
  }
}

//******************************************************************************
void TextureView::OnMouseScroll(MouseEvent* e)
{
  if(Texture* texture = mTexture)
  {
    if(texture->mType == TextureType::TextureCube)
    {
      mSkyboxInput.z += e->Scroll.y;
      mSkyboxInput.z = Math::Clamp(mSkyboxInput.z, 0.0f, 10.0f);
      e->Handled = true;
    }
  }
}

//******************************************************************************
void TextureView::SetUv(Vec2 uv0, Vec2 uv1)
{
  mUv0 = uv0;
  mUv1 = uv1;
}

//******************************************************************************
void TextureView::ClipUvToAspectRatio(float targetAspect)
{
  mUv0 = Vec2(0.0f);
  mUv1 = Vec2(1.0f);

  Texture* texture = mTexture;
  if (texture == nullptr)
    return;

  uint width = texture->mWidth;
  uint height = texture->mHeight;
  float aspect = (float)width / height;
  // height is too large
  if (aspect < targetAspect)
  {
    float targetHeight = width / targetAspect;
    float extra = (height - targetHeight) * 0.5f;
    mUv0.y = extra / height;
    mUv1.y = 1.0f - mUv0.y;
  }
  // width is too large
  else
  {
    float targetWidth = height * targetAspect;
    float extra = (width - targetWidth) * 0.5f;
    mUv0.x = extra / width;
    mUv1.x = 1.0f - mUv0.x;
  }
}

//******************************************************************************
void TextureView::SizeToContents()
{
  if (Texture* texture = mTexture)
    mSize = Math::ToVec2(texture->GetSize());
}

//******************************************************************************
void TextureView::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  Texture* texture = mTexture;
  if(texture == nullptr)
    return;

  Vec2 size = mSize;
  Vec4 color(1, 1, 1, 1);

  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, mTexture);
  FrameNode& frameNode = frameBlock.mFrameNodes.Back();

  if(texture->mType == TextureType::TextureCube)
  {
    Array<ShaderInput>& shaderInputs = frameBlock.mRenderQueues->mRenderTasks->mShaderInputs;

    // Store the starting point
    frameNode.mShaderInputRange.start = shaderInputs.Size();

    // Add our custom shader inputs
    ZilchShaderGenerator* shaderGenerator = Z::gEngine->has(GraphicsEngine)->mShaderGenerator;
    ShaderInput input = shaderGenerator->CreateShaderInput("TextureCubePreview", "SkyboxPreviewInput", ShaderInputType::Vec4, mSkyboxInput);
    shaderInputs.PushBack(input);
    input = shaderGenerator->CreateShaderInput("TextureCubePreview", "TextureViewResolution", ShaderInputType::Vec2, size);
    shaderInputs.PushBack(input);

    // Store the ending point
    frameNode.mShaderInputRange.end = shaderInputs.Size();

    // Update our timer since we last dragged the view
    mTimeSinceLastDrag += 0.016f;

    // Wait 2 seconds before continuing to auto-rotate
    if(mTimeSinceLastDrag >= 2.0f)
      mSkyboxInput.w += 0.016f * Math::Min((mTimeSinceLastDrag - 2.0f), 1.0f);
  }

  frameBlock.mRenderQueues->AddStreamedQuad(viewNode, Vec3(0, 0, 0), Vec3(size, 0), mUv0, mUv1, color * colorTx.ColorMultiply);
}

} // namespace Zero
