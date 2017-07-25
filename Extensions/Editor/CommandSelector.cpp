///////////////////////////////////////////////////////////////////////////////
///
/// \file CommandSelector.cpp
/// Implementation of the CommandSelector class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------ GeneralSearchView
ZilchDefineType(GeneralSearchView, builder, type)
{

}

GeneralSearchView::GeneralSearchView(Composite* parent, Widget* returnFocus)
  : Composite(parent)
{
  mReturnFocus = returnFocus;
  this->SetLayout(CreateStackLayout());

  CommandCaptureContextEvent commandCaptureEvent;
  commandCaptureEvent.ActiveSet = CommandManager::GetInstance();

  if(returnFocus)
    returnFocus->DispatchBubble(Events::CommandCaptureContext,
                                &commandCaptureEvent);

  mSearchView = new SearchView(this);
  mSearchView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  mSearch = mSearchView->mSearch;
  ConnectThisTo(mSearchView, Events::SearchCanceled, OnCancel);
  ConnectThisTo(mSearchView, Events::SearchCompleted, OnComplete);

  AddEditorProviders(*mSearch);

  CommandManager* commandManager = CommandManager::GetInstance();
  mSearch->SearchProviders.PushBack(commandManager->GetCommandSearchProvider());

  Window* window = GetWindowContaining(this);
  ConnectThisTo(window, Events::FocusLostHierarchy, OnFocusOut);
  ConnectThisTo(window, Events::FocusReset, OnFocusOut);

  mSearchView->TakeFocus();
}

void GeneralSearchView::StartSearch()
{
  mSearchView->Search(String());
}

void GeneralSearchView::OnFocusOut(FocusEvent* event)
{
  CloseTabContaining(this);
}

void GeneralSearchView::AutoClose()
{
  // Return focus back to the stored widget
  // if this window still has it. This lets
  // a command that creates a widget take focus
  // and not lose it.
  if(this->HasFocus())
  {
    // Get rid of focus on this object first
    this->LoseFocus();

    // Restore old focus
    if(Widget* reternFocus = mReturnFocus)
      reternFocus->TryTakeFocus();
  }

  CloseTabContaining(this);
}

void GeneralSearchView::OnComplete(SearchViewEvent* event)
{
  event->Element->Interface->RunCommand(event->View, *event->Element);
  AutoClose();
}

void GeneralSearchView::OnCancel(SearchViewEvent* event)
{
  AutoClose();
}

//------------------------------------------------------------ FloatingSearchView

FloatingSearchView::FloatingSearchView(Widget* popUp)
  : PopUp(popUp, PopUpCloseMode::MouseDistance)
{
  mView = new SearchView(this);
}

void FloatingSearchView::UpdateTransform()
{
  LayoutResult r = RemoveThickness(Thickness(Pixels(1,1,1,1)), this->GetSize());
  mView->SetTranslation(r.Translation);
  mView->SetSize(r.Size);
  PopUp::UpdateTransform();
}

}//namespace Zero
