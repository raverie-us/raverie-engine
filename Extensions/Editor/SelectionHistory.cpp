///////////////////////////////////////////////////////////////////////////////
///
/// \file SelectionHistory.hpp
/// 
/// 
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
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
  if(MetaSelection* objectSelection = object.Get<MetaSelection*>())
    return selection->IsEqual(objectSelection);

  // If there's only one object in the selection, compare it with the given obj
  if(selection->Count() == 1)
    return selection->GetPrimary() == object;

  return false;
}

MetaSelection* SelectionHistory::Advance(HandleParam object)
{
  if(mLocked)
    return nullptr;

  if(mCurrent)
  {
    // If the objects are the same, we don't want to do anything
    if(IsEqual(mCurrent, object))
      return nullptr;

    // If the current selection is empty, don't add it to the history
    if(mCurrent->Empty())
      delete mCurrent;
    else
      mPrevious.PushBack(mCurrent);
  }
  DeleteObjectsInContainer(mNext);

  MetaSelection* localSelection = new MetaSelection();

  // If it's already a selection, just copy it over. Otherwise, create
  // a selection for the new object
  if(MetaSelection* selection = object.Get<MetaSelection*>())
    localSelection->Copy(*selection);
  else
    localSelection->SelectOnly(object);

  mCurrent = localSelection;
  return mCurrent;
}

void SelectionHistory::ShowObject()
{
  if(!mCurrent)
    return;

  Handle primary = mCurrent->GetPrimary();
  if(Cog* cog = primary.Get<Cog*>())
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

  Z::gEditor->mSelection->Copy(*selection);
  Z::gEditor->mSelection->FinalSelectionChanged();

  mLocked = false;
}

void SelectionHistory::Next()
{
  if(!mNext.Empty())
  {
    if(mCurrent)
      mPrevious.PushBack(mCurrent);

    MetaSelection* selection = mNext.Back();
    mNext.PopBack();

    mCurrent = selection;
    MovedToObject(selection);
  }
}

void SelectionHistory::Previous()
{
  if(!mPrevious.Empty())
  {
    if(mCurrent)
      mNext.PushBack(mCurrent);

    MetaSelection* selection = mPrevious.Back();
    mPrevious.PopBack();

    mCurrent = selection;
    MovedToObject(selection);
  }
}

class SelectionSearch : public SearchProvider
{
public:
  MetaSelection mSelection;
  HandleOf<Widget> mDestroy;
  SelectionHistory* mHistory;

  SelectionSearch(SelectionHistory* history)
    : mHistory(history)
  {

  }

  void Search(SearchData& search) override
  {
    forRange(Handle object, mSelection.All())
    {
      String text;
      if (MetaDisplay* display = object.StoredType->HasInherited<MetaDisplay>())
        text = display->GetName(object);
      else
        text = object.StoredType->Name;

      int priority = PartialMatch(search.SearchString.All(), text.All(), CaseInsensitiveCompare);
      if(priority != cNoMatch)
      {
        //Add a match
        SearchViewResult& result = search.Results.PushBack();
        result.ObjectHandle = object;
        result.Interface = this;
        result.Name = text;
        result.Priority = 0;
      }
    }
  }

  bool OnMatch(SearchView* searchView, SearchViewResult& element)
  {
    mDestroy.SafeDestroy();
    Handle object = element.ObjectHandle;

    MetaSelection* selection = mHistory->Advance(object);
    if(selection)
      mHistory->MovedToObject(selection);
    return true;
  }

  String GetType(SearchViewResult& element) override
  {
    return element.ObjectHandle.StoredType->Name;
  }

  Composite* CreatePreview(Composite* parent, SearchViewResult& element) override
  {
    Handle instance = element.ObjectHandle;

    if(instance.IsNotNull())
      return ResourcePreview::CreatePreviewWidget(parent, element.Name, instance);
    else
      return nullptr;
  }
};

void SelectionHistory::OnRecent(Composite* parent)
{
  FloatingSearchView* viewPopUp = new FloatingSearchView(parent);
  Vec3 mousePos = ToVector3(Z::gMouse->GetClientPosition());
  SearchView* searchView = viewPopUp->mView;
  viewPopUp->SetSize(Pixels(300,400));
  viewPopUp->ShiftOntoScreen(mousePos);
  viewPopUp->UpdateTransformExternal();

  SelectionSearch* selectionSearch = new SelectionSearch(this);
  selectionSearch->mDestroy = viewPopUp;

  forRange(MetaSelection* selection, mNext.All())
    selectionSearch->mSelection.Add(*selection);
  forRange(MetaSelection* selection, mPrevious.All())
    selectionSearch->mSelection.Add(*selection);
  if(mCurrent)
    selectionSearch->mSelection.Add(*mCurrent);

  searchView->mSearch->SearchProviders.PushBack(selectionSearch) ;
  searchView->TakeFocus();
  viewPopUp->UpdateTransformExternal();
  searchView->Search(String());
}

}//namespace Zero
