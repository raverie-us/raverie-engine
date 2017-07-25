///////////////////////////////////////////////////////////////////////////////
///
/// \file ResourceTableEditor.cpp
/// Implementation of the SearcahableResourceTextBox, WeightedTableBar,
/// ResourceWeightedTableView ResourceTableTreeView and
/// ResourceTableEditor classes.
/// 
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ResourceTableEntryChangeOp
ResourceTableEntryChangeOp::ResourceTableEntryChangeOp(ResourceTableEditor* editor, ResourceTableEntry* oldEntry, const ResourceTable::ValueType& newValue)
{
  mName = "ResourceTableEntryChange";

  mEditor = editor;
  mEndEntry = mStartEntry = *oldEntry;
  mEndEntry.mValue = newValue;
  mIndex = mEditor->mTable->mEntryList.FindIndex(oldEntry);
}

ResourceTableEntryChangeOp::ResourceTableEntryChangeOp(ResourceTableEditor* editor, ResourceTableEntry* oldEntry)
{
  mName = "ResourceTableEntryChange";

  mEditor = editor;
  mEndEntry = mStartEntry = *oldEntry;
  mIndex = mEditor->mTable->mEntryList.FindIndex(oldEntry);
}

void ResourceTableEntryChangeOp::Undo()
{
  mEditor->mTable->Set(mIndex, &mStartEntry);
  mEditor->Rebuild();
}

void ResourceTableEntryChangeOp::Redo()
{
  mEditor->mTable->Set(mIndex, &mEndEntry);
  mEditor->Rebuild();
}

//-------------------------------------------------------------------ResourceTableMaxWeightChangeOp
ResourceTableMaxWeightChangeOp::ResourceTableMaxWeightChangeOp(ResourceTableEditor* editor, float newMaxWeight)
{
  mName = "ResourceTableMaxWeightChange";

  mEditor = editor;
  mNewMaxWeight = newMaxWeight;
  mOldMaxWeight = mEditor->mTable->mMaxWeight;

  //cache all of the old weights (since some may be clipped to the max weight after doing the op)
  ResourceTable& table = mEditor->mTable;
  mOldWeights.Resize(table.Size());
  for(uint i = 0; i < table.Size(); ++i)
    mOldWeights[i] = table[i]->mWeight;
}

void ResourceTableMaxWeightChangeOp::Undo()
{
  mEditor->mTable->SetMaxWeight(mOldMaxWeight);

  //restore all of the old weights
  for(uint i = 0; i < mEditor->mTable->Size(); ++i)
    mEditor->mTable->mEntryList[i]->mWeight = mOldWeights[i];

  mEditor->Rebuild();
  mEditor->mMaxWeightTextBox->SetText(String::Format("%g", mOldMaxWeight));
}

void ResourceTableMaxWeightChangeOp::Redo()
{
  //set the new max weight (which will clip any values if needed)
  mEditor->mTable->SetMaxWeight(mNewMaxWeight);
  mEditor->Rebuild();
  mEditor->mMaxWeightTextBox->SetText(String::Format("%g", mNewMaxWeight));
}

//-------------------------------------------------------------------ResourceTableResourceTypeChangeOp
ResourceTableResourceTypeChangeOp::ResourceTableResourceTypeChangeOp(ResourceTableEditor* editor, StringParam newType)
{
  mName = BuildString("ResourceTableResourceTypeChange to \"", newType, "\"");

  mEditor = editor;
  mOldResourceType = mEditor->mTable->mResourceType;
  mNewResourceType = newType;
}

void ResourceTableResourceTypeChangeOp::Undo()
{
  PerformOp(mOldResourceType);
}

void ResourceTableResourceTypeChangeOp::Redo()
{
  PerformOp(mNewResourceType);
}

void ResourceTableResourceTypeChangeOp::PerformOp(StringParam resourceType)
{
  //set the resource type (which marks the table's dirty bit)
  mEditor->mTable->SetResourceType(resourceType);

  //find the correct index of the resource type
  for(uint i = 0; i < mEditor->mResourceTypeSource.Strings.Size(); ++i)
  {
    if(mEditor->mResourceTypeSource.Strings[i] == resourceType)
    {
      mEditor->mResourceTypeSelector->SetSelectedItem(i, false);
      break;
    }
  }
  mEditor->Rebuild();
}

//-------------------------------------------------------------------ResourceTableAddRemoveRowOp
ResourceTableAddRemoveRowOp::ResourceTableAddRemoveRowOp(ResourceTableEditor* editor, ResourceTableEntry* entry, bool add)
{
  String opType;
  if(add)
    opType = "Add";
  else
    opType = "Remove";

  mName = BuildString(opType, "ResourceTable entry \"", entry->mName, "\"");

  mEditor = editor;
  mAdd = add;
  mEntry = *entry;
  mIndex = editor->mTable->mEntryList.FindIndex(entry);
}

void ResourceTableAddRemoveRowOp::Undo()
{
  PerformOp(!mAdd);
}

void ResourceTableAddRemoveRowOp::Redo()
{
  PerformOp(mAdd);
}

void ResourceTableAddRemoveRowOp::PerformOp(bool add)
{
  if(add)
  {
    ResourceTableEntry* entry = new ResourceTableEntry();
    *entry = mEntry;
    mEditor->mTable->mEntryList.InsertAt(mIndex, entry);
    mEditor->mTable->mEntryMap.Insert(entry->mName, entry);
  }
  else
    mEditor->mTable->RemoveAt(mIndex);
  //make sure the table's dirty bit is set
  mEditor->mTable->SetOutOfDate();

  mEditor->Rebuild();
}

//-------------------------------------------------------------------ResourceTableRowReorderOp
ResourceTableRowReorderOp::ResourceTableRowReorderOp(ResourceTableEditor* editor, uint oldIndex, uint newIndex)
{
  mName = BuildString("ResourceTable row \"", ToString(oldIndex), "\" reorder to \"", ToString(newIndex), "\"");

  mEditor = editor;
  mOldIndex = oldIndex;
  mNewIndex = newIndex;
}

void ResourceTableRowReorderOp::Undo()
{
  PerformOp(mNewIndex, mOldIndex);
}

void ResourceTableRowReorderOp::Redo()
{
  PerformOp(mOldIndex, mNewIndex);
}

void ResourceTableRowReorderOp::PerformOp(uint currentIndex, uint newIndex)
{
  ResourceTable& resourceTable = mEditor->mTable;
  ResourceTableEntry* entry = resourceTable[currentIndex];
  mEditor->mTable->mEntryList.EraseAt(currentIndex);
  mEditor->mTable->mEntryList.InsertAt(newIndex, entry);
  mEditor->mTable->SetOutOfDate();
  mEditor->Rebuild();
}

//-------------------------------------------------------------------ResourceTableBatchRowReorderOp
ResourceTableBatchRowReorderOp::ResourceTableBatchRowReorderOp(ResourceTableEditor* editor, Array<int>& oldIndices, Array<int>& newIndices)
{
  mName = "ResourceTableBatchRowReorder";

  mEditor = editor;
  mOldIndices = oldIndices;
  mNewIndices = newIndices;
}

void ResourceTableBatchRowReorderOp::Undo()
{
  PerformOp(mNewIndices, mOldIndices);
}

void ResourceTableBatchRowReorderOp::Redo()
{
  PerformOp(mOldIndices, mNewIndices);
}

void ResourceTableBatchRowReorderOp::PerformOp(Array<int>& currentIndices, Array<int>& newIndices)
{
  ResourceTable* table = mEditor->mTable;

  // First get all entries at the current indices (so removing doesn't break anything)
  Array<ResourceTableEntry*> entries;
  for(size_t i = 0; i < currentIndices.Size(); ++i)
    entries.PushBack(table->mEntryList[currentIndices[i]]);

  // Now remove all items at the provided indices.
  // Do this in reverse order so the indices are stable.
  for(size_t i = 0; i < currentIndices.Size(); ++i)
  {
    int flippedI = currentIndices.Size() - 1 - i;
    int index = currentIndices[flippedI];
    table->mEntryList.EraseAt(index);
  }

  // No re-insert each item at the new target index.
  // Since we are going front-to-back the indices are stable.
  for(size_t i = 0; i < currentIndices.Size(); ++i)
  {
    int index = newIndices[i];
    ResourceTableEntry* entry = entries[i];
    table->mEntryList.InsertAt(index, entry);
  }

  table->SetOutOfDate();
  mEditor->Rebuild();
}

//-------------------------------------------------------------------SearchableResourceTextBox
SearchableResourceTextBox::SearchableResourceTextBox(Composite* parent, StringParam resourceType,
                                                     StringParam resourceIdName)
  : Composite(parent)
{
  mResourceType = resourceType;

  mDisplayTextBox = new TextBox(this);
  mDisplayTextBox->SetTextClipping(true);
  mDisplayTextBox->HideBackground(true);

  //if the resource type is a string, have the text boxes become editable
  //and set their text to whatever was the input text
  if(mResourceType == "String")
  {
    mDisplayTextBox->SetEditable(true);
    mDisplayTextBox->SetText(resourceIdName);
  }
  //Otherwise, the passed in value is the actual "guid:name" of a resource.
  //We won't make the text box editable (because of the search view) but
  //we do need to string the guid and just show the display name.
  else
  {
    //find the ':' and only keep everything after it
    String displayName = resourceIdName;
    StringRange found = resourceIdName.FindLastOf(':');
    //guard for this not having a ':' for some reason
    if(!found.Empty())
      displayName = resourceIdName.SubString(found.End(), resourceIdName.End());
    mDisplayTextBox->SetText(displayName);
  }
  mResourceIdName = resourceIdName;

  //on mouse down we'll want to create the floating search view
  ConnectThisTo(mDisplayTextBox, Events::LeftMouseDown, OnMouseDown);
}

void SearchableResourceTextBox::UpdateTransform()
{
  //make sure to size the display text
  mDisplayTextBox->SetSize(GetSize());
  Composite::UpdateTransform();
}

void SearchableResourceTextBox::OnMouseDown(MouseEvent* mouseEvent)
{
  //if we're editing a string type don't do anything
  //special, the text box is editable by itself
  if(mResourceType == "String")
    return;

  //otherwise we need to create a floating search view, if we already had
  //an active search view then we don't need to create another one
  FloatingSearchView* activeSearchView = mActiveSearch;
  if(activeSearchView != NULL)
    return;
  
  //create the search view where we clicked
  FloatingSearchView* viewPopUp = new FloatingSearchView(this);
  Vec3 mousePos = ToVector3(mouseEvent->GetMouse()->GetClientPosition());
  SearchView* searchView = viewPopUp->mView;
  viewPopUp->SetSize(Pixels(300,400));
  viewPopUp->ShiftOntoScreen(mousePos);
  viewPopUp->UpdateTransformExternal();

  //have it search resources with the tag of the current resource type
  searchView->AddHiddenTag("Resources");
  searchView->AddHiddenTag(mResourceType);
  searchView->mSearch->SearchProviders.PushBack(GetResourceSearchProvider());

  //then just set up focus and set their starting text to empty
  searchView->TakeFocus();
  viewPopUp->UpdateTransformExternal();
  searchView->Search(String());
  //need to know when we finish the search so we can update the resource
  ConnectThisTo(searchView, Events::SearchCompleted, OnSearchCompleted);

  //store that we currently have a search view open
  mActiveSearch = viewPopUp;
}

void SearchableResourceTextBox::OnSearchCompleted(SearchViewEvent* e)
{
  //grab the resource
  Resource* resource = (Resource*)e->Element->Data;
  //set the display name for the text box
  mDisplayTextBox->SetText(resource->Name);
  //however, store the actual resource guid so we can properly resolve this later
  mResourceIdName = resource->ResourceIdName;
  mActiveSearch.SafeDestroy();

  //This is a little hacky, but oh well. I want this to behave like a
  //text box by sending the same event as a text box when it changes.
  ObjectEvent objectEvent(this);
  DispatchEvent(Events::TextBoxChanged, &objectEvent);
}

Vec2 SearchableResourceTextBox::GetMinSize()
{
  //just forward the text box's min size
  return mDisplayTextBox->GetMinSize();
}

void SearchableResourceTextBox::SetTextOffset(float offset)
{
  mDisplayTextBox->SetTextOffset(offset);
}

//-------------------------------------------------------------------WeightedTableBar

WeightedTableBar::WeightedTableBar(Composite* parent, ResourceWeightedTableView* weightedTableView)
  : Composite(parent)
{
  mWeightedTableView = weightedTableView;

  //create the background element
  static const String className = "Label";
  mDefSet = mDefSet->GetDefinitionSet(className);

  mBackground = CreateAttached<Element>(cBackground);

  ConnectThisTo(this, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(this, Events::RightMouseUp, OnRightMouseUp);
}

void WeightedTableBar::UpdateTransform()
{
  //set the background's size
  mBackground->SetSize(mSize);
  //make sure to update the composite
  Composite::UpdateTransform();
}

void WeightedTableBar::UpdateProbability(float prob, bool queueUndo)
{
  mWeightedTableView->UpdateProbability(this, prob, queueUndo);
}

void WeightedTableBar::UpdateProbability(MouseEvent* mouseEvent, bool queueUndo)
{
  //convert the mouse position to the graph position
  //(scaled means we'll get [0,maxSize] instead of [0,1])
  Vec2 localPos = mWeightedTableView->ToLocal(mouseEvent->Position);
  localPos = mWeightedTableView->mGraph->ToGraphPositionScaled(localPos - mWeightedTableView->mMargins.TopLeft());
  //round the probability to a certain number of places
  //(this will make the bar snap to certain values)
  float prob = Math::Round(localPos.y, mWeightedTableView->mRoundingPlaces);
  //update the internals
  UpdateProbability(prob, queueUndo);
}

void WeightedTableBar::OnLeftMouseDown(MouseEvent* mouseEvent)
{
  //if the table already had a context menu open, fade it out  
  mWeightedTableView->FadeOutContextMenu();

  //create a manipulation that caches the start value for undo and captures the mouse
  new WeightedTableBarDragManipulation(mouseEvent, this, this, mWeightedTableView->mEditor);
}

void WeightedTableBar::OnRightMouseUp(MouseEvent* mouseEvent)
{
  //check to see whether or not the table already has a context menu,
  //if it does fade out the current one
  mWeightedTableView->FadeOutContextMenu();

  //make a new context menu and position it below the mouse
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));
  mWeightedTableView->mContextMenuHandle = menu;

  //Create the duplicate item
  ConnectMenu(menu, "Duplicate Item", OnDuplicate);

  //create the remove item
  ConnectMenu(menu, "Remove Item", OnRemove);
}

void WeightedTableBar::OnProbabilityChanged(Event*)
{
  //convert the text to it's float value
  float prob;
  ToValue(mWeightedTableView->mEntries[mIndex].mProbability->GetText(), prob);
  //update the internals (don't round to places here since we
  //want to give the user explicit control if they type a value in)
  UpdateProbability(prob, true);
}

void WeightedTableBar::OnNameChanged(Event*)
{
  ResourceTable* table = mWeightedTableView->mTable;
  ResourceTableEntry* entry = (*table)[mIndex];
  String oldName = entry->mName;

  //get the new item name and set the name in the table
  StringParam newName = mWeightedTableView->mEntries[mIndex].mNameTag->GetText();
  if(table->GetOrNull(newName) != nullptr)
  {
    String msg = String::Format("An item with name '%s' already exists, "
      "please pick a different name.", newName.c_str());
    DoNotifyWarning("Invalid Item Name", msg);
    mWeightedTableView->mEntries[mIndex].mNameTag->SetText((*table)[mIndex]->mName);
    return;
  }
  
  entry->mName = newName;
  table->SetOutOfDate();
  
  //queue up an undo operation for changing the name
  ResourceTableEntryChangeOp* op = new ResourceTableEntryChangeOp(mWeightedTableView->mEditor, entry);
  op->mStartEntry.mName = oldName;
  mWeightedTableView->mEditor->mQueue.Queue(op);

  //Need to mark the resource as edited
  MetaOperations::NotifyObjectModified(mWeightedTableView->mTable);
}

void WeightedTableBar::OnValueChanged(Event*)
{
  ResourceTable* table = mWeightedTableView->mTable;
  ResourceTableEntry* entry = (*table)[mIndex];
  String oldValue = entry->mValue;

  //get the new item value and set the value in the table
  StringParam newValue = mWeightedTableView->mEntries[mIndex].mValueTag->mResourceIdName;
  entry->mValue = newValue;
  table->ForceRebuild();

  //queue up an undo operation for changing the value
  ResourceTableEntryChangeOp* op = new ResourceTableEntryChangeOp(mWeightedTableView->mEditor, entry);
  op->mStartEntry.mValue = oldValue;
  mWeightedTableView->mEditor->mQueue.Queue(op);
  
  //Need to mark the resource as edited
  MetaOperations::NotifyObjectModified(mWeightedTableView->mTable);
}

void WeightedTableBar::OnDuplicate(Event*)
{
  mWeightedTableView->DuplicateItem(mIndex);
}

void WeightedTableBar::OnRemove(Event*)
{
  mWeightedTableView->RemoveItem(mIndex);
}

//-------------------------------------------------------------------WeightedTableBarDragManipulation
WeightedTableBarDragManipulation::WeightedTableBarDragManipulation(MouseEvent* e, Composite* parent, WeightedTableBar* bar, ResourceTableEditor* editor)
  : MouseManipulation(e->GetMouse(), parent)
{
  mEditor = editor;
  mBar = bar;
  ResourceTable& resourceTable = mEditor->mTable;
  mStartProbability = resourceTable[mBar->mIndex]->mWeight;

  mBar->UpdateProbability(e, false);
}

void WeightedTableBarDragManipulation::OnMouseMove(MouseEvent* e)
{
  mBar->UpdateProbability(e, false);
}

void WeightedTableBarDragManipulation::OnMouseUp(MouseEvent* e)
{
  mBar->UpdateProbability(e, false);

  //only queue up the undo when we are done with all movement
  ResourceTable& resourceTable = mEditor->mTable;
  ResourceTableEntryChangeOp* op = new ResourceTableEntryChangeOp(mEditor, resourceTable[mBar->mIndex]);
  op->mStartEntry.mWeight = mStartProbability;
  mEditor->mQueue.Queue(op);

  this->Destroy();
}

//-------------------------------------------------------------------ResourceWeightedTableView

ResourceWeightedTableView::ResourceWeightedTableView(Composite* parent, ResourceTableEditor* editor)
  : Composite(parent)
{
  mEditor = editor;

  SetLayout(CreateFillLayout());
  // setup some constant variables that could potentially become tweakables later
  mMinBarWidth = Pixels(70);
  mRoundingPlaces = -2;

  // fill the graph within our margins (hardcoded margins for now)
  mMargins = Thickness(Pixels(40), Pixels(20), Pixels(20), Pixels(20));

  Setup(mEditor->mTable);
}

void ResourceWeightedTableView::Setup(ResourceTable* table)
{
  //create a scroll area to put everything in, this allows us
  //to make sure the bars don't become too small
  mScrollArea = new ScrollArea(this);
  mScrollArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  mGraph = new GraphWidget(mScrollArea);
  mGraph->SetSize(Pixels(268, 370));
  mGraph->SetDrawAxisY(false);

  //create a text box where the user can change the max weight
  //value allowed (also changes the bounds of the graph)
  mMaxWeightTextBox = new TextBox(mScrollArea);
  mMaxWeightTextBox->SetSize(Pixels(36, 20));
  mMaxWeightTextBox->SetTranslation(Pixels(3, 10, 0));
  mMaxWeightTextBox->SetNotInLayout(true);
  mMaxWeightTextBox->SetEditable(true);
  mMaxWeightTextBox->mBackgroundColor = ByteColorRGBA(49, 49, 49, 255);

  //create the actual bars and labels for the table
  SetTable(table);

  //for allowing the user to click the empty space above a
  //bar and still start to drag that bar
  ConnectThisTo(mGraph, Events::LeftMouseDown, OnMouseDown);
  //for creating context menus (listen to on the graph so that
  //we don't get the menu when clicking on a bar)
  ConnectThisTo(mGraph, Events::RightMouseUp, OnRightMouseUp);
  //for changing the graph height
  ConnectThisTo(mMaxWeightTextBox, Events::TextSubmit, OnMaxWeightChanged);
}

void ResourceWeightedTableView::Rebuild()
{
  Vec2 scrollPercentage = mScrollArea->GetScrolledPercentage();

  //destroy all of our children since we're going to rebuild them all
  auto children = GetChildren();
  while(!children.Empty())
  {
    Widget& child = children.Front();
    children.PopFront();

    child.Destroy();
  }
  //make sure to clear our the entry list we have as well
  mEntries.Clear();
  
  //now set everything back up
  Setup(mTable);

  //make sure the new sizes of everything are computed
  UpdateTransform();
  //set the scroll to be at the same spot
  mScrollArea->SetScrolledPercentage(scrollPercentage);
}

void ResourceWeightedTableView::SetTable(ResourceTable* table)
{
  mTable = table;

  //create new ui for every entry in the table
  for(uint i = 0; i < mTable->Size(); ++i)
  {
    ResourceTableEntry* entry = (*mTable)[i];
    CreateItem(entry->mName, entry->GetValueOrResourceIdName(), entry->mWeight);
  }

  //make sure to set the graph's height
  SetMaxWeight(mTable->mMaxWeight, false);
}

void ResourceWeightedTableView::UpdateTransform()
{
  float width;
  Vec2 minSize = GetMinSize(width);
  
  //set the scroll area's client area to the min size for displaying all of the weights
  mScrollArea->SetClientSize(minSize);
  
  Rect graphRect = Rect::PointAndSize(Vec2::cZero, minSize);
  graphRect.RemoveThickness(mMargins);
  PlaceWithRect(graphRect, mGraph);


  float currPos = width * .5f;
  uint count = mTable->Size();
  for(uint i = 0; i < count; ++i)
  {
    //the size of any item along the y is it's probability value
    float height = (*mTable)[i]->mWeight;

    float localLeft = currPos / minSize.x;
    float localRight = (currPos + width) / minSize.x;

    //compute the upper left and bottom right of the bar (we have this in
    //graph space as it's easier to work in, convert it to world space for display)
    Vec2 ul = SnapToPixels(mGraph->ToPixelPositionScaled(Vec2(localLeft, height)) + mMargins.TopLeft());
    Vec2 br = SnapToPixels(mGraph->ToPixelPositionScaled(Vec2(localRight, 0)) + mMargins.TopLeft());
    //move the current x placement value by 1 item and 1 spacer
    currPos += 2.0f * width;

    Vec2 barSize = SnapToPixels(br - ul);
    float halfBarSizeX = SnapToPixels(barSize.x * 0.5f);
    float barCenterX = SnapToPixels(ul.x + halfBarSizeX);

    //set the bar's position and size, making sure to snap to pixel boundaries
    mEntries[i].mBar->SetTranslation(Math::ToVector3(ul));
    mEntries[i].mBar->SetSize(barSize);

    //buffer used to slightly shrink in the text so it doesn't get clipped
    Vec2 textBuffer = Pixels(1, 1);

    //get the size of the probability text
    Vec2 probSize = mEntries[i].mProbability->GetMinSize();
    //to avoid warbling, compute the offset from the left edge based upon the difference in size of the two composites
    //(doing it this way avoids warbling because we're computing a fixed delta from the left edge instead of from the center)
    float probSizeDeltaX = Math::Min(halfBarSizeX, (probSize.x - barSize.x) * 0.5f);
    //now we can compute the position from the left edge (that won't warble)
    Vec2 probPos = Vec2(ul.x - probSizeDeltaX, ul.y - probSize.y);
    //now finally we can position the text, making sure to snap to pixels
    mEntries[i].mProbability->SetTranslation(SnapToPixels(Math::ToVector3(probPos)));
    //add a small border because sometimes small text was
    //cut off even though the size should be correct
    mEntries[i].mProbability->SetSize(probSize + textBuffer);
    if(probSize.x + textBuffer.x < barSize.x)
      mEntries[i].mProbability->SetTextOffset(0);

    //now do the same for the name tag, making sure that it is centered
    //between the bar and either its full size or the size of the bar
    Vec2 nameSize = mEntries[i].mNameTag->GetMinSize();
    //compute the position of the name to avoid warbling (see the probability position calculation for details)
    float nameSizeDeltaX = SnapToPixels(Math::Min(halfBarSizeX, (nameSize.x - barSize.x) * 0.5f));
    Vec2 namePos = Vec2(ul.x - nameSizeDeltaX, ul.y + barSize.y);
    mEntries[i].mNameTag->SetTranslation(SnapToPixels(Math::ToVector3(namePos)));
    mEntries[i].mNameTag->SetSize(nameSize + textBuffer);
    if(nameSize.x + textBuffer.x < barSize.x)
      mEntries[i].mNameTag->SetTextOffset(0);

    //same thing for the value tag
    Vec2 valueSize = mEntries[i].mValueTag->GetMinSize();
    //The value position is computed a bit differently to avoid warbling between the name and value text boxes.
    //The difference in size between these two composites is computed and then the value text box is
    //positioned a fixed amount (based upon the size offset) away from the name text box.
    float valueSizeDeltaX = SnapToPixels(Math::Min(halfBarSizeX, (valueSize.x - nameSize.x) * 0.5f));
    Vec2 valuePos = Vec2(namePos.x - valueSizeDeltaX, ul.y + (barSize.y + valueSize.y));
    mEntries[i].mValueTag->SetTranslation(SnapToPixels(Math::ToVector3(valuePos)));
    mEntries[i].mValueTag->SetSize(valueSize + textBuffer);
    if(valueSize.x + textBuffer.x < barSize.x)
      mEntries[i].mValueTag->SetTextOffset(0);
  }

  Composite::UpdateTransform();
}

void ResourceWeightedTableView::CreateItem(StringParam name, StringParam value, float probability)
{
  //make a new entry
  Entry& entry = mEntries.PushBack();
  //make the bar graph and give it the index that it is in the array
  entry.mBar = new WeightedTableBar(mScrollArea, this);
  entry.mBar->mIndex = mEntries.Size() - 1;

  //create a name tag for the bar
  entry.mNameTag = new TextBox(mScrollArea);
  entry.mNameTag->SetText(name);
  entry.mNameTag->SetEditable(true);
  entry.mNameTag->SetTextClipping(true);
  entry.mNameTag->HideBackground(true);

  //Create a searchable text box. This composite wraps the text box and
  //creating the search view on mouse down. This also handles having the
  //text box as editable when the resource type is String.
  entry.mValueTag = new SearchableResourceTextBox(mScrollArea, mTable->mResourceType, value);

  //create a text box for displaying/editing the probability of the bar
  entry.mProbability = new TextBox(mScrollArea);
  entry.mProbability->SetText(String::Format("%g", probability));
  entry.mProbability->SetEditable(true);
  entry.mProbability->SetTextClipping(true);
  entry.mProbability->HideBackground(true);

  //listen to when any of these items are changed
  Zero::Connect(entry.mProbability, Events::TextBoxChanged, entry.mBar,
    &WeightedTableBar::OnProbabilityChanged);
  Zero::Connect(entry.mNameTag, Events::TextBoxChanged, entry.mBar,
    &WeightedTableBar::OnNameChanged);
  Zero::Connect(entry.mValueTag, Events::TextBoxChanged, entry.mBar,
    &WeightedTableBar::OnValueChanged);
}

void ResourceWeightedTableView::DuplicateItem(uint index)
{
  //get the values of the item to duplicate
  ResourceTableEntry* entry = (*mTable)[index];
  String name = entry->mName;
  String value = entry->GetValueOrResourceIdName();
  float weight = entry->mWeight;

  String newName;
  do 
  {
    ++index;
    name = BuildString(name, "_new");
  } while (mTable->GetOrNull(name) != nullptr);
  
  //add a new item in the resource
  mTable->AddNewEntry(name, value, weight);
  //queue up the undo/redo for the add
  mEditor->mQueue.Queue(new ResourceTableAddRemoveRowOp(mEditor, mTable->mEntryList.Back(), true));

  //make a new item in the ui
  CreateItem(name, value, weight);
  //make sure we re-layout everything
  MarkAsNeedsUpdate();
  //we modified the resource, so mark it as such
  MetaOperations::NotifyObjectModified(mTable);
}

void ResourceWeightedTableView::RemoveItem(uint index)
{
  if(mTable->Size() == 1)
  {
    DoNotifyWarning("Table must have 1 item.","Cannot remove this item from the "
      "table because all weighted tables must have at least 1 item. "
      "This is to ensure that there is an item to return during any "
      "sample operation.");
    return;
  }
  //queue up the undo/redo for the remove
  mEditor->mQueue.Queue(new ResourceTableAddRemoveRowOp(mEditor, (*mTable)[index], false));
  //erase the item from the resource
  mTable->RemoveAt(index);

  //destroy all of the ui elements
  mEntries[index].mBar->Destroy();
  mEntries[index].mNameTag->Destroy();
  mEntries[index].mValueTag->Destroy();
  mEntries[index].mProbability->Destroy();
  //change all other item's indices since we removed an item in the middle
  for(uint i = index + 1; i < mEntries.Size(); ++i)
    mEntries[i].mBar->mIndex = i - 1;
  //now we can safely remove the item
  mEntries.EraseAt(index);

  //make sure we re-layout everything
  MarkAsNeedsUpdate();
  //we modified the resource, so mark it as such
  MetaOperations::NotifyObjectModified(mTable);
}

void ResourceWeightedTableView::UpdateProbability(WeightedTableBar* item, float prob, bool queueUndo)
{
  //If we're being set to the value we already are, there's no point in doing
  //anything. This check is done to prevent marking the resource as modified
  //when we didn't actually do anything (happens during creation of this composite).
  //A little hacky, but oh well
  ResourceTableEntry* tableEntry = (*mTable)[item->mIndex];
  if(prob == tableEntry->mWeight)
    return;

  //make sure we clamp the probability to the max value
  float maxVal = mTable->mMaxWeight;
  prob = Math::Clamp(prob, 0.0f, maxVal);

  //update the resource's probability and rebuild the table
  float oldWeight = tableEntry->mWeight;
  tableEntry->mWeight = prob;
  mTable->SetOutOfDate();

  //update the text for the probability
  Entry& entry = mEntries[item->mIndex];
  entry.mProbability->SetText(String::Format("%g", prob));
  //mark the bar as needing an update (this makes sure that 
  //the bar smoothly "animates" when it changes size)
  item->MarkAsNeedsUpdate(true);

  //if we need to, queue up an undo operation for the changing weight value
  if(queueUndo)
  {
    ResourceTableEntryChangeOp* op = new ResourceTableEntryChangeOp(mEditor, tableEntry);
    op->mStartEntry.mWeight = oldWeight;
    mEditor->mQueue.Queue(op);
  }

  MetaOperations::NotifyObjectModified(mTable);
}

void ResourceWeightedTableView::OnMouseDown(MouseEvent* mouseEvent)
{
  //if a context menu was open, fade it out
  FadeOutContextMenu();

  float width;
  Vec2 minSize = GetMinSize(width);

  //convert the mouse position to graph space (ignore if we're below the graph)
  Vec2 localPos = ToLocal(mouseEvent->Position);
  localPos += mScrollArea->GetScrolledOffset();
  localPos = mGraph->ToGraphPositionScaled(localPos);
  if(localPos.y < 0.0f)
    return;

  //iterate through all of the items trying to find if we're above a bar
  float currPos = width * .5f;
  uint count = mTable->Size();
  for(uint i = 0; i < count; ++i)
  {
    //convert the left and right x-values to graph space [0,1]
    float localLeft = currPos / minSize.x;
    float localRight = (currPos + width) / minSize.x;
    //move the current x position by 1 item and 1 spacer
    currPos += 2.0f * width;

    //check to see if we're in-between the left and right of the bar
    if(localPos.x > localLeft && localPos.x < localRight)
    {
      //if so, pretend we clicked on the bar itself (this will cause the
      //bar to capture the mouse and allow proper dragging and everything)
      mEntries[i].mBar->OnLeftMouseDown(mouseEvent);
      return;
    }

    //since our mouse position is to the left of this bar's start,
    //that means we have iterated too far and haven't found a bar
    //we're in-between so we're not selecting anything
    if(localPos.x < localLeft)
      return;
  }
}

void ResourceWeightedTableView::OnRightMouseUp(MouseEvent* mouseEvent)
{
  //if we already had a context menu, fade it out
  FadeOutContextMenu();

  //create a new context menu below the mouse
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));
  mContextMenuHandle = menu;

  //create the add item
  ConnectMenu(menu, "Add Item", OnAddItem);
}

void ResourceWeightedTableView::OnMaxWeightChanged(Event*)
{
  //get the value from the text box
  float maxWeight;
  ToValue(mMaxWeightTextBox->GetText(), maxWeight);

  //if the value hasn't changed, don't do anything
  if(GetMaxWeight() != maxWeight)
    MetaOperations::NotifyObjectModified(mTable);
  //set the height and mark us as modified
  SetMaxWeight(maxWeight, true);
}

void ResourceWeightedTableView::OnAddItem(Event*)
{
  //create a new item (this creates it with default values)
  mTable->AddNewEntry();
  //queue up the undo/redo for the add
  mEditor->mQueue.Queue(new ResourceTableAddRemoveRowOp(mEditor, mTable->mEntryList.Back(), true));

  //now extract the info and set the correct labels
  uint newIndex = mTable->Size() - 1;
  ResourceTableEntry* entry = (*mTable)[newIndex];
  String name = entry->mName;
  String value = entry->GetValueOrResourceIdName();
  float probability = entry->mWeight;

  //create the ui item
  CreateItem(name, value, probability);
  //make sure we re-layout everything
  MarkAsNeedsUpdate();
  //we modified the resource, so mark it as such
  MetaOperations::NotifyObjectModified(mTable);
}

void ResourceWeightedTableView::FadeOutContextMenu()
{
  if(ContextMenu* menu = mContextMenuHandle)
    menu->FadeOut();
}

void ResourceWeightedTableView::FixValuesAfterExpansion(bool queueUndo)
{
  if(queueUndo)
  {
    mEditor->mQueue.BeginBatch( );
    mEditor->mQueue.SetActiveBatchName("FixValuesAfterExpansion");
  }
  //when the max weight was changed, we might have had table values
  //that need to be clipped. Find any probabilities that are too
  //large and clip them to the new max
  for(uint i = 0; i < mTable->Size(); ++i)
  {
    float prob = (*mTable)[i]->mWeight;
    mEntries[i].mBar->UpdateProbability(Math::Min(GetMaxWeight(), prob), queueUndo);
  }
  if(queueUndo)
    mEditor->mQueue.EndBatch();
  
  //could check to see if we changed anything to determine if we
  //need to build the table, but it's rather quick so it doesn't matter
  mTable->ForceRebuild();
}

float ResourceWeightedTableView::GetMaxWeight()
{
  return mTable->mMaxWeight;
}

void ResourceWeightedTableView::SetMaxWeight(float maxHeight, bool queueUndo)
{
  //added a min weight just because, if anyone complains I'll think about removing it
  if(maxHeight < .1f)
  {
    maxHeight = .1f;
    DoNotifyWarning("Too small graph bounds.",
      "Graph bounds are too small. New bounds have been clamped to .1");
  }

  //queue the undo for the max weight change (and also fix all values)
  mEditor->SetMaxWeightWithUndo(maxHeight, queueUndo);

  //setup the graph too
  mGraph->SetWidthMax(maxHeight);
  //make sure the text box displays the correct value
  mMaxWeightTextBox->SetText(String::Format("%0.1f", maxHeight));
  //we might have shrunk, clip values that are too large
  FixValuesAfterExpansion(queueUndo);
}

Vec2 ResourceWeightedTableView::GetMinSize()
{
  float width;
  return GetMinSize(width);
}

Vec2 ResourceWeightedTableView::GetMinSize(float& width)
{
  //we want the table to stretch, however we want to prevent items
  //from getting too small. So compute how big each bar would have
  //to be if we stretched and clamp to a min value
  uint count = mTable->Size();
  float scaledWidth = mSize.x / (count * 2.0f);

  //give the user the width each bar ends up being
  width = scaledWidth;
  if(scaledWidth < mMinBarWidth)
    width = mMinBarWidth;

  //now compute the total required size (* 2 because of the space)
  //(add 30 pixels for the labels at the bottom, clean up later)
  Vec2 minSize = Vec2(width * count * 2, mSize.y - Pixels(30));
  return minSize;
}

//-------------------------------------------------------------------ResourceTableSource

// Strings used to identify columns in the tree view
const String ItemName = "ItemName";
const String ItemValue = "ItemValue";
const String ItemWeight = "ItemWeight";

/// Data source for the resource table. Manages indexing items
/// and getting/setting values from the internal resource.
class ResourceTableSource : public DataSource
{
public:
  typedef ResourceTableEntry Entry;

  /// The resource we're editing.
  ResourceTable* mTable;
  ResourceTableTreeView* mTreeView;
  /// A dummy root entry that all of the items are placed under
  Entry mRoot;

  ResourceTableSource()
  {

  }

  void SetTable(ResourceTable* table)
  {
    mTable = table;
  }

  DataEntry* GetRoot() override
  {
    return &mRoot;
  }

  DataEntry* ToEntry(DataIndex index) override
  {
    //just turn the index into a pointer
    return (DataEntry*)index.Id;
  }

  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    //just turn the pointer into an index
    return DataIndex((u64)(dataEntry));
  }

  DataEntry* Parent(DataEntry* dataEntry) override
  {
    //everyone but the root has the parent of the root
    Entry* root = &mRoot;
    if(dataEntry == root)
      return NULL;
    return root;
  }

  uint ChildCount(DataEntry* dataEntry) override
  {
    //only the root has children, no one else does
    Entry* root = &mRoot;
    if(dataEntry == root)
      return mTable->mEntryList.Size();
    return 0;
  }

  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    Entry* root = &mRoot;
    if(dataEntry == root)
      return mTable->mEntryList[index];
    return NULL;
  }

  bool IsExpandable(DataEntry* dataEntry) override
  {
    //only the root is expandable
    Entry* root = &mRoot;
    if(dataEntry == root)
      return true;
    return false;
  }

  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    //determine which column was requested and return
    //the correct value from the table's entry
    Entry* entry = (Entry*)dataEntry;
    if(column == ItemName)
      variant = entry->mName;
    else if(column == ItemValue)
      variant = entry->GetValueOrResourceIdName();
    else if(column == ItemWeight)
      variant = entry->mWeight;
    else if(column == CommonColumns::ToolTip)
    {
      if(mTable->ValidateEntry(entry) == false)
        variant = String::Format("Resource '%s' does not exist", entry->GetValueOrResourceIdName().c_str());
    }
  }

  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    ResourceTable* table = mTable;
    Entry* entry = (Entry*)dataEntry;
    //store the starting value for undo purposes
    Entry startEntry = *entry;

    if(column == ItemName)
    {
      //make sure that the name is unique
      String name = variant.Get<String>();
      if(table->GetOrNull(name) == nullptr)
        entry->mName = name;
      else
      {
        //if the name is not unique, let the user know and don't change the value
        String msg = String::Format("An item with name '%s' already exists, "
                                    "please pick a different name.", name.c_str());
        DoNotifyWarning("Invalid Item Name", msg);
        return false;
      }
    }
    else if(column == ItemValue)
      entry->mValue = variant.Get<String>();
    else if(column == ItemWeight)
    {
      //convert the passed in string to a float
      String str = variant.Get<String>();
      float weight;
      ToValue(str, weight);

      //clamp it to our max weight just in case a user set too large of a value
      real oldWeight = weight;
      weight = Math::Clamp(weight, 0.0f, table->mMaxWeight);
      entry->mWeight = weight;

      if(oldWeight != weight)
      {
        String msg = String::Format("The weight must be between 0 and %g (Max Weight). "
                                    "Clamping the weight.", table->mMaxWeight);
        DoNotifyWarning("Invalid weight value.", msg);
        return false;
      }
    }

    ResourceTableEntryChangeOp* op = new ResourceTableEntryChangeOp(mTreeView->mEditor, entry);
    op->mStartEntry = startEntry;
    mTreeView->mEditor->mQueue.Queue(op);

    table->ForceRebuild();
    //we modified the table, mark it as modified
    MetaOperations::NotifyObjectModified(mTable);
    return true;
  }

  void CanMove(Status& status, DataEntry* source, DataEntry* destination,
               InsertMode::Type insertMode)
  {
    Entry* entry = static_cast<Entry*>(destination);
    //for now handle on as before
    if(entry == &mRoot)
      status.SetSucceeded("Move to Beginning of List");
    else if(insertMode == InsertMode::Before || insertMode == InsertMode::On)
      status.SetSucceeded(BuildString("Move Before ", entry->mName));
    else if(insertMode == InsertMode::After)
      status.SetSucceeded(BuildString("Move After", entry->mName));
  }

  // Batch move operations don't have a good interface as the Move function is
  // called in hash-map order of the selection. To get around this we cache the entries
  // and insertion mode and then perform everything in one operation during EndBatchMove.
  ResourceTableEntry* mDestinationEntry;
  Array<ResourceTableEntry*> mEntriesToMove;
  InsertMode::Type mInsertMode;

  void BeginBatchMove() override
  {
    // Clear any old batch information
    mDestinationEntry = nullptr;
    mEntriesToMove.Clear();
    mTreeView->mEditor->mQueue.BeginBatch();
    mTreeView->mEditor->mQueue.SetActiveBatchName("ResourceTableEditor_BeginBatchMove");
  }

  bool Move(DataEntry* destinationEntry, DataEntry* movingEntry, InsertMode::Type insertMode) override
  {
    Entry* destination = static_cast<Entry*>(destinationEntry);
    Entry* moving = static_cast<Entry*>(movingEntry);
    
    // Deal with error scenarios
    if(moving == nullptr || destination == nullptr)
      return false;
    // Legacy edge case, shouldn't matter anymore
    if(destination == &mRoot)
      return true;

    // Cache the current item (have to batch the insertion mode and destination each time but whatever...)
    mInsertMode = insertMode;
    mDestinationEntry = destination;
    mEntriesToMove.PushBack(moving);
    return true;
  }

  void BatchMove(ResourceTableEntry* destination, Array<int>& indices, InsertMode::Type insertMode)
  {
    ResourceTable* table = mTable;
    // First get all entries at the current indices (so removing doesn't break anything)
    Array<ResourceTableEntry*> entries;
    for(size_t i = 0; i < indices.Size(); ++i)
      entries.PushBack(table->mEntryList[indices[i]]);

    // Now remove all items at the provided indices.
    // Do this in reverse order so the indices are stable.
    for(size_t i = 0; i < indices.Size(); ++i)
    {
      int flippedI = indices.Size() - 1 - i;
      int index = indices[flippedI];
      table->mEntryList.EraseAt(index);
    }

    // Now get the target destination index after we've removed everything
    uint destIndex = table->mEntryList.FindIndex(destination);
    if(insertMode == InsertMode::After)
      ++destIndex;

    // No re-insert each item at the new target index.
    // Since we are going front-to-back the indices are stable.
    Array<int> newIndices;
    for(size_t i = 0; i < indices.Size(); ++i)
    {
      int oldIndex = indices[i];
      ResourceTableEntry* entry = entries[i];
      table->mEntryList.InsertAt(destIndex, entry);

      newIndices.PushBack(destIndex);
      ++destIndex;
    }

    mTreeView->mEditor->mQueue.Queue(new ResourceTableBatchRowReorderOp(mTreeView->mEditor, indices, newIndices));
  }

  void EndBatchMove() override
  {
    // If there's nothing to do then return (mostly error handling)
    if(mDestinationEntry == nullptr || mEntriesToMove.Empty())
      return;

    ResourceTable* table = mTable;
    // Now we can actually perform the batch operations.
    // First get the indices of each entry that is being moved. We do this so we can visit all
    // entries in the proper order (front to back) by sorting the indices.
    Array<int> indices;
    for(size_t i = 0; i < mEntriesToMove.Size(); ++i)
      indices.PushBack(table->mEntryList.FindIndex(mEntriesToMove[i]));
    Zero::Sort(indices.All());

    // Now we can actually batch move all of the indices
    BatchMove(mDestinationEntry, indices, mInsertMode);
    mTreeView->mEditor->mQueue.EndBatch();

    // Clear cached info just to be safe
    mDestinationEntry = nullptr;
    mEntriesToMove.Clear();

    table->ForceRebuild();
    mTreeView->Rebuild();
    
    //we modified the table, mark it as modified
    MetaOperations::NotifyObjectModified(mTable);
  }

  bool IsValid(DataEntry* entry) override
  {
    Entry* item = (Entry*)entry;
    return mTable->ValidateEntry(item);
  }
};

//-------------------------------------------------------------------ResourceTableTreeView

ResourceTableTreeView::ResourceTableTreeView(Composite* parent, ResourceTableEditor* editor)
  : Composite(parent)
{
  mEditor = editor;
  //store the reference to our resource table
  mTable = mEditor->mTable;
  ResourceTable* table = mTable;

  SetLayout(CreateStackLayout());

  //now to actually create the tree view where all of the items from the table are displayed
  mTreeView = new TreeView(this);
  mTreeView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

  //set up the column formatting for this resource type (we might change how
  //the editors work in the tree depending on the type of resource we contain)
  SetFormatting(table->mResourceType);
  mTreeView->SetRefreshOnValueChange(true);
  //now create the data source and give it to the table
  mSource = new ResourceTableSource();
  mSource->SetTable(table);
  mSource->mTreeView = this;
  mTreeView->SetDataSource(mSource);

  ConnectThisTo(mTreeView, Events::RightMouseUp, OnAddContextMenu);
  ConnectThisTo(mTreeView, Events::MetaDrop, OnMetaDrop);
  ConnectThisTo(mTreeView, Events::MetaDropTest, OnMetaDrop);
  ConnectThisTo(mTreeView, Events::KeyDown, OnKeyDown);
}

ResourceTableTreeView::~ResourceTableTreeView()
{
  SafeDelete(mSource);
}

void ResourceTableTreeView::Rebuild()
{
  //We should technically have unique ids in our data source, however it's
  //easier to store indices. This would break the tree view, so just reset the
  //formatting and refresh the tree which rebuilds it from scratch
  //(it shouldn't get too big so this shouldn't ever have terrible performance).
  SetFormatting(mTable->mResourceType);
  mTreeView->Refresh();
}

void ResourceTableTreeView::SetFormatting(StringParam resourceType)
{
  TreeFormatting formatting;
  formatting.Flags.SetFlag(FormatFlags::ShowHeaders | FormatFlags::ShowSeparators | FormatFlags::ColumnsResizable);
  ResourceTable* table = mTable;

  //set up the common values for all rows
  ColumnFormat format;
  format.Flags = InPlaceTextEditorFlags::EditOnDoubleClick;
  format.Index = formatting.Columns.Size() - 1;
  format.FixedSize = Pixels(100,20);
  format.Editable = true;
  format.FlexSize = 1.0f;
  //create the Name column
  format.HeaderName = "Name";
  format.Name = ItemName;
  format.ColumnType = ColumnType::Flex;
  format.CustomEditor = cDefaultValueEditor;
  formatting.Columns.PushBack(format);

  //create the value column
  format.HeaderName = "Value";
  format.Name = ItemValue;
  format.ColumnType = ColumnType::Flex;
  //change the editor type depending on if we want a resource or value editor
  if(table->mResourceType != "String")
  {
    format.CustomEditor = cDefaultResourceEditor;
    format.CustomEditorData = table->mResourceType;
  }
  else
    format.CustomEditor = cDefaultValueEditor;
  formatting.Columns.PushBack(format);

  //create the weight column
  format.HeaderName = "Weight";
  format.ColumnType = ColumnType::Flex;
  format.Name = ItemWeight;
  format.CustomEditor = cDefaultValueEditor;//later change to float editor
  formatting.Columns.PushBack(format);

  mTreeView->SetFormat(formatting);
}

void ResourceTableTreeView::GetSelection(Array<ResourceTableEntry*>& entries)
{
  DataSelection* selection = mTreeView->GetSelection();
  Array<DataIndex> selectionIndices;
  selection->GetSelected(selectionIndices);

  entries.Clear();
  for(size_t i = 0; i < selectionIndices.Size(); ++i)
  {
    ResourceTableEntry* entry = (ResourceTableEntry*)selectionIndices[i].Id;
    if(entry == &(mSource->mRoot))
      continue;
    entries.PushBack(entry);
  }
}

void ResourceTableTreeView::OnAddContextMenu(MouseEvent* e)
{
  FadeOutContextMenu();

  //create the context menu below the mouse
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));

  menu->MarkAsNeedsUpdate(true);

  Array<ResourceTableEntry*> entries;
  GetSelection(entries);

  // Determine whether to add an add or remove context menu depending on if anything is selected
  if(entries.Empty())
  {
    ConnectMenu(menu, "Add Row", OnAddRow);
  }
  else
  {
    ConnectMenu(menu, "Remove Selected Rows", OnRemoveRow);
  }

  //the context menu will not properly set it's size (it computes the size it
  //needs of all children during the child add, meaning it won't have to
  //correct size of the text) so manually tell it to resize.
  menu->SizeToContents();

  
  mContextMenuHandle = menu;
}

/// Sorting struct for the data indices in use with ResourceTableSource.
struct ResourceDataIndexSorter
{
  bool operator()(const DataIndex& lhs, const DataIndex& rhs)
  {
    return lhs.Id > rhs.Id;
  }
};

void ResourceTableTreeView::OnAddRow(ObjectEvent* e)
{
  mEditor->AddRow(nullptr);
}

void ResourceTableTreeView::OnRemoveRow(ObjectEvent* e)
{
  RemoveSelectedRows();
}

void ResourceTableTreeView::OnKeyDown(KeyboardEvent* e)
{
  // Someone else handled this (such as editing the text field)
  if(e->Handled)
    return;

  if(e->Key == Keys::Delete)
    RemoveSelectedRows();
}

void ResourceTableTreeView::RemoveSelectedRows()
{
  // Get the selected indices from the tree view
  Array<ResourceTableEntry*> entries;
  GetSelection(entries);
  //don't do anything if there's no selection
  if(entries.Empty())
    return;

  ResourceTable* table = mTable;
  mTreeView->GetSelection()->SelectNone();

  mEditor->mQueue.BeginBatch();
  mEditor->mQueue.SetActiveBatchName("ResourceTableTreeView_RemoveSelectedRows");
  for(uint i = 0; i < entries.Size(); ++i)
  {
    ResourceTableEntry* entry = entries[i];
    mEditor->mQueue.Queue(new ResourceTableAddRemoveRowOp(mEditor, entry, false));
    table->RemoveOrError(entry->mName);
  }
  mEditor->mQueue.EndBatch();

  //Unfortunately, the tree requires unique identifies for all items, meaning
  //that they should change from removing another item. We're currently using
  //the index in the resource as the unique identifier, so the entire tree will break.
  //To get around this, just reset the data source which will rebuild the tree.
  //(maybe use a better index later)
  mTreeView->SetDataSource(mSource);
  table->ForceRebuild();

  MetaOperations::NotifyObjectModified(table);
}

void ResourceTableTreeView::OnMetaDrop(MetaDropEvent* e)
{
  Resource* resource = e->Instance.Get<Resource*>();

  //make sure we have a resource being dropped on us
  if(resource == NULL)
    return;

  ResourceTable* table = mTable;
  String givenType = resource->GetManager()->GetResourceType()->Name;
  String expectedType = table->mResourceType;

  //if the resource is of the incorrect type
  //(unless the table is type string, string accepts everything)
  if(expectedType != givenType && expectedType != "String")
  {
    //if this was the test drop then format a nice error
    //message saying why it can't be added
    if(e->Testing)
    {
      e->Result = String::Format("A %s cannot be added to a table of type %s",
                                 givenType.c_str(), expectedType.c_str());
    }
    return;
  }

  //this can be added, but we're testing so tell the user that this will be added
  if(e->Testing)
  {
    e->Result = String::Format("Add %s", resource->Name.c_str());
    return;
  }

  //grab the item name, however if this is of type
  //string then just use the display name
  String name = resource->ResourceIdName;
  if(table->mResourceType == "String")
    name = resource->Name;

  table->AddNewEntry(name);
  //queue the undo/redo for the newly created entry
  mEditor->mQueue.Queue(new ResourceTableAddRemoveRowOp(mEditor, table->mEntryList.Back(), true));
  
  MetaOperations::NotifyObjectModified(table);
  Rebuild();
}

void ResourceTableTreeView::FadeOutContextMenu()
{
  ContextMenu* contextMenu = mContextMenuHandle;
  if(contextMenu)
    contextMenu->FadeOut();
}

//-------------------------------------------------------------------ResourceTableEditor

ResourceTableEditor::ResourceTableEditor(Composite* parent, ResourceTable* table)
  : Composite(parent)
{
  this->SetName(table->Name);
  SetTable(table);

  ConnectThisTo(Z::gResources, Events::ResourceRemoved, OnResourceRemoved);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
}

ResourceTableEditor::~ResourceTableEditor()
{

}

void ResourceTableEditor::SetTable(ResourceTable* table)
{
  mTable = table;
  
  SetLayout(CreateStackLayout());

  CreateToolbar();

  //default view is the tree view
  mTreeView = new ResourceTableTreeView(this, this);
  mTreeView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mWeightedView = NULL;

  ConnectThisTo(mSwapViewButton, Events::LeftMouseUp, OnSwapView);
  ConnectThisTo(this, Events::LeftMouseDown, OnMouseDown);
}

void ResourceTableEditor::Rebuild()
{
  //rebuild the active composite
  if(mTreeView)
    mTreeView->Rebuild();
  else if(mWeightedView)
    mWeightedView->Rebuild();
  //rebuild the underlying table resource and mark it modified
  ResourceTable* table = mTable;
  table->ForceRebuild();
  MetaOperations::NotifyObjectModified(table);
}

void ResourceTableEditor::OnKeyDown(KeyboardEvent* e)
{
  if(e->CtrlPressed == false)
    return;

  if(e->Key == Keys::Z)
    mQueue.Undo();
  if(e->Key == Keys::Y)
    mQueue.Redo();
}

void ResourceTableEditor::CreateToolbar()
{
  ResourceTable* table = mTable;

  //create a dummy composite to Contains this "toolbar"
  Composite* topRow = new Composite(this);
  topRow->SetLayout(CreateRowLayout());

  //create the add row button so the user can manually add a row
  mAddButton = new TextButton(topRow);
  mAddButton->SetText("Add Row");

  //now create the swap view button so the user can swap from tree to weight view
  mSwapViewButton = new TextButton(topRow);
  mSwapViewButton->SetText("Swap View");

  //create a label for the max weight field so the user knows what it is
  Label* label = new Label(topRow);
  label->SetText("Max Weight");
  //now create a text box that allows the user to
  //edit the max allowable weight in the table
  mMaxWeightTextBox = new TextBox(topRow);
  mMaxWeightTextBox->SetText(String::Format("%g", table->mMaxWeight));
  mMaxWeightTextBox->SetEditable(true);
  mMaxWeightTextBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, 40);
 
  //create a combo box (maybe change to search view later) so the user can
  //select the kind of resource that is in the resource table
  mResourceTypeSelector = new ComboBox(topRow);
  mResourceTypeSelector->SetListSource(&mResourceTypeSource);
  mResourceTypeSelector->SetDockMode(DockMode::DockFill);
  mResourceTypeSelector->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);

  //find all of the resource types we can add by iterating though
  //all of the managers and adding their name to the list
  auto managerRange = Z::gResources->Managers.All();
  for(; !managerRange.Empty(); managerRange.PopFront())
  {
    String& resourceTypeName = managerRange.Front().first;
    ResourceManager* manager = managerRange.Front().second;
    //if the manager doesn't have a default resource don't add it to our list,
    //otherwise we have to deal with invalid values that we have no defaults for
    if(manager->GetDefaultResource() == NULL)
      continue;
    
    mResourceTypeSource.Strings.PushBack(resourceTypeName);
  }
  //also add string as a type
  mResourceTypeSource.Strings.PushBack("String");
  //sort the list of resource types so it's easier to find one
  Sort(mResourceTypeSource.Strings.All());
  //then set the current text to whatever resource type we are currently a table of
  mResourceTypeSelector->SetText(table->mResourceType);

  ConnectThisTo(mAddButton, Events::LeftMouseUp, AddRow);
  ConnectThisTo(mResourceTypeSelector, Events::ItemSelected, OnResourceTypeSelected);
  ConnectThisTo(mMaxWeightTextBox, Events::TextSubmit, OnMaxWeightChanged);
}

void ResourceTableEditor::OnSwapView(Event*)
{
  //if we had a tree view then we're swapping to the weight view
  if(mTreeView != NULL)
  {
    //destroy the tree
    mTreeView->Destroy();
    mTreeView = NULL;
    //create the weighted view in dock mode
    mWeightedView = new ResourceWeightedTableView(this, this);
    mWeightedView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  }
  //otherwise swap to the tree view
  else
  {
    mWeightedView->Destroy();
    mWeightedView = NULL;
    mTreeView = new ResourceTableTreeView(this, this);
    mTreeView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  }
}

Vec2 ResourceTableEditor::GetMinSize()
{
  return Vec2(200, 200);
}

void ResourceTableEditor::AddRow(MouseEvent* e)
{
  //add a new row and refresh the tree view, the user can now edit the new row
  //(it starts with whatever the default values are for the resource type)
  ResourceTable* table = mTable;
  table->AddNewEntry();
  mQueue.Queue(new ResourceTableAddRemoveRowOp(this, table->mEntryList.Back(), true));
  
  Rebuild();
}

void ResourceTableEditor::OnResourceTypeSelected(Event* e)
{
  //make sure a valid index was selected
  int index = mResourceTypeSelector->GetSelectedItem();
  if(index < 0 || index >= int(mResourceTypeSource.Strings.Size()))
    return;
  
  String resourceType = mResourceTypeSource.Strings[index];

  //queue up the undo for the resource type change (we'll also queue
  //up the resource remappings which is why there's a batch)
  mQueue.BeginBatch();
  mQueue.SetActiveBatchName("ResourceTableEditor_OnResourceTypeSelected");
  mQueue.Queue(new ResourceTableResourceTypeChangeOp(this, resourceType));

  ResourceTable* table = mTable;
  table->SetResourceType(resourceType);
  table->SetOutOfDate();

  //now remap all old resource names to new ones, if an old value
  //isn't a valid resource we must map it to the default resource
  
  RemapResources();
  mQueue.EndBatch();
}

void ResourceTableEditor::OnMaxWeightChanged(Event* e)
{
  //convert the text to the new value
  float maxWeight;
  ToValue(mMaxWeightTextBox->GetText(), maxWeight);
  
  //set the new max weight on the table and refresh the
  //view so it can see any newly clamped values
  SetMaxWeightWithUndo(maxWeight, true);
  Rebuild();
}

void ResourceTableEditor::OnResourceRemoved(ResourceEvent* e)
{
  ResourceTable* table = mTable;
  //if the resource being removed is the one we're editing, close our window
  if(e->EventResource == table)
  {
    CloseTabContaining(this);
    return;
  }

  //not the type we're editing, we don't care
  if(e->Manager->GetResourceType()->Name != table->mResourceType)
    return;

  //a resource was removed of our type and we might have been referencing it,
  //validate all of our current entries and display an error if any of them are bad
  table->ValidateEntries();
}

void ResourceTableEditor::OnMouseDown(MouseEvent* mouseEvent)
{
  if(mWeightedView != NULL)
    mWeightedView->FadeOutContextMenu();
  else
    mTreeView->FadeOutContextMenu();
}

void ResourceTableEditor::SetMaxWeightWithUndo(float maxWeight, bool queueUndo)
{
  if(queueUndo == false)
    mTable->SetMaxWeight(maxWeight);
  else
  {
    //just perform the redo to change the max weight and all values (for code re-use)
    ResourceTableMaxWeightChangeOp* op = new ResourceTableMaxWeightChangeOp(this, maxWeight);
    op->Redo();
    mQueue.Queue(op);
  }
}

void ResourceTableEditor::RemapResources()
{
  ResourceTable& resourceTable = mTable;

  String resourceType = resourceTable.mResourceType;
  resourceTable.SetOutOfDate();


  //if the new type is string, all old values are fine, but we should
  //change them to display names instead of actual guid names and then
  //rebuild the table (so we don't have search views, just editable text boxes)
  if(resourceType == "String")
  {
    for(uint i = 0; i < resourceTable.mEntryList.Size(); ++i)
    {
      ResourceTableEntry* entry = resourceTable[i];
      String resourceIdName = entry->mValue;
      //find the ':' and only keep everything after it
      String displayName = resourceIdName;
      StringRange found = resourceIdName.FindLastOf(':');
      //guard for this not having a ':' for some reason
      if(!found.Empty())
        displayName = resourceIdName.SubString(found.Begin() + 1, resourceIdName.End());

      mQueue.Queue(new ResourceTableEntryChangeOp(this, entry, displayName));
      entry->mValue = displayName;
    }
    Rebuild();
    return;
  }

  //otherwise we have to make sure everything is a valid resource
  //first find the manager for the resource type, if we don't find one for some reason bail
  auto range = Z::gResources->Managers.Find(resourceType);
  if(range.Empty())
    return;

  //now that we've found the manager, we need to update all of the
  //values in the table to the correct guid or to be the correct default
  ResourceManager* manager = range.Front().second;
  for(uint i = 0; i < resourceTable.mEntryList.Size(); ++i)
  {
    ResourceTableEntry* entry = resourceTable[i];
    //grab the name of the resource we're trying to match
    String resourceName = entry->mValue;
    Resource* resource = manager->GetResource(resourceName, ResourceNotFound::ReturnNull);

    //if we failed to get the resource, we need to see if we can get the default
    if(resource == NULL)
      //we only add resource types that have defaults, so this should always work
      resource = manager->GetDefaultResource();
    
    mQueue.Queue(new ResourceTableEntryChangeOp(this, entry, resource->ResourceIdName));
    entry->mValue = resource->ResourceIdName;
  }
  //now just rebuild everything
  Rebuild();
}

}//namespace Zero
