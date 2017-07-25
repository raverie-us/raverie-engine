/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_COMPOSITION_HPP
#define ZILCH_COMPOSITION_HPP

namespace Zilch
{
  template <typename T>
  class ComponentRange;

  // Allows the user to attach any type to a Zilch primitive
  // The primitive must be default constructable
  class ZeroShared Composition : public IZilchObject
  {
  public:
    Composition();
    ~Composition();

    // When adding, this will overwrite any component that already exists by the same type
    // The last added component will overwrite any components in its place (as well as base types of the component)
    // Returns the composition so you can chain operations
    template <typename T>
    Composition* Add(const T* component)
    {
      Type* type = ZilchVirtualTypeId(component);
      
      do
      {
        // Map the component's most derived type all the way up to its root base type
        this->Components[type] = component;
        type = Type::GetBaseType(type);
      }
      while (type != nullptr);
      return this;
    }

    // Attempts to get a component by type, or creates it if it does not exist (expects a default constructor)
    // Warning, this may invalidate other references to components
    template <typename T>
    T* HasOrAdd()
    {
      // Attempt to get the component, and if we fail then create it immediately
      T* component = this->Has<T>();
      if (component == nullptr)
      {
        component = new T();
        this->Add(component);
      }
      return component;
    }

    // Attempts to get a component by type, or creates it if it does not exist (expects a default constructor)
    // Warning, this may invalidate other references to components (one parameter)
    template <typename T, typename A0>
    T* HasOrAdd(A0 a0)
    {
      // Attempt to get the component, and if we fail then create it immediately
      T* component = this->Has<T>();
      if (component == nullptr)
      {
        component = new T(a0);
        this->Add(component);
      }
      return component;
    }

    // Attempts to get a component by type, or creates it if it does not exist
    // Warning, this may invalidate other references to components
    template <typename T>
    T* Has()
    {
      // Grab component by type
      Handle* handle = this->Components.FindPointer(ZilchTypeId(T));
      if (handle == nullptr || handle->IsNull())
        return nullptr;

      return (T*)handle->Dereference();
    }

    // Attempts to get a component by type from anywhere up the base class hierarchy,
    // or creates it if it does not exist (expects a default constructor)
    // Warning, this may invalidate other references to components
    template <typename T>
    T* HasInheritedOrAdd()
    {
      // Attempt to get the component, and if we fail then create it immediately
      T* component = this->GetInherited<T>();
      if (component == nullptr)
      {
        component = new T();
        this->Add(new T());
      }
      return component;
    }

    // Attempts to get a component by type from anywhere up the
    // base class hierarchy, or creates it if it does not exist
    // Warning, this may invalidate other references to components
    template <typename T>
    T* HasInherited()
    {
      T* component = this->Has<T>();
      // If we didn't find a component, walk up the base class composition recursively (if we have one)
      if (component == nullptr)
      {
        if(Composition* baseComposition = this->GetBaseComposition())
          return baseComposition->HasInherited<T>();
      }
      return component;
    }

    // Removes a component and properly destructs it
    // Safe to call even if the object does not exist
    // Returns the composition so you can chain operations
    template <typename T>
    Composition* Remove()
    {
      // Walk up the base types and remove all the handles from the component map
      // Note: This may release the memory of the component if it is reference counted
      Type* type = ZilchTypeId(T);
      do
      {
        this->Components.Erase(type);
        type = Type::GetBaseType(type);
      }
      while (type != nullptr);
      return this;
    }

    // Returns a range of all components on this, and all base compositions
    template <typename T>
    ComponentRange<T> HasAll()
    {
      return ComponentRange<T>(this);
    }

    // Gets a pointer to the base class composition (must be set externally)
    // This returns null by default, unless overridden
    virtual Composition* GetBaseComposition();

    // Clear the map and release all handles to components
    void ClearComponents();

  private:
    // Store components by type id
    // We also store the destructors so that we can safely destruct
    HashMap<Type*, Handle> Components;
  };

  // Used to walk all components of a single type on an inheritance chain of compositions
  template <typename T>
  class ComponentRange
  {
  public:
    ComponentRange(Composition* currentComposition) :
      mCurrentComposition(currentComposition)
    {
      FindNextValid();
    }

    T* Front()
    {
      if(mCurrentComposition)
        return mCurrentComposition->Has<T>();
      return nullptr;
    }

    void PopFront()
    {
      // FindNextValid won't move forward if we're already on a valid composition,
      // so we want to move to our base before calling it
      mCurrentComposition = mCurrentComposition->GetBaseComposition();
      FindNextValid();
    }

    bool Empty()
    {
      return (mCurrentComposition == nullptr);
    }

  private:
    // Finds the next composition that has the component we're looking for
    void FindNextValid()
    {
      while(mCurrentComposition && mCurrentComposition->Has<T>() == nullptr)
      {
        mCurrentComposition = mCurrentComposition->GetBaseComposition();
      }
    }

    Composition* mCurrentComposition;
  };
}

#endif
