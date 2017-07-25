///////////////////////////////////////////////////////////////////////////////
///
/// \file JointTools.hpp
/// Declaration of the JointTools classes.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum16(JointToolTypes, StickJoint, PositionJoint, PrismaticJoint, WeldJoint, RevoluteJoint, GearJoint, WheelJoint,
  LinearAxisJoint, FixedAngleJoint, PulleyJoint, UniversalJoint, UprightJoint, PrismaticJoint2d, RevoluteJoint2d, WheelJoint2d, ObjectLink);

/// Tool for connecting two objects together with a joint.
class JointTool : public ObjectConnectingTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  JointTool();

  void OnMouseEndDrag(Event*) override;

  void DoConnection() override;

  cstr GetJointName();

#define DeclareInternalProperty(name, type) \
  type Get##name() const { return mJointCreator.m##name; };\
  void Set##name(type val) { mJointCreator.m##name = val; };

  DeclareInternalProperty(Length,float);
  DeclareInternalProperty(MaxImpulse,float);

#undef DeclareInternalProperty

#define DeclareInternalBitfield(name) \
  bool Get##name() const { return mJointCreator.mFlags.IsSet(JointCreatorFlags::name); };\
  void Set##name(bool val) { mJointCreator.mFlags.SetState(JointCreatorFlags::name,val); };

  DeclareInternalBitfield(OverrideLength);
  DeclareInternalBitfield(UseCenter);
  DeclareInternalBitfield(AutoSnaps);
  DeclareInternalBitfield(AttachToCommonParent);
  DeclareInternalBitfield(AttachToWorld);
#undef DeclareInternalBitfield

  JointCreator mJointCreator;

  JointToolTypes::Enum mJointType;
};

}//namespace Zero
