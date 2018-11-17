/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_HANDLE_HPP
#define ZILCH_HANDLE_HPP

// Enable this to add extra debugging support (for Zilch internal use, such as working on the compiler)
// Warning: This will make Zilch run significantly slower
//#define ZILCH_HANDLE_DEBUG

namespace Zilch
{
  // A unique id that lets us identify objects
  typedef unsigned Uid;

  // A special code that signifies that a Uid is not set or invalid
  const unsigned InvalidUid = 0xFFFFFFFF;

  // The user has to be able to fit their entire handle inside of the user data on the handle
  // If the user cannot fit the data inside the handle, then they must create another object
  // that acts as a reference to their object
  const size_t HandleUserDataSize = 24;

  // Any flags we put on the handle
  namespace HandleFlags
  {
    enum Enum
    {
      // No flags
      None = 0,

      // In this mode, we don't do reference counting (this is a useful optimization for handles!)
      // This flag is also set when we're deleting a handle so that it can be passed into a destructor without
      // increasing or decreasing reference counts (this is safe because the handle is already going to be deleted)
      NoReferenceCounting = 1,

      // This flag is set only if ZILCH_HANDLE_DEBUG is enabled
      // This is used to track which handles need be removed from the intrusive linked list of all handles
      // If a handle was purely initialized via memory setting to zero, we ignore it
      InitializedByConstructor = 2,
    };
    typedef byte Compact;
  }

  DeclareEnum2(GetOptions, ReturnDefaultOrNull, AssertOnNull);

  // A handle is an object that has the ability to point at an object
  // (both on the heap, as a member of an object on the heap, etc)
  class ZeroShared Handle
  {
  public:
    // Friends
    friend class VirtualMachine;
    friend class ExecutableState;

    // Constructor that creates a null handle
    Handle();
    Handle(nullptr_t);

    template <typename T>
    Handle(const T& value, HandleManager* manager = nullptr, ExecutableState* state = nullptr);

    // HandleOf is special cased because C++ chooses the templated constructor over the copy constructor.
    template <typename T>
    Handle(const HandleOf<T>& rhs);

    template <typename T>
    T Get(GetOptions::Enum options = GetOptions::ReturnDefaultOrNull) const;

    // Copy/move constructors for a handle
    Handle(const Handle& other);
    Handle(Handle&& other);

    // Construction from an Any
    // The type contained within the Any must be a Handle
    // If the Any is keeping a value type, then this will use the PointerManager to point at the type (the Any MUST be kept alive)
    explicit Handle(const Any& other);

    // Constructs a handle from a type and a data pointer
    // This will invoke ObjectToHandle on the handle manager
    // The 'data' passed in should be a pointer to the object (the same as that returned from Dereference)
    Handle(const byte* data, BoundType* type, HandleManager* manager = nullptr, ExecutableState* state = nullptr);

    // Destructor
    ~Handle();

    // Copy from assignment
    Handle& operator=(const Handle& rhs);

    // Are two handles the exact same?
    bool operator==(const Handle& rhs) const;
    bool operator==(Zero::NullPointerType) const;

    // Are two handles different?
    bool operator!=(const Handle& rhs) const;
    bool operator!=(Zero::NullPointerType) const;

    // Hashes a handle (generally used by hashable containers)
    size_t Hash() const;

    // Dereferences the handle with the HandleManager and determines if it is null
    // It is recommended for performance that you just call Dereference and
    // check if it returns a null if you ALSO need to use the dereferenced value
    bool IsNull() const;
    bool IsNotNull() const;

    // Converts the internal value to string (used for debugging)
    String ToString() const;

    // Get a pointer to the data, or return null if it isn't valid
    byte* Dereference() const;

    // Releases the handle and clears it out to zero
    void Clear();

    // Attempts to delete the object pointed at by the handle
    // Returns true if the object was deleted, or if the handle was already null
    // If the object is deleted, this handle will be cleared to null
    // It is possible for the destructor to throw an exception (though not highly recommended)
    bool Delete();

    // Checks to see if this handle is reference counted, or if it's just a standard handle
    inline bool IsReferenceCounted();

    // If the handle is looking at a reference type, this will return the bound type
    // Otherwise if the handle is looking at a value type, then this must be an indirect type
    // Returns null if the handle stores nothing
    Type* GetBoundOrIndirectType() const;

    // Use this only in the case where we are allocating a handle for an object, but we need to return
    // that object via binding (not a handle)
    // This situation would prematurely decrement the only reference count and delete the object, however
    // we know that binding will implicitly add a reference count immediately after
    // This clears the handle so that its destruction will not delete the object, and it also releases the only
    // reference count so the object does not leak
    // This is only valid when the reference count is 1
    void ReleaseAndClearForBinding();

  private:
    void Initialize(const byte* data, BoundType* type, HandleManager* manager, ExecutableState* state = nullptr);
    
    // Clear the handle to a null handle
    // Warning: This does not do reference counting or properly delete the object!
    void InternalClear();

    // Called only when we know it's ok to delete the object
    void DestructAndDelete();

    // Increment the reference count (only if we're counted)
    void AddReference();

    // Decrement the reference count (only if we're counted), and destruct/delete the object if necessary
    void ReleaseReference();

  public:

#ifdef ZILCH_HANDLE_DEBUG
    // In order to verify that handles are valid, we keep a global list of all handles
    // and we check to see if any of them are ever improperly destructed or get written over
    Handle* DebugNext;
    Handle* DebugPrev;
    static Handle* DebugHead;
    static Handle* DebugTail;
    static void ValidateAllHandles();
    void Validate() const;
    void DebugLink();
    void DebugUnlink();
#endif

    // The type of this handle (if the type is a value type, then it means we were assigned via ref/indirection type)
    // Unlike most OOP designs, our handle actually Contains the type (or the v-table)
    // rather than it being on the object itself. This allows us to invoke interface
    // functions on a stack object via a handle, rather than by boxing the value type
    BoundType* StoredType;
    //DebugPlaceholder<BoundType*> Type;

    // The manager controls the lifetime of the handle, and how we interface with it
    HandleManager* Manager;

    // An offset from the base of the handle (used in dereferencing)
    size_t Offset;

    union
    {
      // The user data stored on the handle (of a fixed size)
      byte Data[HandleUserDataSize];

      // These are only provided for convenience (rather than casting Data into a structure)
      void* HandlePointer;
      s32 HandleS32;
      s64 HandleS64;
      u32 HandleU32;
      u64 HandleU64;
    };

    // Flags we put on the handle
    HandleFlags::Compact Flags;
  };

  typedef const Handle& HandleParam;
}

#endif
