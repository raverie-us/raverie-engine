///////////////////////////////////////////////////////////////////////////////
///
/// \file MainPropertyView.cpp
/// 
/// 
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------- Main Property View
ZilchDefineType(MainPropertyView, builder, type)
{
}

//******************************************************************************
MainPropertyView::MainPropertyView(Composite* parent, MetaSelection* selection,
                                   OperationQueue* queue)
  : Composite(parent)
{
  // The name of this widget
  SetName("Properties");

  mPrimaryOpQueue = queue;
  mUndoInterface = new PropertyToUndo(mPrimaryOpQueue);
  mMultiInterface = new MultiPropertyInterface(mPrimaryOpQueue, selection);

  mLocalOpQueue = new OperationQueue();

  mSelectionHistory = new SelectionHistory();

  // Default size
  SetSize(Pixels(280, 600));

  this->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0,5), Thickness::cZero));

  // Create the row of buttons up top
  mButtonRow = new Composite(this);
  mButtonRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(2,0), Thickness(1, 0, 0, 0)));
  {
    // Previous button
    IconButton* previousButton = new IconButton(mButtonRow);
    previousButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
    previousButton->SetIcon("PreviousObject");
    previousButton->SetToolTip("Previous Selection");
    ConnectThisTo(previousButton, Events::ButtonPressed, OnPreviousPressed);

    // Next button
    IconButton* nextButton = new IconButton(mButtonRow);
    nextButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
    nextButton->SetIcon("NextObject");
    nextButton->SetToolTip("Next Selection");
    ConnectThisTo(nextButton, Events::ButtonPressed, OnNextPressed);

    // Show button
    IconButton* showButton = new IconButton(mButtonRow);
    showButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
    showButton->SetIcon("ShowObject");
    showButton->SetToolTip("Show Selection");
    ConnectThisTo(showButton, Events::ButtonPressed, OnShowPressed);

    // Recent button
    IconButton* recentButton = new IconButton(mButtonRow);
    recentButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
    recentButton->SetIcon("RecentObjects");
    recentButton->SetToolTip("Recent..");
    ConnectThisTo(recentButton, Events::ButtonPressed, OnRecentPressed);
  }

  // Create the property view
  mPropertyView = new PropertyView(this);
  mPropertyView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 80);

  // Create a splitter between the property view and preview area
  mSplitter = new Splitter(this);
  mSplitter->mAxis = SizeAxis::Y;

  // Create the preview area
  mPreviewArea = new Composite(this);
  mPreviewArea->SetName("PreviewArea");
  mPreviewArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mPreviewArea->SetLayout(CreateRatioLayout());

  // Start with the preview area inactive
  ClearPreview();

  ConnectThisTo(selection, Events::SelectionChanged, OnSelectionChanged);
  ConnectThisTo(selection, Events::SelectionFinal, OnSelectionFinal);
  ConnectThisTo(ArchetypeManager::GetInstance(), Events::ResourceReload, OnArchetypeReload);

  mSpecialEdit = new MetaSelection();
}

//******************************************************************************
MainPropertyView::~MainPropertyView()
{
  SafeDelete(mUndoInterface);
  SafeDelete(mMultiInterface);
  SafeDelete(mLocalOpQueue);
  SafeDelete(mSelectionHistory);
}

//******************************************************************************
void MainPropertyView::EditObject(HandleParam object, bool advanceHistory)
{
  mUndoInterface->mOperationQueue = mPrimaryOpQueue;
  mSpecialEdit->Clear();

  // Advance the selection history if specified
  if(advanceHistory)
    mSelectionHistory->Advance(object);

  // If it is a resource, we want to enable the resource preview UI
  if(object.Get<Resource*>())
  {
    EditResource(object);
    return;
  }

  // If it's a meta selection, we want to display the primary object if
  // there's only one object, or enable multi edit if there are more than one
  if(MetaSelection* selection = object.Get<MetaSelection*>())
  {
    // Special logic for if we're multi-selecting resources
    if(selection->ContainsType<Resource>())
    {
      EditResources(selection);
      return;
    }
    else
    {
      // Set the selection on the multi property interface
      mMultiInterface->mSelection = selection;

      // Set the object
      if(selection->Count() > 1)
        mPropertyView->SetObject(selection, mMultiInterface);
      else
        mPropertyView->SetObject(selection->GetPrimaryAs<Object>(), mUndoInterface);
    }
  }
  else
  {
    // Just set the single object
    mPropertyView->SetObject(object, mUndoInterface);
  }

  // Remove the resource buttons
  DestroyResourceButtons();
  ClearPreview();
}

//******************************************************************************
void MainPropertyView::Refresh()
{
  Handle object = mPropertyView->GetObject();
  if(ShouldDisplayNetPropertyIcon(object))
    mPropertyView->AddCustomPropertyIcon(CreateNetPropertyIcon, mPropertyView);
  else
    mPropertyView->RemoveCustomPropertyIcon(CreateNetPropertyIcon);

  mPropertyView->Refresh();
}

//******************************************************************************
void MainPropertyView::HardReset()
{
  ClearPreview();
  mSelectionHistory->Clear();
  mPropertyView->SetObject(Handle());
}

//******************************************************************************
SelectionHistory* MainPropertyView::GetHistory()
{
  return mSelectionHistory;
}

//******************************************************************************
PropertyView* MainPropertyView::GetPropertyView()
{
  return mPropertyView;
}

//******************************************************************************
void MainPropertyView::OnSelectionChanged(SelectionChangedEvent* e)
{
  if(!e->Selection->Empty())
    Z::gEditor->ShowWindow("Properties");
}

//******************************************************************************
void MainPropertyView::OnSelectionFinal(SelectionChangedEvent* e)
{
  // We don't want to advance the selection history if the selection was just
  // updated from an Archetype rebuild
  EditObject(e->Selection, !e->Updated);
  if(!e->Selection->Empty())
    Z::gEditor->ShowWindow("Properties");
}

//******************************************************************************
void MainPropertyView::OnArchetypeReload(ResourceEvent* event)
{
  // Re-create the preview for the given archetype if it was the one
  // we were modifying
  if(mSelectionHistory->mCurrent)
  {
    Resource* resource = event->EventResource;
    Handle primary = mSelectionHistory->mCurrent->GetPrimaryAs<Object>();
    if(primary == resource)
    {
      Handle instance = PreviewResource(resource);
      mPropertyView->SetObject(instance, mUndoInterface);
    }
  }
}

//******************************************************************************
void MainPropertyView::OnPreviousPressed(Event* e)
{
  mSelectionHistory->Previous();
}

//******************************************************************************
void MainPropertyView::OnNextPressed(Event* e)
{
  mSelectionHistory->Next();
}

//******************************************************************************
void MainPropertyView::OnShowPressed(Event* e)
{
  mSelectionHistory->ShowObject();
}

//******************************************************************************
void MainPropertyView::OnRecentPressed(Event* e)
{
  mSelectionHistory->OnRecent(mButtonRow);
}

//******************************************************************************
void MainPropertyView::OnExternalEdit(Event* e)
{
  MetaSelection* selection = mSelectionHistory->mCurrent;
  if(selection == nullptr)
    return;

  // Open each resource with an external editor
  forRange(Resource* resource, selection->AllOfType<Resource>())
  {
    EditResourceExternal(resource);
  }
}

//******************************************************************************
void MainPropertyView::OnReloadContent(Event* e)
{
  MetaSelection* selection = mSelectionHistory->mCurrent;
  if(selection == nullptr)
    return;

  // Reload each resource
  forRange(Resource* resource, selection->AllOfType<Resource>())
  {
    ReloadResource(resource);
  }

  // Reselect the resource
  Z::gEditor->mMainPropertyView->GetHistory()->Reselect();

}

//******************************************************************************
void MainPropertyView::EditResource(HandleParam object)
{
  Resource* resource = object.Get<Resource*>(GetOptions::AssertOnNull);

  // Preview the object
  Handle editObject = PreviewResource(object);

  if(object.Get<Archetype*>())
  {
    // We want to use a local operation queue for editing the archetype object
    mLocalOpQueue->ClearAll();
    mUndoInterface->mOperationQueue = mLocalOpQueue;

    // Edit the archetype object in the preview
    mPropertyView->SetObject(editObject, mUndoInterface);
  }
  //for some reason the resource is occasionally null, for now just guard against it
  else if(resource != nullptr)
  {
    // The object we're editing should either be the resource itself,
    // of whatever the content item specifies should be edited
    ContentItem* contentItem = resource->mContentItem;
    Object* editingObject = resource;
    if(contentItem != nullptr)
      editingObject = contentItem->GetEditingObject(resource);

    // Edit the final object
    mPropertyView->SetObject(editingObject, mUndoInterface);
  }

  AddResourceButtons();
}

//******************************************************************************
void MainPropertyView::EditResources(MetaSelection* selection)
{
  // If there's only one resource in the selection, no need for the extra logic
  if(selection->Count() == 1)
  {
    EditResource(selection->GetPrimaryAs<Object>());
    return;
  }

  // Add the resource buttons back
  AddResourceButtons();

  // We're going to display all selected objects in a group preview
  mPreviewTile.SafeDestroy();
  PreviewWidgetGroup* group = new PreviewWidgetGroup(mPreviewArea);
  mPreviewTile = group;

  // Maximum amount of items to display
  const uint cMaxItems = 9;
  uint count = 0;

  // Add all resources
  forRange(Resource* resource, selection->AllOfType<Resource>())
  {
    bool addedSpecialEdit = false;

    if(count < cMaxItems)
    {
      // Create the preview widget
      PreviewWidget* preview = group->AddPreviewWidget(resource->Name, resource,
                                                       PreviewImportance::High);

      // If it's a resource, we want to edit the object from the preview
      if(preview && ZilchVirtualTypeId(resource)->IsA(ZilchTypeId(Archetype)))
      {
        mSpecialEdit->Add(preview->GetEditObject());
        addedSpecialEdit = true;
      }
    }

    ++count;

    // If it was an archetype, the special object was already added to the selection
    if(!addedSpecialEdit)
    {
      // Get the object we should be editing from the content item
      Object* editingObject = resource;
      if(resource->mContentItem != nullptr)
        editingObject = resource->mContentItem->GetEditingObject(resource);

      // Add the object
      mSpecialEdit->Add(editingObject);
    }
  }

  // Activate the splitter and preview areas
  mSplitter->SetActive(true);
  mPreviewArea->SetActive(true);
  group->SetSize(group->GetMinSize());

  mPreviewTile = group;

  // 
  MetaSelection* selectionToEdit = selection;
  if(mSpecialEdit->Count() > 0)
    selectionToEdit = mSpecialEdit;

  mMultiInterface->mSelection = selectionToEdit;
  mPropertyView->SetObject(selectionToEdit, mMultiInterface);
}

//******************************************************************************
void MainPropertyView::AddResourceButtons()
{
  if(mExternalEditButton)
    return;

  // External edit button
  IconButton* externalEditButton = new IconButton(mButtonRow);
  externalEditButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
  externalEditButton->SetIcon("ExternalEdit");
  externalEditButton->SetToolTip("External Edit");
  ConnectThisTo(externalEditButton, Events::ButtonPressed, OnExternalEdit);
  mExternalEditButton = externalEditButton;

  // Reload button
  IconButton* reloadButton = new IconButton(mButtonRow);
  reloadButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(40));
  reloadButton->SetIcon("ReloadContent");
  reloadButton->SetToolTip("Reload Content");
  ConnectThisTo(reloadButton, Events::ButtonPressed, OnReloadContent);
  mReloadResourceButton = reloadButton;

  mButtonRow->MarkAsNeedsUpdate();
}

//******************************************************************************
void MainPropertyView::DestroyResourceButtons()
{
  mExternalEditButton.SafeDestroy();
  mReloadResourceButton.SafeDestroy();
}

//******************************************************************************
Handle MainPropertyView::PreviewResource(HandleParam object)
{
  // Clear any old preview object
  ClearPreview();

  Resource* resource = object.Get<Resource*>();
  if(resource == nullptr)
    return Handle();

  // Create the preview widget
  PreviewWidget* tile = ResourcePreview::CreatePreviewWidget(mPreviewArea, resource->Name, resource, PreviewImportance::High);
  mPreviewTile = tile;

  if (tile == nullptr)
    return Handle();

  // We want it to be active
  tile->AnimatePreview(PreviewAnimate::Always);

  // Activate the splitter and preview areas
  mSplitter->SetActive(true);
  mPreviewArea->SetActive(true);

  return tile->GetEditObject();
}

//******************************************************************************
void MainPropertyView::ClearPreview()
{
  this->MarkAsNeedsUpdate();
  mPreviewTile.SafeDestroy();
  mSplitter->SetActive(false);
  mPreviewArea->SetActive(false);
}

}//namespace Zero
