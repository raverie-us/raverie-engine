///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------CollisionTableItem

Vec2 CollisionMatrixItem::mPadding = Pixels(1,1);

namespace CollisionTableSettings
{
  const cstr cLocation = "EditorUi/CollisionTable";
  Tweakable(real, LabelRightPadding, real(5.0), cLocation);
  Tweakable(real, LabelYPadding, real(5.0), cLocation);
}

CollisionMatrixItem::CollisionMatrixItem(Composite* parent, CollisionTableEditor* tableEditor,
                                         CollisionFilter* filter, CollisionFilter& searchFilter)
  : Composite(parent)
{
  mTableEditor = tableEditor;
  mFilter = filter;
  mSearchFilter = searchFilter;

  // Create the background element
  mDefSet = mDefSet->GetDefinitionSet("CollisionMatrixItem");
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(Vec4(0.3f,0.3f,0.3f, 1.0f));


  mIcon = CreateAttached<Element>("Resolve");
  mIcon->SetSize(GetSize() - 2.0f * mPadding);
  
  // Create the label to display our state for now
  mTextLabel = new Text(this, cText);
  UpdateDisplay();

  // Listen for a whole bunch of mouse events on ourself
  ConnectThisTo(this, Events::LeftMouseUp, OnMouseUp);
  ConnectThisTo(this, Events::RightMouseUp, OnRightMouseUp);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);

  // Toggle here whether to use icons or text labels
  mIcon->SetVisible(false);
}

void CollisionMatrixItem::UpdateTransform()
{
  // Set the background's size
  Vec2 size = GetSize();
  mBackground->SetSize(size);
  // Center the label's text within the item
  Vec2 minSize = mTextLabel->GetMinSize();
  Vec2 offset = (size - minSize) * .5f;
  mTextLabel->SetTranslation(Math::ToVector3(offset,1.0f));
  mTextLabel->SetSize(minSize);

  mIcon->SetSize(size - 2.0f * mPadding);
  mIcon->SetTranslation(Math::ToVector3(mPadding,1.0f));
  
  // Make sure to call the base's function
  Composite::UpdateTransform();
}

void CollisionMatrixItem::OnMouseUp(MouseEvent* event)
{
  // If our current filter was null, create the filter
  if(mFilter == nullptr)
    mFilter = mTableEditor->AddFilter(mSearchFilter.TypeA,mSearchFilter.TypeB);
  // Set this as the filter to edit for the property view
  mTableEditor->SetEditingFilter(mFilter);
}

void CollisionMatrixItem::OnRightMouseUp(MouseEvent* event)
{
  //i If our current filter was null, create the filter to edit
  if(mFilter == nullptr)
    mFilter = mTableEditor->AddFilter(mSearchFilter.TypeA,mSearchFilter.TypeB);
  
  // Cycle to the next state
  CollisionFilterCollisionFlags::Enum newState = (CollisionFilterCollisionFlags::Enum)((mFilter->GetCollisionFlag() + 1) % CollisionFilterCollisionFlags::Size);
  mFilter->SetCollisionFlag(newState);

  // Mark the table as modified and update the visuals
  mTableEditor->MarkModified();
  UpdateDisplay();
}

void CollisionMatrixItem::OnMouseEnter(MouseEvent* e)
{
  DisplayPopup();
}

void CollisionMatrixItem::DisplayPopup()
{
  // Get the label's description (what groups it is)
  String filterLabel = GroupFilterDisplay(&mSearchFilter);

  // Get the current collision state flag
  uint flag = CollisionFilterCollisionFlags::Resolve;
  String currentState;
  if(mFilter != nullptr)
    flag = mFilter->GetCollisionFlag();

  // Convert that flag to the correct text
  if(flag == CollisionFilterCollisionFlags::Resolve)
    currentState = "Resolve";
  else if(flag == CollisionFilterCollisionFlags::SkipDetection)
    currentState = "Skip Detection";
  else if(flag == CollisionFilterCollisionFlags::SkipResolution)
    currentState = "Skip Resolution";

  // Build the whole tool tip text and display it
  String tipText = BuildString(filterLabel,": ",currentState);
  if(!tipText.Empty())
    ToolTip* toolTip = new ToolTip(this, tipText);
}

void CollisionMatrixItem::UpdateDisplay()
{
  // Update the label for now based upon the collision state
  if(mFilter == nullptr)
  {
    mTextLabel->SetText("R");
    mIcon->ChangeDefinition(mDefSet->GetDefinition("Resolve"));
  }
  else if(mFilter->GetCollisionFlag() == CollisionFilterCollisionFlags::Resolve)
  {
    mTextLabel->SetText("R");
    mIcon->ChangeDefinition(mDefSet->GetDefinition("Resolve"));
  }
  else if(mFilter->GetCollisionFlag() == CollisionFilterCollisionFlags::SkipDetection)
  {
    mTextLabel->SetText("SD");
    mIcon->ChangeDefinition(mDefSet->GetDefinition("SkipDetection"));
  }
  else if(mFilter->GetCollisionFlag() == CollisionFilterCollisionFlags::SkipResolution)
  {
    mTextLabel->SetText("SR");
    mIcon->ChangeDefinition(mDefSet->GetDefinition("SkipResolution"));
  }

  // Center the label's text within the item
  Vec2 size = GetSize();
  Vec2 minSize = mTextLabel->GetMinSize();
  Vec2 offset = (size - minSize) * .5f;
  mTextLabel->SetTranslation(Math::ToVector3(offset,1.0f));
  mTextLabel->SetSize(minSize);

  // Make sure the property view is updated
  mTableEditor->RefreshPropertyView();
}

//-------------------------------------------------------------------CollisionGroupLabel
CollisionGroupLabel::CollisionGroupLabel(Composite* parent, CollisionTableEditor* tableEditor, CollisionGroup* group, bool vertical)
  : Composite(parent)
{
  mTableEditor = tableEditor;
  mGroup = group;

  mText = new Text(this, cText);
  if(vertical == false)
    mText->SetText(group->Name);
  else
  {
    StringBuilder builder;
    StringIterator it = group->Name.Begin();
    StringIterator end = group->Name.End();
    for(; it < end; ++it)
    {
      builder.Append(*it);
      builder.Append("\n");
    }
    mText->SetText(builder.ToString());
  }

  mText->SetSize(mText->GetMinSize());
  SetSize(mText->GetMinSize());

  ConnectThisTo(mText,Events::RightMouseUp,OnRightMouseUp);
}

void CollisionGroupLabel::UpdateTransform()
{
  // Update the text's size first (that's important), then call the base function
  mText->SetSize(GetSize());
  Composite::UpdateTransform();
}

void CollisionGroupLabel::OnRightMouseUp(Event* e)
{
  // If we already had a context menu, fade it out
  if(ContextMenu* menu = mTableEditor->mMatrix->mContextMenuHandle)
  {
    menu->FadeOut();
  }

  // Create a new context menu below the mouse
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));
  mTableEditor->mMatrix->mContextMenuHandle = menu;

  //create all of the items with their correct callbacks on pressed
  ConnectMenu(menu, "Set all to skip detection", OnSetSkipDetect);
  ConnectMenu(menu, "Set all to skip resolution", OnSetSkipResolve);
  ConnectMenu(menu, "Set all to resolve", OnSetResolve);
  ConnectMenu(menu, "Remove group", OnRemoveGroup);
}

void CollisionGroupLabel::OnSetSkipDetect(Event* event)
{
  UpdateGroupState(CollisionFilterCollisionFlags::SkipDetection);
}

void CollisionGroupLabel::OnSetSkipResolve(Event* event)
{
  UpdateGroupState(CollisionFilterCollisionFlags::SkipResolution);
}

void CollisionGroupLabel::OnSetResolve(Event* event)
{
  UpdateGroupState(CollisionFilterCollisionFlags::Resolve);
}

void CollisionGroupLabel::UpdateGroupState(uint flags)
{
  // We're updating the state of all groups with relation to our current group,
  // this means we have to iterate through all groups and change their
  // relation with our current group.
  auto range = mTableEditor->mTable->mRegisteredGroups.All();
  for(; !range.Empty(); range.PopFront())
  {
    CollisionGroupInstance* groupInstance = range.Front().second;

    // Make a search filter so we can find the filter between ourself and the other type
    CollisionFilter searchFilter(mGroup->mResourceId,groupInstance->mResource->mResourceId);

    // If the filter type doesn't exist, create it so we can set the state
    CollisionFilter* filter = mTableEditor->mTable->FindFilter(searchFilter);
    if(filter == nullptr)
      filter = mTableEditor->AddFilter(searchFilter.TypeA,searchFilter.TypeB);

    filter->SetCollisionFlag((CollisionFilterCollisionFlags::Enum)flags);
  }

  // Now make sure that we mark the resource as modified and update the property grid
  mTableEditor->MarkModified();
  mTableEditor->RefreshAll();
}

void CollisionGroupLabel::OnRemoveGroup(Event* event)
{
  // Prevent removing the default group
  if(mGroup.Dereference() == mTableEditor->mTable->GetDefaultGroup())
  {
    DoNotifyWarning("Cannot remove default group.", 
      "The default collision group cannot be removed from a table as "
      "it is used in case another collision group cannot be found.");
    return;
  }

  // Remove the group from the table and set that we aren't editing any filter currently
  mTableEditor->mTable->UnRegisterGroup(mGroup);
  mTableEditor->SetEditingFilter(nullptr);
  // Now make sure that we mark the resource as modified and update the property grid
  mTableEditor->MarkModified();
  mTableEditor->RefreshAll();
}

//-------------------------------------------------------------------GroupSorter

/// Sorts the collision groups alphabetically by their name. This is used since
/// the groups are internally stored in a hashmap and the order will change.
/// To make the editor more usable sort them by name.
struct GroupSorter
{
  bool operator()(CollisionGroup* lhs, CollisionGroup* rhs)
  {
    return lhs->Name < rhs->Name;
  }
};

//-------------------------------------------------------------------CollisionTableMatrix

CollisionTableMatrix::CollisionTableMatrix(Composite* parent, CollisionTableEditor* tableEditor)
  : Composite(parent)
{
  mAddableGroupSelector = nullptr;
  mMinSize = Vec2(0,0);

  // Create the matrix for the current collision table
  SetTable(tableEditor);
}

void CollisionTableMatrix::SetTable(CollisionTableEditor* tableEditor)
{
  // We're not keeping track of all the ui we create. This means that when we're
  // recreating the ui for a new table we somehow need to delete what we created
  // before. To do this, just destroy all of our children.
  auto childRange = mChildren.All();
  while(!childRange.Empty())
  {
    Widget* child = &childRange.Front();
    childRange.PopFront();
    child->Destroy();
  }
  
  //Set the new editor
  mTableEditor = tableEditor;
  if(mTableEditor == nullptr)
    return;

  // The size of every item in the table
  Vec2 size = Pixels(23,23);
  // The buffer size in-between each item
  float buffer = Pixels(2);

  uint groupCount = tableEditor->mTable->mRegisteredGroups.Size();

  // We want the groups to be created in alphabetical order, so iterate through
  // all registered groups and store them in an array
  Array<CollisionGroup*> groups;
  groups.Resize(groupCount);
  uint i = 0;
  for(auto range = tableEditor->mTable->mRegisteredGroups.All(); !range.Empty(); range.PopFront())
  {
    groups[i] = range.Front().second->mResource;
    ++i;
  }
  // Now sort alphabetically
  Sort(groups.All(),GroupSorter());

  // Now create the labels for all of the groups, this tells us the max length
  // of label so we know at what x position to start laying out the matrix
  float xStart, yStart;
  CreateLabels(groups,size.y,buffer,xStart,yStart);
  xStart += CollisionTableSettings::LabelRightPadding;
  // Now that we know where to start the matrix at we can lay it out 
  CreateMatrix(groups,xStart,yStart,size,buffer);

  // Now create a combobox that allows the user to add/register more groups to this table
  Vec3 startPosition(Pixels(150),yStart + groupCount * (size.y + buffer),0);
  mAddableGroupSelector = new ComboBox(this);
  mAddableGroupSelector->SetTranslation(startPosition);
  mAddableGroupSelector->SetSize(Pixels(150,20));
  mAddableGroupSelector->SetListSource(&mAvailableGroups);
  mAddableGroupSelector->SetText("Add group");
  // Now build the list of what we can add (anything that exists that not already in the list)
  BuildGroupListing();

  // Now store the minimum size we need to display everything. This is used to
  // determine the client area size needed for our scrollable area.
  mMinSize = Vec2(xStart + groupCount * (size.x + buffer), yStart + groupCount * (size.y + buffer) + Pixels(20));
  // Add in the end position of the addable group selector
  mMinSize.x = Math::Max(mMinSize.x, Pixels(300));
  mTableEditor->UpdateScrollArea();

  // Listen for when a new group is added through the selector
  ConnectThisTo(mAddableGroupSelector, Events::ItemSelected, OnRegisterNewGroup);
}

void CollisionTableMatrix::Refresh()
{
  // Refresh is just reseting the table to the current table
  // (could maybe do this more efficiently, but it's ui, who cares)
  SetTable(mTableEditor);
}

Vec2 CollisionTableMatrix::GetMinSize()
{
  // We stored the min size we need to display everything during the last table set
  return mMinSize;
}

void CollisionTableMatrix::CreateLabels(Array<CollisionGroup*>& groups, float height, float buffer, float& xStart, float& yStart)
{
  // Cap the max length for a label as 100 pixels,
  // otherwise long names could cause the ui to be too big
  float maxLabelSize = Pixels(100);

  // We need to iterate through all the groups once to measure the max size.
  // This is done by creating the label and getting it's min size.
  Array<CollisionGroupLabel*> xLabels;
  Array<CollisionGroupLabel*> yLabels;
  // Store the max length we need
  float maxLength = 0.0f;
  for(uint x = 0; x < groups.Size(); ++x)
  {
    CollisionGroup* group = groups[x];

    // Create a new label object for this group
    CollisionGroupLabel* label = new CollisionGroupLabel(this,mTableEditor,group);
    label->SetTranslation(Vec3(0,x * (height + buffer),0.0f));
    xLabels.PushBack(label);

    // Keep track of the max required length
    maxLength = Math::Max(label->mText->GetMinSize().x,maxLength);
  }
  // Clamp to the max label size
  maxLength = Math::Min(maxLength,maxLabelSize);

  float maxHeight = 0.0f;

  // Now that we know the max size, go position and size
  // the labels so that their right sides line up
  for(uint i = 0; i < xLabels.Size(); ++i)
  {
    // Figure out their correct size based upon the known max size
    Vec3 pos = xLabels[i]->GetTranslation();
    Vec2 size = xLabels[i]->mText->GetMinSize();
    // Position so the right sides are lined up
    pos.x = Math::Max(0.0f,maxLength - size.x);
    pos.y = maxHeight + i * (height + buffer) + CollisionTableSettings::LabelYPadding;
    size.x = Math::Min(size.x,maxLength);
    // Set the new position and size
    xLabels[i]->SetTranslation(pos);
    xLabels[i]->SetSize(size + Pixels(1,1));
  }

  xStart = maxLength;
  yStart = maxHeight;
}

void CollisionTableMatrix::CreateMatrix(Array<CollisionGroup*>& groups, float xStart, float yStart, Vec2Param size, float buffer)
{
  uint count = groups.Size();

  // Create the matrix as upper left
  for(uint y = 0; y < count; ++y)
  {
    CollisionGroup* group1 = groups[y];
    ResourceId id1 = group1->mResourceId;

    for(uint x = count - y - 1; x < count; --x)
    {
      CollisionGroup* group2 = groups[count - x - 1];
      ResourceId id2 = group2->mResourceId;

      // Search for the filter of these two types
      CollisionFilter searchFilter = CollisionFilter(id1,id2);
      CollisionFilter* filter = mTableEditor->mTable->FindFilter(searchFilter);

      // Create a new item for this pairing
      CollisionMatrixItem* item = new CollisionMatrixItem(this,mTableEditor,filter,searchFilter);

      // Position the item at the correct x,y
      Vec3 pos = Vec3(xStart + x * (size.x + buffer), yStart + y * (size.y + buffer),0.0f);
      item->SetTranslation(pos);
      item->SetSize(size);
    }
  }
}

void CollisionTableMatrix::BuildGroupListing()
{
  // Clear the current list of what groups are available
  mAvailableGroups.Strings.Clear();

  // Enumerate all of the current collision groups
  Array<String> groupsList;
  CollisionGroupManager* groupManager = CollisionGroupManager::GetInstance();
  groupManager->EnumerateResources(groupsList);

  // Just store a reference to reduce the line lengths
  CollisionTable::RegisteredGroups& registeredGroups = mTableEditor->mTable->mRegisteredGroups;
  
  // Now iterate through all of the groups
  for(uint i = 0; i < groupsList.Size(); ++i)
  {
    // Get the actual resource (this shouldn't fail, but it doesn't hurt to check)
    CollisionGroup* group = groupManager->FindOrNull(groupsList[i]);
    if(group == nullptr)
      continue;

    // Now see if that group is already registered, if
    // it is not then add it to the available groups list
    CollisionTable::RegisteredGroups::range range = registeredGroups.Find(group->mResourceId);
    if(range.Empty())
      mAvailableGroups.Strings.PushBack(groupsList[i]);
  }
}

void CollisionTableMatrix::OnRegisterNewGroup(ObjectEvent* event)
{
  // Make sure an item was selected
  int index = mAddableGroupSelector->GetSelectedItem();
  if(index < 0 || index >= int(mAvailableGroups.Strings.Size()))
    return;
  
  // Get the name of the selected group
  String groupName = mAvailableGroups.Strings[index];
  // Now get the actual group for that name
  CollisionGroupManager* groupManager = CollisionGroupManager::GetInstance();
  CollisionGroup* newGroup = groupManager->FindOrNull(groupName);
  //should never fail, but check anyways
  if(newGroup == nullptr)
    return;

  // Register this group to the table
  mTableEditor->mTable->RegisterGroup(newGroup);
  // Make sure to refresh the entire table and mark the resource as modified
  mTableEditor->RefreshAll();
  mTableEditor->MarkModified();
}

//-------------------------------------------------------------------CollisionTableEditor

CollisionTableEditor::CollisionTableEditor(Composite* parent, CollisionTable* table)
  : Composite(parent)
{
  mTable = table;
  // Clear all of the starting values to null. This is necessary because some
  // resizing functions might happen during setup before everything is set to
  // a valid pointer. To mitigate this the resizing functions check for null.
  // This should maybe be reworked to deal with that case later.
  mEditingFilter = nullptr;
  mPropertyView = nullptr;
  mMatrix = nullptr;
  mScrollArea = nullptr;

  this->SetName(mTable->Name);
  this->SetLayout(CreateRowLayout());
 
  // Now we're going to create the matrix, however to deal with resizing
  // of the window it's put inside of a scrollable area.
  mScrollArea = new ScrollArea(this);
  mScrollArea->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);

  //create a splitter between the property view (the dummy composite technically) and the matrix.
  Splitter* splitter = new Splitter(this);

  // Create a dummy parent composite for the property view and the editing label.
  // This is used so that the property grid and its label can be docked easily
  // and the sash can be used to resize this area as a whole.
  Composite* rightArea = new Composite(this);
  rightArea->SetLayout(CreateStackLayout());
  rightArea->SetSizing(SizeAxis::X, SizePolicy::Fixed, 200);

  // Create a label for the item currently being edited by the property view.
  // Without this it's not clear which item is being edited.
  mCurrentFilterLabel = new Label(rightArea);
  mCurrentFilterLabel->SetText("No filter selected");

  // Now create the property view with a default size.
  mPropertyView = new PropertyView(rightArea);
  mPropertyView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

  // Now create the matrix and set it to fill up all of the scrollable area
  mMatrix = new CollisionTableMatrix(mScrollArea, this);

  // We want the scrollable area to properly fit the matrix, so set the client
  // size to the minimum size required to display the matrix
  mScrollArea->SetClientSize(mMatrix->GetMinSize());

  // When the property view changes any properties that means we have
  // to update the ui and mark the resource as modified
  ConnectThisTo(mPropertyView,Events::PropertyModified,OnPropertyChanged);

  // Listen to the collision group manager for resources being added/modified/removed.
  // This is done so that the property view can display correctly when items are
  // added/deleted and when they are renamed.
  CollisionGroupManager* groupManager = CollisionGroupManager::GetInstance();
  ConnectThisTo(groupManager, Events::ResourceAdded, OnCollisionGroupAdded);
  ConnectThisTo(groupManager, Events::ResourceModified, OnCollisionGroupModified);
  ConnectThisTo(groupManager, Events::ResourceRemoved, OnCollisionGroupRemoved);

  CollisionTableManager* tableManager = CollisionTableManager::GetInstance();
  ConnectThisTo(tableManager, Events::ResourceRemoved, OnCollisionTableRemoved);
}

void CollisionTableEditor::SetEditingFilter(CollisionFilter* filter)
{
  mEditingFilter = filter;
  // If we are set to edit a filter, grab its display string
  if(mEditingFilter != nullptr)
  {
    String filterString = GroupFilterDisplay(filter);
    mCurrentFilterLabel->SetText(filterString);
  }
  else
    mCurrentFilterLabel->SetText("No filter selected");

  // Tell the property grid to edit this filter
  mPropertyView->SetObject(mEditingFilter);
  mPropertyView->UpdateTransform();
}

CollisionFilter* CollisionTableEditor::AddFilter(ResourceId id1, ResourceId id2)
{
  // Create and set up the new filter
  CollisionFilter* filter = new CollisionFilter();
  filter->SetDefaults();
  filter->mTable = mTable;
  // Set the pair for this filter to be the two types
  filter->SetPair(id1,id2);

  // Add the new filter and fix the internals of the table
  mTable->mCollisionFilters.PushBack(filter);
  mTable->ReconfigureGroups();
  // Make sure we mark the resource as modified
  MarkModified();
  // Return the new filter so users can edit it
  return filter;
}

void CollisionTableEditor::OnPropertyChanged(PropertyEvent* propEvent)

{
  // The property grid was modified, refresh the matrix and
  // make sure we mark the resource as modified
  RefreshMatrix();
}

void CollisionTableEditor::RefreshAll()
{
  // Refresh both the property view and the matrix
  RefreshPropertyView();
  RefreshMatrix();

  // Reset the current editing filter to the current editing filter
  SetEditingFilter(mEditingFilter);
}

void CollisionTableEditor::RefreshPropertyView()
{
  if(mPropertyView)
    mPropertyView->Refresh();
}

void CollisionTableEditor::RefreshMatrix()
{
  if(mMatrix)
    mMatrix->Refresh();
}

void CollisionTableEditor::UpdateScrollArea()
{
  // Update the client size to what the min size is required for the matrix
  if(mScrollArea && mMatrix)
    mScrollArea->SetClientSize(mMatrix->GetMinSize());
}

void CollisionTableEditor::MarkModified()
{
  // Just mark the table as modified
  MetaOperations::NotifyObjectModified(mTable);
}

void CollisionTableEditor::OnCollisionGroupAdded(ResourceEvent* e)
{
  // A group was added, rebuild the matrix which will grab the new
  // group if it was added to the table resource.
  CollisionGroup* group = (CollisionGroup*)e->EventResource;
  RefreshMatrix();
}

void CollisionTableEditor::OnCollisionGroupModified(ResourceEvent* e)
{
  // Something was modified, most likely a name. Just rebuild everything.
  // The matrix needs to change because of the new name and the filter might also need to change.
  RefreshAll();
}

void CollisionTableEditor::OnCollisionGroupRemoved(ResourceEvent* e)
{
  // We could check to see if the removed group was one we were editing, but
  // that might have a lot of edge cases. Instead, just clear the editing
  // filter to nothing and rebuild the matrix.
  SetEditingFilter(nullptr);
  RefreshMatrix();
}

void CollisionTableEditor::OnCollisionTableRemoved(ResourceEvent* e)
{
  CollisionTable* table = (CollisionTable*)e->EventResource;
  if(table == mTable)
  {
    mMatrix->SetTable(nullptr);
    SetEditingFilter(nullptr);
  }
}

}//namespace Zero
