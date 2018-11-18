///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------BaseJointTool
ZilchDefineType(JointTool, builder, type)
{
  ZeroBindComponent();
  ZilchBindGetterSetterProperty(OverrideLength);
  ZilchBindGetterSetterProperty(Length);
  ZilchBindGetterSetterProperty(MaxImpulse);
  ZilchBindGetterSetterProperty(UseCenter);
  ZilchBindGetterSetterProperty(AutoSnaps);
  ZilchBindGetterSetterProperty(AttachToWorld);
  ZilchBindGetterSetterProperty(AttachToCommonParent);
  ZilchBindFieldProperty(mJointType);
}

JointTool::JointTool()
{
  mJointType = JointToolTypes::StickJoint;
}

void JointTool::OnMouseEndDrag(Event* e)
{
  if(GetAttachToWorld())
    ObjectB = ObjectA;

  ObjectConnectingTool::OnMouseEndDrag(e);
}

void JointTool::DoConnection()
{
  if(Cog* a = ObjectA)
  {
    if(Cog* b = ObjectB)
    {
      Transform* t0 = a->has(Transform);
      Transform* t1 = b->has(Transform);
      if(t0 == nullptr || t1 == nullptr)
        return;

      Cog* jointCog = mJointCreator.CreateWorldPoints(a, b, GetJointName(), PointOnObjectA, PointOnObjectB);
      // Queue up an undo operation on this joint being created
      if(jointCog != nullptr)
      {
        OperationQueue* queue = Z::gEditor->GetOperationQueue();
        ObjectCreated(queue, jointCog);
      }
    }
  }
}

cstr JointTool::GetJointName()
{
  return JointToolTypes::Names[mJointType];
}

}//namespace Zero
