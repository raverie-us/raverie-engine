///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Collider;
class RigidBody;
class PhysicsNode;
struct AttachmentInfo;

void BodyInitialize(RigidBody* body, bool dynamicallyCreated);
void BodyOnAllObjectsCreated(RigidBody* body, bool dynamicallyCreated);
void BodyStateChanged(RigidBody* body);
void BodyOnDestroy(RigidBody* body, bool dynamicallyDestroyed);

void ColliderInitialize(Collider* collider, bool dynamicallyCreated);
void ColliderOnAllObjectsCreated(Collider* collider, bool dynamicallyCreated);
void ColliderOnDestroy(Collider* collider, bool dynamicallyDestroyed);

void PhysicsAttachTo(PhysicsNode* node, AttachmentInfo& info);
void PhysicsDetach(PhysicsNode* node, AttachmentInfo& info);

}//namespace Zero
