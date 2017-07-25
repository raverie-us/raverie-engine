///////////////////////////////////////////////////////////////////////////////
///
/// \file DebugDraw.hpp
/// Declaration of the Debug Drawing classes and functions.
///
/// Authors: Chris Peters, Nathan Carlson
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#define CustomPropertySetter(type, name, propName) \
  ReturnType& propName(type var)                   \
  {                                                \
    m##name = var;                                 \
    return *(ReturnType*)this;                     \
  }

#define PropertySetter(type, name) \
  CustomPropertySetter(type, name, name)

#define PropertySetterBit(name, flagName)         \
  ReturnType& name(bool value)                    \
  {                                               \
    if (value)                                    \
      mDrawFlags |= DebugDrawFlags::flagName;     \
    else                                          \
      mDrawFlags &= ~DebugDrawFlags::flagName;    \
    return *(ReturnType*)this;                    \
  }                                               \
  void Set##name(bool value)                      \
  {                                               \
    if (value)                                    \
      mDrawFlags |= DebugDrawFlags::flagName;     \
    else                                          \
      mDrawFlags &= ~DebugDrawFlags::flagName;    \
  }                                               \
  bool Get##name() const                          \
  {                                               \
    return mDrawFlags & DebugDrawFlags::flagName; \
  } 

#define DebugObjectsArray(DebugObjectType) \
  PodArray<DebugObjectType> m##DebugObjectType##Objects;

#define DebugObjectAddMethods(DebugObjectType)                     \
  void Add(const DebugObjectType& object)                          \
  {                                                                \
    ErrorIf(mSpaceIdStack.Empty(), "No SpaceId on stack.");        \
    Add(mSpaceIdStack.Back(), object);                             \
  }                                                                \
  void Add(uint spaceId, const DebugObjectType& object)            \
  {                                                                \
    mDebugObjects[spaceId].PushBack(DebugDrawObjectAny(object));   \
  }

namespace Zero
{

namespace Debug
{

DeclareBitField8(DebugDrawFlags, BackShade, Border, Filled, OnTop, ViewAligned, ViewScaled, Special1, Special2);

DeclareEnum14(DebugType, None, Arc, Box, Capsule, Circle, Cone, Cylinder, Frustum, Line, LineCross, Obb, Sphere, Text, Triangle);

class Vertex
{
public:
  Vertex() {}
  Vertex(Vec3 position, Vec4 color) : mPosition(position), mColor(color) {}

  Vec3 mPosition;
  Vec4 mColor;
};

typedef Array<Vertex> DebugVertexArray;

class DebugViewData
{
public:
  Vec3 mEyePosition;
  Vec3 mEyeDirection;
  Vec3 mEyeUp;
  float mFieldOfView;
  float mOrthographicSize;
  bool mOrthographic;
};

const float cViewScale = 0.05f;

float GetViewScale(Vec3Param location, const DebugViewData& viewData);
float GetViewScale(float viewDistance, float fieldOfView, float orthographicSize, bool orthographic);
float GetViewDistance(Vec3Param location, Vec3Param eyePosition, Vec3 viewDirection);

class DebugDrawObjectBase
{
public:
  DebugDrawObjectBase()
    : mColor(Vec4(1.0f)), mViewScaleOffset(Vec3::cZero), mDuration(0.0f), mWidth(1.0f), mDrawFlags(0) {}
  virtual ~DebugDrawObjectBase() {}

  virtual DebugType::Enum GetDebugType() { return DebugType::None; }
  virtual void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) {}

  bool IsSet(DebugDrawFlags::Enum flag)
  {
    return mDrawFlags & flag;
  }

  /// ViewScaleOffset is used to calculate an origin about which view scaling occurs.
  /// If unused the origin will simply be the debug drawable's position.
  Vec3 mViewScaleOffset;

  Vec4 mColor;
  float mDuration;
  float mWidth;
  uint mDrawFlags;
};

DebugDrawObjectBase* GetDebugConfig();

//-------------------------------------------------------------- DebugDrawObject
template<typename DebugObjectType>
class DebugDrawObject : public DebugDrawObjectBase
{
public:
  typedef DebugObjectType ReturnType;
  typedef DebugDrawObject self_type;

  static void SetupTypeHelper(LibraryBuilder& builder, BoundType* type);

  DebugDrawObject()
  {
    *(DebugDrawObjectBase*)this = *GetDebugConfig();
  }

  // Color - The color the object will be drawn with.
  DebugObjectType& Color(Vec4 color)
  {
    mColor = color;
    return *(DebugObjectType*)this;
  }

  DebugObjectType& Color(ByteColor color)
  {
    mColor = ToFloatColor(color);
    return *(DebugObjectType*)this;
  }

  DebugObjectType& Alpha(uint a)
  {
    mColor.w = a / 255.0f;
    return *(DebugObjectType*)this;
  }

  DebugObjectType& ViewScaleOffset(Vec3 origin)
  {
    mViewScaleOffset = origin;
    return *(DebugObjectType*)this;
  }
  
  // Length of time the object will be visible, default is zero or one frame.
  PropertySetter(float, Duration);

  // Width of the lines used to draw the object.
  PropertySetter(float, Width);

  // Darken the back lines of the shape.
  PropertySetterBit(BackShade, BackShade);

  // Draws darkened lines on triangle borders when using Filled option.
  PropertySetterBit(Border, Border);

  // Object is drawn with a triangle mesh instead of wireframe.
  PropertySetterBit(Filled, Filled);

  // Does the object appear on top of everything or is it occluded by geometry.
  PropertySetterBit(OnTop, OnTop);

  // The object will be aligned to the view plane.
  PropertySetterBit(ViewAligned, ViewAligned);

  // The object will be the same size on screen no matter how far from the eye it is.
  PropertySetterBit(ViewScaled, ViewScaled);
};

//-------------------------------------------------------------------------- Arc
class Arc : public DebugDrawObject<Arc>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);

  DebugType::Enum GetDebugType() override { return DebugType::Arc; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Arc() {}

  Arc(Vec3Param start, Vec3Param mid, Vec3Param end)
    : mStart(start), mMid(mid), mEnd(end) {}

  Vec3 mStart;
  Vec3 mMid;
  Vec3 mEnd;

  PropertySetter(Vec3, Start);
  PropertySetter(Vec3, Mid);
  PropertySetter(Vec3, End);
};

//-------------------------------------------------------------------------- Box
class Box : public DebugDrawObject<Box>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Box; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Box() {}

  Box(Vec3Param position, Vec2Param halfExtents)
    : mPosition(position), mHalfExtents(halfExtents), mRotation(Quat::cIdentity) {}

  Box(Vec3Param position, float halfExtents)
    : mPosition(position), mHalfExtents(Vec2(1, 1) * halfExtents), mRotation(Quat::cIdentity) {}

  Box(Vec3Param position, Vec2Param halfExtents, QuatParam rotation)
    : mPosition(position), mHalfExtents(halfExtents), mRotation(rotation) {}

  Box(Vec3Param position, float halfExtents, QuatParam rotation)
    : mPosition(position), mHalfExtents(Vec2(1, 1) * halfExtents), mRotation(rotation) {}

  Box(Vec3Param position, Vec3Param halfExtents, QuatParam rotation = Quat::cIdentity)
    : mPosition(position), mHalfExtents(Math::ToVector2(halfExtents)), mRotation(rotation) {}

  Box(const Zero::Aabb& aabb)
    : mPosition(aabb.GetCenter()), mHalfExtents(Math::ToVector2(aabb.GetHalfExtents())), mRotation(Quat::cIdentity) {}

  Vec3 mPosition;
  Vec2 mHalfExtents;
  Quat mRotation;

  PropertySetter(Vec3, Position);
  PropertySetter(Vec2, HalfExtents);
  PropertySetter(Quat, Rotation);

  PropertySetterBit(Corners, Special1);
};

//---------------------------------------------------------------------- Capsule
class Capsule : public DebugDrawObject<Capsule>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Capsule; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Capsule() {}

  Capsule(Vec3Param start, Vec3Param end, float radius)
    : mStart(start), mEnd(end), mRadius(radius) {}

  Capsule(Vec3Param pos, Vec3Param axis, float height, float radius)
    : mStart(pos + axis * height), mEnd(pos - axis * height), mRadius(radius) {}

  Capsule(const Zero::Capsule& capsule)
    : mStart(capsule.PointA), mEnd(capsule.PointB), mRadius(capsule.Radius) {}

  Vec3 mStart;
  Vec3 mEnd;
  float mRadius;

  PropertySetter(Vec3, Start);
  PropertySetter(Vec3, End);
  PropertySetter(float, Radius);
};

//----------------------------------------------------------------------- Circle
class Circle : public DebugDrawObject<Circle>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Circle; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Circle() {}

  Circle(Vec3Param position, Vec3Param axis, float radius)
    : mPosition(position), mAxis(axis), mRadius(radius) {}

  Vec3 mPosition;
  Vec3 mAxis;
  float mRadius;

  PropertySetter(Vec3, Position);
  PropertySetter(Vec3, Axis);
  PropertySetter(float, Radius);
};

//------------------------------------------------------------------------- Cone
class Cone : public DebugDrawObject<Cone>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Cone; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Cone() {}

  Cone(Vec3Param position, Vec3Param direction, float length, float radius)
    : mPosition(position), mDirection(direction), mLength(length), mRadius(radius) {}

  Vec3 mPosition;
  Vec3 mDirection;
  float mLength;
  float mRadius;

  PropertySetter(Vec3, Position);
  PropertySetter(Vec3, Direction);
  PropertySetter(float, Length);
  PropertySetter(float, Radius);
};

//--------------------------------------------------------------------- Cylinder
class Cylinder : public DebugDrawObject<Cylinder>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Cylinder; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Cylinder() {}

  Cylinder(Vec3Param start, Vec3Param end, float radius)
    : mStart(start), mEnd(end), mRadius(radius) {}

  Cylinder(Vec3Param pos, Vec3Param axis, float height, float radius)
    : mStart(pos + axis * height), mEnd(pos - axis * height), mRadius(radius) {}

  Cylinder(const Zero::Cylinder& cylinder)
    : mStart(cylinder.PointA), mEnd(cylinder.PointB), mRadius(cylinder.Radius) {}

  Vec3 mStart;
  Vec3 mEnd;
  float mRadius;

  PropertySetter(Vec3, Start);
  PropertySetter(Vec3, End);
  PropertySetter(float, Radius);
};

//---------------------------------------------------------------------- Frustum
class Frustum : public DebugDrawObject<Frustum>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Frustum; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Frustum() {}

  Frustum(Vec3 points[8])
  {
    for(uint i = 0; i < 8; ++i)
      mPoints[i] = points[i];
    Math::Swap(mPoints[0], mPoints[1]);
    Math::Swap(mPoints[4], mPoints[5]);
  }

  Frustum(const Zero::Frustum& frustum)
  {
    frustum.GetPoints(mPoints);
    Math::Swap(mPoints[0], mPoints[1]);
    Math::Swap(mPoints[4], mPoints[5]);
  }

  Vec3 mPoints[8];
};

//------------------------------------------------------------------------- Line
class Line : public DebugDrawObject<Line>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Line; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Line() {}

  Line(Vec3Param start, Vec3Param end)
    : mStart(start), mEnd(end), mHeadSize(0.0f) {}

  Line(Vec3Param start, Vec3Param end, float headSize)
    : mStart(start), mEnd(end), mHeadSize(headSize) {}

  Line(const Zero::Ray& ray, float t = 1.0f)
    : mStart(ray.Start), mEnd(ray.Start + ray.Direction * t), mHeadSize(1.0f) {}

  Line(const Zero::Segment& segment)
    : mStart(segment.Start), mEnd(segment.End), mHeadSize(0.0f) {}

  Vec3 mStart;
  Vec3 mEnd;
  float mHeadSize;

  PropertySetter(Vec3, Start);
  PropertySetter(Vec3, End);
  PropertySetter(float, HeadSize);

  PropertySetterBit(DualHeads, Special1);
  PropertySetterBit(BoxHeads, Special2);
};

//------------------------------------------------------------------- Line Cross
class LineCross : public DebugDrawObject<LineCross>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::LineCross; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  LineCross() {}

  LineCross(Vec3Param position, float halfExtents)
    : mPosition(position), mHalfExtents(halfExtents) {}

  Vec3 mPosition;
  float mHalfExtents;

  PropertySetter(Vec3, Position);
  PropertySetter(float, HalfExtents);
};

//-------------------------------------------------------------------------- Obb
class Obb : public DebugDrawObject<Obb>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Obb; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Obb() {}

  Obb(Vec3Param position, Vec3Param halfExtents)
    : mPosition(position), mHalfExtents(halfExtents), mRotation(Quat::cIdentity) {}

  Obb(Vec3Param position, float halfExtents)
    : mPosition(position), mHalfExtents(Vec3(1, 1, 1) * halfExtents), mRotation(Quat::cIdentity) {}

  Obb(Vec3Param position, Vec3Param extents, QuatParam rotation)
    : mPosition(position), mHalfExtents(extents), mRotation(rotation) {}

  Obb(Vec3Param position, float halfExtents, QuatParam rotation)
    : mPosition(position), mHalfExtents(Vec3(1, 1, 1) * halfExtents), mRotation(rotation) {}

  Obb(Vec3Param position, Vec3Param extents, Mat3Param rotation)
    : mPosition(position), mHalfExtents(extents), mRotation(Math::ToQuaternion(rotation)) {}

  Obb(const Zero::Obb& obb)
    : mPosition(obb.Center), mHalfExtents(obb.HalfExtents), mRotation(Math::ToQuaternion(obb.Basis)) {}

  Obb(const Zero::Aabb& aabb)
    : mPosition(aabb.GetCenter()), mHalfExtents(aabb.GetHalfExtents()), mRotation(Quat::cIdentity) {}

  Vec3 mPosition;
  Vec3 mHalfExtents;
  Quat mRotation;

  PropertySetter(Vec3, Position);
  PropertySetter(Vec3, HalfExtents);
  PropertySetter(Quat, Rotation);

  PropertySetterBit(Corners, Special1);
};

//----------------------------------------------------------------------- Sphere
class Sphere : public DebugDrawObject<Sphere>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Sphere; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Sphere() {}

  Sphere(Vec3Param position, float radius)
    : mPosition(position), mRadius(radius) {}

  Sphere(const Zero::Sphere& sphere)
    : mPosition(sphere.mCenter), mRadius(sphere.mRadius) {}

  Vec3 mPosition;
  float mRadius;

  PropertySetter(Vec3, Position);
  PropertySetter(float, Radius);

  PropertySetterBit(Colored, Special1);
};

//------------------------------------------------------------------------- Text
class Text : public DebugDrawObject<Text>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Text; }

  Text() :
    mPosition(Vec3::cZero), mRotation(Quat::cIdentity), mTextHeight(1.0f) { }

  Text(Vec3Param position, float textHeight, StringParam text)
    : mPosition(position), mRotation(Quat::cIdentity), mTextHeight(textHeight), mText(text) {}

  Vec3 mPosition;
  Quat mRotation;
  float mTextHeight;
  String mText;
        
  PropertySetter(Vec3, Position);
  PropertySetter(Quat, Rotation);
  PropertySetter(float, TextHeight);
  CustomPropertySetter(String, Text, DisplayText);

  PropertySetterBit(Centered, Special1);
};

//--------------------------------------------------------------------- Triangle
class Triangle : public DebugDrawObject<Triangle>
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);
  

  DebugType::Enum GetDebugType() override { return DebugType::Triangle; }
  void GetVertices(const DebugViewData& viewData, DebugVertexArray& vertices) override;

  Triangle() {}

  Triangle(Vec3Param point0, Vec3Param point1, Vec3Param point2)
    : mPoint0(point0), mPoint1(point1), mPoint2(point2) {}

  Triangle(const Zero::Triangle& tri)
    : mPoint0(tri.p0), mPoint1(tri.p1), mPoint2(tri.p2) {}

  Vec3 mPoint0;
  Vec3 mPoint1;
  Vec3 mPoint2;

  PropertySetter(Vec3, Point0);
  PropertySetter(Vec3, Point1);
  PropertySetter(Vec3, Point2);
};

//-------------------------------------------------------------------- DebugDraw
class ActiveDrawSpace
{
public:
  ActiveDrawSpace(uint spaceId);
  ~ActiveDrawSpace();
private:
  ActiveDrawSpace(const ActiveDrawSpace&);
  ActiveDrawSpace& operator=(const ActiveDrawSpace&);
};

class ActiveDebugConfig : public DebugDrawObject<ActiveDebugConfig>
{
public:
  ActiveDebugConfig();
  ~ActiveDebugConfig();
private:
  ActiveDebugConfig(const ActiveDebugConfig&);
  ActiveDebugConfig& operator=(const ActiveDebugConfig&);
};

// Chaining ConstMax<sizeof(X), ... >::Result together for every debug type in DebugPrimitives.inl
// The 0 resolves the last trailing comma from ConstMax<sizeof(X),
// The ; ends the statement for cMaxDebugSize
static const uint cMaxDebugSize = 
#define ZeroDebugPrimitive(X) ConstMax<sizeof(X),
#include "DebugPrimitives.inl"
#undef ZeroDebugPrimitive
0
#define ZeroDebugPrimitive(X) >::Result
#include "DebugPrimitives.inl"
#undef ZeroDebugPrimitive
;

typedef VirtualAny<DebugDrawObjectBase, cMaxDebugSize> DebugDrawObjectAny;
typedef Array<DebugDrawObjectAny> DebugDrawObjectArray;

class DebugDraw
{
public:
  static void Initialize();
  static void Shutdown();

  DebugDraw();

  #define ZeroDebugPrimitive(X) DebugObjectAddMethods(X);
  #include "DebugPrimitives.inl"
  #undef ZeroDebugPrimitive

  DebugDrawObjectArray::range GetDebugObjects(uint spaceId);
  void ClearObjects();

  Array<uint> mSpaceIdStack;
  Array<DebugDrawObjectBase*> mDebugConfigStack;
  HashMap<uint, DebugDrawObjectArray> mDebugObjects;

  DebugDrawObjectBase mDefaultConfig;
};

} // namespace Debug

// Global debug drawer
extern Debug::DebugDraw* gDebugDraw;

} // namespace Zero

#undef CustomPropertySetter
#undef PropertySetter
#undef PropertySetterBit
#undef DebugObjectsArray
#undef DebugObjectAddMethods
