///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ImageWidget, builder, type)
{
}

ImageWidget::ImageWidget(Composite* parent, StringParam style, AttachType::Enum attachType)
  :Widget(parent, attachType)
{
  mDefSet = parent->mDefSet;
  mDef = (SlicedDefinition*)mDefSet->GetDefinition(style);

  if(mDef == nullptr)
    mDef = (SlicedDefinition*)mDefSet->GetDefinition("White");

  mSize = mDef->DefaultSize;
  mOrigin = mDef->ImageMode;
}

String ImageWidget::GetDebugName()
{
  return BuildString("Image:", mName, ":", mDef->Name);
}

Vec2 ImageWidget::GetMinSize()
{
  return mDef->DefaultSize;
}

Thickness ImageWidget::GetBorderThickness()
{
  return Thickness(mDef->Borders.x, mDef->Borders.y, mDef->Borders.z, mDef->Borders.w);
}

void ImageWidget::ChangeDefinition(BaseDefinition* def)
{
  mDef = (SlicedDefinition*)def;
}

void ImageWidget::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  Vec4 color = mColor * colorTx.ColorMultiply;

  Vec2 originOffset = Vec2(0, 0);
  if (mOrigin == DisplayOrigin::Center)
    originOffset = mSize * -0.5f;

  Vec3 pos0 = Vec3(Vec2(0, 0) + originOffset, 0);
  Vec3 pos1 = Vec3(Vec2(mSize.x, mSize.y) + originOffset, 0);

  Vec2 uv0 = mDef->Uv0;
  Vec2 uv1 = mDef->Uv1;

  Vec4 slices = mDef->Slices;
  Vec2 texSize = Math::ToVec2(mDef->TexturePtr->GetSize());

  Vec4 posSlices = slices;
  posSlices[SlicesIndex::Left] *= texSize.x;
  posSlices[SlicesIndex::Right] *= texSize.x;
  posSlices[SlicesIndex::Top] *= -texSize.y;
  posSlices[SlicesIndex::Bottom] *= -texSize.y;

  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, mDef->TexturePtr);

  if (mDef->Sliced)
    frameBlock.mRenderQueues->AddStreamedQuadNineSliced(viewNode, pos0, pos1, uv0, uv1, color, posSlices, slices);
  else
    frameBlock.mRenderQueues->AddStreamedQuad(viewNode, pos0, pos1, uv0, uv1, color);
}

}
