///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField3(JointDebugDrawConfigFlags, ObjectAPerspective, ObjectBPerspective, Active);

/// Allows the user to override parameters for debug drawing of joints.
/// Primarily used to debug draw from different object perspectives and to
/// change the size of drawn data.
struct JointDebugDrawConfig : public Component
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  JointDebugDrawConfig();
  virtual ~JointDebugDrawConfig();

  // Component Interface
  void Serialize(Serializer& stream) override;

  /// Whether or not this component is active.
  bool GetActive() const;
  void SetActive(bool state);
  /// If we draw the joint's debug info from the perspective of ObjectA.
  /// Typically used when ObjectA is marked as a static object and ObjectB has free movement.
  bool GetObjectAPerspective() const;
  void SetObjectAPerspective(bool state);
  /// If we draw the joint's debug info from the perspective of ObjectB.
  /// Typically used when ObjectB is marked as a static object and ObjectA has free movement.
  bool GetObjectBPerspective() const;
  void SetObjectBPerspective(bool state);

  /// A size modifier for the debug drawing. 1 is the base size.
  real mSize;
  /// The amount of detail to use when drawing. For example, the detail on a
  /// RevoluteJoint will increase the number of lines used to draw the arc of a circle.
  real mDetail;
  BitField<JointDebugDrawConfigFlags::Enum> mPerspective;
};

}//namespace Zero
