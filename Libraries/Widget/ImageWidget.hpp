// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class SlicedDefinition;

class ImageWidget : public Widget
{
public:
  ZilchDeclareType(ImageWidget, TypeCopyMode::ReferenceType);

  SlicedDefinition* mDef;
  ImageWidget(Composite* parent, StringParam style, AttachType::Enum attachType = AttachType::Normal);

  Thickness GetBorderThickness() override;
  Vec2 GetMinSize() override;
  void ChangeDefinition(BaseDefinition* def) override;

  DisplayOrigin::Type GetDisplayOrigin();
  void SetDisplayOrigin(DisplayOrigin::Type displayOrigin);

  void RenderUpdate(ViewBlock& viewBlock,
                    FrameBlock& frameBlock,
                    Mat4Param parentTx,
                    ColorTransform colorTx,
                    WidgetRect clipRect) override;

  String GetDebugName() const override;
};

} // namespace Zero
