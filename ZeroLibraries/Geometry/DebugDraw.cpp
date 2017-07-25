///////////////////////////////////////////////////////////////////////////////
///
/// \file DebugDraw.cpp
/// Implementation of the Debug Draw classes and functions.
///
/// Authors: Chris Peters, Nathan Carlson, Ryan Edgemon
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

Debug::DebugDraw* gDebugDraw = nullptr;

namespace Debug
{

// Segment limits for circle and arc drawing
const float cSegmentMin = 16.0f;
const float cSegmentMax = 128.0f;
const float cSegmentSize = 0.001f;
const float cMinCircleRadius = 0.001f;

ByteColor AxisColors[] = {Color::Red, Color::Green, Color::Blue};

float GetViewScale(Vec3Param location, const DebugViewData& viewData)
{
  float viewDistance = GetViewDistance(location, viewData.mEyePosition, viewData.mEyeDirection);
  return GetViewScale(viewDistance, viewData.mFieldOfView, viewData.mOrthographicSize, viewData.mOrthographic);
}

float GetViewScale(float viewDistance, float fieldOfView, float orthographicSize, bool orthographic)
{
  if (viewDistance < 0.00001f)
    return Debug::cViewScale;

  float viewScale;

  if (orthographic)
    viewScale = orthographicSize * cViewScale;
  else
    viewScale = 2.0f * Math::Tan(Math::DegToRad(fieldOfView * 0.5f)) * viewDistance * cViewScale;

  return viewScale;
}

float GetViewDistance(Vec3Param location, Vec3Param eyePosition, Vec3 viewDirection)
{
  return Math::Dot(location - eyePosition, viewDirection);
}

template <typename DebugObjectType>
void DebugDrawObject<DebugObjectType>::SetupTypeHelper(LibraryBuilder& builder, BoundType* type)
{
  typedef DebugObjectType ZilchSelf;
  ZilchBindFieldProperty(mColor);
  type->AddAttribute(ExportDocumentation);
  //ZilchBindFieldProperty(mDuration); // Not implemented
  //ZilchBindFieldProperty(mWidth); // Not implemented

  //ZilchBindGetterSetterProperty(BackShade); // Not implemented
  //ZilchBindGetterSetterProperty(Border); // Not implemented
  //ZilchBindGetterSetterProperty(Filled); // Not implemented
  ZilchBindGetterSetterProperty(OnTop);
  ZilchBindGetterSetterProperty(ViewAligned);
  ZilchBindGetterSetterProperty(ViewScaled);
  ZilchBindFieldProperty(mViewScaleOffset);
}

ZilchDefineType(Arc, builder, type)
{
  DebugDrawObject<Arc>::SetupTypeHelper(builder, type);
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "start, mid, end", Vec3, Vec3, Vec3);

  ZilchBindFieldProperty(mStart);
  ZilchBindFieldProperty(mMid);
  ZilchBindFieldProperty(mEnd);
}


ZilchDefineType(Box, builder, type)
{
  DebugDrawObject<Box>::SetupTypeHelper(builder, type);
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, halfExtents", Vec3, Vec2);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, halfExtents", Vec3, float);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, halfExtents, rotation", Vec3, Vec2, Quat);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, halfExtents, rotation", Vec3, float, Quat);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "aabb", Aabb);

  ZilchBindFieldProperty(mPosition);
  ZilchBindFieldProperty(mHalfExtents);
  ZilchBindFieldProperty(mRotation);

  ZilchBindGetterSetterProperty(Corners); // Not implemented
}


ZilchDefineType(Capsule, builder, type)
{
  DebugDrawObject<Capsule>::SetupTypeHelper(builder, type);
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "start, end, radius", Vec3, Vec3, float);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, axis, height, radius", Vec3, Vec3, float, float);

  ZilchBindFieldProperty(mStart);
  ZilchBindFieldProperty(mEnd);
  ZilchBindFieldProperty(mRadius);
}


ZilchDefineType(Circle, builder, type)
{
  DebugDrawObject<Circle>::SetupTypeHelper(builder, type);
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, axis, radius", Vec3, Vec3, float);

  ZilchBindFieldProperty(mPosition);
  ZilchBindFieldProperty(mAxis);
  ZilchBindFieldProperty(mRadius);
}


ZilchDefineType(Cone, builder, type)
{
  DebugDrawObject<Cone>::SetupTypeHelper(builder, type);
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, direction, length, radius", Vec3, Vec3, float, float);

  ZilchBindFieldProperty(mPosition);
  ZilchBindFieldProperty(mDirection);
  ZilchBindFieldProperty(mLength);
  ZilchBindFieldProperty(mRadius);
}


ZilchDefineType(Cylinder, builder, type)
{
  DebugDrawObject<Cylinder>::SetupTypeHelper(builder, type);
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "start, end, radius", Vec3, Vec3, float);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, axis, height, radius", Vec3, Vec3, float, float);

  ZilchBindFieldProperty(mStart);
  ZilchBindFieldProperty(mEnd);
  ZilchBindFieldProperty(mRadius);
}


ZilchDefineType(Frustum, builder, type)
{
  DebugDrawObject<Frustum>::SetupTypeHelper(builder, type);
  
  ZilchBindConstructor();
  ZilchBindConstructor(Zero::Frustum);
  ZilchBindDestructor();
}


ZilchDefineType(Line, builder, type)
{
  DebugDrawObject<Line>::SetupTypeHelper(builder, type);
  
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "start, end", Vec3, Vec3);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "start, end, headSize", Vec3, Vec3, float);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "ray", Ray);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "segment", Segment);

  ZilchBindFieldProperty(mStart);
  ZilchBindFieldProperty(mEnd);
  ZilchBindFieldProperty(mHeadSize);

  ZilchBindGetterSetterProperty(DualHeads);
  ZilchBindGetterSetterProperty(BoxHeads);
}


ZilchDefineType(LineCross, builder, type)
{
  DebugDrawObject<LineCross>::SetupTypeHelper(builder, type);
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, halfExtents", Vec3, float);

  ZilchBindFieldProperty(mPosition);
  ZilchBindFieldProperty(mHalfExtents);
}


ZilchDefineType(Obb, builder, type)
{
  DebugDrawObject<Obb>::SetupTypeHelper(builder, type);
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, halfExtents", Vec3, Vec3);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, halfExtents", Vec3, float);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, halfExtents, rotation", Vec3, Vec3, Quat);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, halfExtents, rotation", Vec3, float, Quat);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, halfExtents, rotation", Vec3, Vec3, Mat3);
  //ZilchFullBindConstructor(builder, type, ZilchSelf, "obb", Obb);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "aabb", Aabb);

  ZilchBindFieldProperty(mPosition);
  ZilchBindFieldProperty(mHalfExtents);
  ZilchBindFieldProperty(mRotation);

  ZilchBindGetterSetterProperty(Corners);
}


ZilchDefineType(Sphere, builder, type)
{
  DebugDrawObject<Sphere>::SetupTypeHelper(builder, type);
  
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, radius", Vec3, float);
  ZilchFullBindConstructor(builder, type, ZilchSelf, "sphere", Sphere);

  ZilchBindFieldProperty(mPosition);
  ZilchBindFieldProperty(mRadius);

  ZilchBindGetterSetterProperty(Colored);
}


ZilchDefineType(Text, builder, type)
{
  DebugDrawObject<Text>::SetupTypeHelper(builder, type);
  
  
  ZilchBindDestructor();
  ZilchFullBindConstructor(builder, type, ZilchSelf, "position, textHeight, text", Vec3, float, String);

  ZilchBindFieldProperty(mPosition);
  ZilchBindFieldProperty(mRotation);
  ZilchBindFieldProperty(mTextHeight);
  ZilchBindFieldProperty(mText);

  ZilchBindGetterSetterProperty(Centered);
}


ZilchDefineType(Triangle, builder, type)
{
  DebugDrawObject<Triangle>::SetupTypeHelper(builder, type);
  
  
  ZilchBindDestructor();
  ZilchBindConstructor(Vec3, Vec3, Vec3);

  ZilchBindFieldProperty(mPoint0);
  ZilchBindFieldProperty(mPoint1);
  ZilchBindFieldProperty(mPoint2);
}

//----------------------------------------------------------- Debug Draw Helpers
//// Draw a circle clipped to given plane
//void DrawClippedCircle(DebugViewState& viewState, Vec3Param position, float radius, Vec3Param axis,
//                       float lineWidth, ByteColor frontColor, ByteColor backColor, Plane clipPlane)
//{
//  if (radius < cMinCircleRadius)
//    return;
//
//  // Generate a basis for drawing the circle
//  Vec3 u;
//  Vec3 v;
//  Math::GenerateOrthonormalBasis(axis, &v, &u);
//
//  // Compute number of segments is based on size on screen for auto LOD
//  float eyeDis = (position - viewState.EyePosition).Length();
//  float viewSize = ScreenSize(viewState, radius * 2.0f, eyeDis);
//  float numOfSegments = Math::Floor(viewSize / cSegmentSize);
//
//  // Clamp values
//  numOfSegments = Math::Clamp(numOfSegments, cSegmentMin, cSegmentMax);
//
//  // Radians moved per section
//  const float increment = (2.0f * Math::cPi) / numOfSegments;
//
//  viewState.Draw->LineState(viewState, lineWidth);
//
//  float theta = 0.0f;
//  float cosTheta = Math::Cos(theta);
//  float sinTheta = Math::Sin(theta);
//
//  // Draw line segments around the circle
//  Vec3 points[2];
//  points[0] = position + u * radius * cosTheta + v * radius * sinTheta;
//  for (int i = 0; i < numOfSegments; ++i)
//  {
//    theta += increment;
//    cosTheta = Math::Cos(theta);
//    sinTheta = Math::Sin(theta);
//
//    points[1] = position + u * radius * cosTheta + v * radius * sinTheta;
//
//    float c0 = clipPlane.SignedDistanceToPlane(points[0]);
//    float c1 = clipPlane.SignedDistanceToPlane(points[1]);
//
//    ByteColor color = frontColor;
//
//    // If back facing change to back color
//    if (c0 < 0 && c1 < 0)
//      color = backColor;
//
//    viewState.Draw->LineSegment(color, points[0], points[1]);
//
//    points[0] = points[1];
//  }
//}
//
//uint BoxIndices[] = { 0,2,1, 0,3,2,
//                      4,5,6, 4,6,7,
//                      1,6,5, 1,2,6,
//                      0,4,7, 0,7,3,
//                      7,6,2, 7,2,3,
//                      4,1,5, 4,0,1 };
//
//uint EdgeIndices[] = { 0,1, 0,3, 1,2, 3,2, 5,6, 6,7,
//                       7,4, 4,5, 1,5, 0,4, 3,7, 6,2 };
//
//uint Adjacent[] = { 0,5, 0,3, 0,2, 0,4, 2,1, 1,4,
//                    1,3, 1,5, 2,5, 5,3, 3,4, 2,4 };
//
//void DrawOrientedBoxVerts(DebugViewState& viewState, ByteColor color, float width, uint filled, uint backShade, Vec3* verts)
//{
//  bool faceVisible[] = {true, true, true, true, true, true};
//
//  // If back shading compute visibility for each face
//  if (backShade)
//  {
//    Vec3 transformedPoints[8];
//
//    for(uint i=0;i<8;++i)
//      transformedPoints[i] = Math::TransformPointProjectedCol(viewState.WorldViewProj, verts[i]);
//
//    for(uint f = 0; f < 6; ++f)
//    {
//      uint i = f * 6;
//      Vec3 points[3] = { transformedPoints[BoxIndices[i+0]],
//        transformedPoints[BoxIndices[i+1]],
//        transformedPoints[BoxIndices[i+2]]};
//
//      Vec3 normal = Cross(points[1] - points[0], points[2] - points[0]);
//      normal.AttemptNormalize();
//      faceVisible[f] =  Dot(Vec3(0,0,1), normal) > 0;
//    }
//  }
//
//  uint backColor = color;
//  SetAlphaByte(backColor, 32);
//
//  viewState.Draw->LineState(viewState, width);
//  for(uint c = 0; c < 24; c += 2)
//  {
//    bool visible = faceVisible[ Adjacent[c] ] || faceVisible[ Adjacent[c+1] ];
//    ByteColor lineColor = visible ? color : backColor;
//    viewState.Draw->LineSegment(lineColor, verts[EdgeIndices[c]], verts[EdgeIndices[c+1]]);
//  }
//}
//
//void DrawOrientedBox(DebugViewState& viewState, ByteColor color,
//                     Vec3Param position, Vec3 extents, Mat3Param basis,
//                     float width, uint onlyCorners, uint filled, uint backShade,
//                     uint onTop)
//{
//  Vec3 boxAxis[3] = { extents.x * basis.BasisX(),
//                      extents.y * basis.BasisY(),
//                      extents.z * basis.BasisZ() };
//
//  //     7----------6
//  //    /|         /|
//  //   / |        / |
//  //  3----------2  |
//  //  |  4 ------|--5
//  //  | /        | /
//  //  |/         |/
//  //  0----------1
//  Vec3 verts[8];
//  verts[0] = position - boxAxis[0] - boxAxis[1] - boxAxis[2];
//  verts[1] = position + boxAxis[0] - boxAxis[1] - boxAxis[2];
//  verts[2] = position + boxAxis[0] + boxAxis[1] - boxAxis[2];
//  verts[3] = position - boxAxis[0] + boxAxis[1] - boxAxis[2];
//  verts[4] = position - boxAxis[0] - boxAxis[1] + boxAxis[2];
//  verts[5] = position + boxAxis[0] - boxAxis[1] + boxAxis[2];
//  verts[6] = position + boxAxis[0] + boxAxis[1] + boxAxis[2];
//  verts[7] = position - boxAxis[0] + boxAxis[1] + boxAxis[2];
//
//  ByteColor lineColor = color;
//  if (filled)
//  {
//    lineColor = MultiplyByteColor(lineColor, 0.5f);
//    SetAlphaByte(lineColor, 255);
//  }
//
//  bool depthTest = !onTop;
//  bool culling = !backShade;
//  if (viewState.DepthTest != depthTest || viewState.Culling != culling)
//  {
//    viewState.DepthTest = depthTest;
//    viewState.Culling = culling;
//  }
//
//  viewState.Draw->LineState(viewState, 1.0f);
//
//  if (onlyCorners)
//  {
//    for(uint c = 0; c < 24; c += 2)
//    {
//      Vec3 p0 = verts[EdgeIndices[c]];
//      Vec3 p1 = verts[EdgeIndices[c+1]];
//
//      Vec3 lineSegment = p0 - p1;
//      float length = lineSegment.AttemptNormalize();
//      float edgeLength = length * 0.3f;
//
//      viewState.Draw->LineSegment(lineColor, p0, p0 - lineSegment * edgeLength);
//      viewState.Draw->LineSegment(lineColor, p1, p1 + lineSegment * edgeLength);
//    }
//  }
//  else
//  {
//    DrawOrientedBoxVerts(viewState, lineColor, width, filled, backShade, verts);
//  }
//
//  if (filled)
//  {
//    viewState.Draw->TriangleState(viewState);
//
//    // Front Face
//    viewState.Draw->AddTri(color, verts[2], verts[1], verts[0]);
//    viewState.Draw->AddTri(color, verts[3], verts[2], verts[0]);
//
//    // Back Face
//    viewState.Draw->AddTri(color, verts[7], verts[4], verts[5]);
//    viewState.Draw->AddTri(color, verts[6], verts[7], verts[5]);
//
//    // Top Face
//    viewState.Draw->AddTri(color, verts[6], verts[2], verts[3]);
//    viewState.Draw->AddTri(color, verts[7], verts[6], verts[3]);
//
//    // Bottom Face
//    viewState.Draw->AddTri(color, verts[1], verts[5], verts[4]);
//    viewState.Draw->AddTri(color, verts[0], verts[1], verts[4]);
//
//    // Right Face
//    viewState.Draw->AddTri(color, verts[6], verts[5], verts[1]);
//    viewState.Draw->AddTri(color, verts[2], verts[6], verts[1]);
//
//    // Left Face
//    viewState.Draw->AddTri(color, verts[3], verts[0], verts[4]);
//    viewState.Draw->AddTri(color, verts[7], verts[3], verts[4]);
//  }
//}
//
//------------------------------------------------------------ End Helpers
//
//void DrawDebugObject(DebugDrawer* drawer, DebugViewState& viewState, Sphere& sphere)
//{
//  viewState.DepthTest = !sphere.GetOnTop();
//  float distanceToEye = (sphere.mPosition - viewState.EyePosition).Length();
//
//  if (sphere.GetFilled())
//  {
//    ByteColor color = sphere.mColor;
//
//    uint primCount = viewState.GlobalDraw->SpherePrimitives;
//    Vec3* vertices = viewState.GlobalDraw->SpherePoints;
//    uint* indices = viewState.GlobalDraw->SphereIndices;
//
//    float radius = sphere.mRadius;
//    Vec3 pos = sphere.mPosition;
//
//    uint c = 0;
//    for(uint p = 0; p < primCount; ++p)
//    {
//      Vec3 points[3] = { vertices[indices[c + 0]] * radius + pos,
//                         vertices[indices[c + 1]] * radius + pos,
//                         vertices[indices[c + 2]] * radius + pos };
//      viewState.GlobalDraw->Add(Debug::Triangle(points[0], points[1],
//                                                points[2]).Color(color));
//      c += 3;
//    }
//  }
//
//  // Compute horizon so a silhouette can be draw
//  Vec3 eyeDir = viewState.EyeDirection;
//  Vec3 eyeToCenter = (sphere.mPosition - viewState.EyePosition);
//  HorizonCircle circle = ComputeHorizon(viewState, sphere.mPosition, sphere.mRadius, eyeToCenter);
//
//  // horizon clip plane
//  Plane clipPlane(-eyeToCenter, circle.Center);
//
//  if (sphere.GetColored())
//  {
//    // Draw each axis circle in axis color
//    for(uint i=0;i<3;++i)
//    {
//      ByteColor frontColor = AxisColors[i];
//      ByteColor backColor = frontColor;
//      SetAlphaByte(backColor, 32);
//
//      DrawClippedCircle(viewState, sphere.mPosition, sphere.mRadius, Vec3::Axes[i],
//        sphere.mWidth, frontColor, backColor, clipPlane);
//    }
//
//    // Draw border horizon circle
//    DrawCircle(viewState, circle.Center, circle.Radius, eyeToCenter, sphere.mWidth, Color::Gray);
//  }
//  else
//  {
//    ByteColor color = sphere.mColor;
//    ByteColor backColor = sphere.mColor;
//    SetAlphaByte(backColor, 32);
//
//    // Draw each axis circle
//    for(uint i=0;i<3;++i)
//    {
//      DrawClippedCircle(viewState, sphere.mPosition, sphere.mRadius, Vec3::Axes[i],
//                        sphere.mWidth, color, backColor, clipPlane);
//    }
//
//    // Draw border horizon circle
//    DrawCircle(viewState, circle.Center, circle.Radius, eyeToCenter, sphere.mWidth, color);
//  }
//}

//------------------------------------------------------------- Helper functions
void AddLine(DebugVertexArray& vertices, Vec4 color, Vec3 start, Vec3 end)
{
  vertices.PushBack(Vertex(start, color));
  vertices.PushBack(Vertex(end, color));
}

// expected ccw
void AddTriangle(DebugVertexArray& vertices, Vec4 color, Vec3 v[3])
{
  vertices.PushBack(Vertex(v[0], color));
  vertices.PushBack(Vertex(v[1], color));
  vertices.PushBack(Vertex(v[2], color));
}

// expected ccw
void AddTriangle(DebugVertexArray& vertices, Vec4 color, Vec3 v0, Vec3 v1, Vec3 v2)
{
  vertices.PushBack(Vertex(v0, color));
  vertices.PushBack(Vertex(v1, color));
  vertices.PushBack(Vertex(v2, color));
}

void AddQuad(DebugVertexArray& vertices, Vec4 color, Vec3 corners[], bool fill = false)
{
  if (fill)
  {
    //  1----------0
    //  |\         |
    //  |  \   t2  |
    //  |    \     |
    //  |  t1  \   |
    //  |        \ |
    //  2----------3

    AddTriangle(vertices, color, corners[2], corners[3], corners[1]);
    AddTriangle(vertices, color, corners[1], corners[3], corners[0]);
  }
  else
  {
    for(uint i = 0; i < 4; ++i)
      AddLine(vertices, color, corners[i], corners[(i + 1) % 4]);
  }

}

void AddBox(DebugVertexArray& vertices, Vec4 color, Vec3 center, Vec2 extents, Mat3 basis, bool fill = false)
{
  Vec3 extentsX = basis.BasisX() * extents.x;
  Vec3 extentsY = basis.BasisY() * extents.y;

  Vec3 corners[] =
  {
    center + extentsX + extentsY,
    center - extentsX + extentsY,
    center - extentsX - extentsY,
    center + extentsX - extentsY,
  };

  AddQuad(vertices, color, corners, fill);
}

void AddBox(DebugVertexArray& vertices, Vec4 color, Vec3 points[8], bool fill = false, bool cornersOnly = false)
{
  if (fill)
  {
    uint faces[] = { 1,3,2,0 , 0,2,6,4 , 5,4,6,7 , 1,5,7,3 , 4,5,1,0 , 2,3,7,6 };
    uint indexCount = sizeof(faces) / sizeof(uint);

    for(uint i = 0; i < indexCount; i += 4)
    {
      Vec3 quad[4] = { points[faces[i+0]], points[faces[i+1]], points[faces[i+2]], points[faces[i+3]] };
      //Vec3 quad[4] = { points[1], points[3], points[2], points[0] };
      AddQuad(vertices, color, quad, fill);
    }

    return;
  }

  uint edges[] = {0,1 , 0,2 , 0,4 , 3,1 , 3,2 , 3,7 , 5,1 , 5,4 , 5,7 , 6,2 , 6,4 , 6,7};
  uint indexCount = sizeof(edges) / sizeof(uint);

  if (cornersOnly)
  {
    for (uint i = 0; i < indexCount; i += 2)
    {
      Vec3 p0 = points[edges[i]];
      Vec3 p1 = points[edges[i + 1]];

      Vec3 edge = p0 - p1;
      float length = edge.AttemptNormalize();
      float edgeLength = length * 0.3f;

      AddLine(vertices, color, p0, p0 - edge * edgeLength);
      AddLine(vertices, color, p1, p1 + edge * edgeLength);
    }
  }
  else
  {
    for (uint i = 0; i < indexCount; i += 2)
      AddLine(vertices, color, points[edges[i]], points[edges[i + 1]]);
  }
}

void AddBox(DebugVertexArray& vertices, Vec4 color, Vec3 center, Vec3 extents, Mat3 basis, bool fill = false, bool cornersOnly = false)
{
  Vec3 extentsX = basis.BasisX() * extents.x;
  Vec3 extentsY = basis.BasisY() * extents.y;
  Vec3 extentsZ = basis.BasisZ() * extents.z;

  //     5----------4
  //    /|         /|
  //   / |        / |
  //  1----------0  |
  //  |  7 ------|--6
  //  | /        | /
  //  |/         |/
  //  3----------2
  Vec3 corners[] =
  {
    center + extentsX + extentsY + extentsZ,
    center - extentsX + extentsY + extentsZ,
    center + extentsX - extentsY + extentsZ,
    center - extentsX - extentsY + extentsZ,
    center + extentsX + extentsY - extentsZ,
    center - extentsX + extentsY - extentsZ,
    center + extentsX - extentsY - extentsZ,
    center - extentsX - extentsY - extentsZ,
  };

  AddBox(vertices, color, corners, fill, cornersOnly);
}

void AddArc(DebugVertexArray& vertices, Vec4 color, Vec3 center, float radius, Vec3 axis, Vec3 startDir, Vec3 endDir)
{
  float angle = Math::SignedAngle(startDir, endDir, axis);
  if (angle < 0.0f || angle + 0.001f >= Math::cPi)
  {
    Vec3 midDir = Math::RotateVector(startDir, axis, Math::cPi * 0.5f);
    AddArc(vertices, color, center, radius, axis, startDir, midDir);
    AddArc(vertices, color, center, radius, axis, midDir, endDir);
    return;
  }

  if (radius < cMinCircleRadius)
    return;

  float numOfSegments = Math::Floor(radius * 2.0f / cSegmentSize);
  numOfSegments = Math::Clamp(numOfSegments, cSegmentMin, cSegmentMax);

  Vec3 prevPoint = center + startDir * radius;
  for (uint i = 1; i <= (uint)numOfSegments; ++i)
  {
    Vec3 dir = Math::Slerp(startDir, endDir, i / numOfSegments);
    Vec3 point = center + dir * radius;
    AddLine(vertices, color, prevPoint, point);
    prevPoint = point;
  }
}

void AddArc(DebugVertexArray& vertices, Vec4 color, Vec3 start, Vec3 mid, Vec3 end)
{
  Vec3 secant1 = mid - start;
  Vec3 secant2 = end - mid;

  // want interpolation from start to end to be CCW about this axis
  Vec3 axis = Math::Cross(secant1, secant2);
  axis.AttemptNormalize();
  // either colinear or start and end are the same
  if (axis.Length() <= 0.001f)
    return;

  Vec3 bisect1 = start + secant1 * 0.5f;
  Vec3 bisect2 = mid + secant2 * 0.5f;
  Vec3 bisectDir1 = Math::Cross(secant1, axis).Normalized();
  Vec3 bisectDir2 = Math::Cross(secant2, axis).Normalized();

  Vec3 pointA, pointB;
  Intersection::IntersectionType result;
  result = Intersection::ClosestPointsOfTwoLines(bisect1, bisectDir1, bisect2, bisectDir2, &pointA, &pointB);
  // should never happen unless points are very close or very far
  if (result == Intersection::None)
    return;

  Vec3 center = pointA;
  float radius = (start - center).Length();

  Vec3 startDir = (start - center).Normalized();
  Vec3 midDir = (mid - center).Normalized();
  Vec3 endDir = (end - center).Normalized();

  AddArc(vertices, color, center, radius, axis, startDir, midDir);
  AddArc(vertices, color, center, radius, axis, midDir, endDir);
}

void AddCircle(DebugVertexArray& vertices, Vec4 color, Vec3 center, float radius, Vec3 axis, bool fill = false)
{
  if (radius < cMinCircleRadius)
    return;

  if (axis.Length() < Math::Epsilon())
    return;

  float numOfSegments = Math::Floor(radius * 2.0f / cSegmentSize);
  numOfSegments = Math::Clamp(numOfSegments, cSegmentMin, cSegmentMax);

  Vec3 right, up;
  Math::GenerateOrthonormalBasis(axis, &right, &up);
  right *= radius;
  up *= radius;

  const float increment = Math::cTwoPi / numOfSegments;
  float theta = 0.0f;
  float cosTheta = Math::Cos(theta);
  float sinTheta = Math::Sin(theta);
  Vec3 prevPoint = center + right * cosTheta + up * sinTheta;

  for (uint i = 0; i < numOfSegments; ++i)
  {
    theta += increment;
    cosTheta = Math::Cos(theta);
    sinTheta = Math::Sin(theta);
    Vec3 point = center + right * cosTheta + up * sinTheta;

    if (fill)
      AddTriangle(vertices, color, center, point, prevPoint);
    else
      AddLine(vertices, color, prevPoint, point);

    prevPoint = point;
  }
}

void AddHorizonCircle(DebugVertexArray& vertices, Vec4 color, Vec3 center, float radius, Vec3 eyeToCenter)
{
  float d = eyeToCenter.AttemptNormalize();

  if (d < radius)
  {
    AddCircle(vertices, color, center, radius, eyeToCenter);
  }
  else
  {
    float r = radius;
    float l = Math::Sqrt(d * d - r * r);
    float h = l * r / d;
    float z = Math::Sqrt(r * r - h * h);

    Vec3 horizonCenter = center - eyeToCenter * z;
    float horizonRadius = h;

    AddCircle(vertices, color, horizonCenter, horizonRadius, eyeToCenter);
  }
}

void AddArrowHeadFill(DebugVertexArray& vertices, Vec4 color, Vec3 tip, Vec3 bottom, Vec3 right, Vec3 up, float radius)
{
  if (radius < cMinCircleRadius)
    return;

  float numOfSegments = Math::Floor(radius * 2.0f / cSegmentSize);
  numOfSegments = Math::Clamp(numOfSegments, cSegmentMin, cSegmentMax);

  const float increment = Math::cTwoPi / numOfSegments;
  float theta = 0.0f;
  float cosTheta = Math::Cos(theta);
  float sinTheta = Math::Sin(theta);
  Vec3 prevPoint = bottom + right * cosTheta + up * sinTheta;

  for(uint i = 0; i < numOfSegments; ++i)
  {
    theta += increment;
    cosTheta = Math::Cos(theta);
    sinTheta = Math::Sin(theta);

    Vec3 point = bottom + right * cosTheta + up * sinTheta;
    AddTriangle(vertices, color, tip, point, prevPoint);

    prevPoint = point;
  }
}

void AddArrowHead(DebugVertexArray& vertices, Vec4 color, Vec3 tip, Vec3 direction, float diameter, bool fill, Vec3 eyePos)
{
  if (direction.Length() < Math::Epsilon())
    return;

  const float goldenRatio = 1.618f;
  float radius = diameter * 0.5f;
  float coneLength = goldenRatio * diameter;
  Vec3 bottom = tip - direction * coneLength; 

  Vec3 right, up;
  Math::GenerateOrthonormalBasis(direction, &right, &up);
  right *= radius;
  up *= radius;

  if (fill)
  {
    AddArrowHeadFill(vertices, color, tip, bottom, right, up, radius);
    return;
  }

  AddLine(vertices, color, tip, bottom + right);
  AddLine(vertices, color, tip, bottom - right);
  AddLine(vertices, color, tip, bottom + up);
  AddLine(vertices, color, tip, bottom - up);

  AddCircle(vertices, color, bottom, radius, direction);
}

void AddCylinder(DebugVertexArray& vertices, Vec4 color, Vec3 start, Vec3 end, float radius, uint lineCount, bool capsule = false)
{
  Vec3 axis = end - start;
  axis.AttemptNormalize();
  if (axis.Length() < Math::Epsilon())
    return;

  AddCircle(vertices, color, start, radius, axis);
  AddCircle(vertices, color, end, radius, axis);
  AddCircle(vertices, color, (start + end) * 0.5f, radius, axis);

  Vec3 u, v;
  Math::GenerateOrthonormalBasis(axis, &u, &v);
  u *= radius;
  v *= radius;

  for (uint i = 0; i < lineCount / 2; ++i)
  {
    float radians = Math::cTwoPi * (i / (float)lineCount);
    Vec3 dir = u * Math::Cos(radians) + v * Math::Sin(radians);
    AddLine(vertices, color, start + dir, end + dir);
    AddLine(vertices, color, start - dir, end - dir);
  }

  // Capsule ends
  if (capsule)
  {
    for (uint i = 0; i < lineCount / 2; ++i)
    {
      float radians = Math::cTwoPi * (i / (float)lineCount);
      Vec3 dir = u * Math::Cos(radians) + v * Math::Sin(radians);
      AddArc(vertices, color, start + dir, start - axis * radius, start - dir);
      AddArc(vertices, color, end + dir, end + axis * radius, end - dir);
    }
  }
}

//-------------------------------------------------------- GetVertices functions
void Arc::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  Vec3 origin = (mStart + 0.5f * (mEnd-mStart) + mViewScaleOffset);

  float viewScale = 1.0f;
  if (GetViewScaled())
    viewScale = GetViewScale(origin, viewData);

  Vec3 start = origin + (mStart - origin) * viewScale;
  Vec3 mid = origin + (mMid - origin) * viewScale;
  Vec3 end = origin + (mEnd - origin) * viewScale;

  AddArc(vertices, mColor, start, mid, end);
}

void Box::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  if (!GetFilled() && mWidth > 1.0f)
    SetBorder(true);

  Vec3 origin = (mPosition + mViewScaleOffset);

  float viewScale = 1.0f;
  if (GetViewScaled())
    viewScale = GetViewScale(origin, viewData);

  Vec2 extents = mHalfExtents * viewScale;
  Vec3 position = origin + (mPosition - origin) * viewScale;

  Mat3 basis;
  if (GetViewAligned())
  {
    if (viewData.mOrthographic)
      basis = Math::ToMatrix3(viewData.mEyeDirection.AttemptNormalized(), viewData.mEyeUp);
    else
      basis = Math::ToMatrix3((position - viewData.mEyePosition).AttemptNormalized(), viewData.mEyeUp);
  }
  else
  {
    basis = Math::ToMatrix3(mRotation);
  }

  AddBox(vertices, mColor, position, extents, basis, GetFilled());
}

void Capsule::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  if (!GetFilled() && mWidth > 1.0f)
    SetBorder(true);

  Vec3 position = mStart + 0.5f * (mEnd - mStart);
  Vec3 origin = (position + mViewScaleOffset);

  float viewScale = 1.0f;
  if (GetViewScaled())
  {
    viewScale = GetViewScale(origin, viewData);

    origin = (mStart + mViewScaleOffset);
    Vec3 start = origin + (mStart - origin) * viewScale;

    origin = (mEnd + mViewScaleOffset);
    Vec3 end = origin + (mEnd - origin) * viewScale;

    float radius = viewScale * mRadius;
    AddCylinder(vertices, mColor, start, end, radius, 8, true);

    return;
  }

    // else no view scaling
  AddCylinder(vertices, mColor, mStart, mEnd, mRadius, 8, true);
}

void Circle::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  if (!GetFilled() && mWidth > 1.0f)
    SetBorder(true);

  Vec3 origin = (mPosition + mViewScaleOffset);

  float viewScale = 1.0f;
  if (GetViewScaled())
    viewScale = GetViewScale(origin, viewData);

  float radius = mRadius * viewScale;
  Vec3 position = origin + (mPosition - origin) * viewScale;

  if (GetViewAligned())
    if (viewData.mOrthographic)
      AddCircle(vertices, mColor, position, radius, viewData.mEyeDirection);
    else
      AddHorizonCircle(vertices, mColor, position, radius, position - viewData.mEyePosition);
  else
    AddCircle(vertices, mColor, position, radius, mAxis);
}

void Cone::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  if (mDirection.Length() < Math::Epsilon())
    return;

  if (!GetFilled() && mWidth > 1.0f)
    SetBorder(true);

  Vec3 origin = (mPosition + mViewScaleOffset);

  float viewScale = 1.0f;
  if (GetViewScaled())
    viewScale = GetViewScale(origin, viewData);

  float radius = mRadius * viewScale;

  Vec3 position = origin + (mPosition - origin) * viewScale;
  Vec3 endPoint = position + mDirection * mLength * viewScale;

  AddCircle(vertices, mColor, endPoint, radius, mDirection);

  Vec3 u;
  Vec3 v;
  GenerateOrthonormalBasis(mDirection, &u, &v);

  Vec3 endPoint1 = endPoint + (u * radius);
  Vec3 endPoint2 = endPoint - (u * radius);
  Vec3 endPoint3 = endPoint + (v * radius);
  Vec3 endPoint4 = endPoint - (v * radius);

  AddLine(vertices, mColor, position, endPoint1);
  AddLine(vertices, mColor, position, endPoint2);
  AddLine(vertices, mColor, position, endPoint3);
  AddLine(vertices, mColor, position, endPoint4);
}

void Cylinder::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  if (!GetFilled() && mWidth > 1.0f)
    SetBorder(true);

  Vec3 position = mStart + 0.5f * (mEnd - mStart);
  Vec3 origin = (position + mViewScaleOffset);

  float viewScale = 1.0f;
  if (GetViewScaled())
  {
    viewScale = GetViewScale(origin, viewData);

    origin = (mStart + mViewScaleOffset);
    Vec3 start = origin + (mStart - origin) * viewScale;

    origin = (mEnd + mViewScaleOffset);
    Vec3 end = origin + (mEnd - origin) * viewScale;

    float radius = viewScale * mRadius;
    AddCylinder(vertices, mColor, start, end, radius, 8);

    return;
  }

    // else no view scaling
  AddCylinder(vertices, mColor, mStart, mEnd, mRadius, 8);
}

void Frustum::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  if (!GetFilled() && mWidth > 1.0f)
    SetBorder(true);

  float viewScale = 1.0f;
  if (GetViewScaled())
  {
      // doesn't matter which frustum point is used, the view scale will come out the same
    viewScale = GetViewScale(mPoints[0], viewData);

    Vec3 points[8];
    for(int i = 0; i < 8; ++i)
    {
      Vec3 origin = (mPoints[i] + mViewScaleOffset);
      points[i] = origin + (mPoints[i] - origin) * viewScale;
    }

    AddBox(vertices, mColor, points);
    return;
  }

  AddBox(vertices, mColor, mPoints);
}

void Line::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  if (!GetFilled() && mWidth > 1.0f)
    SetBorder(true);

  float viewScale = 1.0f;
  if (GetViewScaled())
    viewScale = GetViewScale(mStart, viewData);

  Vec3 start = mStart;
  Vec3 end = mEnd;
  Vec3 direction = end - start;

  float headSize = mHeadSize * viewScale;
  float length = direction.AttemptNormalize() * viewScale;
  end = start + direction * length;

  if (!GetFilled())
    AddLine(vertices, mColor, start, end);

  if (mHeadSize < 0.0001f || length <= 0.0f)
    return;

  if (GetBoxHeads())
  {
    Vec3 center = end - direction * headSize;
    Vec3 extents = Vec3(headSize, headSize, headSize);
    AddBox(vertices, mColor, center, extents, Mat3::cIdentity, GetFilled());
  }
  else
  {
    AddArrowHead(vertices, mColor, end, direction, headSize, GetFilled(), viewData.mEyePosition);
    if (GetDualHeads())
      AddArrowHead(vertices, mColor, start, -direction, headSize, GetFilled(), viewData.mEyePosition);
  }
}

void LineCross::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  if (!GetFilled() && mWidth > 1.0f)
    SetBorder(true);

  float viewScale = 1.0f;
  if (GetViewScaled())
    viewScale = GetViewScale(mPosition, viewData);

  float axisSize = mHalfExtents * viewScale;
  for (uint i = 0; i < 3; ++i)
    AddLine(vertices, ToFloatColor(AxisColors[i]), mPosition - Vec3::Axes[i] * axisSize, mPosition + Vec3::Axes[i] * axisSize);
}

void Obb::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  if (!GetFilled() && mWidth > 1.0f)
    SetBorder(true);

  Vec3 origin = (mPosition + mViewScaleOffset);

  float viewScale = 1.0f;
  if (GetViewScaled())
    viewScale = GetViewScale(origin, viewData);

  Vec3 extents = mHalfExtents * viewScale;
  Vec3 position = origin + -mViewScaleOffset * viewScale;

  Mat3 basis;
  if (GetViewAligned())
  {
    if (viewData.mOrthographic)
      basis = Math::ToMatrix3(viewData.mEyeDirection.AttemptNormalized(), viewData.mEyeUp);
    else
      basis = Math::ToMatrix3((position - viewData.mEyePosition).AttemptNormalized(), viewData.mEyeUp);
  }
  else
  {
    basis = Math::ToMatrix3(mRotation);
  }

  AddBox(vertices, mColor, position, extents, basis, GetFilled(), GetCorners());
}

void Sphere::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  if (!GetFilled() && mWidth > 1.0f)
    SetBorder(true);

  Vec3 origin = (mPosition + mViewScaleOffset);

  float viewScale = 1.0f;
  if (GetViewScaled())
    viewScale = GetViewScale(origin, viewData);

  float radius = mRadius * viewScale;
  Vec3 position = origin + (mPosition - origin) * viewScale;

  if (GetColored())
  {
    for (uint i = 0; i < 3; ++i)
      AddCircle(vertices, ToFloatColor(AxisColors[i]), position, radius, Vec3::Axes[i]);
    if (viewData.mOrthographic)
      AddCircle(vertices, ToFloatColor(Color::Gray), position, radius, viewData.mEyeDirection);
    else
      AddHorizonCircle(vertices, ToFloatColor(Color::Gray), position, radius, position - viewData.mEyePosition);
  }
  else
  {
    for (uint i = 0; i < 3; ++i)
      AddCircle(vertices, mColor, position, radius, Vec3::Axes[i]);
    if (viewData.mOrthographic)
      AddCircle(vertices, mColor, position, radius, viewData.mEyeDirection);
    else
      AddHorizonCircle(vertices, mColor, position, radius, position - viewData.mEyePosition);
  }
}

void Triangle::GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices)
{
  AddLine(vertices, mColor, mPoint0, mPoint1);
  AddLine(vertices, mColor, mPoint1, mPoint2);
  AddLine(vertices, mColor, mPoint2, mPoint0);
}

//-------------------------------------------------------------------- DebugDraw
ActiveDrawSpace::ActiveDrawSpace(uint spaceId)
{
  gDebugDraw->mSpaceIdStack.PushBack(spaceId);
}

ActiveDrawSpace::~ActiveDrawSpace()
{
  gDebugDraw->mSpaceIdStack.PopBack();
}

ActiveDebugConfig::ActiveDebugConfig()
{
  gDebugDraw->mDebugConfigStack.PushBack(this);
}

ActiveDebugConfig::~ActiveDebugConfig()
{
  gDebugDraw->mDebugConfigStack.PopBack();
}

DebugDrawObjectBase* GetDebugConfig()
{
  return gDebugDraw->mDebugConfigStack.Back();
}

void DebugDraw::Initialize()
{
  gDebugDraw = new DebugDraw();
}

void DebugDraw::Shutdown()
{
  delete gDebugDraw;
}

DebugDraw::DebugDraw()
{
  mDebugConfigStack.PushBack(&mDefaultConfig);
}

DebugDrawObjectArray::range DebugDraw::GetDebugObjects(uint spaceId)
{
  if (mDebugObjects.ContainsKey(spaceId))
    return mDebugObjects[spaceId].All();
  else
    return DebugDrawObjectArray::range();
}

void DebugDraw::ClearObjects()
{
  mDebugObjects.Clear();
}

} // namespace Debug

} // namespace Zero
