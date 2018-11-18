///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Search box that will filter a tree view with a give
// DataSourceFilter. Used for searching in tree views
class TreeViewSearch : public Composite
{
public:
  typedef TreeViewSearch ZilchSelf;
  TreeViewSearch(Composite* parent, TreeView* treeView, DataSourceFilter* filter);
  ~TreeViewSearch();

  TreeView* mTreeView;
  TextBox* mSearchField;
  DataSourceFilter* mFiltered;
  DataSource* mUnFiltered;
  Element* mIcon;

  void CancelFilter();
  void OnTextEntered(Event* event);
  void OnEnter(ObjectEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  void OnSourceDestroy(Event* event);
  void OnDataModified(Event* event);
};

}
