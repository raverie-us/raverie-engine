///////////////////////////////////////////////////////////////////////////////
///
/// \file TextureView.hpp
/// Declaration of Texture View Composites.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum2(Centering, UpperLeft, Middle);
DeclareEnum2(TextureUvMode, Normal, RenderTarget);

//----------------------------------------------------------------- Texture View
///Display displays a texture with an effect.
class TextureView : public Widget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TextureView(Composite* composite);

  void SetTexture(StringParam name);
  void SetTexture(Texture* texture);

  void OnLeftMouseDown(MouseEvent* e);
  void OnMouseScroll(MouseEvent* e);

  void SetUv(Vec2 uv0, Vec2 uv1);
  void ClipUvToAspectRatio(float targetAspect);

  void SizeToContents() override;

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  HandleOf<Texture> mTexture;
  Vec2 mUv0;
  Vec2 mUv1;
  float mTimeSinceLastDrag;

  /// XY are mouse position
  /// Z is zoom
  /// W is current auto-rotation
  Vec4 mSkyboxInput;
};

}//namespace Zero
