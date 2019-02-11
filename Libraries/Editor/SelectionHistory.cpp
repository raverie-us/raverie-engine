// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

SelectionHistory::SelectionHistory()
{
  mLocked = false;
  mCurrent = nullptr;
}

SelectionHistory::~SelectionHistory()
{
  SafeDelete(mCurrent);
  DeleteObjectsInContainer(mPrevious);
  DeleteObjectsInContainer(mNext);
}

bool IsEqual(MetaSelection* selection, HandleParam object)
{
  // If the given object is a selection, compare the contents of the selections
  if (MetaSelection* objectSelection = object.Get<MetaSelection*>())
    return selection->IsEqual(objectSelection);

  // If there's only one object in the selection, compare it with the given obj
  if (selection->Count() == 1)
    return selection->GetPrimary() == object;

  return false;
}

MetaSelection* SelectionHistory::Advance(HandleParam object)
{
  if (mLocked)
    return nullptr;

  if (mCurrent)
  {
    // If the objects are the same, we don't want to do anything
    if (IsEqual(mCurrent, object))
      return nullptr;

    // If the current selection is empty, don't add it to the history
    if (mCurrent->Empty())
      delete mCurrent;
    else
      mPrevious.PushBack(mCurrent);
  }
  DeleteObjectsInContainer(mNext);

  MetaSelection* localSelection = new MetaSelection();

  // If it's already a selection, just copy it over. Otherwise, create
  // a selection for the new object
  if (MetaSelection* selection = object.Get<MetaSelection*>())
    localSelection->Copy(*selection, SendsEvents::False);
  else
    localSelection->SelectOnly(object);

  mCurrent = localSelection;
  return mCurrent;
}

void SelectionHistory::ShowObject()
{
  if (!mCurrent)
    return;

  Handle primary = mCurrent->GetPrimary();
  if (Cog* cog = primary.Get<Cog*>())
  {
    Space* space = cog->GetSpace();
    Z::gEditor->mManager->ShowWidgetWith(space);
    FocusOnSelectedObjects();
  }
}

void SelectionHistory::Reselect()
{
  MovedToObject(mCurrent);
}

void SelectionHistory::Clear()
{
  SafeDelete(mCurrent);
  DeleteObjectsInContainer(mPrevious);
  DeleteObjectsInContainer(mNext);
}

void SelectionHistory::MovedToObject(MetaSelection* selection)
{
  // Prevent selection changes from moving history
  // when history is moving it does not want to create loops
  mLocked = true;

  Z::gEditor->mSelection->Copy(*selection, SendsEvents::False);
  Z::gEditor->mSelection->FinalSelectionChanged();

  mLocked = false;
}

void SelectionHistory::Next()
{
  if (!mNext.Empty())
  {
    if (mCurrent)
      mPrevious.PushBack(mCurrent);

    MetaSelection* selection = mNext.Back();
    mNext.PopBack();

    mCurrent = selection;
    MovedToObject(selection);
  }
}

void SelectionHistory::Previous()
{
  if (!mPrevious.Empty())
  {
    if (mCurrent)
      mNext.PushBack(mCurrent);

    MetaSelection* selection = mPrevious.Back();
    mPrevious.PopBack();

    mCurrent = selection;
    MovedToObject(selection);
  }
}

} // namespace Zero
