/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_SHARED_REFERENCE_HPP
#define ZILCH_SHARED_REFERENCE_HPP

namespace Zilch
{
  // A standard deletion policy
  template <typename Type>
  class ZeroSharedTemplate StandardDelete
  {
  public:
    void operator()(Type* object)
    {
      // Use the standard delete operator
      delete object;
    }
  };

  // An array deletion policy
  template <typename Type>
  class ZeroSharedTemplate ArrayDelete
  {
  public:
    void operator()(Type* object)
    {
      // Use the array delete operator
      delete[] object;
    }
  };

  class ZeroShared WeakPolicy
  {
  public:
    template <typename RefType>
    void AddLinkToList(RefType* ref)
    {
      // This is a hack we have to do because member pointers are stupid and don't respect the mutable flag
      // We basically use this typedef and cast our reference type to a different type then it really is
      typedef typename RefType::ReferencedType::RefLinkType::WeakType ChangedType;

      // Add ourself to the object's list of shared references
      // This will initialize our link and add it intrusively
      ref->Object->Referencers.WeakReferences.PushBack((ChangedType*)ref);
    }

    template <typename RefType>
    void RemoveLinkFromList(RefType* ref)
    {
      // This is a hack we have to do because member pointers are stupid and don't respect the mutable flag
      // We basically use this typedef and cast our reference type to a different type then it really is
      typedef typename RefType::ReferencedType::RefLinkType::WeakType ChangedType;

      // Unlink ourselves from the list
      InList<ChangedType, &ChangedType::InternalLink>::Unlink((ChangedType*)ref);
    }
  };

  class ZeroShared NormalPolicy
  {
  public:
    template <typename RefType>
    void AddLinkToList(RefType* ref)
    {
      // This is a hack we have to do because member pointers are stupid and don't respect the mutable flag
      // We basically use this typedef and cast our reference type to a different type then it really is
      typedef typename RefType::ReferencedType::RefLinkType::SharedType ChangedRefType;
      typedef typename RefType::ReferencedType::RefLinkType::ForType ChangedType;

      ChangedRefType* changedRef = (ChangedRefType*)ref;

      // Add ourself to the object's list of shared references
      // This will initialize our link and add it intrusively
      ref->Object->Referencers.SharedReferences.PushBack(changedRef);
    }

    template <typename RefType>
    void RemoveLinkFromList(RefType* ref)
    {
      // This is a hack we have to do because member pointers are stupid and don't respect the mutable flag
      // We basically use this typedef and cast our reference type to a different type then it really is
      typedef typename RefType::ReferencedType::RefLinkType::SharedType ChangedType;

      // Unlink ourselves from the list
      InList<ChangedType, &ChangedType::InternalLink>::Unlink((ChangedType*)ref);

      // If nobody else is referencing this object...
      if (ref->Object->Referencers.SharedReferences.Empty())
      {
        // Get a range so we can walk over all weak references
        ZilchAutoVal(weakRefs, ref->Object->Referencers.WeakReferences.All());

        // Loop until we run out of weak references
        while (weakRefs.Empty() == false)
        {
          // Get the current weak reference and iterate to the next one
          ZilchAutoRef(weakRef, weakRefs.Front());
          weakRefs.PopFront();

          // Set the weak reference's object to null
          weakRef.Object = nullptr;
        }

        // Clear out the weak reference list
        ref->Object->Referencers.WeakReferences.Clear();

        // Invoke the deletor on the object
        ref->Deletor(ref->Object);
      }
    }
  };

  // The virtual type is used primarily for debugging
  class ZeroShared VirtualType
  {
  public:
    virtual ~VirtualType() { }
  };

  // A shared reference to a particular object
  // The shared reference requires that your class Contains a
  // 'ZilchRefLink(YourClass);' member, which is intrusively
  // used as the reference count. This approach decreases memory
  // allocation and adds very little to the shared object itself
  template <typename Type, typename ModePolicy = NormalPolicy, typename DeletePolicy = StandardDelete<Type> >
  class ZeroSharedTemplate Ref
  {
  public:

    // Type-defines
    typedef Type ReferencedType;
    
    // Default constructor results in an empty reference
    Ref()
    {
      this->Object = nullptr;
    }

    // Construct the object that we are going to point to and pass it in
    Ref(Type* object, DeletePolicy deletor = DeletePolicy(), ModePolicy mode = ModePolicy())
    {
      // Store the deletor away so we can use it later
      this->Deletor = deletor;

      // Store away the mode policy
      this->Mode = mode;

      // Store the object
      this->Object = object;

      // If the object was a non null object
      if (object != nullptr)
      {
        // Add ourselves to whatever list we belong to (controlled by the policy)
        mode.AddLinkToList(this);
      }
    }

    // Copy constructor (for ourselves, apparently ours wont get called even with the template version below)
    Ref(const Ref& other)
    {
      // Copy over the information, and increment the
      // reference count since we now point at it
      CopyFrom(other);
    }

    // Move constructor
    Ref(Zero::MoveReference<Ref> move)
    {
      Ref& other = *move;
      this->Deletor = other.Deletor;
      this->Mode = other.Mode;
      CopyFrom<Type>(other);
      other.Clear();
    }

    // Move assignment
    Ref& operator=(Zero::MoveReference<Ref> move)
    {
      Ref& other = *move;
      this->Deletor = other.Deletor;
      this->Mode = other.Mode;
      CopyFrom<Type>(other);
      other.Clear();
      return *this;
    }
  
    // Copy constructor
    template <typename TypeOrDerived, typename OtherModePolicy>
    Ref(const Ref<TypeOrDerived, OtherModePolicy>& other)
    {
      // Copy over the information, and increment the
      // reference count since we now point at it
      CopyFrom(other);
    }
  
    // Destructor (decrements the reference count and potentially cleans up memory)
    ~Ref()
    {
      // Clear the object this is referencing (which also can delete the object)
      Clear();
    }

    // When hashed, we just return the function pointer hash
    size_t Hash() const
    {
      // Use pointer hashing
      return HashPolicy<Type*>()(this->Object);
    }

    // Assignment operator (for ourselves, apparently ours wont get called even with the template version below)
    Ref& operator=(const Ref& other)
    {
      return this->operator=<Type>(other);
    }

    // Assignment operator
    template <typename TypeOrDerived, typename OtherModePolicy>
    Ref& operator=(const Ref<TypeOrDerived, OtherModePolicy>& other)
    {
      // Check for assignment to the same object (or ourselves)
      if (this->Object == other.Object)
        return *this;

      // Clear the object we're referencing, since we're changing references
      Clear();

      // Copy over the information, and increment the
      // reference count since we now point at it
      CopyFrom(other);

      // Return ourself for chaining
      return *this;
    }

    // Compare two shared references
    template <typename TypeOrDerived, typename OtherModePolicy>
    bool operator==(const Ref<TypeOrDerived, OtherModePolicy>& other) const
    {
      return this->Object == other.Object;
    }

    // Compare the pointers
    bool operator==(const Type* other) const
    {
      return this->Object == other;
    }

    // Compare two shared references
    template <typename TypeOrDerived, typename OtherModePolicy>
    bool operator!=(const Ref<TypeOrDerived, OtherModePolicy>& other) const
    {
      return this->Object != other.Object;
    }

    // Compare the pointers
    bool operator!=(const Type* other) const
    {
      return this->Object != other;
    }

    // Return if we have an object or not
    inline operator Type*() const
    {
      return this->Object;
    }

    // Access the object via arrow operator
    inline Type* operator->() const
    {
      return this->Object;
    }

    // Dereference the shared reference to get the object
    inline Type& operator*() const
    {
      return *this->Object;
    }

    // Get the underlying object. This should only be used for optimization
    // and in cases where you know the object's lifetime is guaranteeed
    inline Type* GetObject() const
    {
      return this->Object;
    }

    // Clear the object this pointer is referencing
    void Clear()
    {
      // If the object is not null
      if (this->Object != nullptr)
      {
        // Unlink ourselves from the list (and potentially delete the object)
        this->Mode.RemoveLinkFromList(this);

        // Null out the object and the count
        this->Object = nullptr;
      }
    }

  public:

    // Copy over the information from another object and increment the reference to it
    inline void CopyFrom(const Ref& other)
    {
      // Straight up copy over the info we need
      this->Deletor = other.Deletor;
      this->Mode = other.Mode;
      CopyFrom<Type>(other);
    }

    // Copy over the information from another object and increment the reference to it
    template <typename TypeOrDerived, typename OtherModePolicy, typename OtherDeletePolicy>
    inline void CopyFrom(const Ref<TypeOrDerived, OtherModePolicy, OtherDeletePolicy>& other)
    {
      // Straight up copy over the info we need
      this->Object = other.Object;

      // Do we actually need to copy the deletor from derived to base?
      // this->Deletor = other.Deletor;

      // If the object was non null (as in we didn't just copy a null ref)
      if (this->Object != nullptr)
      {
        // Add ourselves to whatever list we belong to (controlled by the policy)
        this->Mode.AddLinkToList(this);
      }
    }

  public:

    // Store the deletion policy
    DeletePolicy Deletor;

    // The mode that this reference is in
    ModePolicy Mode;

    // A pointer back to the object
    Type* Object;

    // Store our intrusive link
    IntrusiveLink(Ref, InternalLink);
  };


  // This structure should be placed as a mutable member of your class
  template <typename Type, typename DeletePolicy = StandardDelete<Type> >
  class ZeroSharedTemplate RefLink
  {
  public:

    // Type-defines
    typedef Type ForType;
    typedef Ref<Type, NormalPolicy, DeletePolicy>         SharedType;
    typedef Ref<Type, WeakPolicy,   DeletePolicy>         WeakType;
    typedef InList<SharedType, &SharedType::InternalLink> SharedInList;
    typedef InList<WeakType, &WeakType::InternalLink>     WeakInList;

    // Store the list of shared references to our object
    SharedInList SharedReferences;

    // Store the list of weak references to our object
    // (they will not keep us alive, but we will null them out if we die)
    WeakInList WeakReferences;
  };

// A macro that creates an intrusive reference link
// This is typically placed at the end of your class
#define ZilchRefLink(type)                           \
  public:                                            \
  typedef ::Zilch::RefLink<const type> RefLinkType;  \
  mutable RefLinkType Referencers;

// If a particular object is going to be used as a referencer,
// we use a trick that lets the Visual Studio debugger show our
// object, but it needs to be virtual for the trick to work
#define ZilchDebuggableReferencer() \
  virtual void z() const {}
}

#endif
