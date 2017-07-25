///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField5(JointCreatorFlags, UseCenter, OverrideLength, AutoSnaps, AttachToCommonParent, AttachToWorld);

/// A helper class to create joints of various configurations. Each joint is configured from
/// two points. Any other specific joint properties are calculated from these two points.
struct JointCreator
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  struct ConnectionInfo
  {
    ConnectionInfo(){}
    ConnectionInfo(Cog* obj0, Cog* obj1, bool worldConnect);

    void SetLocalPoints(Vec3Param localPoint0, Vec3Param localPoint1);
    void SetWorldPoint(Vec3Param worldPoint);
    void SetWorldPoints(Vec3Param worldPoint0, Vec3Param worldPoint1);

    Cog* a,* b;
    Vec3 mBodyRs[2];
    Vec3 mWorldPoints[2];
    Mat4 mTransforms[2];
    float mLength;
    bool mAttachToWorld;
  };

  JointCreator();

  /// Create a joint (by component name) attached to the two given cogs.
  Cog* Create(Cog* objectA, Cog* objectB, StringParam jointName);
  /// Create a joint (by component name) attached to the two given cogs.
  /// Both world points on the joint are set to the same world point value.
  Cog* CreateWorldPoints(Cog* objectA, Cog* objectB, StringParam jointName, Vec3Param bothWorldPoints);
  /// Create a joint (by component name) attached to the two given cogs.
  Cog* CreateWorldPoints(Cog* objectA, Cog* objectB, StringParam jointName, Vec3Param worldPointA, Vec3Param worldPointB);
  /// Create a joint (by component name) attached to the two given cogs.
  Cog* CreateLocalPoints(Cog* objectA, Cog* objectB, StringParam jointName, Vec3Param localPointA, Vec3Param localPointB);

  /// Add a JointLimit to the given joint cog.
  JointLimit* AddJointLimit(Cog* joint);
  /// Add a JointMotor to the given joint cog.
  JointMotor* AddJointMotor(Cog* joint);
  /// Add a JointSpring to the given joint cog.
  JointSpring* AddJointSpring(Cog* joint);

  /// Should the length of the joint be overridden or computed
  /// from the two points? Mainly used for StickJoint.
  bool GetOverrideLength() const;
  void SetOverrideLength(bool overrideLength);
  /// Should the center of each object be used instead of the given points?
  bool GetUseCenter() const;
  void SetUseCenter(bool useCenter);
  /// Should the joint auto-snap when the force limit is reached?
  bool GetAutoSnaps() const;
  void SetAutoSnaps(bool autoSnaps);
  /// Used to create a connection to a dummy object. Instead of connecting to object B,
  /// the connection will be between object A and "the world".
  bool GetAttachToWorld() const;
  void SetAttachToWorld(bool attachToWorld);
  /// Should the cog of the joint be added as a child of the common parent of the two given cogs?
  /// Useful for putting the joint in the same hierarchy so that archetypes can be created.
  bool GetAttachToCommonParent() const;
  void SetAttachToCommonParent(bool attachToCommonParent);

private:
  Cog* AttachInternal(ConnectionInfo& info, StringParam jointName);
  Cog* FindCommonParent(Cog* cogA, Cog* cogB);
  
  bool ObjectsValid(Cog* a, Cog* b, StringParam jointName);
  void ConfigureInfo(ConnectionInfo& info);
  static Cog* CreateJoint(StringParam fileName, ConnectionInfo& info);
  void SetBasicProperties(Joint* joint);
  template <typename JointType>
  void CallJointFunctions(Joint* joint, ConnectionInfo& info);
  void CallJointFunctions(Joint* joint, ConnectionInfo& info);

  void SetPointsAtLength(ConnectionInfo& info);
  void FixPoints(Joint* joint, ConnectionInfo& info);
  void SpecificSetup(Joint* joint, ConnectionInfo& info) {};

  void FixPoints(StickJoint* joint, ConnectionInfo& info);

  void SpecificSetup(StickJoint* joint, ConnectionInfo& info);
  void SpecificSetup(PositionJoint* joint, ConnectionInfo& info);
  void SpecificSetup(ManipulatorJoint* joint, ConnectionInfo& info);
  void SpecificSetup(RevoluteJoint* joint, ConnectionInfo& info);
  void SpecificSetup(PrismaticJoint* joint, ConnectionInfo& info);
  void SpecificSetup(WheelJoint* joint, ConnectionInfo& info);
  void SpecificSetup(RevoluteJoint2d* joint, ConnectionInfo& info);
  void SpecificSetup(PrismaticJoint2d* joint, ConnectionInfo& info);
  void SpecificSetup(PhyGunJoint* joint, ConnectionInfo& info);
  void SpecificSetup(WheelJoint2d* joint, ConnectionInfo& info);
  void SpecificSetup(WeldJoint* joint, ConnectionInfo& info);

public:

  BitField<JointCreatorFlags::Enum> mFlags;
  float mMaxImpulse;
  float mLength;
  Vec3 mAxis;
};

template <typename JointType>
void JointCreator::CallJointFunctions(Joint* joint, ConnectionInfo& info)
{
  JointType* typedJoint = static_cast<JointType*>(joint);
  FixPoints(typedJoint, info);
  SpecificSetup(typedJoint, info);
}

}//namespace Zero
