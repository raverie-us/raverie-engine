/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_HANDLE_MANAGER_HPP
#define ZILCH_HANDLE_MANAGER_HPP

namespace Zilch
{
  // A function we use to create the handle managers by index
  typedef HandleManager* (*CreateHandleManagerFn)(ExecutableState* state);

  // Generates handle manager creator functions
  template <typename T>
  ZeroSharedTemplate HandleManager* HandleManagerCreator(ExecutableState* state)
  {
    return new T(state);
  }

  // This holds any shared handle manager memory
  class ZeroShared HandleManagers
  {
  public:
    // Get the singleton instance
    static HandleManagers& GetInstance();

    // Constructor
    HandleManagers();

    // Destructor
    ~HandleManagers();

    // Add a shared handle manager
    void AddSharedManager(HandleManagerId index, HandleManager* manager);

    // Add a unique creator function
    void AddUniqueCreator(HandleManagerId index, CreateHandleManagerFn creator);

    // Get a handle manager by id
    // This will first attempt to look up a shared manager
    // If the shared manager cannot be found, it will use the executable state
    // Note that if the executable state is not passed in, an assert will fire and the default PointerManager will be returned
    HandleManager* GetManager(HandleManagerId id, ExecutableState* state = nullptr);

    // Get a unique creator function
    CreateHandleManagerFn GetUniqueCreator(HandleManagerId index);

    // Get the next available id in the handle manager
    HandleManagerId GetNextId();

    // Lock it so we cannot add anymore handle managers
    void Lock();

  private:
    // All the shared handle managers, by index
    HashMap<size_t, HandleManager*> Shared;

    // For unique handle managers, this maps the index to a function that will create them
    HashMap<size_t, CreateHandleManagerFn> Unique;

    // Counter for assigning handle manager indices
    HandleManagerId UniqueCounter;

    // We consider this locked once we create ZilchSetup
    bool Locked;
  };

  template <typename T>
  ZeroSharedTemplate class HandleManagerGuid
  {
  public:
    // The index into the executable state that this manager gets
    // This index is also used to uniquely identify the manager
    static const HandleManagerId Id;
  };

  // Auto-increment the handle manager index as we register more managers
  template <typename T>
  ZeroSharedTemplate const HandleManagerId HandleManagerGuid<T>::Id = HandleManagers::GetInstance().GetNextId();

  // Get the id of a handle manager
  #define ZilchManagerId(Type) Zilch::HandleManagerGuid<Type>::Id

  // Creates and registers a shared handle manager
  // Any extra arguments will be given to the constructor of your type
  // Shared handle managers MUST be implemented in a thread safe fashion
  #define ZilchRegisterSharedHandleManager(Type)  \
    Zilch::HandleManagers::GetInstance().AddSharedManager(ZilchManagerId(Type), new Type(nullptr))

  // Registers a unique (per ExecutableState) handle manager
  // The types registered are expected to have a default constructor
  #define ZilchRegisterUniqueHandleManager(Type)  \
    Zilch::HandleManagers::GetInstance().AddUniqueCreator(ZilchManagerId(Type), Zilch::HandleManagerCreator<Type>)
  
  // The result we get back from releasing a reference to a handle object
  namespace ReleaseResult
  {
    enum Enum
    {
      // Zilch will invoke the destructors and call delete on the object
      DeleteObject,

      // This should be returned when the object is either still alive
      // or the reference counting interface automatically deleted the object
      TakeNoAction
    };
  }

  // An interface that users must implement for their own handle types
  // Common handle types are slot map handles (index into a table with unique id)
  // Other types include reference counted objects, or even garbage collected objects
  // Note that objects allocated within the language are managed by the language
  class ZeroShared HandleManager
  {
  public:

    // Constructor
    // The state will be null in the case that this is a shared handle manager
    HandleManager(ExecutableState* state);

    // Virtualize the destructor
    virtual ~HandleManager();

    // Initializes a handle given a pointer to an object (generally used for Write / WriteRef in C++)
    // The 'object' is the same pointer that we would get back from Dereferencing the handle
    // Note that the only portion of the handle the needs to be initialized is the Data field,
    // and that the handle will have been memory cleared to all 0 (the Type and Manager will be set externally)
    virtual void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) = 0;

    // Dereference the user-data stored on the handle and
    // turn it into an instance pointer to the user's object
    // Return a null pointer if the object handle is null
    virtual byte* HandleToObject(const Handle& handle) = 0;

    // Get the name of the handle manager, only for debugging and exceptions
    virtual String GetName();

    // Allocate the type and initialize a handle to that allocated object
    // The custom flags can be used to pass in any information to the allocation scheme
    // For example, the HeapHandleManager uses the flags to specify whether the type should be reference counted or not
    // Note that the only portion of the handle the needs to be initialized is the Data field,
    // and that the handle will have been memory cleared to all 0 (the Type and Manager will be set externally)
    virtual void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags);

    // Delete all references to stored objects (if we have any allocated)
    // This is called while the ExecutableState is being torn down, before we destruct all static memory
    // Shortly after this is called on all HandleManagers owned by that ExecutableState, the manager will be destructed
    // In general if Allocate is implemented to do anything, then this should be overridden
    // Leaks should be reported to the executable state via the MemoryLeak event
    virtual void DeleteAll(ExecutableState* state);

    // Retrieve the hash value of a given handle (generally just the pointer value or some unique value)
    // This is generally used to compare handles when added to hashable containers
    // All null handles should return 0
    virtual size_t Hash(const Handle& handle);

    // Add a reference to the object
    // The default behavior is to do nothing (assumes the object is a global and never dies)
    virtual void AddReference(const Handle& handle);

    // Releases the reference to the object and returns whether the object should be deleted by Zilch
    // This function may destroy the object, but should NOT do so via Zilch calls such as Delete on the Handle
    virtual ReleaseResult::Enum ReleaseReference(const Handle& handle);

    // Destroys the object and frees its memory
    // Deleting the object should always try and ensure that all references to it become null
    // This will only ever be called if CanDelete return true
    // Prior to deleting we always check for null, so you do not need to test for null here
    virtual void Delete(const Handle& handle);

    // Returns true if the object is deletable, false otherwise
    // Note that returning false will cause a 'non-deletable object' exception in the language
    // The default behavior is to return false (assumes the object can never be explicitly deleted)
    virtual bool CanDelete(const Handle& handle);

    // Set/Get that this type has been fully constructed (including any native base classes)
    // This happens only when types are either native or derive from a native type
    virtual void SetNativeTypeFullyConstructed(const Handle& handle, bool value);
    virtual bool GetNativeTypeFullyConstructed(const Handle& handle);

    // Compare two handles with each other; return true if they are equal, false otherwise
    // The default behavior is to compare the dereferenced pointers
    // Since this check is only done after the handles have already been dereferenced,
    // we also pass in the dereferenced byte pointers we received
    virtual bool IsEqual
    (
      const Handle& handleLhs,
      const Handle& handleRhs,
      const byte* objectLhs,
      const byte* objectRhs
    );

  public:

    // The executable state (only used in the case that we're not shared)
    ExecutableState* const State;

    // Handle managers are non-copyable
    ZilchNoCopy(HandleManager);
  };
  
  // Flags passed into the heap Allocate function
  namespace HeapFlags
  {
    enum Enum
    {
      // Allocates a regular reference counted object (default)
      ReferenceCounted = 0,

      // Allocates a non-reference counted object, however supports safe handle behavior
      NonReferenceCounted
    };
  }
  
  // This MUST align exactly with the HeapFlags (it gets initialized to whatever the user passes in)
  namespace HeapObjectFlags
  {
    enum Enum
    {
      // A regular reference counted object (default)
      None = 0,

      // A non-reference counted object, however supports safe handle behavior
      NonReferenceCounted = 1,

      // Whether or not we completed the entire base class constructor chain
      // For example, if we throw an exception before fully initializing our base class, and our base class is native C++
      // and has a virtual table, then it would be VERY bad to invoke the C++ destructor if we're not fully constructed
      NativeFullyConstructed = 2
    };
  }

  // The header exists at the beginning of the allocated object (the Data pointer on ObjectSlot)
  class ZeroShared ObjectHeader
  {
  public:
    BoundType*            Type;
    Uid                   UniqueId;
    unsigned              ReferenceCount;
    HeapObjectFlags::Enum Flags;
  };

  // The structure of our heap handle's inner data
  class ZeroShared HeapHandleData
  {
  public:
    // This is the pointer to the header of the object
    // which we implicitly allocate behind every object
    ObjectHeader* Header;
    Uid           UniqueId;
  };
  ZilchStaticAssert(sizeof(HeapHandleData) <= HandleUserDataSize,
    "The HeapHandleData class must fit within Handle::Data (make handle Data bigger)",
    HeapHandleDataMustFitWithinHandleData);

  // This setting is potentially dangerous!!!
  // Currently we add an extra amount to the end of every allocation to support patching and adding fields
  // If the value were entirely allocated by us (and not pointed to by C++) then it would always be safe to
  // entirely reallocate the memory and continue. However, often times Zilch objects inherit from C++ objects
  // and user C++ code does not expect the memory to change pointers, therefore it is unsafe to do
  // Instead, we use this extra buffer space to put parameters (much like edit and continue on the stack)
  // Honestly, we would love to use 'realloc' to attempt to at least resize the memory block, except realloc
  // is allowed to allocate and move memory, which we do NOT want (there is no "fail if it can't resize" C function)
  // We could implement this on a platform basis (such as HeapReAlloc on Windows with the flag of no moving)
  const size_t HeapManagerExtraPatchSize = 256;

  // This manages heap objects allocated in the language (including references to heap members via offset)
  class ZeroShared HeapManager : public HandleManager
  {
  public:

    // HandleManager interface
    HeapManager(ExecutableState* state);
    String GetName() override;
    void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override;
    byte* HandleToObject(const Handle& handle) override;
    void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override;
    void DeleteAll(ExecutableState* state) override;
    void Delete(const Handle& handle) override;
    bool CanDelete(const Handle& handle) override;
    void AddReference(const Handle& handle) override;
    ReleaseResult::Enum ReleaseReference(const Handle& handle) override;
    void SetNativeTypeFullyConstructed(const Handle& handle, bool value) override;
    bool GetNativeTypeFullyConstructed(const Handle& handle) override;

    // A unique ID counter (so we can Assign objects unique IDs...)
    Uid UidCount;

    // When we validate a handle, we first check if the object is live (this is NOT a pointer to the header)
    // Because a completely different object could have been allocated in the exact same place (pointer)
    // then we also have to check the version stored in the handle against the version in the object's header
    // If the pointer given to 'ObjectToHandle' does not exist here, we implicitly allocate a new object and
    // invoke the copy constructor on the object
    HashSet<const byte*> LiveObjects;
  };

  // The structure of our stack handle's inner data
  class ZeroShared StackHandleData
  {
  public:
    Uid           UniqueId;
    PerScopeData* Scope;
    byte*         StackLocation;
  };
  ZilchStaticAssert(sizeof(StackHandleData) <= HandleUserDataSize,
    "The StackHandleData class must fit within Handle::Data (make handle Data bigger)",
    StackHandleDataMustFitWithinHandleData);

  // This manages stack objects initialized in the language (including references to stack members via offset)
  class ZeroShared StackManager : public HandleManager
  {
  public:
    // HandleManager interface
    StackManager(ExecutableState* state);
    String GetName() override;
    void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override;
    byte* HandleToObject(const Handle& handle) override;
    void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override;
  };

  // This manages insertion of pointers into the language, which are assumed to be global
  class ZeroShared PointerManager : public HandleManager
  {
  public:
    // HandleManager interface
    PointerManager(ExecutableState* state);
    String GetName() override;
    void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override;
    void Delete(const Handle& handle) override;
    bool CanDelete(const Handle& handle) override;
    byte* HandleToObject(const Handle& handle) override;
    void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override;
  };

  // This manages string nodes for the string class, which is always a reference type
  class ZeroShared StringManager : public HandleManager
  {
  public:
    // HandleManager interface
    StringManager(ExecutableState* state);
    String GetName() override;
    size_t Hash(const Handle& handle) override;
    void Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags) override;
    byte* HandleToObject(const Handle& handle) override;
    void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override;
    void AddReference(const Handle& handle) override;
    ReleaseResult::Enum ReleaseReference(const Handle& handle) override;
    bool IsEqual
    (
      const Handle& handleLhs,
      const Handle& handleRhs,
      const byte* objectLhs,
      const byte* objectRhs
    ) override;
  };
  ZilchStaticAssert(sizeof(String) <= HandleUserDataSize,
    "The String class must fit within Handle::Data (make handle Data bigger)",
    StringMustFitWithinHandleData);
}

#endif
