///////////////////////////////////////////////////////////////////////////////
///
/// \file MetaSelection.cpp
/// Implementation of the MetaSelection class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
  DefineEvent(SelectionChanged);
  DefineEvent(SelectionFinal);
}

ZilchDefineType(SelectionChangedEvent, builder, type)
{
}

//---------------------------------------------------------------- MetaSelection
ZilchDefineType(MetaSelection, builder, type)
{
  ZilchBindMethod(Count);
  ZilchBindMethod(Empty);
  ZilchFullBindMethod(builder, type, &MetaSelection::Clear, (void(MetaSelection::*)()), "Clear", ZilchNoNames);
  ZilchBindMethod(SelectOnly);
  ZilchFullBindMethod(builder, type, &MetaSelection::Add, (void(MetaSelection::*)(HandleParam)), "Add", ZilchNoNames);
  ZilchBindMethod(Remove);
  ZilchBindMethod(Replace);
  ZilchBindMethod(Contains);
  ZilchBindMethod(FinalSelectionChanged);
  ZilchBindMethod(FinalSelectionChanged);
  ZilchBindGetterSetterProperty(Primary);

  ZeroBindEvent(Events::SelectionChanged, Event);
  ZeroBindEvent(Events::SelectionFinal, Event);

  ZilchBindCustomGetter(All);
}

MetaSelection::MetaSelectionArray MetaSelection::sSelections;

//******************************************************************************
MetaSelection::MetaSelection()
{
  sSelections.PushBack(this);
}

//******************************************************************************
MetaSelection::~MetaSelection()
{
  sSelections.EraseValueError(this);
}

//******************************************************************************
bool MetaSelection::IsEqual(MetaSelection* rhs)
{
  if (mPrimary != rhs->mPrimary)
    return false;

  return (this->mSelectedObjects == rhs->mSelectedObjects);
}

//******************************************************************************
Handle MetaSelection::GetPrimary()
{
  // If the primary isn't valid, search all objects for another valid primary
  if (mPrimary.IsNull())
    FindNewPrimary();

  return mPrimary;
}

//******************************************************************************
void MetaSelection::SetPrimary(HandleParam object)
{
  // Do nothing if they're the same
  if (object == mPrimary)
    return;

  // Add the object, but don't send the even as we will after
  Add(object, SendsEvents::False);

  // Set the object as the primary
  mPrimary = object;

  // The selection has changed
  SelectionChanged();
}

//******************************************************************************
void MetaSelection::SelectOnly(HandleParam object)
{
  // We don't send events here because 'SetPrimaryObject' will send an event at the end
  Clear(SendsEvents::False);
  SetPrimary(object);
}

//******************************************************************************
void MetaSelection::Add(MetaSelection& other)
{
  forRange(const Handle& object, other.All())
    this->Add(object, SendsEvents::False);

  SelectionChanged();
}

//******************************************************************************
void MetaSelection::Copy(MetaSelection& other)
{
  Clear(SendsEvents::False);

  Add(other);

  mPrimary = other.mPrimary;

  SelectionChanged();
}

//******************************************************************************
void MetaSelection::Add(HandleParam object)
{
  if (!object.IsNull())
    Add(object, SendsEvents::True);
}

//******************************************************************************
void MetaSelection::Add(HandleParam object, SendsEvents::Enum sendsEvents)
{
  // Do nothing if it's already in the selection
  if (Contains(object))
    return;

  // Store the object as a handle
  mSelectedObjects.Insert(object);

  // Send the event if specified
  if (sendsEvents == SendsEvents::True)
    SelectionChanged();
}

//******************************************************************************
void MetaSelection::Remove(HandleParam object)
{
  // Do nothing if the object isn't in the selection
  if (!Contains(object))
    return;

  // Remove the object
  mSelectedObjects.Erase(object);

  // Check if it was the primary
  if (mPrimary == object)
  {
    mPrimary = Handle();
    FindNewPrimary();
  }

  // The selection has changed
  SelectionChanged();
}

//******************************************************************************
bool MetaSelection::Contains(HandleParam object)
{
  // Attempt to find it in the handle map
  return mSelectedObjects.FindPointer(object) != NULL;
}

//******************************************************************************
void MetaSelection::Clear()
{
  Clear(SendsEvents::True);
}

//******************************************************************************
void MetaSelection::Clear(SendsEvents::Enum sendsEvents)
{
  // We don't want to send out an event if it's already cleared
  if (Empty())
    return;

  // Clear the selection
  mSelectedObjects.Clear();
  mPrimary.Clear();

  // The selection has changed
  if (sendsEvents == SendsEvents::True)
    SelectionChanged();
}

//******************************************************************************
uint MetaSelection::Count()
{
  return mSelectedObjects.Size();
}

//******************************************************************************
bool MetaSelection::Empty()
{
  return mSelectedObjects.Empty();
}

//******************************************************************************
void MetaSelection::SelectionChanged()
{
  // Dispatch an event on ourself
  SelectionChangedEvent event;
  event.Selection = this;
  DispatchEvent(Events::SelectionChanged, &event);
}

//******************************************************************************
void MetaSelection::FinalSelectionChanged()
{
  // Dispatch an event on ourself
  SelectionChangedEvent event;
  event.Selection = this;
  DispatchEvent(Events::SelectionFinal, &event);
}

//******************************************************************************
void MetaSelection::FinalSelectionUpdated()
{
  // Dispatch an event on ourself
  SelectionChangedEvent event;
  event.Selection = this;
  event.Updated = true;
  DispatchEvent(Events::SelectionFinal, &event);
}

//******************************************************************************
void MetaSelection::Replace(HandleParam toBeReplaced, HandleParam replacedWith)
{
  // Replace the primary if it was the primary
  if (mPrimary == toBeReplaced)
    mPrimary = replacedWith;

  mSelectedObjects.Erase(toBeReplaced);
  mSelectedObjects.Insert(replacedWith);
}

//******************************************************************************
MetaSelection::MetaSelectionArray::range MetaSelection::GetAllSelections()
{
  return sSelections.All();
}

//******************************************************************************
void MetaSelection::ReplaceInAllSelections(Object* oldObject, Object* newObject)
{
  forRange(MetaSelection* selection, MetaSelection::GetAllSelections())
  {
    if (selection->Contains(oldObject))
    {
      selection->Replace(oldObject, newObject);
      selection->FinalSelectionUpdated();
    }
  }
}

//******************************************************************************
void MetaSelection::RemoveObjectFromAllSelections(Object* object)
{
  forRange(MetaSelection* selection, MetaSelection::GetAllSelections())
  {
    if (selection->Contains(object))
    {
      selection->Remove(object);
      selection->FinalSelectionUpdated();
    }
  }
}

//******************************************************************************
void MetaSelection::FindNewPrimary()
{
  if (mPrimary.IsNull())
  {
    range r = All();
    if (!r.Empty())
      mPrimary = r.Front();
  }
}

//--------------------------------------------------------------- SelectionRange
//******************************************************************************
MetaSelection::range::range()
{

}

//******************************************************************************
MetaSelection::range::range(SetRange range)
  : mRange(range)
{
  FindNextValidId();
}

//******************************************************************************
const Handle& MetaSelection::range::Front()
{
  return mCurrent;
}

//******************************************************************************
void MetaSelection::range::PopFront()
{
  FindNextValidId();
}

//******************************************************************************
bool MetaSelection::range::Empty()
{
  return mCurrent.IsNull();
}

//******************************************************************************
void MetaSelection::range::FindNextValidId()
{
  mCurrent.Clear();

  while (!mRange.Empty())
  {
    /* Attempt to resolve the handle */
    mCurrent = mRange.Front();

    /* Step forward */
    mRange.PopFront();

    /* If we've found a valid object, we're done */
    if (mCurrent.IsNull() == false)
      break;
  }
}

//******************************************************************************
MetaSelection::range MetaSelection::All()
{
  return range(mSelectedObjects.All());
}

}//namespace Zero
