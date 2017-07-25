///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#define DeclareAnchorAccessors(ConstraintType, anchor)                                   \
  /* The local point of the anchor on object A.*/                                        \
  Vec3 GetLocalPointA() const;                                                           \
  void SetLocalPointA(Vec3Param localPoint);                                             \
  /* The local point of the anchor on object B.*/                                        \
  Vec3 GetLocalPointB() const;                                                           \
  void SetLocalPointB(Vec3Param localPoint);                                             \
  /* The position of the anchor on object A given a position in world space*/            \
  Vec3 GetWorldPointA();                                                                 \
  void SetWorldPointA(Vec3Param worldPoint);                                             \
  /* The position of the anchor on object B given a position in world space*/            \
  Vec3 GetWorldPointB();                                                                 \
  void SetWorldPointB(Vec3Param worldPoint);                                             \
  /* Sets the position of the anchor on object A and B given a position in world space*/ \
  void SetWorldPoints(Vec3Param point);                                                  \
  /* Virtual function for when an object link point changes*/                            \
  void ObjectLinkPointUpdated(size_t edgeIndex, Vec3Param localPoint) override;

#define ImplementAnchorAccessors(ConstraintType, anchor)                              \
  Vec3 ConstraintType::GetLocalPointA() const                                         \
  {                                                                                   \
    return GetLocalPointHelper(anchor, 0);                                            \
  }                                                                                   \
  void ConstraintType::SetLocalPointA(Vec3Param bodyPoint)                            \
  {                                                                                   \
    SetLocalPointHelper(anchor, 0, bodyPoint);                                        \
  }                                                                                   \
  Vec3 ConstraintType::GetLocalPointB() const                                         \
  {                                                                                   \
    return GetLocalPointHelper(anchor, 1);                                            \
  }                                                                                   \
  void ConstraintType::SetLocalPointB(Vec3Param bodyPoint)                            \
  {                                                                                   \
    SetLocalPointHelper(anchor, 1, bodyPoint);                                        \
  }                                                                                   \
  Vec3 ConstraintType::GetWorldPointA()                                               \
  {                                                                                   \
    return GetWorldPointHelper(anchor, 0);                                            \
  }                                                                                   \
  void ConstraintType::SetWorldPointA(Vec3Param worldPoint)                           \
  {                                                                                   \
    this->SetWorldPointAHelper(anchor, worldPoint);                                   \
  }                                                                                   \
  Vec3 ConstraintType::GetWorldPointB()                                               \
  {                                                                                   \
    return GetWorldPointHelper(anchor, 1);                                            \
  }                                                                                   \
  void ConstraintType::SetWorldPointB(Vec3Param worldPoint)                           \
  {                                                                                   \
    this->SetWorldPointBHelper(anchor, worldPoint);                                   \
  }                                                                                   \
  void ConstraintType::SetWorldPoints(Vec3Param point)                                \
  {                                                                                   \
    SetWorldPointsHelper(anchor, point);                                              \
  }                                                                                   \
  void ConstraintType::ObjectLinkPointUpdated(size_t edgeIndex, Vec3Param localPoint) \
  {                                                                                   \
    ObjectLinkPointUpdatedHelper(anchor, edgeIndex, localPoint);                      \
  }

#define BindAnchorAccessors(defaults)                       \
  ZilchBindGetterSetterProperty(LocalPointA)->ZeroSerialize(defaults);      \
  ZilchBindGetterSetterProperty(LocalPointB)->ZeroSerialize(defaults);      \
  ZilchBindMethod(SetWorldPoints);                                          \
  ZilchBindGetterSetterProperty(WorldPointA);               \
  ZilchBindGetterSetterProperty(WorldPointB);               \
  ZilchBindMethod(SetWorldPoints); 

#define DeclareAxisAccessors(ConstraintType, axes)          \
  /* The locally defined axis on object A.*/                \
  Vec3 GetLocalAxisA() const;                               \
  void SetLocalAxisA(Vec3Param axis);                       \
  /* The locally defined axis on object B.*/                \
  Vec3 GetLocalAxisB() const;                               \
  void SetLocalAxisB(Vec3Param axis);                       \
  /* The axis in world space that is being rotated about.*/ \
  Vec3 GetWorldAxis() const;                                \
  void SetWorldAxis(Vec3Param axis);

#define ImplementAxisAccessors(ConstraintType, axes)            \
  Vec3 ConstraintType::GetLocalAxisA() const                    \
  {                                                             \
    return GetLocalAxisHelper(axes, 0);                         \
  }                                                             \
  void ConstraintType::SetLocalAxisA(Vec3Param axis)            \
  {                                                             \
    SetLocalAxisHelper(axes, 0, axis);                          \
  }                                                             \
  Vec3 ConstraintType::GetLocalAxisB() const                    \
  {                                                             \
    return GetLocalAxisHelper(axes, 1);                         \
  }                                                             \
  void ConstraintType::SetLocalAxisB(Vec3Param axis)            \
  {                                                             \
    SetLocalAxisHelper(axes, 1, axis);                          \
  }                                                             \
  Vec3 ConstraintType::GetWorldAxis() const                     \
  {                                                             \
    return GetWorldAxisHelper(axes);                            \
  }                                                             \
  void ConstraintType::SetWorldAxis(Vec3Param axis)             \
  {                                                             \
    SetWorldAxisHelper(axes, axis);                             \
  }

#define BindAxisAccessors(defaults)                        \
  ZilchBindGetterSetterProperty(LocalAxisA)->ZeroSerialize(defaults);                \
  ZilchBindGetterSetterProperty(LocalAxisB)->ZeroSerialize(defaults);                \
  ZilchBindGetterSetterProperty(WorldAxis);

#define DeclareAngleAccessors(ConstraintType, angleAtom)                   \
  /* The local space reference frame of object A. This frame is*/          \
  /* transformed to world space and then aligned with object B's frame. */ \
  Quat GetLocalBasisA() const;                                             \
  void SetLocalBasisA(QuatParam angle);                                    \
  /* The local space reference frame of object B. This frame is*/          \
  /* transformed to world space and then aligned with object A's frame. */ \
  Quat GetLocalBasisB() const;                                             \
  void SetLocalBasisB(QuatParam angle);

#define ImplementAngleAccessors(ConstraintType, angleAtom) \
  Quat ConstraintType::GetLocalBasisA() const              \
  {                                                        \
    return GetLocalAngleHelper(angleAtom, 0);              \
  }                                                        \
  void ConstraintType::SetLocalBasisA(QuatParam angle)     \
  {                                                        \
    SetLocalAngleHelper(angleAtom, 0, angle);              \
  }                                                        \
  Quat ConstraintType::GetLocalBasisB() const              \
  {                                                        \
    return GetLocalAngleHelper(angleAtom, 1);              \
  }                                                        \
  void ConstraintType::SetLocalBasisB(QuatParam angle)     \
  {                                                        \
    SetLocalAngleHelper(angleAtom, 1, angle);              \
  }

#define BindAngleAccessors()       \
  ZilchBindGetterSetterProperty(LocalBasisA); \
  ZilchBindGetterSetterProperty(LocalBasisB);
