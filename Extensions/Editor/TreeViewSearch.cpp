///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

TreeViewSearch::TreeViewSearch(Composite* parent, TreeView* treeView, DataSourceFilter* filter)
  :Composite(parent)
{
  mTreeView = treeView;
  mFiltered = filter;
  mUnFiltered = NULL;
  this->SetLayout(CreateRowLayout());

  Composite* searchRow = new Composite(this);
  searchRow->SetLayout(CreateFillLayout(Thickness(1, 0, 1, 0)));
  searchRow->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  searchRow->SetSizing(SizeAxis::Y, SizePolicy::Fixed, 16.0f);
  {
    mSearchField = new TextBox(searchRow);
    mSearchField->SetSizing(SizePolicy::Flex, 1.0f);
    mSearchField->SetEditable(true);
    mSearchField->SetHintText("Search...");
    mSearchField->mTextOffset = Pixels(28);
  }

  mIcon = CreateAttached<Element>("MainSearchDropdown");
  mIcon->SetNotInLayout(true);
  mIcon->SetTranslation(Pixels(2, 1, 0));

  ConnectThisTo(mSearchField, Events::TextChanged, OnTextEntered);
  ConnectThisTo(mSearchField, Events::KeyDown, OnKeyDown);
  ConnectThisTo(mSearchField, Events::KeyRepeated, OnKeyDown);  
}

TreeViewSearch::~TreeViewSearch()
{
  SafeDelete(mFiltered);
}

void TreeViewSearch::OnKeyDown(KeyboardEvent* event)
{
  if(event->Key == Keys::Enter)
  {
    // When enter is press cancel the search
    // and select the item
    CancelFilter();

    // Make sure the selected rows are visible
    // normally there will be only one row
    Array<DataIndex> selectedIndices;
    mTreeView->GetSelection()->GetSelected(selectedIndices);
    forRange(DataIndex index, selectedIndices.All())
      mTreeView->ShowRow(index);

    // Move focus to the tree view
    mTreeView->TryTakeFocus();
  }

  if(event->Key == Keys::Down || 
     event->Key == Keys::Up)
  {
    // Just forward the key logic
    // to move which item selected
    mTreeView->HandleKeyLogic(event);
  }


  if(event->Key == Keys::Escape)
    CancelFilter();
}


void TreeViewSearch::OnTextEntered(Event* event)
{
  if(!mSearchField->HasFocus())
    return;

  // If the text has been cleared cancel the filter
  String currentText = mSearchField->GetText();
  if(currentText.Empty())
  {
    CancelFilter();
    return;
  }

  // Change to filtered list if not already active
  if(mUnFiltered == NULL)
  {
    mUnFiltered = mTreeView->mDataSource;
    ConnectThisTo(mUnFiltered, Events::DataDestroyed, OnSourceDestroy);
    ConnectThisTo(mUnFiltered, Events::DataModified, OnDataModified);
    mFiltered->SetSource(mUnFiltered);
    mTreeView->SetDataSource(mFiltered);
  }

  // Update the filter with the new string
  mFiltered->Filter(currentText);
  mTreeView->Refresh();
}

void TreeViewSearch::OnSourceDestroy(Event* event)
{
  // Data source that is being filtered has been destroyed 
  // Cancel the filter
  mUnFiltered = NULL;
  CancelFilter();
}

void TreeViewSearch::OnDataModified(Event* event)
{
  // The data source that is being filtered has 
  // been modified

  // There is two options cancel the search
  // or refilter the data

  // Refilter the list with new objects
  if(mFiltered)
  {
    mFiltered->Filter(mSearchField->GetText());
    mTreeView->Refresh();
  }

  // or

  // CancelFilter();

}

void TreeViewSearch::CancelFilter()
{
  mSearchField->SetText(String());
  if(mUnFiltered)
  {
    mUnFiltered->GetDispatcher()->Disconnect(this);
    mTreeView->SetDataSource(mUnFiltered);
    mUnFiltered = NULL;
  }

  mFiltered->SetSource(NULL);
}

}
