///////////////////////////////////////////////////////////////////////////////
///
/// \file Viewport.cpp
/// Implementation of the Viewport widget.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ViewportUi
{
  const cstr cLocation = "EditorUi/Viewport";
  Tweakable(Vec4, BorderColor, Vec4(1,1,1,1), cLocation);
}

//--------------------------------------------------------------------- Viewport
ZilchDefineType(Viewport, builder, type)
{
}

Viewport::Viewport(Composite* parent, Space* space, Camera* camera)
  : Composite(parent)
  , mTargetSpace(space)
  , mCamera(camera)
{
  for (uint i = 0; i < 4; ++i)
  {
    mMargin[i] = CreateAttached<Element>("White");
    mMargin[i]->SetInteractive(false);
  }

  mViewportTexture = nullptr;
  new ViewportDisplay(this);
}

void Viewport::OnDestroy()
{
  Composite::OnDestroy();
  mViewportInterface = nullptr;
}

Space* Viewport::GetTargetSpace()
{
  return mTargetSpace;
}

Camera* Viewport::GetCamera()
{
  return mCamera;
}

//void Viewport::SetScalingAndSize(ViewportScaling::Enum scalingMode, uint width, uint height)
//{
//  mRenderView->mTargetSize = IntVec2(width, height);
//  mRenderView->mScalingMode = scalingMode;
//}

//void Viewport::SetMarginColor(Vec4 color)
//{
//  mRenderView->mMarginColor = color;
//}

//void Viewport::ScreenCaptureBackBuffer(Image& image)
//{
//  mRenderView->ScreenCaptureBackBuffer(image);
//}
//
//void Viewport::ScreenCapture(StringParam filename)
//{
//  mRenderView->ScreenCaptureBackBuffer(filename);
//}

Vec2 Viewport::ScreenToViewport(Vec2Param screenPoint)
{
  return ToLocal(screenPoint);
}

Vec2 Viewport::ViewportToScreen(Vec2Param viewportPoint)
{
  return ToScreen(viewportPoint);
}

Vec3 Viewport::ViewportToWorld(Vec2Param viewportPoint)
{
  Vec2 sizePercent = viewportPoint / mSize;

  Transform* transform = mCamera->mTransform;
  float fov = Math::DegToRad(mCamera->mFieldOfView);

  Vec2 nearSize;
  if (mCamera->mPerspectiveMode == PerspectiveMode::Perspective)
    nearSize.y = Math::Tan(fov * 0.5f) * mCamera->mNearPlane * 2.0f;
  else
    nearSize.y = mCamera->mSize;
  nearSize.x = nearSize.y * (mSize.x / mSize.y);

  Mat3 rotation = Math::ToMatrix3(transform->GetWorldRotation());
  Vec3 basisZ = -rotation.BasisZ();
  Vec3 basisX = rotation.BasisX();
  Vec3 basisY = rotation.BasisY();

  // Start at near plane center
  Vec3 worldPoint = transform->GetWorldTranslation() + basisZ * mCamera->mNearPlane;
  // Offset from the center to the final point
  // Half offsets on sizePercent values are because viewport (0, 0) is top left of near plane
  worldPoint += (basisX * nearSize.x) * (sizePercent.x - 0.5f);
  worldPoint += (basisY * nearSize.y) * (0.5f - sizePercent.y);

  return worldPoint;
}

Vec2 Viewport::WorldToViewport(Vec3Param worldPoint)
{
  Mat4 viewTransform = mCamera->GetViewTransform();

  Vec3 viewPoint = Math::TransformPoint(viewTransform, worldPoint);
  if (Math::IsZero(viewPoint.z))
    return Vec2::cZero;

  Vec2 nearPoint = Vec2(viewPoint.x, viewPoint.y);
  if (mCamera->mPerspectiveMode == PerspectiveMode::Perspective)
    nearPoint *= -(mCamera->mNearPlane / viewPoint.z);

  float fov = Math::DegToRad(mCamera->mFieldOfView);
  Vec2 nearSize;
  if (mCamera->mPerspectiveMode == PerspectiveMode::Perspective)
    nearSize.y = Math::Tan(fov * 0.5f) * mCamera->mNearPlane * 2.0f;
  else
    nearSize.y = mCamera->mSize;
  nearSize.x = nearSize.y * (mSize.x / mSize.y);

  Vec2 sizePercent = nearPoint / nearSize + Vec2(0.5f, 0.5f);
  sizePercent.y = 1.0f - sizePercent.y;

  return mSize * sizePercent;
}

Ray Viewport::ScreenToWorldRay(Vec2Param screenPoint)
{
  Vec2 viewportPoint = ScreenToViewport(screenPoint);

  Vec3 worldPoint = ViewportToWorld(viewportPoint);
  Vec3 dir;
  if (mCamera->mPerspectiveMode == PerspectiveMode::Perspective)
    dir = (worldPoint - mCamera->GetWorldTranslation()).Normalized();
  else
    dir = mCamera->GetWorldDirection();

  return Ray(worldPoint, dir);
}

Vec3 Viewport::ScreenToWorldZPlane(Vec2Param screenPoint, float worldDepth)
{
  Ray ray = ScreenToWorldRay(screenPoint);

  float t = 0.0f;
  if(ray.Direction.z != 0.0f)
    t = (worldDepth - ray.Start.z) / ray.Direction.z;
  return ray.GetPoint(t);
}

Vec3 Viewport::ScreenToWorldViewPlane(Vec2Param screenPoint, float viewDepth)
{
  Ray ray = ScreenToWorldRay(screenPoint);

  Vec3 viewDirection = Vec3(0, 0, -1);
  Transform* t = mCamera->mTransform;
  if (t != nullptr)
  {
    viewDirection = t->TransformNormal(viewDirection);
    viewDirection.AttemptNormalize();
  }

  Vec3 planePoint = ray.GetPoint(viewDepth);

  Plane plane;
  plane.Set(viewDirection, planePoint);

  Intersection::IntersectionPoint iPoint;
  Intersection::Type result = Intersection::RayPlane(ray.Start, ray.Direction, plane.GetNormal(), plane.GetDistance(), &iPoint);
  if (result == Intersection::None)
    return ray.GetPoint(0.0f);
  return iPoint.Points[0];
}

Vec3 Viewport::ScreenToWorldPlane(Vec2Param screenPoint, Vec3Param worldPlaneNormal, Vec3Param worldPlanePosition)
{
  Ray ray = ScreenToWorldRay(screenPoint);

  Plane plane;
  plane.Set(worldPlaneNormal, worldPlanePosition);

  Intersection::IntersectionPoint iPoint;
  Intersection::Type result = Intersection::RayPlane(ray.Start, ray.Direction, plane.GetNormal(), plane.GetDistance(), &iPoint);
  if(result == Intersection::None)
    return ray.GetPoint(0.0f);
  return iPoint.Points[0];
}

Vec2 Viewport::WorldToScreen(Vec3Param worldPoint)
{
  Vec2 viewportPoint = WorldToViewport(worldPoint);
  return ViewportToScreen(viewportPoint);
}

Vec2 Viewport::ViewPlaneSize(float viewDepth)
{
  float frustumDepth = viewDepth -= mCamera->mNearPlane;

  Vec2 screenSize = mSize;

  float aspect = screenSize.x / screenSize.y;
  Frustum frustum = mCamera->GetFrustum(aspect);

  Vec3 boxPoints[4];
  frustum.PointsAtDepth(boxPoints, frustumDepth);

  return Vec2((boxPoints[0] - boxPoints[1]).Length(),
              (boxPoints[1] - boxPoints[2]).Length());

}

Frustum Viewport::SubFrustum(Vec2Param upperLeftScreen, Vec2Param lowerRightScreen)
{
  float farPlaneDistance = mCamera->mFarPlane;

  Vec3 points[8];

  Vec2 lowerLeftScreen = Vec2(upperLeftScreen.x, lowerRightScreen.y);
  Vec2 upperRightScreen = Vec2(lowerRightScreen.x, upperLeftScreen.y);

  Ray upperLeft = ScreenToWorldRay(upperLeftScreen);
  Ray upperRight = ScreenToWorldRay(upperRightScreen);
  Ray lowerLeft = ScreenToWorldRay(lowerLeftScreen);
  Ray lowerRight = ScreenToWorldRay(lowerRightScreen);

  points[0] = upperLeft.Start;
  points[1] = upperRight.Start;
  points[2] = lowerRight.Start;
  points[3] = lowerLeft.Start;
  points[4] = upperLeft.GetPoint(farPlaneDistance);
  points[5] = upperRight.GetPoint(farPlaneDistance);
  points[6] = lowerRight.GetPoint(farPlaneDistance);
  points[7] = lowerLeft.GetPoint(farPlaneDistance);

  Frustum frustum;
  frustum.Generate(points);

  return frustum;
}

void Viewport::SetSize(Vec2 baseSize, Vec2 newSize)
{
  Composite::SetSize(newSize);

  mMargin[0]->SetSize(Pixels(baseSize.x, (baseSize.y - newSize.y) * 0.5f)); // T
  mMargin[1]->SetSize(Pixels(baseSize.x, baseSize.y - newSize.y - mMargin[0]->mSize.y)); // B
  mMargin[2]->SetSize(Pixels((baseSize.x - newSize.x) * 0.5f, newSize.y)); // L
  mMargin[3]->SetSize(Pixels(baseSize.x - newSize.x - mMargin[2]->mSize.x, newSize.y)); // R
}

void Viewport::SetTranslation(Vec3 baseOffset, Vec3 newOffset)
{
  Composite::SetTranslation(newOffset);

  Vec3 marginBase = baseOffset - newOffset;
  mMargin[0]->SetTranslation(marginBase); // T
  mMargin[1]->SetTranslation(marginBase + Vec3(0.0f, mMargin[0]->mSize.y + mSize.y, 0.0f)); // B
  mMargin[2]->SetTranslation(marginBase + Vec3(0.0f, mMargin[0]->mSize.y, 0.0f)); // L
  mMargin[3]->SetTranslation(marginBase + Vec3(mMargin[2]->mSize.x + mSize.x, mMargin[0]->mSize.y, 0.0f)); // R
}

void Viewport::SetMarginColor(Vec4 color)
{
  mMargin[0]->SetColor(color);
  mMargin[1]->SetColor(color);
  mMargin[2]->SetColor(color);
  mMargin[3]->SetColor(color);
}

ViewportDisplay::ViewportDisplay(Composite* parent)
  : Widget(parent)
{
  mViewport = (Viewport*)parent;
}

void ViewportDisplay::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  if (mViewport->mViewportTexture == nullptr)
    return;

  Vec2 size = mViewport->mSize;
  Vec4 color = mViewport->mColor * colorTx.ColorMultiply;

  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, mViewport->mViewportTexture);
  frameBlock.mRenderQueues->AddStreamedQuad(viewNode, Vec3(0, 0, 0), Vec3(size, 0), Vec2(0, 0), Vec2(1, 1), color);
}

}//namespace Zero
