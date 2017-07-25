/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
#ifdef ZILCH_HANDLE_DEBUG
    // Statics implemented for debugging
    Handle* Handle::DebugHead = nullptr;
    Handle* Handle::DebugTail = nullptr;
#endif

  //***************************************************************************
  Handle::Handle()
  {
    ZilchErrorIfNotStarted(Handle);

    // Clear out the handle object to a null handle
    this->InternalClear();

#ifdef ZILCH_HANDLE_DEBUG
    // Link ourselves to the global list of handles
    this->DebugLink();
#endif
  }

  //***************************************************************************
  Handle::Handle(nullptr_t)
  {
    ZilchErrorIfNotStarted(Handle);

    // Clear out the handle object to a null handle
    this->InternalClear();

#ifdef ZILCH_HANDLE_DEBUG
    // Link ourselves to the global list of handles
    this->DebugLink();
#endif
  }

  //***************************************************************************
  Handle::Handle(const Handle& rhs) :
    StoredType(rhs.StoredType),
    Manager(rhs.Manager),
    Offset(rhs.Offset),
    Flags(rhs.Flags)
  {
    // The data of a handle type is always memory-copyable
    memcpy(this->Data, rhs.Data, sizeof(this->Data));
    
#ifdef ZILCH_HANDLE_DEBUG
    // Link ourselves to the global list of handles
    this->DebugLink();
#endif

    // Increment the reference count since we're now referencing the same thing
    this->AddReference();
  }

  //***************************************************************************
  Handle::Handle(const Any& other)
  {
    // If the other is null, then just become a null handle
    if (other.StoredType == nullptr)
    {
      this->InternalClear();
    }
    else if (other.StoredType->IsHandle())
    {
      Handle& handle = *(Handle*)other.GetData();

      this->StoredType = handle.StoredType;
      this->Manager = handle.Manager;
      this->Offset = handle.Offset;
      this->Flags = handle.Flags;
        
      // The data of a handle type is always memory-copyable
      memcpy(this->Data, handle.Data, sizeof(this->Data));

#ifdef ZILCH_HANDLE_DEBUG
      // Link ourselves to the global list of handles
      this->DebugLink();
#endif
      // Increment the reference count since we're now referencing the same thing
      this->AddReference();
    }
    else if (other.StoredType->IsValue())
    {
      // Just get a pointer to the value inside the variant
      HandleManager* manager = HandleManagers::GetInstance().GetManager(ZilchManagerId(PointerManager));
      this->Initialize(other.GetData(), static_cast<BoundType*>(other.StoredType), manager);
    }
    else
    {
      Error("Cannot create a handle from this Any because the value it stores cannot be converted to a handle");
      this->InternalClear();
    }
  }

  //***************************************************************************
  Handle::Handle(Handle&& other)
  {
    memcpy(this, &other, sizeof(*this));
    memset(&other, 0, sizeof(*this));
  }

  //***************************************************************************
  Handle::Handle(const byte* data, BoundType* type, HandleManager* manager, ExecutableState* state)
  {
    this->Initialize(data, type, manager, state);
  }

  //***************************************************************************
  void Handle::Initialize(const byte* data, BoundType* type, HandleManager* manager, ExecutableState* state)
  {
    type->IsInitializedAssert();
    this->StoredType = type;
    this->Offset = 0;
    this->Flags = HandleFlags::None;

    // Grab the handle manager for this type
    if (manager == nullptr)
      manager = HandleManagers::GetInstance().GetManager(type->HandleManager, state);
    this->Manager = manager;

    // Zero out the data (this is a guarantee we make before we ask the manager to initialize the data)
    memset(this->Data, 0, sizeof(this->Data));

    // Construct this handle from the manager
    manager->ObjectToHandle(data, type, *this);
  }

  //***************************************************************************
  Handle::~Handle()
  {
#ifdef ZILCH_HANDLE_DEBUG
    // Unlink ourselves from the global list of handles
    this->DebugUnlink();
#endif

    // Decrement the reference count for ourselves
    this->ReleaseReference();

#ifdef ZILCH_HANDLE_DEBUG
    // We cannot clear 'Type' because it actually needs to be auto-destructed
    memset(&this->Manager, 0xCD, sizeof(this->Manager));
    memset(&this->Offset, 0xCD, sizeof(this->Offset));
    memset(&this->Data, 0xCD, sizeof(this->Data));
    memset(&this->Flags, 0xCD, sizeof(this->Flags));
#endif
  }


#ifdef ZILCH_HANDLE_DEBUG
  //***************************************************************************
  void Handle::ValidateAllHandles()
  {
    // Loop forwards through the list and validate each handle
    {
      HashSet<Handle*> visited;

      Handle* value = DebugHead;
      Handle* lastHandle = nullptr;
      while (value != nullptr)
      {
        ErrorIf(visited.Contains(value),
          "Attempted to visit a link twice");
        visited.Insert(value);
        value->Validate();
        lastHandle = value;
        value = value->DebugNext;
      }
    }

    // Loop backwards through the list and validate each handle
    {
      HashSet<Handle*> visited;

      Handle* value = DebugTail;
      Handle* lastHandle = nullptr;
      while (value != nullptr)
      {
        ErrorIf(visited.Contains(value),
          "Attempted to visit a link twice");
        visited.Insert(value);
        value->Validate();
        lastHandle = value;
        value = value->DebugPrev;
      }
    }
  }

  //***************************************************************************
  void Handle::Validate() const
  {
    // Verify the next and prev values
    ErrorIf(this->DebugNext == nullptr && this->DebugPrev == nullptr && DebugHead != this,
      "A handle was possibly mem-cleared");
    ErrorIf(this->DebugNext != nullptr && this->DebugNext == this->DebugPrev,
      "Possibly corrupted handle");
    ErrorIf(this->DebugNext == (Handle*)0xCDCDCDCD || this->DebugPrev == (Handle*)0xCDCDCDCD,
      "Possibly corrupted handle");

    // Make sure the offset size is ok
    ErrorIf(this->Offset > 0x7777,
      "Possibly corrupted handle based on the large offset size");

    // Clear all flags and see if any other bits were set
    ErrorIf((this->Flags & ~(HandleFlags::NoReferenceCounting | HandleFlags::InitializedByConstructor)) != 0,
      "Possibly corrupted handle (bits set even when we cleared all flags)");
    
    // See if we have a manager
    if (this->Manager)
    {
      // Verify that we can access the manager
      ErrorIf(this->Manager->GetName().Empty(),
        "Possibly corrupted handle (could not get the handle manager name)");

      // Attempt to derefence the handle, see what happens
      this->Manager->HandleToObject(*this);
    }

    // See if we have a type...
    if (this->Type)
    {
      // Check that we can get the type name
      ErrorIf(this->Type->ToString().Empty(),
        "Possibly corrupted handle (could not get the type name)");
    }

    // Make sure we either have no type and no manager, or a type and a manager
    //ErrorIf((this->Manager != nullptr) != (this->Type != nullptr),
    //  "Possibly corrupted handle (manager or type incosistently set)");
  }

  //***************************************************************************
  void Handle::DebugLink()
  {
    this->Flags |= HandleFlags::InitializedByConstructor;
    this->DebugNext = nullptr;

    if (DebugTail)
    {
      ErrorIf(DebugTail->DebugNext != nullptr);

      DebugTail->DebugNext = this;
      this->DebugPrev = DebugTail;
      DebugTail = this;
    }
    else
    {
      ErrorIf(DebugHead != nullptr);

      DebugHead = this;
      DebugTail = this;
      this->DebugPrev = nullptr;
    }
  }

  //***************************************************************************
  void Handle::DebugUnlink()
  {
    // We only want to unlink this if it was ever linked in the first place
    if ((this->Flags & HandleFlags::InitializedByConstructor) == 0)
      return;

    // First, just attempt to validate ourselves
    this->Validate();

    // If I am the tail...
    if (this->DebugNext == nullptr)
    {
      ErrorIf(this != DebugTail);
      DebugTail = this->DebugPrev;
    }
    else
    {
      ErrorIf(this == DebugTail);
      this->DebugNext->DebugPrev = this->DebugPrev;
    }

    // If I am the head...
    if (this->DebugPrev == nullptr)
    {
      ErrorIf(this != DebugHead);
      DebugHead = this->DebugNext;
    }
    else
    {
      ErrorIf(this == DebugHead);
      this->DebugPrev->DebugNext = this->DebugNext;
    }

    this->DebugNext = (Handle*)0xCDCDCDCD;
    this->DebugPrev = (Handle*)0xCDCDCDCD;
  }

#endif

  //***************************************************************************
  Handle& Handle::operator=(const Handle& rhs)
  {
    // Check for self assignment (which we ignore)
    if (this == &rhs)
    {
      // Return ourselves for chaining
      return *this;
    }

#ifdef ZILCH_HANDLE_DEBUG
    // Unlink ourselves from the global list of handles
    this->Validate();
    rhs.Validate();
#endif

    // Decrement the reference count for ourselves
    this->ReleaseReference();

    // Copy all the members over
    this->StoredType = rhs.StoredType;
    this->Manager = rhs.Manager;
    this->Offset = rhs.Offset;
    this->Flags = rhs.Flags;
    
    // The data of a handle type is always memory-copyable
    memcpy(this->Data, rhs.Data, sizeof(this->Data));

    // Increment the reference count since we're now referencing the same thing
    this->AddReference();

    // Return ourselves for chaining
    return *this;
  }

  //***************************************************************************
  bool Handle::operator==(const Handle& rhs) const
  {
    // Check whether either handle is null
    byte* objectLhs = this->Dereference();
    byte* objectRhs = rhs.Dereference();
    bool nullLhs = (objectLhs == nullptr);
    bool nullRhs = (objectRhs == nullptr);

    ZilchTodo("There are two dereferences for comparing user handles, we should refactor this code path to make it one");

    // If both are non null, then we need to do more checking
    if (!nullLhs && !nullRhs)
    {
      // First, check that both of them use the same handle manager
      if (this->Manager != rhs.Manager)
      {
        // If they don't use the same handle manager, then at least dereference and check if the memory is the same
        return objectLhs == objectRhs;
      }

      // We can also check if the types are the same
      //if (this->Type != rhs.Type)
      //  return false;

      // We shouldn't need to check if the managers are null since both objects are non-null
      // Now ask the handle manager if these two handles / objects are equal
      return this->Manager->IsEqual(*this, rhs, objectLhs, objectRhs);
    }
    // Otherwise, if either of them is null, or both...
    else
    {
      // If they are both null, then the handles are equal
      // If the left is null but not the right, or the right is null
      // but not the left, then they are not equal
      return nullLhs && nullRhs;
    }
  }

  //***************************************************************************
  bool Handle::operator==(Zero::NullPointerType) const
  {
    return this->IsNull();
  }

  //***************************************************************************
  bool Handle::operator!=(const Handle& rhs) const
  {
    return !(*this == rhs);
  }

  //***************************************************************************
  bool Handle::operator!=(Zero::NullPointerType) const
  {
    return !this->IsNull();
  }
  
  //***************************************************************************
  size_t Handle::Hash() const
  {
    // If this handle is the null manager... (this is the trivial case of a cleared handle)
    if (this->Manager == nullptr)
    {
      // We must always return a 0 hash for null handles
      return 0;
    }

    // Otherwise let the manager handle it
    return this->Manager->Hash(*this);
  }

  //***************************************************************************
  bool Handle::IsNull() const
  {
    // Just dereference the handle and see if it's null
    return (this->Dereference() == nullptr);
  }

  //***************************************************************************
  bool Handle::IsNotNull() const
  {
    return !IsNull();
  }

  //***************************************************************************
  String Handle::ToString() const
  {
    // Return an empty string if we store nothing
    static String NullString("null");
    if (this->StoredType == nullptr)
      return NullString;

    // Generically convert to string using either an indirect type or the bound type
    // Note we CANNOT just use StoredType because it is always a BoundType, even when
    // a handle is pointing at a value type, which can only occur if it is an IndirectType
    Type* type = this->GetBoundOrIndirectType();
    return type->GenericToString((const byte*)this);
  }

  //***************************************************************************
  bool Handle::IsReferenceCounted()
  {
    return !(this->Flags & HandleFlags::NoReferenceCounting);
  }

  //***************************************************************************
  void Handle::InternalClear()
  {
    // Clear out all our members
    this->Flags = HandleFlags::None;
    this->StoredType = nullptr;
    this->Manager = nullptr;
    this->Offset = 0;

    // Clear out the handle object
    memset(this->Data, 0, sizeof(this->Data));
  }

  //***************************************************************************
  byte* Handle::Dereference() const
  {
    // If this handle is the null manager... (this is the trivial case of a cleared handle)
    if (this->Manager == nullptr || this->StoredType == nullptr)
      return nullptr;

    ZilchTodo("If a handle returns null but is not the 'null handle manager' then it should be an optimization to Clear it");

    // Dereference the handle and get a pointer to the object (or nullptr if it's a null handle)
    return this->Manager->HandleToObject(*this) + this->Offset;
  }

  //***************************************************************************
  void Handle::AddReference()
  {
    // Increment the reference count since we're now referencing the same thing
    // We need to check for null because we do not want to randomly increment reference counts of invalid handles
    if (this->Manager && this->IsReferenceCounted() && this->IsNull() == false)
    {
      // Add a reference via the manager
      this->Manager->AddReference(*this);
    }
  }

  //***************************************************************************
  void Handle::ReleaseReference()
  {
    // If the handle has a manager...
    if (this->Manager && this->IsReferenceCounted())
    {
      // We could put this responsibility upon the managers themselves, however,
      // we should never call release on a handle to an object that may have been deleted
      // For example, new an object in Zilch, then create a temporary handle, and delete the original
      // The temporary handle will get ref counted up, and after the scope ends down again
      // The ref count down should NOT be applied to then manager since the object was deleted
      if (this->IsNull() == false)
      {
        // Ask the manager to release the reference
        ReleaseResult::Enum result = this->Manager->ReleaseReference(*this);

        // If we're supposed to delete the object...
        if (result == ReleaseResult::DeleteObject)
        {
          // Invoke the destructor and ask the manager to delete the memory
          this->DestructAndDelete();
        }
      }
    }
  }

  //***************************************************************************
  void Handle::DestructAndDelete()
  {
    // Mark the stack copy of the handle as not being reference counted
    // That way, when it runs the destructor it won't attempt to increment the reference count again
    this->Flags |= HandleFlags::NoReferenceCounting;

    // Make sure this only gets called from places where we know we have a manager
    HandleManager* manager = this->Manager;
    ErrorIf(manager == nullptr,
      "This should only get called from non-null handles");

    // Grab the state from the manager (this could be null!)
    ExecutableState* state = ExecutableState::CallingState;

    // Get a pointer to our own object's data
    byte* self = this->Dereference();

    // Error checking
    ErrorIf(self == nullptr,
      "Somehow we're attempting to delete a null object!");

    // Don't allow any sort of allocation while we are destructing objects
    ++state->DoNotAllowAllocation;

    // Store the type 
    BoundType* type = this->StoredType;

    // See if we've patched this type with another type
    type = state->PatchedBoundTypes.FindValue(type, type);

    // While we haven't reached the root class
    while (type != nullptr)
    {
      // Invoke the destructor
      if (type->Destructor != nullptr)
      {
        // Only invoke the destructor if its is not native, or it is native and fully constructed
        if (type->Native == false || manager->GetNativeTypeFullyConstructed(*this))
        {
          // We currently execute the destructor and basically ignore any exceptions
          // They still get reported, but will be caught here
          ExceptionReport report;
          Call call(type->Destructor, state);
          call.SetHandle(Call::This, *this);
          call.Invoke(report);
        }
      }
      else
      {
        ErrorIf
        (
          type->Native && !type->IgnoreNativeDestructor,
          "All native types must bind a destructor. If it is not possible to "
          "bind, then set the flag on BoundType->IgnoreNativeDestructor"
        );
      }

      // Invoke the post-destructor
      if (type->PostDestructor != nullptr)
      {
        type->PostDestructor(type, self);
      }

      // If the type is native, don't bother traversing up any further
      // This is because a native destructor also handles calling all of its base class destructors
      if (type->Native)
      {
        // A special case is if we're dealing with a native type generated from a plugin library
        // Because plugins can potentially inherit from stub generated types which don't invoke actual destructors
        Library* typeLibrary = type->SourceLibrary;
        if (typeLibrary->CreatedByPlugin)
        {
          bool foundNonStubDestructor = false;

          // Walk up until we hit a different library that is NOT generated by a plugin
          ZilchLoop
          {
            type = type->BaseType;

            // We walked all the way up the base hierarchy and did not find a non-plugin base
            if (type == nullptr)
              break;

            if (type->SourceLibrary->CreatedByPlugin == false)
            {
              foundNonStubDestructor = true;
              break;
            }
          }

          // We're done running destructors
          if (foundNonStubDestructor == false)
            break;
        }
        else
        {
          // We always immediately break here
          break;
        }
      }
      // Not a native type
      else
      {
        // Iterate up to the base type
        type = type->BaseType;
      }
    }

    // We may allow allocation again (depends on if this is the last destructor on the stack)
    // Allocation is allowed again when this is 0
    --state->DoNotAllowAllocation;

    // Delete this handle
    manager->Delete(*this);
  }

  //***************************************************************************
  bool Handle::Delete()
  {
    // If the handle is already null, then don't bother with it!
    // Note for below: this actually checks for 'Null' typed handles
    // Moreover, after this we can assume that the manager is valid
    if (this->IsNull())
      return true;

    // Make sure this only gets called from places where we know we have a manager
    ErrorIf(this->Manager == nullptr,
      "This should only get called from non-null handles");
    
    // If we have an offset, we cannot be deleted
    if (this->Offset != 0)
    {
      return false;
    }

    // We make a stack copy just in case this handle resides in memory that we are about to delete
    // Note: Leave this up here just in case CanDelete would return false if we made the copy
    // Note: Even though we mark THIS handle as being no longer reference counted, since this is an explicit delete
    // then OTHER handles could still be live that cause reference counts
    // We could technically turn off reference counting on the object with the manager, however, we don't need to because
    // this stack copy will increment the reference by 1, and the call to 'this->Clear()' does NOT release the reference
    // Therefore we add a dangling reference, but we're about to delete this object so its all ok!
    Handle stackCopy(*this);

    // If we cannot delete the handle, then early out!
    if (this->Manager->CanDelete(stackCopy) == false)
    {
      return false;
    }

    // Clear this handle to a null handle (for safety)
    // This is safe to do because we're only deleting our stack copy
    // Note: See stack copy notes above for why this does NOT decrement a reference
    this->InternalClear();

    // Invoke the destructor on the stack copy
    stackCopy.DestructAndDelete();
    return true;
  }

  //***************************************************************************
  void Handle::Clear()
  {
    *this = Handle();
  }
  
  //***************************************************************************
  Type* Handle::GetBoundOrIndirectType() const
  {
    // If this is a cleared handle, it will store no type yet
    if (this->StoredType == nullptr)
      return nullptr;

    // If this is a value type
    if (this->StoredType->IndirectType != nullptr)
      return this->StoredType->IndirectType;

    // If this is a reference type
    return this->StoredType;
  }
  
  //***************************************************************************
  void Handle::ReleaseAndClearForBinding()
  {
    if (this->IsNull())
      return;

    // Ask the manager to release the reference
    this->Manager->ReleaseReference(*this);

    // This version of clear does NOT decrement the reference
    this->InternalClear();
  }
}
