///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class SlicedDefinition;

class ImageWidget : public Widget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SlicedDefinition* mDef;
  ImageWidget(Composite* parent, StringParam style, AttachType::Enum attachType = AttachType::Normal);

  Thickness GetBorderThickness() override;
  Vec2 GetMinSize() override;
  void ChangeDefinition(BaseDefinition* def) override;

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  String GetDebugName() override;
};

}
