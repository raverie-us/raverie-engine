///////////////////////////////////////////////////////////////////////////////
///
/// \file CommandSelector.hpp
/// Declaration of the GeneralSearchView class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct SearchViewResult;
class SearchViewEvent;

//GeneralSearchView that can be opened anywhere.
class GeneralSearchView : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GeneralSearchView(Composite* parent, Widget* returnFocus);

  HandleOf<Widget> mReturnFocus;
  SearchView* mSearchView;
  SearchData* mSearch;

  void StartSearch();
  void AutoClose();
  void OnCancel(SearchViewEvent* event);
  void OnComplete(SearchViewEvent* event);
  void OnFocusOut(FocusEvent* event);
};

/// Pop up version of search view.
class FloatingSearchView : public PopUp
{
public:
  FloatingSearchView(Widget* popUp);
  void UpdateTransform() override;
  SearchView* mView;
};

}
