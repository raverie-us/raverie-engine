///////////////////////////////////////////////////////////////////////////////
///
/// \file AddRemoveListBox.hpp
/// Declaration of the AddRemoveListBox Widget.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(AddPressed);
  DeclareEvent(RemovePressed);
  DeclareEvent(MoveUpPressed);
  DeclareEvent(MoveDownPressed);
}

class ListSource;
class ListBox;
class IconButton;

class AddRemoveListBox : public Composite
{
public:
  /// Define the self type for connections.
  typedef AddRemoveListBox ZilchSelf;

  /// Constructor.  dataTypeName is used for tool tips.
  AddRemoveListBox(Composite* parent, float width, StringParam dataTypeName);

  /// Sets the displayed data.
  void SetDataSource(ListSource* dataSource);

  /// Sets the selected item.
  void SetSelectedItem(uint index, bool sendEvent);

  /// Returns the index of the selected item.
  uint GetSelectedItem();

private:
  // Creation
  void BuildLayout(Composite* parent, float width);
  void CreateButtons(Composite* parent);

  // Item Selection
  void ItemSelected(ObjectEvent* event);

  // Button Presses.
  void AddPressed(ObjectEvent* event);
  void RemovePressed(ObjectEvent* event);
  void MoveUpPressed(ObjectEvent* event);
  void MoveDownPressed(ObjectEvent* event);

  String mDataTypeName;
  ListBox* mListBox;
  IconButton* mAddButton;
  IconButton* mRemoveButton;
  IconButton* mMoveUpButton;
  IconButton* mMoveDownButton;
};

}//namespace Zero
