///////////////////////////////////////////////////////////////////////////////
///
/// \file TreeView.cpp
/// Implementation of Tree View
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Constants
const String cArrowRight = "ArrowRight";
const String cArrowDown = "ArrowDown";
const String cItemIcon = "ItemIcon";
const float cResizerWidth = Pixels(10.0f);

namespace TreeViewUi
{
const cstr cLocation = "EditorUi/TreeView";
Tweakable(Vec4,  HeaderColor,          Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  HeaderHighlight,      Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  ColumnSeparatorColor, Vec4(1,1,1,1), cLocation);
Tweakable(float, ItemHeight,           Pixels(20),    cLocation);
Tweakable(float, ItemSpacing,          Pixels(2),     cLocation);
Tweakable(float, IndentSize,           Pixels(8),     cLocation);
Tweakable(float, MaxDragScrollSpeed,   Pixels(4),     cLocation);
Tweakable(float, DragScrollSize,       Pixels(30),    cLocation);
Tweakable(float, DragInsertSize,       Pixels(3.15f),  cLocation);
}

namespace TreeViewValidUi
{
const cstr cLocation = "EditorUi/TreeView/ValidRows";
Tweakable(Vec4, PrimaryColor,   Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, SecondaryColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, MouseOverColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, SelectedColor,  Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ToolTipColor,   Vec4(1,1,1,1), cLocation);
}

namespace TreeViewInvalidUi
{
const cstr cLocation = "EditorUi/TreeView/InvalidRows";
Tweakable(Vec4, PrimaryColor,   Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, SecondaryColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, MouseOverColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, SelectedColor,  Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, ToolTipColor,   Vec4(1,1,1,1), cLocation);
}

namespace Events
{
  DefineEvent(TreeRightClick);
  DefineEvent(TreeKeyPress);
  DefineEvent(MouseEnterRow);
  DefineEvent(MouseExitRow);
}

ZilchDefineType(TreeEvent, builder, type)
{
}

//----------------------------------------------------------------- Row Selector
class RowSelector : public MouseManipulation
{
public:
  TreeView* mTree;
  Widget* mClientArea;

  /// Where the drag was started
  Vec2 mDragStart;

  /// Displays the drag selection
  Element* mSelectBox;

  RowSelector(MouseDragEvent* dragEvent, TreeView* tree)
    : MouseManipulation(dragEvent->GetMouse(), tree), mTree(tree)
  {
    mClientArea = tree->mArea->GetClientWidget();

    mDragStart = mClientArea->ToLocal(dragEvent->StartPosition);

    // Create a selection box on the scroll area
    mSelectBox = mClientArea->CreateAttached<Element>(cDragBox);
    mSelectBox->SetVisible(false);
  }

  void OnMouseUp(MouseEvent* event) override
  {
    mSelectBox->Destroy();
    mTree->GetSelection()->SelectFinal();
    this->Destroy();
  }

  void OnMouseMove(MouseEvent* event) override
  {
    mSelectBox->SetVisible(true);
  }

  void OnMouseUpdate(MouseEvent* event) override
  {
    Vec2 local = mClientArea->ToLocal(event->Position);
    Vec2 min = Math::Min(local, mDragStart);
    Vec2 max = Math::Max(local, mDragStart);
    min.x = Math::Max(min.x, 0.0f);
    min.y = Math::Max(min.y, 0.0f);
    // Clamp to either the window size, or the client area size
    // Whichever is larger
    max.x = Math::Min(max.x, Math::Max(mClientArea->mSize.x, mTree->mSize.x));
    max.y = Math::Min(max.y, Math::Max(mClientArea->mSize.y, mTree->mSize.y));

    mSelectBox->SetSize(max - min);
    mSelectBox->SetTranslation(Vec3(min.x, min.y, 0));

    // If the header is visible, shift the selection before calculating
    // the selected indices
    min.y -= mTree->GetHeaderRowHeight();
    max.y -= mTree->GetHeaderRowHeight();

    float rowHeight = mTree->mRowHeight;
    uint minIndex = uint((min.y) / rowHeight) + 1;
    uint maxIndex = uint((max.y) / rowHeight) + 1;
    mTree->SelectRowsInRange(minIndex, maxIndex);

    // Scroll if we're at the top or bottom of the tree view
    mTree->DragScroll(event->Position);
  }

  void OnKeyDown(KeyboardEvent* event) override
  {
    if(event->Key == Keys::Escape)
    {
      this->Destroy();
      mSelectBox->Destroy();
    }
  }
};

//--------------------------------------------------------------------- Tree Row
ZilchDefineType(TreeRow, builder, type)
{

}

TreeRow::TreeRow(TreeView* treeView, TreeRow* rowparent, DataEntry* entry)
  :TreeBase(treeView->mArea)
{
  mExpanded = false;
  mTree = treeView;
  mActive = true;
  mParentRow = rowparent;
  mDepth = rowparent ? rowparent->mDepth + 1 : 0;
  mIndex = treeView->mDataSource->ToIndex(entry);
  mTree->mRowMap[mIndex.Id] = this;
  treeView->mRows.PushBack(this);
  mExpandIcon = NULL;
  mGraphicBackground = CreateAttached<Element>(cWhiteSquare);
  mGraphicBackground->SetInteractive(false);
  mBackground = new Spacer(this);
  mValid = true;

  //Can this node be expanded?
  bool isExpandable = treeView->mDataSource->IsExpandable(entry);
  //Is this node currently expanded. Nodes can be expanded even if
  //there is node TreeNode currently displaying them.
  bool isExpaned = mTree->mExpanded->IsSelected(mIndex);

  //Create selection highlight
  mSelection = CreateAttached<Element>(cWhiteSquare);
  mSelection->SetTranslation( Pixels(0,1,0));
  mSelection->SetInteractive(false);

  if(mTree->mFormatting.Flags.IsSet(FormatFlags::ShowSeparators))
  {
    mSeparator = CreateAttached<Element>("HorizontalDivider");
    mSeparator->SetColor(Vec4(1,1,1,0.2f));
  }
  
  //Create the expand icon
  mExpandIcon = CreateAttached<Element>(cArrowRight);
  //Clicking on it will expand the row
  ConnectThisTo(mExpandIcon, Events::LeftMouseDown, OnMouseDownExpander);

  if(!isExpandable)
    mExpandIcon->SetVisible(false);

  //Build Edit Columns
  ValueEditorFactory* factory = ValueEditorFactory::GetInstance();
  float offsetX = Pixels(20);
  TreeFormatting* formatting = &mTree->mFormatting;
  uint columns = formatting->Columns.Size();
  for(uint i=0;i<columns;++i)
  {
    ColumnFormat& format = formatting->Columns[i];

    String customEditor = format.CustomEditor;
    String editorType = (customEditor == String()) ? cDefaultValueEditor : customEditor;
    ValueEditor* valueEditor = factory->GetEditor(editorType, this, format.CustomEditorData, format.Flags);
    valueEditor->Editable = format.Editable;

    valueEditor->Name = format.Name;
    mEditorColumns.PushBack(valueEditor);
    valueEditor->SetTranslation(Vec3(offsetX, 0, 0));
    valueEditor->SetSize(format.FixedSize);
    valueEditor->SetClipping(true);
    offsetX += format.FixedSize.x;
    ConnectThisTo(valueEditor, Events::TextUpdated, OnTextChanged);
    ConnectThisTo(valueEditor, Events::ValueChanged, OnValueChanged);

    Array<Widget*> dragWidgets;
    valueEditor->GetDragWidgets(dragWidgets);
    forRange(Widget* dragWidget, dragWidgets.All())
      ConnectThisTo(dragWidget, Events::LeftMouseDrag, OnMouseDragRow);
  }

  //Connect all needed events
  ConnectThisTo(this, Events::KeyUp, OnKeyUp);
  ConnectThisTo(this, Events::LeftClick, OnMouseClick);
  ConnectThisTo(this, Events::DoubleClick, OnDoubleClick);
  ConnectThisTo(this, Events::RightMouseUp, OnRightUp);
  ConnectThisTo(mBackground, Events::LeftMouseDrag, OnMouseDragBackground);
  ConnectThisTo(this, Events::MetaDrop, OnMetaDrop);
  ConnectThisTo(this, Events::MetaDropTest, OnMetaDrop);
  ConnectThisTo(this, Events::ObjectPoll, OnObjectPoll);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);

  //Load data from data source
  RefreshData();

  //Expand the node if necessary
  if(isExpaned)
    Expand();
}

bool TreeRow::IsRoot()
{
  DataEntry* root = mTree->mDataSource->GetRoot();
  DataIndex rootIndex = mTree->mDataSource->ToIndex(root);
  return (rootIndex == mIndex);
}

void TreeRow::RefreshData()
{
  TreeFormatting& formatting = mTree->mFormatting;

  // No need to refresh the data if we're the root and not displaying the root
  bool showRoot = formatting.Flags.IsSet(FormatFlags::ShowRoot);
  if(IsRoot() && !showRoot)
    return;

  /// Get Data and Update Editors
  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);
  if(entry)
  {
    uint columns = formatting.Columns.Size();
    for(uint i=0;i<columns;++i)
    {
      ColumnFormat& format = formatting.Columns[i];
      Any value;
      mTree->mDataSource->GetData(entry, value, format.Name);
      mEditorColumns[i]->SetVariant(value);
    }
  }
  else
  {
    ErrorIf(true, "Lost Entry");
  }
}


//--------------------------------------------------------------------- Row Edit
void TreeRow::OnValueChanged(ObjectEvent* event)
{
  //Editor has changed value update the Tree
  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);
  if(entry)
  {
    //Editor has changed the value
    ValueEditor* editor = (ValueEditor*)event->Source;

    //Get the new value
    Any newValue;
    editor->GetVariant(newValue);

    //Apply the value to the data source
    mTree->mDataSource->SetData(entry, newValue, editor->Name);
  }

  if(mTree->mRefreshOnValueChange)
    Refresh();
}

void TreeRow::OnTextChanged(TextUpdatedEvent* event)
{
  //Editor has changed text value update the Tree
  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);
  if (entry)
  {
    //Editor has changed the in place text
    InPlaceTextEditor* editor = (InPlaceTextEditor*)event->GetSource();

    //Get the text value
    Any newTextValue;
    editor->GetEditTextVariant(newTextValue);

    //Apply the value to the data source
    event->mChangeAccepted = mTree->mDataSource->SetData(entry, newTextValue, editor->Name);
  }

  if (mTree->mRefreshOnValueChange)
    Refresh();
}

void TreeRow::Edit(int column)
{
  ColumnFormat& format = mTree->mFormatting.Columns[column];
  if(format.Editable)
    mEditorColumns[column]->Edit();
}

void TreeRow::Edit(StringParam column)
{
  ColumnFormat& format = mTree->GetColumn(column);
  if(format.Editable)
    mEditorColumns[format.Index]->Edit();
}

void TreeRow::UpdateTransform()
{
  if(mActive == false)
    return;

  // We want the background to take the indent into account so that we
  // can drag items to the left of rows (without activating that row)
  float backgroundOffset = TreeViewUi::IndentSize * float(mDepth - 1);
  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);
  
  if(entry && !mTree->mDataSource->IsExpandable(entry))
    backgroundOffset += Pixels(16);
  
  mBackground->SetTranslation(Vec3(backgroundOffset, 0, 0));
  mBackground->SetSize(mSize - Vec2(backgroundOffset, 0));

  if(mTree->mFormatting.Flags.IsSet(FormatFlags::ShowSeparators))
  {
    mSeparator->SetTranslation(Vec3(0,mSize.y - Pixels(2),0));
    mSeparator->SetSize(Vec2(mSize.x, Pixels(2)));
  }

  // Update the column sizes and locations
  UpdateColumnsTransform(mSize);

  // If the mouse is over it (while dragging), make a lighter highlight
  if(mTree->mMouseOver == mIndex)
  {
    Highlight(HighlightType::Preview, mTree->mMouseOverMode);
  }
  // If this element is selected, make the darker highlight visible
  else if(mTree->mSelection->IsSelected(mIndex))
  {
    Highlight(HighlightType::Selected);
  }
  // Otherwise, it's not visible
  else
  {
    Highlight(HighlightType::None);
  }

  Composite::UpdateTransform();
}

void TreeRow::UpdateColumnsTransform(Vec2Param size)
{
  // We want to offset the expand icon by the indent
  float indentWidth = TreeViewUi::IndentSize * float(mDepth - 1);
  mExpandIcon->SetTranslation(SnapToPixels(Vec3(indentWidth, 2, 0)));

  // Add extra room for the expand icon
  indentWidth += Pixels(16);

  TreeFormatting& formatting = mTree->mFormatting;

  uint columnCount = formatting.Columns.Size();

  // We want to find the first non fixed column because anything before it
  // will be pushed over by the indent width
  uint firstNonFixedColumn = 0;
  for(uint i = 0; i < columnCount; ++i)
  {
    // Grab the formatting
    ColumnFormat& column = formatting.Columns[i];

    // If it's the flex, set it and break out of the loop
    if(column.ColumnType == ColumnType::Flex)
    {
      firstNonFixedColumn = i;
      break;
    }
  }

  // Walk through each column and set its size and translation
  for(uint i = 0; i < columnCount; ++i)
  {
    // Grab the formatting
    ColumnFormat& column = formatting.Columns[i];

    // Grab the editor
    ValueEditor* valueEditor = mEditorColumns[i];

    Vec2 desiredSize = column.CurrSize;
    
    // The final x position
    float finalX = column.StartX;

    // If it's before the first non fixed column, we have to apply the indent
    if(i <= firstNonFixedColumn)
    {
      finalX += indentWidth;

      // If it's the first flex, it needs to eat (shrink in size) the indent
      // so that everything after it can retain its desired position and size
      if(column.ColumnType == ColumnType::Flex)
        desiredSize.x -= indentWidth;
    }

    // Set the final transform
    valueEditor->SetTranslation(SnapToPixels(Vec3(finalX, 0, 0)));
    valueEditor->SetSize(SnapToPixels(desiredSize));
  }
}

void TreeRow::Refresh()
{
  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);

  if(entry==NULL)
    return;
  
  bool isExpandable = mTree->mDataSource->IsExpandable(entry);

  if(isExpandable)
  {
    // Show Expander 
    mExpandIcon->SetVisible(true);

    // We may have been added to the expanded list externally
    mExpanded = mTree->mExpanded->IsSelected(mIndex);

    //Rebuild the tree if expanded
    if(this->mExpanded)
      RebuildChildren();
  }
  else
  {
    //Not expandable

    //If node used to be expanded collapse
    if(this->mExpanded)
      Collapse();

    //Hide Expander
    mExpandIcon->SetVisible(false);
  }

  //Reload columns
  this->RefreshData();
}

//-------------------------------------------------------------- Row Destruction 

TreeRow::~TreeRow()
{
  //Remove from parent
  if(mParentRow)
  {
    mParentRow = NULL;
    TreeRowList::Unlink(this);

    //If the index still refers to this row
    //erase this row, it will be rebuilt if needed
    if(mTree->mRowMap.FindValue(mIndex.Id, NULL) == this)
      mTree->mRowMap.Erase(mIndex.Id);
  }

  while(!mChildren.Empty())
  {
    mChildren.Front().mParentRow = NULL;
    TreeRowList::Unlink(&mChildren.Front());
  }
 
  mToolTip.SafeDestroy();
}

void TreeRow::OnDestroy()
{
  Composite::OnDestroy();
}


void TreeRow::DestroyChildren()
{
  forRange(TreeRow& child, mChildren.All())
  {
    child.RecursiveDestroy();
  }
}

void TreeRow::RecursiveDestroy()
{
  DestroyChildren();
  this->Destroy();
}

void TreeRow::Remove()
{
  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);
  mTree->mDataSource->Remove(entry);
}

void TreeRow::UpdateBgColor(uint index)
{
  Vec4 color;
  if(index)
  {
    if(mValid)
      color = TreeViewValidUi::PrimaryColor;
    else
      color = TreeViewInvalidUi::PrimaryColor;
  }
  else
  {
    if(mValid)
      color = TreeViewValidUi::SecondaryColor;
    else
      color = TreeViewInvalidUi::SecondaryColor;
  }

  mGraphicBackground->SetColor(color);
}

//-------------------------------------------------------- Row Expand / Collapse
void TreeRow::RebuildChildren()
{
  //Rebuild children by reusing valid rows and destroying invalid rows

  //Look up the entry
  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);

  if(entry == NULL)
    return;

  uint numChildren = mTree->mDataSource->ChildCount(entry);

  //Swap out old children
  TreeRowList oldChildren;
  oldChildren.Swap(mChildren);

  DataEntry* prev = NULL;
  for(uint i = 0;i<numChildren;++i)
  {
    DataEntry* child = mTree->mDataSource->GetChild(entry, i, prev);
    DataIndex index = mTree->mDataSource->ToIndex(child);

    //Is this row still valid?
    TreeRow* row = mTree->FindRowByIndex(index);
    if(row && row->mParentRow==this)
    {
      //node is still valid and this row is still the parent
      //move to current children list
      oldChildren.Erase(row);
      mChildren.PushBack(row);

      row->Refresh();
    }
    else
    {
      //No rows created for this data index 
      //Create a new child TreeRow
      TreeRow* treeRow = new TreeRow(mTree, this, child);
      mChildren.PushBack(treeRow);
    }

    prev = child;
  }

  //Destroy all children that are no longer valid.
  while(!oldChildren.Empty())
  {
    TreeRow* current = &oldChildren.Front();
    current->RecursiveDestroy();
    oldChildren.Erase(current);
    //Child removes itself in destruction.
    mChildren.PushBack(current);
  }

  this->MarkAsNeedsUpdate();
}

void TreeRow::Collapse()
{
  //Do not repeat
  if(!mExpanded)
    return;

  //Change the expander icon
  if(mExpandIcon)
    mExpandIcon->ChangeDefinition(mDefSet->GetDefinition(cArrowRight));

  //Mark the index as not expanded
  mTree->mExpanded->Deselect(mIndex);

  DestroyChildren();
  mExpanded = false;
  mTree->MarkAsNeedsUpdate();
}

void TreeRow::Expand()
{
  //Do not repeat
  if(mExpanded)
    return;

  //Change the expander icon
  if(mExpandIcon)
    mExpandIcon->ChangeDefinition(mDefSet->GetDefinition(cArrowDown));

  //Mark the index as expanded
  mTree->mExpanded->Select(mIndex);

  //Tell the data source to expand
  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);
  if(entry == NULL)
    return;

  mTree->mDataSource->Expand(entry);

  //Build children
  RebuildChildren();
  this->RefreshData();

  //Mark TreeRow as expanded
  mExpanded = true;
}

void TreeRow::Select(bool singleSelect)
{
  //Already selected
  bool wasSelected = mTree->mSelection->IsSelected(mIndex);

  //Always clear everyone with onlySelected
  if(singleSelect) 
    mTree->mSelection->SelectNone();

  //Select this row
  mTree->mSelection->Select(mIndex);

  //Only send the event if this was not already selected.
  if(!wasSelected)
  {
    //Send out selected event
    DataEvent dataEvent;
    dataEvent.Index = mIndex;
    mTree->mDataSource->DispatchEvent(Events::DataSelected, &dataEvent);
  }
  else
  {
    //When group selecting
    //selecting an already selected item removes it
    if(!singleSelect)
    {
      //Remove from select
      mTree->mSelection->Deselect(mIndex);
    }
  }

  mTree->MarkAsNeedsUpdate();
  mTree->mSelection->SelectFinal();
}

void TreeRow::MultiSelect()
{
  DataSelection* selection = mTree->mSelection;

  uint rowIndex = mTree->FindRowIndex(this);

  // If the selection size is 0, select from the root to this row
  if(selection->Size() == 0)
  {
    mTree->SelectRowsInRange(0, rowIndex);
  }
  else
  {
    // If there are items selected, build a range of the selected indices
    uint minIndex, maxIndex;
    mTree->GetSelectionRange(&minIndex, &maxIndex);

    // We want to take into account this row as well
    minIndex = Math::Min(minIndex, rowIndex);
    maxIndex = Math::Max(maxIndex, rowIndex);

    // Select all in the range
    mTree->SelectRowsInRange(minIndex, maxIndex);
  }

  mTree->mSelection->SelectFinal();
}

void TreeRow::Highlight(HighlightType::Type type, InsertMode::Type insertMode)
{
  // Reset the selection size
  mSelection->SetSize(mSize - Pixels(2, 0));
  mSelection->SetTranslation(Vec3(Pixels(1), 0, 0));

  // Set it invisible if it's none
  if(type == HighlightType::None)
  {
    mSelection->SetVisible(false);
  }
  else if(type == HighlightType::Selected)
  {
    if(mValid)
      mSelection->SetColor(TreeViewValidUi::SelectedColor);
    else
      mSelection->SetColor(TreeViewInvalidUi::SelectedColor);

    // Make it visible again
    mSelection->SetVisible(true);
  }
  else if(type == HighlightType::Preview)
  {
    if(mValid)
      mSelection->SetColor(TreeViewValidUi::MouseOverColor);
    else
      mSelection->SetColor(TreeViewInvalidUi::MouseOverColor);

    // Make it visible again
    mSelection->SetVisible(true);

    float indent = TreeViewUi::IndentSize * float(mDepth - 1);

    // If the mouse over mode is before, set the highlight to only cover
    // the top of the row
    if(insertMode == InsertMode::Before)
    {
      mSelection->SetSize(Vec2(mSize.x, Pixels(3)));
      mSelection->SetTranslation(Vec3(indent, -1.5f, 0));
    }
    // If it's set to after, make it cover the bottom of the row
    else if(insertMode == InsertMode::After)
    {
      mSelection->SetSize(Vec2(mSize.x, Pixels(3)));
      mSelection->SetTranslation(Vec3(indent, mSize.y - 1.5f, 0));
    }
  }
}

//------------------------------------------------------------ Row Events
void TreeRow::OnKeyPress(KeyboardEvent* event)
{
  if(event->Handled)
    return;

  //Select(true);
}

void TreeRow::OnMouseClick(MouseEvent* event)
{
  if(event->Handled)
    return;

  if(event->ShiftPressed)
    MultiSelect();
  else if(event->CtrlPressed)
    Select(false);
  else
    Select(true);
}

void TreeRow::OnMouseDragBackground(MouseDragEvent* event)
{
  if(event->Handled)
    return;

  // If this row is selected, start the meta drag of the row
  if(mTree->mSelection->IsSelected(mIndex))
  {
    OnMouseDragRow(event);
  }
  // Otherwise, start the group select on the tree
  else
  {
    new RowSelector(event, mTree);
  }

  event->Handled = true;
}

void TreeRow::OnMouseDragRow(MouseEvent* event)
{
  if(event->Handled)
    return;

  if(event->ShiftPressed)
  {
    MultiSelect();
  }
  else if(event->CtrlPressed)
  {
    Select(false);
  }
  else
  {
    // Only select the object if it isn't already in the selection,
    // otherwise we want the selection to stay the same and move all of them
    if(!mTree->mSelection->IsSelected(mIndex))
      Select(true);
  }

  //Create Drag
  MetaDrag* drag = new MetaDrag(event->GetMouse(), mTree, this);

  //Get all currently selected items in the library view
  Array<DataIndex> selected;
  DataSelection* selection = mTree->mSelection;
  selection->GetSelected(selected);
 
  //Drag all the meta objects for these entries if they are valid for
  //drag and drop onto other Ui
  for (size_t i = 0; i < selected.Size(); ++i)
  {
    DataEntry* entry = mTree->mDataSource->ToEntry(selected[i]);
    Handle instance = mTree->mDataSource->ToHandle(entry);
    if(instance.IsNotNull())
      drag->AddObject(instance);
  }
}

void TreeRow::OnObjectPoll(ObjectPollEvent* event)
{
  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);
  Handle instance = mTree->mDataSource->ToHandle(entry);
  event->FoundObject = instance.Get<Cog*>();
  event->OwnedWidget = mTree->mArea->GetClientWidget();
}

void TreeRow::OnMouseEnter(MouseEvent* event)
{
  TreeEvent e;
  e.Row  = this;
  mTree->DispatchEvent(Events::MouseEnterRow, &e);

  Any toolTipData;
  DataSource* source = mTree->GetDataSource();
  DataEntry* entry = source->ToEntry(mIndex);
  source->GetData(entry, toolTipData, CommonColumns::ToolTip);

  if(toolTipData.IsHoldingValue())
  {
    ErrorIf(!toolTipData.Is<String>(), "ToolTips must be a string.");
    String message = toolTipData.Get<String>();
    if(message.Empty())
      return;

    // Create the tooltip
    ToolTip* toolTip = new ToolTip(this);
  
    toolTip->SetText(message);
    if(mValid)
      toolTip->SetColor(ToolTipColor::Default);
    else
      toolTip->SetColor(ToolTipColor::Red);
  
    // Position the tooltip
    ToolTipPlacement placement;
    placement.SetScreenRect(GetScreenRect());
    placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, 
                          IndicatorSide::Top, IndicatorSide::Bottom);
    toolTip->SetArrowTipTranslation(placement);
  
    mToolTip.SafeDestroy();
    mToolTip = toolTip;
  }
}

void TreeRow::OnMouseExit(MouseEvent* event)
{
  mToolTip.SafeDestroy();
  TreeEvent e;
  e.Row  = this;
  mTree->DispatchEvent(Events::MouseExitRow, &e);
}

InsertMode::Enum TreeRow::GetInsertPosition(DataEntry* entry, Vec2Param screenPos)
{
  Vec2 localPos = mBackground->ToLocal(screenPos);
  if(localPos.y < TreeViewUi::DragInsertSize)
    return InsertMode::Before;
  if((mSize.y - localPos.y) < TreeViewUi::DragInsertSize)
    return InsertMode::After;
  return InsertMode::On;
}

void TreeRow::GetInsertMode(Status& status, DataEntry* movingEntry,
                            Vec2Param screenPos, InsertMode::Type& mode)
{
  // Determine where we're moving to
  // If the mouse is near the top or bottom of the row, we want to Insert
  // in between items instead of on top

  Vec2 localPos = mBackground->ToLocal(screenPos);

  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);

  // Check if we can Insert on each
  Status results[3];

  // Default the text of the results
  results[InsertMode::Before].Message = "Move Item Before";
  results[InsertMode::On].Message = "Move Item";
  results[InsertMode::After].Message = "Move Item After";

  for(uint i = 0; i < InsertMode::Size; ++i)
    mTree->mDataSource->CanMove(results[i], movingEntry, entry, i);

  // Start as invalid
  mode = uint(-1);

  do 
  {
    bool beforeSupported = results[InsertMode::Before].Context != InsertError::NotSupported;
    bool onSupported = results[InsertMode::On].Context != InsertError::NotSupported;
    bool afterSupported = results[InsertMode::After].Context != InsertError::NotSupported;

    // Check to move before
    if(beforeSupported)
    {
      // Mouse is at the top small area
      if(localPos.y < TreeViewUi::DragInsertSize)
      {
        mode = InsertMode::Before;
        break;
      }

      // Cannot move on top, and the mouse is in the upper half 
      if(!onSupported && localPos.y <= (mSize.y * 0.5f))
      {
        mode = InsertMode::Before;
        break;
      }
    }

    // Check to move after
    if(afterSupported)
    {
      // Mouse is at the bottom small area
      if((mSize.y - localPos.y) < TreeViewUi::DragInsertSize)
      {
        mode = InsertMode::After;
        break;
      }

      // Cannot move on top, and the mouse is in the lower half 
      if(!onSupported && localPos.y > (mSize.y * 0.5f))
      {
        mode = InsertMode::After;
        break;
      }
    }

    if(onSupported)
    {
      mode = InsertMode::On;
      break;
    }
  } while (false);

  // Cannot move if no mode was set
  if(mode == uint(-1))
  {
    status.SetFailed(String(), InsertError::NotSupported);
    return;
  }

  // Set the text
  status  = results[mode];
}

void UpdateToolTipPlacement(TreeRow* row, InsertMode::Type mode, MetaDropEvent* e)
{
  if(row->IsRoot())
    return;
  ToolTipPlacement& placement = e->mToolTipPlacement;

  // Notify that we want our tooltip placement used
  e->mUseTooltipPlacement = true;

  // Center it to the row
  Rect rect = row->GetScreenRect();
  placement.SetScreenRect(rect);

  // If we're inserting before, make it point to the top left corner
  if(mode == InsertMode::Before)
    placement.mHotSpot = rect.TopLeft();
  // If we're inserting after, point to the bottom left
  else if(mode == InsertMode::After)
    placement.mHotSpot = rect.BottomLeft();

  placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left,
                        IndicatorSide::Top, IndicatorSide::Bottom);
}

void TreeRow::OnMetaDrop(MetaDropEvent* event)
{
  DataEntry* entry = mTree->mDataSource->ToEntry(mIndex);

  mTree->mMouseOver.Id = -1;

  //Dropped on the Row
  if(event->Instance.StoredType->IsA(ZilchTypeId(TreeRow)))
  {
    //Do not drop on self
    TreeRow* sourceRow = event->Instance.Get<TreeRow*>();
    if(sourceRow == NULL || this == sourceRow)
      return;

    //Do not drop on rows from other trees
    if(sourceRow->mTree != this->mTree)
      return;

    //Selected do not drop
    if(mTree->mSelection->IsSelected(mIndex))
      return;

    // Get the Insert mode based on the mouse position
    InsertMode::Type insertMode;
    Status insertStatus;
    DataEntry* movingEntry = mTree->mDataSource->ToEntry(sourceRow->mIndex);
    GetInsertMode(insertStatus, movingEntry, event->Position, insertMode);
    event->Result = insertStatus.Message;

    // Create a custom placement for the tooltip
    UpdateToolTipPlacement(this, insertMode, event);
    
    // Do nothing if we cannot Insert anywhere
    if(insertStatus.Context != InsertError::None)
    {
      event->Failed = true;
      return;
    }

    // If we're expanding, forward the event to our first child row
    if(insertMode == InsertMode::After && mExpanded && !mChildren.Empty())
    {
      if(!IsRoot())
      {
        TreeRow* firstChild = &mChildren.Front();
        firstChild->OnMetaDrop(event);
        return;
      }
    }

    // Drop is good
    
    // If testing, set highlight and return
    if(event->Testing)
    {
      mTree->mMouseOver = mIndex;
      mTree->mMouseOverMode = insertMode;
      MoveToFront();

      return;
    }

    // If something was dropped, we want to invalidate the mouse over
    // so that nothing is highlighted anymore
    mTree->mMouseOver.Id = -1;

    //Move all selected data indexes
    Array<DataIndex> indexes;
    mTree->mSelection->GetSelected(indexes);
    mTree->mDataSource->BeginBatchMove();
    mTree->mDataSource->Move(entry, indexes, insertMode);

    mTree->mDataSource->EndBatchMove();
    mTree->MarkAsNeedsUpdate();
  }
  else
  {
    // Get the insert mode based on where the mouse is relative to the mouse over row
    InsertMode::Enum insertMode = GetInsertPosition(entry, event->Position);

    // Create a custom placement for the tooltip
    UpdateToolTipPlacement(this, insertMode, event);

    mTree->mDataSource->OnMetaDrop(event, entry, insertMode);

    if(event->Testing && event->Handled)
    {
      mTree->mMouseOver = mIndex;
      mTree->mMouseOverMode = insertMode;
      MoveToFront();
    }
  }
}

void TreeRow::OnMouseDownExpander(MouseEvent* event)
{
  if(mExpanded)
  {
    Collapse();
  }
  else
  {
    Expand();
  }
}

void TreeRow::Fill(Array<TreeRow*>& nodes, uint depth)
{
  if(!mActive || mDestroyed)
    return;

  nodes.PushBack(this);
  this->mDepth = depth;
  this->mVisibleRowIndex = nodes.Size() - 1;
  forRange(TreeRow& child, mChildren.All())
  {
    child.Fill(nodes, depth+1);
  }
}

void TreeRow::OnDoubleClick(MouseEvent* event)
{
  if(!event->Handled)
  {
    DataEvent e;
    e.Index = mIndex;
    mTree->mDataSource->DispatchEvent(Events::DataActivated, &e);
  }
}

void TreeRow::OnRightUp(MouseEvent* event)
{
  if(!mTree->GetSelection()->IsSelected(mIndex))
    OnMouseClick(event);

  TreeEvent e;
  e.Row  = this;
  mTree->DispatchEvent(Events::TreeRightClick, &e);
}

void TreeRow::OnKeyUp(KeyboardEvent* event)
{
  //if(!mTree->GetSelection()->IsSelected(mIndex))
  //  OnKeyPress(event);

  TreeEvent e;
  e.Row = this;
  mTree->DispatchEvent(Events::TreeKeyPress, &e);
}

//---------------------------------------------------------------- Column Header
//******************************************************************************
ColumnHeader::ColumnHeader(TreeView* tree, ColumnFormat* format)
  : Composite(tree), mTree(tree), mFormat(format)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(TreeViewUi::HeaderColor);
  mLabel = NULL;
  mIcon = NULL;
  mSortFlip = false;

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
}

//******************************************************************************
void ColumnHeader::SetText(StringParam name)
{
  if(mLabel == NULL)
    mLabel = new Label(this, "BoldText");
  mLabel->SetText(name);
}

//******************************************************************************
void ColumnHeader::SetIcon(StringParam iconName)
{
  if(mIcon == NULL)
    mIcon = CreateAttached<Element>(iconName);
  mIcon->SetSize(Pixels(16, 16));
  mIcon->SetTranslation(SnapToPixels(Pixels(2, 2, 0)));
}

//******************************************************************************
void ColumnHeader::UpdateTransform()
{
  mBackground->SetSize(GetSize());
  
  if(mLabel)
  {
    mLabel->SetTranslation(Pixels(0, 0, 0));
//     Thickness borderThickness = mBackground->GetBorderThickness();
// 
//     LayoutResult lr = RemoveThickness(borderThickness, mSize, Pixels(0,-1,0));
//     mLabel->SetTranslation(lr.Translation);
    mLabel->SizeToContents();

    mLabel->SetColor(Vec4(1, 1, 1, 0.33f));
  }
  Composite::UpdateTransform();
}

//******************************************************************************
void ColumnHeader::OnMouseEnter(MouseEvent* e)
{
  mBackground->SetColor(TreeViewUi::HeaderHighlight);
}

//******************************************************************************
void ColumnHeader::OnLeftClick(MouseEvent* e)
{
  if (DataSource* source = mTree->GetDataSource())
  {
    source->Sort(NULL, mFormat->Name, mSortFlip);
    mTree->Refresh();

    mSortFlip = !mSortFlip;
  }
}

//******************************************************************************
void ColumnHeader::OnMouseExit(MouseEvent* e)
{
  mBackground->SetColor(TreeViewUi::HeaderColor);
}

//--------------------------------------------------- Column Resizer Manipulator
class ColumnResizerManipulator : public MouseManipulation
{
public:
  ColumnResizer* mResizer;

  ColumnResizerManipulator(Mouse* mouse, ColumnResizer* resizer) 
    : MouseManipulation(mouse, resizer)
  {
    mResizer = resizer;
  }

  void OnMouseMove(MouseEvent* event) override
  {
    Vec2 localPos = mResizer->GetParent()->ToLocal(event->Position);
    mResizer->ResizeHeaders(localPos.x);
  }

  void OnMouseUp(MouseEvent* event) override
  {
    this->Destroy();
    event->GetMouse()->SetCursor(Cursor::Arrow);
  }
};

//--------------------------------------------------------------- Column Resizer
ColumnResizer::ColumnResizer(Composite* parent, ColumnFormat* left, 
                           ColumnFormat* right)
  : Composite(parent)
{
  mSpacer = new Spacer(this);
  mLeftFormat = left;
  mRightFormat = right;

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
}

void ColumnResizer::UpdateTransform()
{
  mSpacer->SetSize(GetSize());
  Composite::UpdateTransform();
}

void ColumnResizer::ResizeHeaders(float pos)
{
  // Clamp the position into the movable range
  float leftBound = (mLeftFormat->StartX + mLeftFormat->MinWidth);
  float rightBound = (mRightFormat->StartX + mRightFormat->CurrSize.x - mRightFormat->MinWidth);
  pos = Math::Clamp(pos, leftBound, rightBound);

  // The size (in pixels) of the left column
  float pixelSizeLeft = pos - mLeftFormat->StartX;

  // Store the old flex value of the left in case they are both flex
  float oldFlexLeft = mLeftFormat->FlexSize;

  ColumnType::Type leftType = mLeftFormat->ColumnType;
  ColumnType::Type rightType = mRightFormat->ColumnType;

  // If either size is fixed then don't update their sizes
  if(leftType == ColumnType::Fixed || rightType == ColumnType::Fixed)
    return;

  // Update Left column
  if(leftType == ColumnType::Flex)
  {
    // Apply the difference in scale in pixels to the flex size
    float scaleDifference = (pixelSizeLeft / mLeftFormat->CurrSize.x);
    mLeftFormat->FlexSize *= scaleDifference;
  }
  else if(leftType == ColumnType::Sizeable)
  {
    // Set it to the newly calculated size
    mLeftFormat->FixedSize.x = pixelSizeLeft;
  }

  // Update right column
  if(rightType == ColumnType::Flex)
  {
    // If the left is flex as well, we want 
    if(leftType == ColumnType::Flex)
      mRightFormat->FlexSize += oldFlexLeft - mLeftFormat->FlexSize;
    else
    {
      // Apply the difference in scale in pixels to the flex size
      float newSizeRight = mRightFormat->StartX + mRightFormat->CurrSize.x - pos;
      float scaleDifference = (newSizeRight / mRightFormat->CurrSize.x);
      mRightFormat->FlexSize *= scaleDifference;
    }
  }
  else if(rightType == ColumnType::Sizeable)
  {
    // Add the difference to the right
    mRightFormat->FixedSize.x -= pixelSizeLeft - mLeftFormat->CurrSize.x;
  }
  MarkAsNeedsUpdate(true);
}

void ColumnResizer::OnMouseEnter(MouseEvent* event)
{
  event->GetMouse()->SetCursor(Cursor::SizeWE);
}

void ColumnResizer::OnMouseDown(MouseEvent* event)
{
  new ColumnResizerManipulator(event->GetMouse(), this);
}

void ColumnResizer::OnMouseExit(MouseEvent* event)
{
  event->GetMouse()->SetCursor(Cursor::Arrow);
}

//-------------------------------------------------------------------- Tree View
TreeView::TreeView(Composite* parent)
  :Composite(parent)
{
  static const String className = "TreeGrid";
  mDefSet = mDefSet->GetDefinitionSet(className);
  mRefreshOnValueChange = false;
  mRowHeight = TreeViewUi::ItemHeight;
  mArea = new ScrollArea(this);
  mArea->SetClientSize(Pixels(20, 20));
  mMinSize = Vec2(100, 100);

  mMouseOver = -1;
  mMouseOverMode = InsertMode::On;

  mSelection = new HashDataSelection();
  mExpanded = new HashDataSelection();
  mDataSource = NULL;

  mRoot = NULL;

  SetFormatNameAndType();

  Widget* clientWidget = mArea->GetClientWidget();

  ConnectThisTo(clientWidget, Events::MouseExit, OnMouseExit);
  ConnectThisTo(clientWidget, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(clientWidget, Events::MouseExitHierarchy, OnMouseExitHierarchy);
 
  ConnectThisTo(clientWidget, Events::MouseUpdate, OnMouseUpdate);

  ConnectThisTo(clientWidget, Events::KeyDown, OnKeyDown);
  ConnectThisTo(clientWidget, Events::KeyUp, OnKeyUp);
  ConnectThisTo(clientWidget, Events::KeyRepeated, OnKeyRepeated);
  
  //Forward background Right click and meta drop to root
  ConnectThisTo(mArea->GetBackground(), Events::MetaDrop, OnMetaDropBg);
  ConnectThisTo(mArea->GetBackground(), Events::MetaDropTest, OnMetaDropBg);
  ConnectThisTo(mArea->GetBackground(), Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(mArea->GetBackground(), Events::LeftClick, OnLeftClickBg);
  ConnectThisTo(mArea->GetBackground(), Events::LeftMouseDrag, OnMouseDragBg);

  ConnectThisTo(this, Events::MetaDropUpdate, OnMetaDropUpdate);
}

void TreeView::SetFormatName()
{
  TreeFormatting formatting;
  formatting.Columns.Resize(2);
  formatting.Columns[0].Index = 0;
  formatting.Columns[0].Name = CommonColumns::Icon;
  formatting.Columns[0].ColumnType = ColumnType::Fixed;
  formatting.Columns[0].FixedSize = Pixels(16, 20);
  formatting.Columns[0].Editable = false;
  formatting.Columns[0].CustomEditor = cDefaultIconEditor;
  
  formatting.Columns[1].Index = 1;
  formatting.Columns[1].Name = CommonColumns::Name;
  formatting.Columns[1].ColumnType = ColumnType::Flex;
  formatting.Columns[1].FixedSize.y = Pixels(20);
  formatting.Columns[1].FlexSize = 1;
  formatting.Columns[1].Editable = true;

  SetFormat(formatting);
}

void TreeView::SetFormatNameAndType()
{
  TreeFormatting formatting;
  formatting.Columns.Resize(3);
  formatting.Columns[0].Index = 0;
  formatting.Columns[0].Name = CommonColumns::Icon;
  formatting.Columns[0].ColumnType = ColumnType::Fixed;
  formatting.Columns[0].FixedSize = Pixels(16, 20);
  formatting.Columns[0].Editable = false;
  formatting.Columns[0].CustomEditor = cDefaultIconEditor;

  formatting.Columns[1].Index = 1;
  formatting.Columns[1].ColumnType = ColumnType::Flex;
  formatting.Columns[1].FlexSize = 1.3f;
  formatting.Columns[1].Name = CommonColumns::Name;
  formatting.Columns[1].HeaderName = "Resource";
  formatting.Columns[1].Editable = true;

  formatting.Columns[2].Index = 2;
  formatting.Columns[2].ColumnType = ColumnType::Flex;
  formatting.Columns[2].FlexSize = 1;
  formatting.Columns[2].Name = CommonColumns::Type;
  formatting.Columns[2].HeaderName = "Type";
  formatting.Columns[2].Editable = false;

  SetFormat(formatting);
}

void TreeView::SetFormat(TreeFormatting& format)
{
  mFormatting = format;
  
  auto headers = mHeaders.All();
  while(!headers.Empty())
  {
    ColumnHeader* header = headers.Front();
    headers.PopFront();
    header->Destroy();
  }
  auto resizers = mHeaderResizers.All();
  while(!headers.Empty())
  {
    ColumnResizer* resizer = resizers.Front().second;
    resizers.PopFront();
    resizer->Destroy();
  }

  mHeaders.Clear();
  mHeaderResizers.Clear();
  ClearAllRows();
  if(mDataSource)
  {
    //Build new Tree
    DataEntry* root = mDataSource->GetRoot();
    mRoot = new TreeRow(this, NULL, root);
    mRoot->Expand();
  }
}

void TreeView::SetRowHeight(float height)
{
  mRowHeight = height;
  MarkAsNeedsUpdate();
}

void TreeView::ShowRow(DataIndex& index)
{
  DataEntry* entry = mDataSource->ToEntry(index);
  if(entry == NULL)
    return;

  TreeRow* row = FindRowByIndex(index);

  DataEntry* parent = mDataSource->Parent(entry);
  DataIndex parentIndex = mDataSource->ToIndex(parent);

  // We want to search up starting at our parent for the first visible row
  while(parent)
  {
    DataIndex currIndex = mDataSource->ToIndex(parent);

    // If there is a visible row for this index, expand it and we're done
    row = FindRowByIndex(currIndex);
    if(row)
    {
      row->Expand();
      break;
    }

    // It doesn't have a row, but when the row is created, we want it 
    // to be expanded (instead of recursively creating and then expanding).
    // Adding it to this data selection will make it automatically be
    // expanded when it's created
    mExpanded->Select(currIndex);

    // Go to the next parent
    parent = mDataSource->Parent(parent);
  }

  // Rebuild the tree
  Refresh();

  // Update transform will update the size of the scroll area
  UpdateTransform();

  // Scroll to this object
  Zero::TreeRow* dataRow = FindRowByIndex(index);

  if(dataRow)
  {
    uint rowIndex = dataRow->mVisibleRowIndex;
    float y = float(rowIndex+1) / (float)mRows.Size();
    mArea->SetScrolledPercentage(Vec2(0, y));
  }


  MarkAsNeedsUpdate();
}

void TreeView::SetRefreshOnValueChange(bool state)
{
  mRefreshOnValueChange = state;
}

void TreeView::Refresh()
{
  if(mRoot)
    mRoot->Refresh();
  UpdateTransform();
}

void TreeView::SetDataSource(DataSource* dataSource)
{
  if (mDataSource)
  {
    //Stop listening to old data source
    mDataSource->GetDispatcher()->Disconnect(this);
    mDataSource = NULL;
  }

  ClearAllRows();

  mDataSource = dataSource;

  if (mDataSource)
  {
    //Connect all events
    ConnectThisTo(mDataSource, Events::DataRemoved, OnDataErased);
    ConnectThisTo(mDataSource, Events::DataAdded, OnDataAdded);
    ConnectThisTo(mDataSource, Events::DataModified, OnDataModified);
    ConnectThisTo(mDataSource, Events::DataReplaced, OnDataReplaced);

    //Build new Tree
    DataEntry* root = mDataSource->GetRoot();
    mRoot = new TreeRow(this, NULL, root);
    mRoot->Expand();
  }
}

DataSource* TreeView::GetDataSource()
{
  return mDataSource;
}

void TreeView::SetSelection(DataSelection* selection)
{
  SafeDelete(mSelection);
  mSelection = selection;
}

void TreeView::ClearAllRows()
{
  if(mRoot)
  {
    mRoot->RecursiveDestroy();
    mRoot = NULL;
  }
}

ScrollArea* TreeView::GetScrollArea()
{
  return mArea;
}

TreeView::~TreeView()
{
  SafeDelete(mSelection);
  SafeDelete(mExpanded);
}

void TreeView::OnKeyDown(KeyboardEvent* event)
{
  HandleKeyLogic(event);
}

void TreeView::OnKeyRepeated(KeyboardEvent* event)
{
  HandleKeyLogic(event);
}

void TreeView::HandleKeyLogic(KeyboardEvent* event)
{
  // Do nothing if the event was already handled (likely from editing a text box)
  if(event->Handled)
    return;

  // We're going to move based on what's currently selected
  uint minIndex, maxIndex;
  GetSelectionRange(&minIndex, &maxIndex);

  // If Up is pressed, move up the tree
  if(event->Key == Keys::Up)
  {
    // Move up an index ( -1 )
    int newMin = (int)minIndex - 1;
    
    // Make sure we don't go passed the first row (take into account the root)
    newMin = Math::Max(newMin, 1);

    // Select the new row
    if(event->ShiftPressed)
      SelectRowsInRange(newMin, maxIndex);
    else
      SelectRowsInRange(newMin, newMin);
    mSelection->SelectFinal();

    float y = float(newMin - 1) * mRowHeight;
    mArea->ScrollAreaToView(Vec2(0, y), Vec2(0,y));
  }
  // If Down is pressed, move down the tree
  else if(event->Key == Keys::Down)
  {
    // Move down an index
    uint newMax = maxIndex + 1;

    // Make sure we don't go passed the last row
    newMax = Math::Min((size_t)newMax, (size_t)(mRows.Size() - 1));

    // Select the new row
    if(event->ShiftPressed)
      SelectRowsInRange(minIndex, newMax);
    else
      SelectRowsInRange(newMax, newMax);
    mSelection->SelectFinal();

    float y = float(newMax + 1) * mRowHeight;
    mArea->ScrollAreaToView(Vec2(0, y), Vec2(0,y));
  }
  else if(event->Key == Keys::Right && minIndex < mRows.Size())
  {
    // If right is pressed, attempt to expand the row
    TreeRow* row = mRows[minIndex];
    row->Expand();
  }
  else if(event->Key == Keys::Left && minIndex < mRows.Size())
  {
    // If the row is expanded, collapse it and remain selected on it
    TreeRow* row = mRows[minIndex];
    if(row->mExpanded)
    {
      row->Collapse();
    }
    // If it's not expanded and it has a parent that's not the root, 
    // move the selection to the parent
    else if(row->mParentRow && row->mParentRow != mRoot)
    {
      mSelection->SelectNone();
      mSelection->Select(row->mParentRow->mIndex);
    }
  }

  MarkAsNeedsUpdate();
}

//Ui Events
void TreeView::OnKeyUp(KeyboardEvent* event)
{
  
}

void TreeView::OnMouseExit(MouseEvent* event)
{
  if(!this->IsAncestorOf(event->Source))
  {
    if(mMouseOver.Id != -1)
    {
      TreeRow* row = FindRowByIndex(mMouseOver);
      if(row)
        row->UpdateTransformExternal();
    }
    mMouseOver = -1;
  }
}

void TreeView::OnMouseExitHierarchy(MouseEvent* event)
{
  mMouseOver = -1;
  MarkAsNeedsUpdate();
}

void TreeView::DoFocus()
{

}

void TreeView::OnMouseEnter(MouseEvent* event)
{
  DoFocus();
}

void TreeView::OnLeftClickBg(MouseEvent* event)
{
  mSelection->SelectNone();
  mSelection->SelectFinal();
}

void TreeView::OnMouseDragBg(MouseDragEvent* event)
{
  DoFocus();
  new RowSelector(event, this);
}

void TreeView::OnMouseUpdate(MouseEvent* event)
{
  Vec2 local = mArea->ToLocal(event->Position);
  Vec2 offset = mArea->GetClientOffset(); 
  offset.y += GetHeaderRowHeight();
  Vec2 world = local - offset;

  MarkAsNeedsUpdate();
  
  uint index = uint(world.y / mRowHeight) + 1;
  if(index < mRows.Size())
  {
    TreeRow* row = mRows[index];
    mMouseOver = row->mIndex;
  }
  else
  {
    mMouseOver = -1;
  }
  mMouseOverMode = InsertMode::On;
}

void TreeView::OnMetaDropBg(MetaDropEvent* event)
{
  mRoot->OnMetaDrop(event);
}

void TreeView::OnRightClickBg(MouseEvent* event)
{
  if(mRoot)
    mRoot->OnRightUp(event);
}

TreeRow* TreeView::FindRowByIndex(DataIndex& index)
{
  return mRowMap.FindValue(index.Id, NULL);
}

uint TreeView::FindRowIndex(TreeRow* row)
{
  return mRows.FindIndex(row);
}

void TreeView::MoveToView(TreeRow* row)
{
  Vec3 offset = row->GetTranslation();
  Vec2 min = Vec2(offset.x, offset.y);
  mArea->ScrollAreaToView(min, min + Pixels(0, 20));
  this->MarkAsNeedsUpdate();
}

TreeRow* TreeView::FindRowByColumnValue(StringParam column, StringParam value)
{
  //brute force search through all visible rows

  //Which column to search in
  ColumnFormat& format = GetColumn(column);

  forRange(TreeRow* row, mRows.All())
  {
    //Get the value of the column
    Any var;
    row->mEditorColumns[format.Index]->GetVariant(var);

    //Check the value, strings for now
    if(var.ToString() == value)
      return row;
  }

  return NULL;
}

void TreeView::SelectFirstRow()
{
  if(mRows.Size() < 2)
    return;
  mSelection->SelectNone();
  mSelection->Select(mRows[1]->mIndex);
  MarkAsNeedsUpdate();

  mSelection->SelectFinal();
}

//Data Source Events
void TreeView::OnDataErased(DataEvent* event)
{
  TreeRow* row = FindRowByIndex(event->Index);
  if(row)
  {
    mRowMap.Erase(event->Index.Id);
    row->RecursiveDestroy();
  }
}

void TreeView::OnDataAdded(DataEvent* event)
{
  //Id is parent
  TreeRow* row = FindRowByIndex(event->Index);
  if(row)
    row->Refresh();
}

void TreeView::OnDataModified(DataEvent* event)
{
  TreeRow* row = FindRowByIndex(event->Index);
  if(row)
    row->RefreshData();
  else
    Refresh();
}

void TreeView::OnDataReplaced(DataReplaceEvent* e)
{
  if(mExpanded->IsSelected(e->mOldIndex))
  {
    mExpanded->Deselect(e->mOldIndex);
    mExpanded->Select(e->mNewIndex, false);
  }
}

ColumnFormat& TreeView::GetColumn(StringParam columnName)
{
  for(uint i=0;i<mFormatting.Columns.Size();++i)
  {
    if(mFormatting.Columns[i].Name == columnName)
      return mFormatting.Columns[i];
  }
  return mFormatting.Columns[0];
}

void TreeView::GetSelectionRange(uint* minIndex, uint* maxIndex)
{
  *minIndex = (uint)-1;
  *maxIndex = 0;

  // Get all selected
  Array<DataIndex> selected;
  mSelection->GetSelected(selected);

  // Walk through the selected and min / max
  for(uint i = 0; i < selected.Size(); ++i)
  {
    // Get the index of the current row
    TreeRow* currRow = FindRowByIndex(selected[i]);
    uint currIndex = FindRowIndex(currRow);

    // Min / max
    *minIndex = Math::Min(currIndex, *minIndex);
    *maxIndex = Math::Max(currIndex, *maxIndex);
  }
}

void TreeView::SelectRowsInRange(uint min, uint max)
{
  if(mDataSource == NULL)
    return;

  // Clear the selection
  mSelection->SelectNone(false);

  // Select all in the given range
  for(uint i = min; i <= max && i < mRows.Size(); ++i)
  {
    TreeRow* row = mRows[i];
    mSelection->Select(row->mIndex, false);
  }

  MarkAsNeedsUpdate();

  mSelection->SelectionModified();
}

void TreeView::SelectAll()
{
  // you can't delete the editor camera so size is always at least 1
  SelectRowsInRange(0, mRows.Size()-1);
}

float TreeView::GetHeaderRowHeight()
{
  if(mFormatting.Flags.IsSet(FormatFlags::ShowHeaders))
    return mRowHeight;
  return 0.0f;
}

void TreeView::OnMetaDropUpdate(MetaDropEvent* e)
{
  DragScroll(e->Position);
}

void TreeView::DragScroll(Vec2Param screenPosition)
{
  Vec2 local = mArea->ToLocal(screenPosition);

  float scrollSpeed = 0;
  float maxSpeed = TreeViewUi::MaxDragScrollSpeed;

  if(mArea->IsScrollBarVisible(1))
  {
    if(local.y < TreeViewUi::DragScrollSize)
    {
      float percentage = 1.0f - (local.y / TreeViewUi::DragScrollSize);
      scrollSpeed = -percentage * maxSpeed;
    }
    else if(local.y > (mSize.y - TreeViewUi::DragScrollSize))
    {
      float percentage = 1.0f - (mSize.y - local.y) / TreeViewUi::DragScrollSize;
      scrollSpeed = percentage * maxSpeed;
    }
  }

  scrollSpeed = Math::Clamp(scrollSpeed, -maxSpeed, maxSpeed);
  mArea->ScrollPixels(Vec2(0, -scrollSpeed));
}

void TreeView::UpdateTransform()
{
  // Update the scroll area to be the same size as us
  mArea->SetSize(mSize);
  mArea->DisableScrollBar(0);

  // Clear all current nodes
  mRows.Clear();

  // Collect all visible nodes
  if(mRoot)
    mRoot->Fill(mRows, 0);

  // Build the column starting / ending positions
  UpdateColumnTransforms();

  // Build and place the separators
  UpdateSeparators();

  UpdateHeaders();

  float rowWidth = mSize.x;
  if(mArea->IsScrollBarVisible(1))
    rowWidth -= mArea->GetScrollBarSize();

  // Header floats inside the search area
  float headerRowHeight = GetHeaderRowHeight();
  float areaForItems = mSize.y - headerRowHeight;

  Vec2 scrollOffset = mArea->GetClientOffset();

  // Set the transform of each row in the tree
  float rowY = headerRowHeight;

  // Even though the value is 'ceil'ed the remainder area may be divided
  // between two visible rows, one at the top and one at bottom.
  // Simply add another row to the number of visible rows to correct this
  uint visibleRows = uint(Math::Ceil(areaForItems / mRowHeight));
  visibleRows++;

  uint activeRows = mRows.Size();

  // Update the client size with the size of all the rows
  mArea->SetClientSize(Vec2(mSize.x, float(activeRows) * mRowHeight));

  uint startVisible = uint(Math::Floor((-scrollOffset.y) / mRowHeight));
  startVisible = Math::Min(activeRows, startVisible);

  // Skip the root
  bool showRoot = mFormatting.Flags.IsSet(FormatFlags::ShowRoot);
  if(!showRoot && !mRows.Empty())
  {
    ++startVisible;
    startVisible = Math::Min(activeRows, startVisible);
    // Shift back row y 
    rowY -= mRowHeight;
  }

  // Compute the end of the visible rows
  uint endVisible = Math::Min((size_t)mRows.Size(), (size_t)(startVisible + visibleRows));

  // Deactivate all rows before the first visible
  for(uint i = 0; i < startVisible; ++i)
  {
    TreeRow* item = mRows[i];
    item->SetActive(false);
    rowY += mRowHeight;
  }

  // Activate and move all visible rows
  for(uint i = startVisible; i < endVisible; ++i)
  {
    TreeRow* item = mRows[i];
    item->SetActive(true);
    item->SetTranslation(Vec3(0, rowY, 0));
    item->SetSize(Vec2(rowWidth, mRowHeight));

    item->mGraphicBackground->MoveToBack();
    item->mGraphicBackground->SetTranslation(Vec3(0, 0, 0));
    item->mGraphicBackground->SetSize(Vec2(rowWidth, mRowHeight));

    // Update whether or not the row is valid
    DataEntry* entry = mDataSource->ToEntry(item->mIndex);
    item->mValid = mDataSource->IsValid(entry);

    // Update the background color
    item->UpdateBgColor(i % 2);

    rowY += mRowHeight;
  }

  // Deactivate all rows from first non visible to the number of rows
  for(uint i = endVisible; i < activeRows; ++i)
  {
    TreeRow* item = mRows[i];
    item->SetTranslation(Vec3(0, rowY, 0));
    item->SetActive(false);
    rowY += mRowHeight;
  }

  Composite::UpdateTransform();
}

bool TreeView::TakeFocusOverride()
{
  mArea->GetClientWidget()->SoftTakeFocus();
  return true;
}

void TreeView::UpdateColumnTransforms()
{
  // Here we're setting the position and size of each column.
  // This assumes we have the entire width of the row and does not take into
  // account indentations.  That will be applied later when the row is updated

  // We want to find out how much room we have after all fixed columns
  float unfixedWidth = mSize.x;
  if(mArea->IsScrollBarVisible(1))
    unfixedWidth -= mArea->GetScrollBarSize();

  float totalFlex = 0.0f;

  forRange(ColumnFormat& column, mFormatting.Columns.All())
  {
    // If it's fixed, subtract its size
    if(column.ColumnType != ColumnType::Flex)
      unfixedWidth -= column.FixedSize.x;
    // If it's flex, add to the total flex size
    else
      totalFlex += column.FlexSize;
  }

  // Used to keep track of where each column should start
  float currentX = 0.0f;

  // Walk through each column and set its size and translation
  forRange(ColumnFormat& column, mFormatting.Columns.All())
  {
    // The starting position doesn't rely on the column type
    column.StartX = currentX;

    // If it's fixed, we just Assign it its desired size
    if(column.ColumnType != ColumnType::Flex)
    {
      column.CurrSize = column.FixedSize;
    }
    else
    {
      // If it's not fixed, calculate the size based off the total flex
      float flex = column.FlexSize / totalFlex;
      float width = unfixedWidth * flex;
      column.CurrSize = Vec2(width, mRowHeight);
    }

    // Add the size to move onto the next column
    currentX += column.CurrSize.x;
  }
}

void TreeView::UpdateHeaders()
{
  // Do nothing if we're not showing headers
  if(!mFormatting.Flags.IsSet(FormatFlags::ShowHeaders))
    return;

  uint currHeader = 0;
  for(uint i = 0; i < mFormatting.Columns.Size(); ++i)
  {
    // Get the formatting of the column
    ColumnFormat& format = mFormatting.Columns[i];

    // Do nothing if there's no header
    if(format.HeaderName.Empty() && format.HeaderIcon.Empty())
      continue;

    // Create a new header
    if(currHeader >= mHeaders.Size())
      mHeaders.PushBack(new ColumnHeader(this, &format));
    
    ColumnHeader* header = mHeaders[currHeader];

    float startPos = format.StartX;
    float extraWidth = 0.0f;

    // We want to walk left and keep pushing the start of the header left
    // to eat any columns that don't have a header
    // We may have to do something similar for eating right
    for(int j = (int)i - 1; j >= 0; --j)
    {
      // Get the formatting of the column
      ColumnFormat& testFormat = mFormatting.Columns[j];
      if(!testFormat.HeaderIcon.Empty() || !testFormat.HeaderName.Empty())
        break;
      startPos -= testFormat.CurrSize.x;
      extraWidth += testFormat.CurrSize.x;
    }

    header->SetTranslation(SnapToPixels(Vec3(startPos, 0, 0)));

    // Add 1 pixel to the last columns width
    if(i != mFormatting.Columns.Size() - 1)
      extraWidth += Pixels(1);

    float finalWidth = format.CurrSize.x + extraWidth;

    // Set the size to the final width
    header->SetSize(SnapToPixels(Vec2(finalWidth, TreeViewUi::ItemHeight)));

    // Set the respective header type
    if(!format.HeaderName.Empty())
      header->SetText(format.HeaderName);
    else
      header->SetIcon(format.HeaderIcon);

    // Update the column resizers
    if(mFormatting.Flags.IsSet(FormatFlags::ColumnsResizable))
    {
      // Attempt to find the current resizer for this header
      ColumnResizer* resizer = mHeaderResizers.FindValue(header, NULL);
      if(resizer == NULL)
      {
        if((i + 1) < mFormatting.Columns.Size())
        {
          ColumnFormat* nextFormat = &mFormatting.Columns[i + 1];
          resizer = new ColumnResizer(this, &format, nextFormat);
          mHeaderResizers.Insert(header, resizer);
        }
      }

      // Place the resizer to the right side of this header.
      if(resizer)
      {
        Vec3 resizerPos(startPos + finalWidth - cResizerWidth * 0.5f, 0, 0);
        resizer->SetTranslation(SnapToPixels(resizerPos));
        resizer->SetSize(SnapToPixels(Vec2(cResizerWidth, format.CurrSize.y)));
      }
    }

    ++currHeader;
  }

  // Move all the resizers to the front of the headers so that they get hit
  // first with ray casts
  typedef Pair<ColumnHeader*, ColumnResizer*> ResizerPair;
  forRange(ResizerPair& entry, mHeaderResizers.All())
  {
    ColumnResizer* resizer = entry.second;
    resizer->MoveToFront();
  }
}

void TreeView::UpdateSeparators()
{
  if(!mFormatting.Flags.IsSet(FormatFlags::ShowSeparators))
    return;

  UpdateColumnSeparators();
}

void TreeView::UpdateColumnSeparators()
{
  bool foundFlex = false;
  uint separatorCount = 0;

  // Rebuild separators
  for(uint i = 0; i < mFormatting.Columns.Size(); ++i)
  {
    ColumnFormat& column = mFormatting.Columns[i];

    // We want to skip everything until after we found the first flex column
    if(!foundFlex)
    {
      if(column.ColumnType == ColumnType::Flex)
        foundFlex = true;

      continue;
    }

    // Create a new separator if needed
    if(separatorCount >= mColumnSeparators.Size())
    {
      Element* separator = CreateAttached<Element>(cWhiteSquare);
      separator->SetColor(TreeViewUi::ColumnSeparatorColor);
      mColumnSeparators.PushBack(separator);
    }

    Element* separator = mColumnSeparators[separatorCount];

    ++separatorCount;

    // Set its transform
    separator->SetTranslation(SnapToPixels(Vec3(column.StartX, 0, 0)));
    separator->SetSize(SnapToPixels(Vec2(Pixels(2), mSize.y)));
  }
}

}//namespace Zero
