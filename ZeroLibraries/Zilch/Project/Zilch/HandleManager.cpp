/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  HandleManagers& HandleManagers::GetInstance()
  {
    static HandleManagers instance;
    return instance;
  }

  //***************************************************************************
  HandleManagers::HandleManagers() :
    UniqueCounter(0),
    Locked(false)
  {
  }

  //***************************************************************************
  HandleManagers::~HandleManagers()
  {
    // Destroy all the shared managers
    for (size_t i = 0; i < this->Shared.Size(); ++i)
    {
      // Delete all the shared entries
      delete this->Shared[i];
    }

    // Clear the shared array, as well as the creators
    this->Shared.Clear();
    this->Unique.Clear();
  }

  //***************************************************************************
  void HandleManagers::AddSharedManager(HandleManagerId id, HandleManager* manager)
  {
    // Error checking
    ReturnIf(this->Locked,,
      "We cannot add to the handle managers after we've created the ZilchSetup");

    this->Shared.Insert(id, manager);
  }

  //***************************************************************************
  void HandleManagers::AddUniqueCreator(HandleManagerId id, CreateHandleManagerFn creator)
  {
    // Error checking
    ReturnIf(this->Locked,,
      "We cannot add to the handle managers after we've created the ZilchSetup");

    this->Unique.Insert(id, creator);
  }
  
  //***************************************************************************
  HandleManager* HandleManagers::GetManager(HandleManagerId id, ExecutableState* state)
  {
    // First look globally for the shared manager
    HandleManager* manager = this->Shared.FindValue(id, nullptr);
    if (manager != nullptr)
      return manager;

    // If no state was given, attempt to get the calling state
    if (state == nullptr)
      state = ExecutableState::CallingState;

    // If we didn't find it in the shared, and we were given no executable state...
    if (state == nullptr)
    {
      Error("We were unable to find the shared handle manager and no executable state was given to look up local handle managers");
      return this->Shared.FindValue(ZilchManagerId(PointerManager), nullptr);
    }

    // See if we have a unique manager on us...
    manager = state->UniqueManagers.FindValue(id, nullptr);

    // If we found the unique manager, just return it
    if (manager != nullptr)
      return manager;

    // Otherwise, we need to see if we can create one
    CreateHandleManagerFn creator = this->GetUniqueCreator(id);

    // If the creator is valid...
    if (creator != nullptr)
    {
      // Run the creator to create the manager
      // The creator needs an executable state (will not modify it, but stores it)
      manager = creator(state);

      // Insert the manager into the map
      state->UniqueManagers.Insert(id, manager);

      // Return the manager
      return manager;
    }
    else
    {
      // Error handling
      Error("We were not able to instantiate a handle manager or find a shared one for id '%d'", id);
      return nullptr;
    }
  }

  //***************************************************************************
  CreateHandleManagerFn HandleManagers::GetUniqueCreator(HandleManagerId id)
  {
    return this->Unique.FindValue(id, nullptr);
  }

  //***************************************************************************
  HandleManagerId HandleManagers::GetNextId()
  {
    // Error checking
    ReturnIf(this->Locked, (HandleManagerId)-1,
      "We cannot get new unique ids for handle managers after we've created the ZilchSetup");

    return this->UniqueCounter++;
  }

  //***************************************************************************
  void HandleManagers::Lock()
  {
    this->Locked = true;
  }

  //***************************************************************************
  HandleManager::~HandleManager()
  {
  }

  //***************************************************************************
  HandleManager::HandleManager(ExecutableState* state) :
    State(state)
  {
  }

  //***************************************************************************
  bool HandleManager::IsEqual
  (
    const Handle& handleLhs,
    const Handle& handleRhs,
    const byte* objectLhs,
    const byte* objectRhs
  )
  {
    // Compare the dereferenced handles
    return objectLhs == objectRhs;
  }

  //***************************************************************************
  void HandleManager::AddReference(const Handle& handle)
  {
  }

  //***************************************************************************
  ReleaseResult::Enum HandleManager::ReleaseReference(const Handle& handle)
  {
    return ReleaseResult::TakeNoAction;
  }

  //***************************************************************************
  size_t HandleManager::Hash(const Handle& handle)
  {
    return HashString((const char*)handle.Data, sizeof(handle.Data));
  }

  //***************************************************************************
  String HandleManager::GetName()
  {
    static const String Name("HandleManager");
    return Name;
  }

  //***************************************************************************
  void HandleManager::Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags)
  {
    Error("This HandleManager did not override the Allocate method (the allocated handle will be null)");
  }

  //***************************************************************************
  void HandleManager::DeleteAll(ExecutableState* state)
  {
  }

  //***************************************************************************
  void HandleManager::Delete(const Handle& handle)
  {
  }
  
  //***************************************************************************
  bool HandleManager::CanDelete(const Handle& handle)
  {
    return false;
  }
  
  //***************************************************************************
  void HandleManager::SetNativeTypeFullyConstructed(const Handle& handle, bool value)
  {
  }

  //***************************************************************************
  bool HandleManager::GetNativeTypeFullyConstructed(const Handle& handle)
  {
    // By default we return true and assume everything worked
    // This could crash if an exception occurs and we never fully constructed the base, and then we invoke the C++ destructor
    return true;
  }

  //***************************************************************************
  HeapManager::HeapManager(ExecutableState* state) :
    HandleManager(state)
  {
    // Initialize the counter to zero
    this->UidCount = 0;
  }

  //***************************************************************************
  String HeapManager::GetName()
  {
    static String Name("Heap Object");
    return Name;
  }
  
  //***************************************************************************
  byte* HeapManager::HandleToObject(const Handle& handle)
  {
    HeapHandleData& data = *(HeapHandleData*)handle.Data;

    // The pointer to the object is just after the header
    byte* object = ((byte*)data.Header) + sizeof(ObjectHeader);

    // First check if the object is even live
    if (this->LiveObjects.Contains(object) == false)
      return nullptr;

    // If the unique-ids for that slot don't match (it was reused)
    // then we return null since this handle is no longer valid
    if (data.UniqueId != data.Header->UniqueId)
      return nullptr;

    // The object must be valid!
    return object;
  }
  
  //***************************************************************************
  void HeapManager::Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags)
  {
    // Create a buffer that's the size of the object we'd like to allocate
    // At the beginning of the buffer, we put the object slot pointer so that
    // 'ObjectToHandle' can recreate a handle via the slot data pointer
    size_t objectSize = type->GetAllocatedSize();
    size_t fullSize = sizeof(ObjectHeader) + objectSize + HeapManagerExtraPatchSize;
    byte* memory = (byte*)Zero::zAllocate(fullSize);

    // If the memory failed to allocate, early out
    if (memory == nullptr)
    {
      Error("Failed memory allocation within a Zilch heap object");
      handleToInitialize.Manager = nullptr;
      return;
    }

    // Make sure we mark this as a live object
    byte* object = memory + sizeof(ObjectHeader);
    this->LiveObjects.Insert(object);

    // All primitives should support being zeroed out
    memset(memory, 0, fullSize);

    // Store a pointer back to the slot on the memory itself
    ObjectHeader& header = *(ObjectHeader*)memory;
    header.Type = type;
    header.UniqueId = this->UidCount;
    header.ReferenceCount = 1;
    header.Flags = (HeapObjectFlags::Enum)customFlags;

    // Increment the unique ID counter
    ++this->UidCount;

    // If specified, we won't do reference counting on this handle
    // This means the only way to destroy the handle is via delete
    if (customFlags & HeapObjectFlags::NonReferenceCounted)
      handleToInitialize.Flags = HandleFlags::NoReferenceCounting;

    // We are always guaranteed that the handle data is cleared before we get the user data portion
    HeapHandleData& data = *(HeapHandleData*)handleToInitialize.Data;
    data.Header = &header;
    data.UniqueId = header.UniqueId;
  }
  
  //***************************************************************************
  void HeapManager::ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize)
  {
    // We weren't given a valid object, just null out the handle
    if (object == nullptr)
    {
      handleToInitialize.Manager = nullptr;
      return;
    }

    // First, check if this object was even allocated through us
    if (this->LiveObjects.Contains(object) == false)
    {
      // Since the object that was passed in isn't managed by us, the only valid way to get a handle to it is to use the pointer manager
      // Most likely this will be fine since we're passing through binding and not typically invoking user code
      ExceptionReport report;
      HandleManager* manager = HandleManagers::GetInstance().GetManager(ZilchManagerId(PointerManager));
      Handle fromHandle(object, type, manager);

      // Copy construct the object into an allocated version (with reference counting and safe handles that we now manage)
      handleToInitialize = this->State->AllocateCopyConstructedHeapObject(type, report, HeapFlags::ReferenceCounted, fromHandle);
      return;
    }

    // Just behind the allocated object is the header
    ObjectHeader& header = *(ObjectHeader*)(object - sizeof(ObjectHeader));

    // If specified, we won't do reference counting on this handle
    // This means the only way to destroy the handle is via delete
    if (header.Flags & HeapObjectFlags::NonReferenceCounted)
      handleToInitialize.Flags = HandleFlags::NoReferenceCounting;
    else
      ++header.ReferenceCount;
    
    // We are always guaranteed that the handle data is cleared before we get the user data portion
    HeapHandleData& data = *(HeapHandleData*)handleToInitialize.Data;
    data.Header = &header;
    data.UniqueId = header.UniqueId;
  }
  
  //***************************************************************************
  void HeapManager::DeleteAll(ExecutableState* state)
  {
    // Note: The executable state has a special flag set to not allow allocation of any more
    // objects (and all destructors catch exceptions that occur)
    // This is so we don't need to continue cleaning up objects at the end, since we know all will be destructed

    // Theoretically all memory from a system should be cleaned up by itself
    // We really need to incorporate leak detection here
    // Leak detection includes the stack frame of who allocated it
    // as well as all those still referencing it

    while (this->LiveObjects.Empty() == false)
    {
      // Just get the first object and attempt to free it
      const byte* object = this->LiveObjects.All().Front();

      // Just behind the allocated object is the header
      ObjectHeader& header = *(ObjectHeader*)(object - sizeof(ObjectHeader));

      // Create a temporary handle to point at the object
      Handle handle(object, header.Type, this);

      // Send out an event letting the user know that a memory leak occurred
      MemoryLeakEvent toSend;
      toSend.State = state;
      toSend.LeakedObject = &handle;
      EventSend(state, Events::MemoryLeak, &toSend);

      // Delete the object forcibly
      // Note that this Delete should call HeapManager::Delete, which will remove this from LiveObjects!
      bool deleted = handle.Delete();
      ErrorIf(deleted != true,
        "Delete on the handle returned that the object was not deleted (it always should be deletable)");
    }

    ErrorIf(this->LiveObjects.Empty() == false,
      "All objects should be cleared by this point");
  }
  
  //***************************************************************************
  void HeapManager::Delete(const Handle& handle)
  {
    // Get the associated slot
    HeapHandleData& data = *(HeapHandleData*)handle.Data;

    // The pointer to the object is just after the header
    byte* object = ((byte*)data.Header) + sizeof(ObjectHeader);
    
    // Remove the object from the list of live objects
    this->LiveObjects.Erase(object);

    // Delete the data in the slot and null it out
    Zero::zDeallocate(data.Header);
  }

  //***************************************************************************
  bool HeapManager::CanDelete(const Handle& handle)
  {
    // Currently we can always delete heap handles
    return true;
  }

  //***************************************************************************
  void HeapManager::AddReference(const Handle& handle)
  {
    // Get the associated slot
    HeapHandleData& data = *(HeapHandleData*)handle.Data;
    ++data.Header->ReferenceCount;
  }

  //***************************************************************************
  ReleaseResult::Enum HeapManager::ReleaseReference(const Handle& handle)
  {
    // Get the associated slot
    HeapHandleData& data = *(HeapHandleData*)handle.Data;
    --data.Header->ReferenceCount;

    // If the reference count reached zero, nobody else
    // knows about it so it's time to free this object!
    if (data.Header->ReferenceCount == 0)
    {
      // Return that the object must be deleted
      return ReleaseResult::DeleteObject;
    }
    else
    {
      // The decrementing of the reference didn't kill the object
      return ReleaseResult::TakeNoAction;
    }
  }
  
  //***************************************************************************
  void HeapManager::SetNativeTypeFullyConstructed(const Handle& handle, bool value)
  {
    HeapHandleData& data = *(HeapHandleData*)handle.Data;
    data.Header->Flags = (HeapObjectFlags::Enum)(data.Header->Flags | HeapObjectFlags::NativeFullyConstructed);
  }
  
  //***************************************************************************
  bool HeapManager::GetNativeTypeFullyConstructed(const Handle& handle)
  {
    HeapHandleData& data = *(HeapHandleData*)handle.Data;
    return (data.Header->Flags & HeapObjectFlags::NativeFullyConstructed) != 0;
  }

  //***************************************************************************
  StackManager::StackManager(ExecutableState* state) :
    HandleManager(state)
  {
  }

  //***************************************************************************
  String StackManager::GetName()
  {
    static String Name("Stack Object");
    return Name;
  }
  
  //***************************************************************************
  void StackManager::Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags)
  {
    Error("Allocating an object using a the StackObjectManager is not supported");
  }
  
  //***************************************************************************
  byte* StackManager::HandleToObject(const Handle& handle)
  {
    StackHandleData& data = *(StackHandleData*)handle.Data;
    
    // If the scope we're looking at has a different id than this handle, then
    // it means the original stack/scope we looked at is gone, so the value is no longer valid
    if (data.Scope->UniqueId != data.UniqueId)
      return nullptr;

    // The stack handle must be valid
    return data.StackLocation;
  }
  
  //***************************************************************************
  void StackManager::ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize)
  {
    StackHandleData& data = *(StackHandleData*)handleToInitialize.Data;
    data.StackLocation = const_cast<byte*>(object);
    Error("Use ExecutableState's InitializeStackHandle to create a handle to an object on the stack");
  }
  
  //***************************************************************************
  PointerManager::PointerManager(ExecutableState* state) :
    HandleManager(state)
  {
  }

  //***************************************************************************
  String PointerManager::GetName()
  {
    static String Name("Global Pointer");
    return Name;
  }
  
  //***************************************************************************
  byte* PointerManager::HandleToObject(const Handle& handle)
  {
    return *(byte**)handle.Data;
  }
  
  //***************************************************************************
  void PointerManager::Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags)
  {
    handleToInitialize.Flags |= HandleFlags::NoReferenceCounting;
    handleToInitialize.HandlePointer = Zero::zAllocate(type->Size);
  }

  //***************************************************************************
  void PointerManager::Delete(const Handle& handle)
  {
    Zero::zDeallocate(handle.HandlePointer);
  }

  //***************************************************************************
  bool PointerManager::CanDelete(const Handle& handle)
  {
    return true;
  }
  
  //***************************************************************************
  void PointerManager::ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize)
  {
    handleToInitialize.Flags |= HandleFlags::NoReferenceCounting;
    handleToInitialize.HandlePointer = (void*)object;
  }

  //***************************************************************************
  StringManager::StringManager(ExecutableState* state) :
    HandleManager(state)
  {
  }

  //***************************************************************************
  String StringManager::GetName()
  {
    static String Name("String");
    return Name;
  }
  
  //***************************************************************************
  byte* StringManager::HandleToObject(const Handle& handle)
  {
    // Interpret the user data as if it was the 'String' type
    return (byte*)(String*)handle.Data;
  }

  //***************************************************************************
  size_t StringManager::Hash(const Handle& handle)
  {
    // Every string has it's own hash value
    return (int)((String*)handle.Data)->Hash();
  }
  
  //***************************************************************************
  void StringManager::Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags)
  {
    Error("Allocating an object using a the StringManager is not supported");
  }
  
  //***************************************************************************
  void StringManager::ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize)
  {
    String& str = *(String*)object;
    new (handleToInitialize.Data) String(str);
  }
  
  //***************************************************************************
  bool StringManager::IsEqual
  (
    const Handle& handleLhs,
    const Handle& handleRhs,
    const byte* objectLhs,
    const byte* objectRhs
  )
  {
    // Get the two string nodes so we can compare them
    String& stringLhs = *(String*)objectLhs;
    String& stringRhs = *(String*)objectRhs;
      
    // Compare the two nodes, then compare the hash values and lengths, then directly compare the strings
    return stringLhs == stringRhs;
  }
  
  //***************************************************************************
  void StringManager::AddReference(const Handle& handle)
  {
    // Directly increment a refernece on the string node
    String& str = *(String*)handle.Data;
    Zero::StringNode* node = str.GetNode();
    node->addRef();
  }
  
  //***************************************************************************
  ReleaseResult::Enum StringManager::ReleaseReference(const Handle& handle)
  {
    // Directly decrement a refernece on the string node, delete it if it reaches 0
    String& str = *(String*)handle.Data;
    Zero::StringNode* node = str.GetNode();
    node->release();
    return ReleaseResult::TakeNoAction;
  }
}
