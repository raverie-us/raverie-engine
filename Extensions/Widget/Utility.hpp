///////////////////////////////////////////////////////////////////////////////
///
/// \file Utility.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ColorBlock : public Widget
{
public:
  ColorBlock(Composite* parent, AttachType::Enum attachType = AttachType::Normal)
    :Widget(parent, attachType)
  {
  }

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
  {
    Texture* texture = TextureManager::FindOrNull("White");
    if (texture == nullptr)
      return;

    Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

    Vec4 color = mColor * colorTx.ColorMultiply;

    Vec3 pos0 = Vec3(0.0f);
    Vec3 pos1 = Vec3(mSize, 0.0f);

    Vec2 uv0 = Vec2(0.0f);
    Vec2 uv1 = Vec2(1.0f);

    ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, texture);
    frameBlock.mRenderQueues->AddStreamedQuad(viewNode, pos0, pos1, uv0, uv1, color);
  }
};

inline ColorBlock* CreateBlackOut(Composite* composite, AttachType::Enum attach = AttachType::Normal)
{
  ColorBlock* block = new ColorBlock(composite, attach);

  block->SetSize(composite->GetSize());
  block->SetColor(Vec4(0,0,0,0.8f));
  return block;
}

}