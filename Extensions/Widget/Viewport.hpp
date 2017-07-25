///////////////////////////////////////////////////////////////////////////////
///
/// \file Viewport.hpp
/// Declaration of the Viewport widget.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------------------- Viewport
/// Viewport widget is a widget that displays a 3d scene.
class Viewport : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Viewport(Composite* parent, Space* space, Camera* camera);

  void OnDestroy();

  Space* GetTargetSpace();
  Camera* GetCamera();

  // Set the scaling mode for locked aspect ratio or fixed size.
  //void SetScalingAndSize(ViewportScaling::Enum scalingMode, uint width, uint height);

  // Border when inner viewport size is reduced with Scaling mode
  //void SetMarginColor(Vec4 color);

  //void ScreenCaptureBackBuffer(Image& image) override;
  //void ScreenCapture(StringParam filename);

  Vec2 ScreenToViewport(Vec2Param screenPoint);
  Vec2 ViewportToScreen(Vec2Param viewportPoint);

  Vec3 ViewportToWorld(Vec2Param viewportPoint);
  Vec2 WorldToViewport(Vec3Param worldPoint);

  Ray  ScreenToWorldRay(Vec2Param screenPoint);
  Vec3 ScreenToWorldZPlane(Vec2Param screenPoint, float worldDepth);
  Vec3 ScreenToWorldViewPlane(Vec2Param screenPoint, float viewDepth);
  Vec3 ScreenToWorldPlane(Vec2Param screenPoint, Vec3Param worldPlaneNormal, Vec3Param worldPlanePosition);

  Vec2 WorldToScreen(Vec3Param worldPoint);

  Vec2 ViewPlaneSize(float viewDepth);
  Frustum SubFrustum(Vec2Param upperLeftScreen, Vec2Param lowerRightScreen);

  void SetSize(Vec2 baseSize, Vec2 newSize);
  void SetTranslation(Vec3 baseOffset, Vec3 newOffset);
  void SetMarginColor(Vec4 color);

  Space* mTargetSpace;
  Camera* mCamera;
  ViewportInterface* mViewportInterface;

  HandleOf<Texture> mViewportTexture;
  Element* mMargin[4];
};

class ViewportDisplay : public Widget
{
public:
  ViewportDisplay(Composite* parent);

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  Viewport* mViewport;
};

}//namespace Zero

