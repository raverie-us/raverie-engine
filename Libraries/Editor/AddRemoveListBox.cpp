///////////////////////////////////////////////////////////////////////////////
///
/// \file AddRemoveListBox.cpp
/// Implementation of the AddRemoveListBox Widget.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(AddPressed);
  DefineEvent(RemovePressed);
  DefineEvent(MoveUpPressed);
  DefineEvent(MoveDownPressed);
}

AddRemoveListBox::AddRemoveListBox(Composite* parent, float width, 
                                   StringParam dataTypeName) : Composite(parent)
{
  mDataTypeName = dataTypeName;

  // Build the layout
  BuildLayout(this, width);
}

void AddRemoveListBox::SetDataSource(ListSource* dataSource)
{
  mListBox->SetDataSource(dataSource);
}

void AddRemoveListBox::SetSelectedItem(uint index, bool sendEvent)
{
  mListBox->SetSelectedItem(index, sendEvent);
}

uint AddRemoveListBox::GetSelectedItem()
{
  return mListBox->GetSelectedItem();
}

void AddRemoveListBox::BuildLayout(Composite* parent, float width)
{
  // Create the list box to display the data
  mListBox = new ListBox(parent);
  mListBox->SetSize(Pixels(width, 65));

  // Connect to the list box
  ConnectThisTo(mListBox, Events::ItemSelected, ItemSelected);

  Composite* buttonComposite = new Composite(parent);
  buttonComposite->SetLayout(CreateStackLayout());
  {
    // Create the buttons
    CreateButtons(buttonComposite);
    buttonComposite->SetSize(Pixels(100, 20));
  }

  // Use a row layout for the whole composite (buttons on the right)
  parent->SetLayout(CreateRowLayout());
}

void AddRemoveListBox::CreateButtons(Composite* parent)
{
  // Make an "add" button so we can add data
  mAddButton = new IconButton(parent);
  mAddButton->SetIcon("BigPlus");
  String toolTip = BuildString("Add ", mDataTypeName);
  mAddButton->SetToolTip(toolTip);
  ConnectThisTo(mAddButton, Events::ButtonPressed, AddPressed);

  // Make a "remove" button so we can remove data
  mRemoveButton = new IconButton(parent);
  mRemoveButton->SetIcon("RedX");
  toolTip = BuildString("Remove Selected ", mDataTypeName);
  mRemoveButton->SetToolTip(toolTip);
  ConnectThisTo(mRemoveButton, Events::ButtonPressed, RemovePressed);

  // Make a "sort up" button so we can move selected items
  mMoveUpButton = new IconButton(parent);
  mMoveUpButton->SetIcon("Up");
  toolTip = BuildString("Move Selected ", mDataTypeName, " Up");
  mMoveUpButton->SetToolTip(toolTip);
  ConnectThisTo(mMoveUpButton, Events::ButtonPressed, MoveUpPressed);

  // Make a "sort down" button so we can move selected items
  mMoveDownButton = new IconButton(parent);
  mMoveDownButton->SetIcon("Down");
  toolTip = BuildString("Move Selected ", mDataTypeName, " Down");
  mMoveDownButton->SetToolTip(toolTip);
  ConnectThisTo(mMoveDownButton, Events::ButtonPressed, MoveDownPressed);
}

void AddRemoveListBox::ItemSelected(ObjectEvent* event)
{
  ObjectEvent objectEvent(this);
  // Forward the event to ourself
  GetDispatcher()->Dispatch(Events::ItemSelected, &objectEvent);
}

void AddRemoveListBox::AddPressed(ObjectEvent* event)
{
  ObjectEvent objectEvent(this);
  // Forward the event to ourself
  GetDispatcher()->Dispatch(Events::AddPressed, &objectEvent);
}

void AddRemoveListBox::RemovePressed(ObjectEvent* event)
{
  // Return if the data source is null or nothing is in there
  ListSource* dataSource = mListBox->GetDataSource();
  if(dataSource == nullptr || dataSource->GetCount() == 0)
    return;

  ObjectEvent objectEvent(this);
  // Forward the event to ourself
  GetDispatcher()->Dispatch(Events::RemovePressed, &objectEvent);
}

void AddRemoveListBox::MoveUpPressed(ObjectEvent* event)
{
  // Return if the data source is null or nothing is in there
  ListSource* dataSource = mListBox->GetDataSource();
  if(dataSource == nullptr || dataSource->GetCount() == 0)
    return;

  ObjectEvent objectEvent(this);
  // Forward the event to ourself
  GetDispatcher()->Dispatch(Events::MoveUpPressed, &objectEvent);
}

void AddRemoveListBox::MoveDownPressed(ObjectEvent* event)
{
  // Return if the data source is null or nothing is in there
  ListSource* dataSource = mListBox->GetDataSource();
  if(dataSource == nullptr || dataSource->GetCount() == 0)
    return;

  ObjectEvent objectEvent(this);
  // Forward the event to ourself
  GetDispatcher()->Dispatch(Events::MoveDownPressed, &objectEvent);
}

}//namespace Zero
