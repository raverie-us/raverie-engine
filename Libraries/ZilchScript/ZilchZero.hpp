// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Zilch Component
/// A base class for all Zilch components defined in script
class ZilchComponent : public Component
{
public:
  ZilchDeclareInheritableType(ZilchComponent, TypeCopyMode::ReferenceType);

  // Component interface
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void ScriptInitialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;
  void Serialize(Serializer& stream) override;
  ObjPtr GetEventThisObject() override;
  void DebugDraw() override;
  void Delete() override;
};

// Zilch Event
/// A base class for all Zilch events defined in script
class ZilchEvent : public Event
{
public:
  ZilchDeclareInheritableType(ZilchEvent, TypeCopyMode::ReferenceType);

  // Event interface
  void Serialize(Serializer& stream) override;
  void Delete() override;
};

// Zilch Object
/// A base class for any object in Zilch that we want to have properties / meta
/// / send and receive events
class ZilchObject : public EventObject
{
public:
  ZilchDeclareInheritableType(ZilchObject, TypeCopyMode::ReferenceType);

  // Object interface
  void Serialize(Serializer& stream) override;
  void Delete() override;
  void DispatchEvent(StringParam eventId, Event* event);
};

DeclareEnum2(FindZilchComponentResult, Success, NameConflictedWithNative);

} // namespace Zero
