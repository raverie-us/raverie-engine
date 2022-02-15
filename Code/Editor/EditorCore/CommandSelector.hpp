// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

struct SearchViewResult;
class SearchViewEvent;

// GeneralSearchView that can be opened anywhere.
class GeneralSearchView : public Composite
{
public:
  ZilchDeclareType(GeneralSearchView, TypeCopyMode::ReferenceType);

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

} // namespace Zero
