// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

DeclareEnum2(Centering, UpperLeft, Middle);
DeclareEnum2(TextureUvMode, Normal, RenderTarget);

/// Display displays a texture with an effect.
class TextureView : public Widget
{
public:
  RaverieDeclareType(TextureView, TypeCopyMode::ReferenceType);

  TextureView(Composite* composite);

  void SetTexture(StringParam name);
  void SetTexture(Texture* texture);

  void OnLeftMouseDown(MouseEvent* e);
  void OnMouseScroll(MouseEvent* e);

  void SetUv(Vec2 uv0, Vec2 uv1);
  void ClipUvToAspectRatio(float targetAspect);

  void SizeToContents() override;

  void RenderUpdate(ViewBlock& viewBlock,
                    FrameBlock& frameBlock,
                    Mat4Param parentTx,
                    ColorTransform colorTx,
                    WidgetRect clipRect) override;

  HandleOf<Texture> mTexture;
  Vec2 mUv0;
  Vec2 mUv1;
  float mTimeSinceLastDrag;

  /// XY are mouse position
  /// Z is zoom
  /// W is current auto-rotation
  Vec4 mSkyboxInput;
};

} // namespace Raverie
