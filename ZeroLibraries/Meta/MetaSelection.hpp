///////////////////////////////////////////////////////////////////////////////
///
/// \file MetaSelection.hpp
/// Declaration of the MetaSelection class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
/// Events
namespace Events
{
  DeclareEvent(SelectionChanged);
  DeclareEvent(SelectionFinal);
}

/// Sent with the SelectionChanged event
struct SelectionChangedEvent : public Event
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  SelectionChangedEvent() : Updated(false) {}
  MetaSelection* Selection;

  /// The two most common ways the selection changes is when the user
  /// selects new objects, or when an Archetype is rebuilt. When an Archetype
  /// rebuilds, it destroys the old object and places the new on in the same
  /// place in the selection. This bool will be set to true if this is the case.
  /// The reason this was added is because the SelectionHistory in the editor
  /// would advance on the selection final event, but if the event was just
  /// from an Archetype update, it shouldn't change the history.
  bool Updated;
};

DeclareEnum2(SendsEvents, False, True);

//---------------------------------------------------------------- MetaSelection
class MetaSelection : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  typedef Array<MetaSelection*> MetaSelectionArray;

  /// Constructor / Destructor.
  MetaSelection();
  virtual ~MetaSelection();

  bool IsEqual(MetaSelection* rhs);

  /// Primary object is the object that is selected for properties
  /// and the basis of transforming etc.
  Handle GetPrimary();
  void SetPrimary(HandleParam object);
  void SelectOnly(HandleParam object);
  template <typename type>
  type* GetPrimaryAs();

  /// Copy the other selection into this selection
  void Copy(MetaSelection& other);

  /// Add all objects from other selection into this selection
  void Add(MetaSelection& other);

  /// Adds a single object to the selection. Will ignore if it's a duplicate.
  void Add(HandleParam object);

  /// This exists because when adding a lot of objects to the selection,
  /// we want to wait until we're done adding them to send the final event.
  void Add(HandleParam object, SendsEvents::Enum sendsEvents);

  /// Attempts to remove the given object from the selection.
  void Remove(HandleParam object);

  /// Replaces one object in the selection with another
  void Replace(HandleParam toBeReplaced, HandleParam replacedWith);

  /// Is this object present in the selection?
  bool Contains(HandleParam object);

  /// Filter the selection for compositions with a particular component.
  template <typename type>
  void FilterComponentType(Array<type*>& destination);

  template <typename type>
  bool ContainsType();

  /// Clear all objects from the selection.
  void Clear();
  void Clear(SendsEvents::Enum sendsEvents);

  /// Count the maximum number of objects in the selection.
  /// May be less objects to due to dead ids.
  uint Count();

  /// Returns whether or not the Count is 0.
  bool Empty();

  /// Dispatches Events::SelectionChanged on ourself.
  void SelectionChanged();
  /// Should be called when all modifications are made to the selection.
  void FinalSelectionChanged();
  /// Should be called when the instances in the selection were updated
  /// (when Archetypes are rebuilt). See the 'Updated' comment on the 
  /// 'SelectionChangedEvent' class.
  void FinalSelectionUpdated();

  //------------------------------------------------------------- SelectionRange
  typedef HashSet<Handle>::range SetRange;

  class range
  {
  public:
    typedef const Handle& FrontResult;
    range();
    range(SetRange range);

    /// Range implementation.
    const Handle& Front();
    void PopFront();
    bool Empty();

  private:
    /// Finds the next valid object in the range and updates mCurrent.
    void FindNextValidId();

    SetRange mRange;
    Handle mCurrent;
  };

  //--------------------------------------------------------- SelectionRangeType
  template <typename type>
  class rangeType
  {
  public:
    typedef type*& FrontResult;

    rangeType();
    rangeType(SetRange range);

    /// Range implementation.
    type*& Front();
    void PopFront();
    bool Empty();

    /// Finds the next valid object in the range and updates mCurrent.
    void FindNextValidObject();

    SetRange mRange;
    type* mCurrent;
  };
  ///Finds the first object in the selection that Contains the given component type.
  template <typename CompositionType, typename ComponentType>
  ComponentType* FindInSelection()
  {
    rangeType<CompositionType> range = AllOfType<CompositionType>();

    while (range.Empty() == false)
    {
      CompositionType* selected = range.Front();
      range.PopFront();

      // If the object is valid...
      if (selected != nullptr)
      {
        // If this object has the component type we're looking for then return it
        ComponentType* component = selected->has(ComponentType);
        if (component != nullptr)
          return component;
      }
    }
    return nullptr;
  }

  /// Get all the objects in the selection.
  range All();

  /// Get all the objects of the given type.
  template <typename type>
  rangeType<type> AllOfType();

  static MetaSelectionArray::range GetAllSelections();
  static void ReplaceInAllSelections(Object* oldObject, Object* newObject);
  static void RemoveObjectFromAllSelections(Object* object);

protected:
  void FindNewPrimary();

  /// The primary selected object. The primary is also inserted into the
  /// selected objects set.
  Handle mPrimary;
  HashSet<Handle> mSelectedObjects;
public:
  static MetaSelectionArray sSelections;
};

typedef MetaSelection::rangeType<Handle> MetaRangeType;

//******************************************************************************
template<typename type>
void MetaSelection::FilterComponentType(Array<type*>& destination)
{
  // The maximum amount of object we could have
  destination.Reserve(mSelectedObjects.Size());

  // The type of the component we're looking for
  BoundType* componentType = ZilchTypeId(Type);

  // Walk all objects in the selection
  forRange(Handle object, All())
  {
    // Look up the component on the object
    MetaComposition* composition = object.StoredType->HasInherited<MetaComposition>();

    // The selection can include things in the library view
    // (such as archetypes) which don't have a valid composition
    if(composition == NULL)
      continue;

    Handle component = composition->GetComponentId(object.Get<Object>(), componentType);

    // Add it if it's valid
    if(component.IsNotNull())
      destination.PushBack(component.Get<type*>());
  }
}

//******************************************************************************
template <typename type>
bool MetaSelection::ContainsType()
{
  rangeType<type> r = AllOfType<type>();
  return !r.Empty();
}

//******************************************************************************
template<typename type>
type* MetaSelection::GetPrimaryAs()
{
  return GetPrimary().Get<type*>();
}

//----------------------------------------------------------- SelectionRangeType
//******************************************************************************
template <typename type>
MetaSelection::rangeType<type>::rangeType(SetRange range)
  : mRange(range)
{
  FindNextValidObject();
}

//******************************************************************************
template <typename type>
type*& MetaSelection::rangeType<type>::Front()
{
  return mCurrent;
}

//******************************************************************************
template <typename type>
void MetaSelection::rangeType<type>::PopFront()
{
  FindNextValidObject();
}

//******************************************************************************
template <typename type>
bool MetaSelection::rangeType<type>::Empty()
{
  return mCurrent == nullptr;
}

//******************************************************************************
template <typename type>
void MetaSelection::rangeType<type>::FindNextValidObject()
{
  mCurrent = nullptr;

  while(!mRange.Empty())
  {
    // Attempt to resolve the handle
    Handle& instance = mRange.Front();

    // Step forward
    mRange.PopFront();

    mCurrent = instance.Get<type*>();
    if (mCurrent)
      break;
  }
}

//******************************************************************************
template <typename type>
MetaSelection::rangeType<type> MetaSelection::AllOfType()
{
  return rangeType<type>(mSelectedObjects.All());
}

}//namespace Zero
