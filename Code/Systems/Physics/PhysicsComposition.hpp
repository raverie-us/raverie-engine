// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
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

} // namespace Raverie
