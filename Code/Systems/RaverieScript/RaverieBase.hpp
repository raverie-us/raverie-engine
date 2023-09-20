// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Raverie Component
/// A base class for all Raverie components defined in script
class RaverieComponent : public Component
{
public:
  RaverieDeclareInheritableType(RaverieComponent, TypeCopyMode::ReferenceType);

  // Component interface
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void ScriptInitialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;
  void Serialize(Serializer& stream) override;
  ObjPtr GetEventThisObject() override;
  void DebugDraw() override;
  void Delete() override;
};

// Raverie Event
/// A base class for all Raverie events defined in script
class RaverieEvent : public Event
{
public:
  RaverieDeclareInheritableType(RaverieEvent, TypeCopyMode::ReferenceType);

  // Event interface
  void Serialize(Serializer& stream) override;
  void Delete() override;
};

// Raverie Object
/// A base class for any object in Raverie that we want to have properties / meta
/// / send and receive events
class RaverieObject : public EventObject
{
public:
  RaverieDeclareInheritableType(RaverieObject, TypeCopyMode::ReferenceType);

  // Object interface
  void Serialize(Serializer& stream) override;
  void Delete() override;
  void DispatchEvent(StringParam eventId, Event* event);
};

DeclareEnum2(FindRaverieComponentResult, Success, NameConflictedWithNative);

} // namespace Raverie
