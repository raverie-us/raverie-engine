////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Tags
{
DeclareTag(Component);
}

// Forward declarations
class CogInitializer;
struct AttachmentInfo;

//---------------------------------------------------------------------------- Transform Update Info
struct TransformUpdateInfo
{
  TransformUpdateInfo()
  { 
    mTransform = NULL;
    mDelta.SetIdentity();
  }

  uint TransformFlags;
  /// The delta transform being applied (so that in-world children can update)
  Mat4 mDelta;
  /// The transform component that the transformation was
  /// applied on (so it can avoid applying the delta on itself)
  Transform* mTransform;
};

//---------------------------------------------------------------------------------------- Component
/// A component is an atomic piece of functionality that is composed into a Cog
/// to form game objects.
class Component : public Object
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component memory management.
  static Memory::Heap* sHeap;
  OverloadedNew();
  friend class Cog;

  /// Constructor / Destructor.
  Component();
  virtual ~Component() {}

  /// Save / loading to stream.
  virtual void Serialize(Serializer& stream) {}

  /// See Composition for the order of initialization
  /// Initialize the component. Activating it in the space.
  virtual void Initialize(CogInitializer& initializer) {}
  virtual void OnAllObjectsCreated(CogInitializer& initializer) {}
  virtual void ScriptInitialize(CogInitializer& initializer) {}

  /// Deletes the object. This allows the component to be responsible
  /// for its own cleanup, such as in the case of script components
  virtual void Delete();

  /// Get the Cog this Component is owned by (not the parent of this composition).
  Cog* GetOwner() const;

  /// This is to guard from Zilch Components accessing the owner inside property setters during
  /// serialization. The owner used to be null, but had to be set for something in CogPath.
  /// This getter is bound to script (as 'Owner'), and it only returns the owner if the
  /// Cog is initialized.
  /// Get the Cog this Component is owned by (not the parent of this composition).
  Cog* GetOwnerScript() const;

  /// The Space where the object is located.
  Space* GetSpace();

  /// Get the object named 'LevelSettings', a special object where we can put components for our level
  Cog* GetLevelSettings();

  /// Get the GameSession that owns us and our space
  GameSession* GetGameSession();

  ///Returns the parent objects event tracker.
  EventReceiver* GetReceiver();
  EventDispatcher* GetDispatcher();

  /// Returns whether or not the owning Cog is initialized.
  bool IsInitialized();

  ///Special function for transform updating from editor.
  virtual void TransformUpdate(TransformUpdateInfo& info) {}

  ///Signal that the Cog has been attached to another Cog.
  virtual void AttachTo(AttachmentInfo& info) {}

  ///Signal that the Cog has been detached from another Cog.
  virtual void Detached(AttachmentInfo& info) {}

  /// Should this component be serialized?
  virtual bool ShouldSerialize() {return true;}

  /// Component added to composition
  virtual void ComponentAdded(BoundType* typeId, Component* component) {}

  /// Component removed from composition.
  virtual void ComponentRemoved(BoundType* typeId, Component* component) {}

  ///Base debug draw for a component. Special for the each type of component.
  virtual void DebugDraw() {}

  /// Set component defaults
  virtual void SetDefaults() {}

  /// Description functions
  void WriteDescription(StringBuilder& builder);
  String GetDescription();

  /// Event functionality.
  virtual void DispatchEvent(StringParam eventId, Event* event);
  EventDispatcher* GetDispatcherObject() { return GetDispatcher(); }
  EventReceiver* GetReceiverObject() { return GetReceiver(); }

  /// Get Pointer for Disconnection
  virtual void* GetEventThisObject() { return this; }

  /// Each component has a pointer back to the base owning composition.
  Cog* mOwner;

private:
  /// Only Cogs can destroy their Components.
  virtual void OnDestroy(uint flags = 0) {}
};

//------------------------------------------------------------------------- Component Handle Manager
class ComponentHandleData : public CogHandleData
{
public:
  BoundType* mComponentType;
};

class ComponentHandleManager : public HandleManager
{
public:
  ComponentHandleManager(ExecutableState* state) : HandleManager(state) {}

  // HandleManager interface
  void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override;
  void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override;
  byte* HandleToObject(const Handle& handle) override;
  bool CanDelete(const Handle& handle) override;
  void Delete(const Handle& handle) override;
};

//--------------------------------------------------------------------------------- Component Handle
template <typename ComponentType>
class ComponentHandle : public HandleOf<ComponentType>
{
public:
  Cog* GetOwner()
  {
    if(ComponentType* component = *this)
      return component->GetOwner();
    return nullptr;
  }

  void operator=(Cog* rhs)
  {
    *this = rhs->has(ComponentType);
  }

  // Our = operator with Cog overload hid the base operator=
  using Handle::operator=;
};

}//namespace Zero
