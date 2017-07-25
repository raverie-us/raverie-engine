///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

inline void DrawAnchors(WorldAnchorAtom& anchors, Collider* obj0, Collider* obj1)
{
  //get the object positions
  Vec3 obj0Pos = obj0->GetWorldTranslation();
  Vec3 obj1Pos = obj1->GetWorldTranslation();

  //draw lines from each object's center to its respective anchor
  gDebugDraw->Add(Debug::Line(obj0Pos, anchors.mWorldPoints[0]).Color(Color::White));
  gDebugDraw->Add(Debug::Line(obj1Pos, anchors.mWorldPoints[1]).Color(Color::Black));
  //draw a line between the anchors
  gDebugDraw->Add(Debug::Line(anchors.mWorldPoints[0], anchors.mWorldPoints[1]).Color(Color::Gray));
}

inline void DrawBasis(Mat3Param basis, Vec3Param pos, real size = real(1.0))
{
  gDebugDraw->Add(Debug::Line(pos, pos + basis.GetBasis(0) * size).Color(Color::Red));
  gDebugDraw->Add(Debug::Line(pos, pos + basis.GetBasis(1) * size).Color(Color::Green));
  gDebugDraw->Add(Debug::Line(pos, pos + basis.GetBasis(2) * size).Color(Color::Blue));
}

inline void DrawArc(Vec3Param center, Vec3Param normal, Vec3Param axis, real radiusA, real radiusB,
  real minAngle, real maxAngle, bool drawBoundaries, ByteColor color, real radianStepSize, bool leftHanded = false)
{
  Vec3 xAxis = axis;
  Vec3 yAxis = Math::Cross(normal, axis).Normalized();
  if(leftHanded)
    yAxis *= -1;
  uint numSteps = (uint)((maxAngle - minAngle) / radianStepSize);

  real angle = minAngle;
  Vec3 prev = center + radiusA * xAxis * Math::Cos(angle) + radiusB * yAxis * Math::Sin(angle);
  Vec3 start = prev;
  for(uint i = 0; i < numSteps; ++i)
  {
    angle = minAngle + (maxAngle - minAngle) * real(i) / real(numSteps);
    angle += radianStepSize;
    Vec3 next = center + radiusA * xAxis * Math::Cos(angle) + radiusB * yAxis * Math::Sin(angle);

    gDebugDraw->Add(Debug::Line(prev, next).Color(color));
    prev = next;
  }

  if(drawBoundaries)
  {
    gDebugDraw->Add(Debug::Line(center, start).Color(color));
    gDebugDraw->Add(Debug::Line(center, prev).Color(color));
  }
}

inline void DrawHinge(Joint* joint, AnchorAtom& anchors, Mat3Param basis0, Mat3Param basis1, uint defaultPerspective = 0)
{
  JointDebugDrawConfig* drawConfig = joint->GetOwner()->has(JointDebugDrawConfig);
  real size = real(1.0);
  real detail = real(10.0);
  uint perspective = defaultPerspective == 0 ? JointDebugDrawConfigFlags::ObjectAPerspective : JointDebugDrawConfigFlags::ObjectBPerspective;
  if(drawConfig && drawConfig->GetActive())
  {
    size = drawConfig->mSize;
    detail = drawConfig->mDetail;
    perspective = drawConfig->mPerspective.Field;
  }

  JointLimit* limit = joint->mNode->mLimit;
  bool drawBoundaries = false;
  real min = -Math::cPi;
  real max = Math::cPi;
  if(limit != nullptr && limit->GetActive())
  {
    min = limit->mMinErr;
    max = limit->mMaxErr;
    if(min < max && min > -Math::cPi && max < Math::cPi)
      drawBoundaries = true;
    else
    {
      min = -Math::cPi;
      max = Math::cPi;
    }
  }

  WorldAnchorAtom worldAnchors(anchors, joint);
  DrawAnchors(worldAnchors, joint->GetCollider(0), joint->GetCollider(1));

  DrawBasis(basis0, worldAnchors.mWorldPoints[0], size);
  DrawBasis(basis1, worldAnchors.mWorldPoints[1], size);

  if(perspective & JointDebugDrawConfigFlags::ObjectAPerspective)
  {
    Vec3 normal = basis0.GetBasis(2);
    Vec3 axis = basis0.GetBasis(0);
    DrawArc(worldAnchors.mWorldPoints[0], normal, axis.Normalized(),
      size, size, min, max, drawBoundaries, Color::Red, real(1.0) / detail, true);
  }
  if(perspective & JointDebugDrawConfigFlags::ObjectBPerspective)
  {
    Vec3 normal = basis1.GetBasis(2);
    Vec3 axis = basis1.GetBasis(0);
    DrawArc(worldAnchors.mWorldPoints[1], normal, axis.Normalized(),
      size, size, min, max, drawBoundaries, Color::Blue, real(1.0) / detail, false);
  }
}

}//namespace Physics

}//namespace Zero
