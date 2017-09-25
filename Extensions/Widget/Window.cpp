///////////////////////////////////////////////////////////////////////////////
///
/// \file Window.cpp
/// Implementation of the Window widget class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

// METAREFACTOR - remove cyclic dependency
#include "Editor/MessageBox.hpp"

namespace Zero
{

namespace WindowUi
{
Tweakable(Vec4,  TextColor, Vec4(1,1,1,1), "EditorUi");

const cstr cLocation = "EditorUi/Window";
Tweakable(Vec4,  TitleBarColor,       Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  TabbedTitleBarColor, Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  TitleBarXColor,      Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  TitleBarXHighlight,  Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  SelectedTabColor,    Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  FocusedTabColor,    Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  MouseOverTabColor,   Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  DeSelectedTabColor,  Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  TabXColor,           Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  TabXHighlight,       Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  BackgroundColor,     Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  BorderColor,         Vec4(1,1,1,1), cLocation);
Tweakable(float, TitleBarHeight,      Pixels(20),    cLocation);
Tweakable(float, CloseSize,           Pixels(16),    cLocation);
Tweakable(Vec2,  BorderSize,          Pixels(2, 2),  cLocation);
Tweakable(Vec4,  ModifiedColor,       Vec4(1),       cLocation);
}

namespace Events
{
  DefineEvent(TabDropTest);
  DefineEvent(TabDrop);
  DefineEvent(TabFind);
  DefineEvent(TabShow);
  DefineEvent(CloseWindow);
  DefineEvent(CloseCheck);
  DefineEvent(TabModified);
  DefineEvent(QueryModifiedSave);
  DefineEvent(ConfirmModifiedSave);
  DefineEvent(NamedChanged);
  DefineEvent(HighlightBorder);
}

ZilchDefineType(WindowTabEvent, builder, type)
{
}

ZilchDefineType(HighlightBorderEvent, builder, type)
{
}

ZilchDefineType(TabModifiedEvent, builder, type)
{
}

ZilchDefineType(QueryModifiedSaveEvent, builder, type)
{
}

Thickness GetBorderThickness()
{
  return Thickness(1,1,1,1);
}

Thickness GetClientPadding()
{
  // Remove custom padding for the client area
  return Thickness(WindowUi::BorderSize.mValue.x, WindowUi::BorderSize.mValue.y, 
                   WindowUi::BorderSize.mValue.x, WindowUi::BorderSize.mValue.y);
}

Thickness TitleBarPadding()
{
  return Thickness(0, WindowUi::TitleBarHeight, 0, 0);
}

Thickness GetTotalWindowPadding()
{
  return GetClientPadding() + TitleBarPadding() + GetBorderThickness();
}

//Erase a value from an array and return its index before removal
template<typename type>
uint EraseValueIndex(Array<type>& values, type& value)
{
  uint size = values.Size();
  uint current = 0;
  for(;current<size;++current)
  {
    if(values[current]==value)
    {
      values.EraseAt(current);
      //Return the new valid index if the value is last
      //use previous unless empty
      return current;
    }
  }
  return 0;
}

const String WindowClientName = "WindowClient";
const String WindowElementsName = "WindowElements";

const String CloseIcon = "Close";
const String ModifiedIcon = "Modified";

//------------------------------------------------------------ TabWidget

TabWidget::TabWidget(Composite* parent)
  : Composite(parent)
{
  static const String className = "Tab";
  mDefSet = mDefSet->GetDefinitionSet(className);
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mClose =  CreateAttached<Element>(CloseIcon);
  mTitle = new Text(this, cText);
  mTitle->SetInteractive(false);
  mOwned = nullptr;

  mHighlight = false;
  mSelected = true;

  ConnectThisTo(mBackground, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(mBackground, Events::LeftMouseDrag, OnMouseDrag);
  ConnectThisTo(mClose, Events::LeftClick, OnClickClose);
  ConnectThisTo(this, Events::MiddleClick, OnClickClose);
  ConnectThisTo(this, Events::RightClick, OnRightClick);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
}

void TabWidget::LockTab()
{
  mClose->SetActive(false);
}

bool TabWidget::UnLocked()
{
  return mClose->GetActive();
}

void TabWidget::UpdateTransform()
{
  Widget* owned = mOwned;
  if(owned == nullptr)
    return;

  mTitle->SetText( owned->GetName() );
  mBackground->SetSize(mSize);
  float disToClose = mSize.x - Pixels(13);

  mClose->SetTranslation(Vec3(disToClose, Pixels(6), 0));
  if(mClose->IsMouseOver())
    mClose->SetColor(WindowUi::TabXHighlight);
  else
    mClose->SetColor(WindowUi::TabXColor);

  mTitle->SetTranslation(Pixels(6, 2, 0));
  mTitle->SetSize(Vec2(disToClose - Pixels(6), mSize.y));

  if(mSelected)
  {
    if(owned->HasFocus())
      mBackground->SetColor(WindowUi::FocusedTabColor);
    else
      mBackground->SetColor(WindowUi::SelectedTabColor);
  }
  else if(this->IsMouseOver())
    mBackground->SetColor(WindowUi::MouseOverTabColor);
  else
    mBackground->SetColor(WindowUi::DeSelectedTabColor);

  Composite::UpdateTransform();
}

void TabWidget::OnClickClose(Event* event)
{
  if (UnLocked())
  {
    mTabArea->RequestCloseTab(this);
  }
}

void TabWidget::OnMouseDown(MouseEvent* event)
{
  mTabArea->ChangeSelectedTab(this);
}

void TabWidget::OnMouseEnter(MouseEvent* event)
{
  MarkAsNeedsUpdate();
}

void TabWidget::OnMouseExit(MouseEvent* event)
{
  MarkAsNeedsUpdate();
}

void TabWidget::OnMouseDrag(MouseEvent* event)
{
  if(!UnLocked())
    return;

  //tab is already dead, we'll be cleaned up at the end of the frame
  Widget* owned = this->mOwned;
  if(owned == nullptr)
    return;

  //If this is the only tab on the tab control forward
  //the mouse down to the windows title bar
  if(mTabArea->mTabs.Size() == 1)
  {
    mTabArea->mParentWindow->ForwardMouseDown(event);
  }
  else
  {
    Window* current = mTabArea->mParentWindow;

    Window* floatWindow = new Window(mTabArea->mParentWindow->GetParent());
    Vec3 mousePos = ToVector3(event->GetMouse()->GetClientPosition());

    floatWindow->SetSize(Pixels(300, 300));
    floatWindow->SetTranslation(mousePos);
    floatWindow->mTabArea->TransferTab(this, -1, true);
    floatWindow->ForwardMouseDown(event);
  }
}

void TabWidget::OnNewWindow(Event* event)
{
  if(mTabArea->mParentWindow->mManager)
    mTabArea->mParentWindow->mManager->Transfer(this, this->GetOwnedWidget());
}

void TabWidget::OnRightClick(Event* event)
{
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0) );
  ConnectMenu(menu, "Close", OnClickClose);
  ConnectMenu(menu, "Close All But This", OnCloseAllOtherTabs);
  // Disabled until reimplemented
  // ConnectMenu(menu, "Move to New Window", OnNewWindow);
}

void TabWidget::OnCloseAllOtherTabs(Event* event)
{
  mTabArea->CloseAllOtherTabs(this);
}

Widget* TabWidget::GetOwnedWidget()
{
  return mOwned;
}

void TabWidget::SetOwnedWidget(Widget* widget)
{
  Widget* previousOwned = this->mOwned;

  // Disconnect from the previous widget if it exists
  if(previousOwned)
    previousOwned->GetDispatcher()->Disconnect(this);

  if(widget)
  {
    ConnectThisTo(widget, Events::TabModified, OnOwnedWidgetModified);
    ConnectThisTo(widget, Events::FocusGainedHierarchy, OnOwnedChangedFocus);
    ConnectThisTo(widget, Events::FocusLostHierarchy, OnOwnedChangedFocus);
  }

  mOwned = widget;
}


void TabWidget::OnOwnedChangedFocus(FocusEvent* event)
{
  this->MarkAsNeedsUpdate();
}


void TabWidget::OnOwnedWidgetModified(TabModifiedEvent* e)
{
  if(e->Modified)
  {
    mTitle->SetColor(WindowUi::ModifiedColor);
  }
  else
    mTitle->SetColor(Vec4(1));
  MarkAsNeedsUpdate();
}

//------------------------------------------------------------ Tab Area
ZilchDefineType(TabArea, builder, type)
{
}

TabArea::TabArea(Composite* parent, Window* window)
  : Composite(parent, AttachType::Direct),
    mCodaTab(nullptr)
{
  static const String className = "Tab";
  mBackground = new Spacer(this);

  mDefSet = mDefSet->GetDefinitionSet(className);
  mSelectedTabIndex = -1;
  mLastTabIndex = -1;
  mPreviewTab = -1;
  mParentWindow = window;
  mSizeForTabs = -1.0f;

  SetName("TabArea");

  this->SetClipping(true);

  ConnectThisTo(this, Events::TabDropTest, OnTabDropTest);
  ConnectThisTo(mBackground, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(mBackground, Events::RightMouseDown, OnRightMouseDown);
}


void TabArea::OnMouseDown(MouseEvent* event)
{
   mParentWindow->ForwardMouseDown(event);
}

void TabArea::RequestCloseTab(TabWidget* tab)
{
  if(tab->UnLocked())
  {
    Widget* ownedWidget = tab->GetOwnedWidget();
    // if the tabs owned widget has been modified prompt the user to save the changes
    QueryModifiedSaveEvent queryEvent;
    ownedWidget->DispatchEvent(Events::QueryModifiedSave, &queryEvent);
    //ownedWidget->DispatchDown(Events::QueryModifiedSave, &queryEvent);
    if (queryEvent.Modified)
    {
      ConfirmationOfDestructiveAction(tab, queryEvent.Title, queryEvent.Message);
    }
    else
    {
      CloseTab(tab);
    }
  }
}

void TabArea::CloseTab(TabWidget* tab)
{
  if (tab->UnLocked())
  {
    // Remove the owned widget 
    Widget* ownedWidget = tab->GetOwnedWidget();
    if (ownedWidget)
    {
      // If the widget has a manager return it 
      // to the manager for hiding etc,
      // otherwise just destroy it
      if (ownedWidget->mManager)
        ownedWidget->mManager->Closed(ownedWidget);
      else
        ownedWidget->Destroy();

      tab->SetOwnedWidget(nullptr);
    }

    InternalRemoveTab(tab);
    tab->Destroy();
  }
}

void TabArea::CloseAllOtherTabs(TabWidget* safeTab)
{
  Array<TabWidget*> closingTabs;

  forRange(TabWidget* tab, mTabs.All())
  {
    if(tab->UnLocked() && tab != safeTab)
      closingTabs.PushBack(tab);
  }

  forRange(TabWidget* tab, closingTabs.All())
  {
    CloseTab(tab);
  }

  ChangeSelectedTab(safeTab);
}

void TabArea::OnRightMouseDown(MouseEvent* event)
{
  if(mParentWindow->CanClose())
    mParentWindow->ToggleMinimized();
}

TabWidget* TabArea::TabFromWidget(Widget* widget)
{
  forRange(TabWidget* tab, mTabs.All())
  {
    if(tab->GetOwnedWidget() == widget)
      return tab;
  }
  return nullptr;
}

Widget* TabArea::GetActiveTabWidget()
{
  if(mTabs.Size())
    return mTabs[mSelectedTabIndex]->mOwned;
  else
    return nullptr;
}

void TabArea::ConfirmationOfDestructiveAction(TabWidget* tab, StringParam title, StringParam message)
{
  Array<String> buttons;
  buttons.Append(String("Save and Close"));
  buttons.Append(String("Close without Saving"));
  buttons.Append(String("Cancel"));
  MessageBox* box = MessageBox::Show(title, message, buttons);
  mCodaTab = tab;
  ConnectThisTo(box, Events::MessageBoxResult, CodaResponse);
}

void TabArea::CodaResponse(MessageBoxEvent* event)
{
  // save our changes and close the tab
  if (event->ButtonIndex == 0)
  {
    Event e;
    mCodaTab->GetOwnedWidget()->DispatchEvent(Events::ConfirmModifiedSave, &e);
    CloseTab(mCodaTab);
  }

  // continue without saving
  if (event->ButtonIndex == 1)
  {
    CloseTab(mCodaTab);
  }

  // 2 is cancel and the tab isn't closed
  mCodaTab = nullptr;
}

bool TabArea::IsTabSelected(TabWidget* tab)
{
  int index = (int)mTabs.FindIndex(tab);
  return index == mSelectedTabIndex;
}

void TabArea::SelectTabWith(Widget* widget)
{
  if(TabWidget* tab = TabFromWidget(widget))
  {
    ChangeSelectedTab(tab);
  }
}

void TabArea::CloseTabWith(Widget* widget)
{
  if(TabWidget* tab = TabFromWidget(widget))
  {
    CloseTab(tab);
  }
}

void TabArea::CloseTabs()
{
  forRange(TabWidget* tab, mTabs.All())
  {
    if(tab->GetOwnedWidget())
    {
      Widget* ownedWidget = tab->GetOwnedWidget();
      if(ownedWidget->mManager)
        ownedWidget->mManager->Closed(ownedWidget);
    }
    tab->SetOwnedWidget(nullptr);
    tab->Destroy();
  }
  mTabs.Clear();
}

void TabArea::LockTabs()
{
  forRange(TabWidget* tab, mTabs.All())
  {
    tab->LockTab();
  }
}

void TabArea::TabSwitch(bool forwards)
{
  if(mTabs.Empty())
    return;

  int destTabIndex = mSelectedTabIndex + (forwards ? 1 : mTabs.Size() - 1);

  destTabIndex = destTabIndex  % mTabs.Size();

  TabWidget* tab = mTabs[destTabIndex];
  ChangeSelectedTab(tab);
}

Vec3 TabArea::GetTabLocation(uint index, Vec2 tabSize)
{
  const float leftSpace = Pixels(1);
  float position = (tabSize.x + Pixels(2)) * float(index) + leftSpace;
  return Vec3(position, 0, 0);
}

Vec2 TabArea::GetTabSize()
{
  const float minTabSize = Pixels(40);
  const float maxTabSize = Pixels(240);

  float tabSizeX = 0.0f;
  float tabSizeY = mSize.y;
  float sizeForTabs = mSize.x - Pixels(20);

  uint numberOfTabs = mTabs.Size();

  if(mPreviewTab != -1)
    ++numberOfTabs;

  if(minTabSize * mTabs.Size() > sizeForTabs)
  {
    //Too many tabs
    tabSizeX = minTabSize;
  }
  else
  {
    tabSizeX = (float)numberOfTabs;
    if(tabSizeX != 0.0f)
      tabSizeX = sizeForTabs / tabSizeX;
    tabSizeX  = SnapToPixels(tabSizeX);
  }

  if(tabSizeX > maxTabSize)
    tabSizeX = maxTabSize;

  return Vec2(tabSizeX, tabSizeY);
}

void TabArea::UpdateTransform()
{
  if(GetTransformUpdateState() != TransformUpdateState::LocalUpdate)
  {
    Composite::UpdateTransform();
    return;
  }

  Vec2 tabSize = GetTabSize();

  Array<LayoutResult> layouts;

  Window* containingWindow = GetWindowContaining(this);
  bool floating = false;
  if(containingWindow)
    floating = (containingWindow->GetDockMode() == DockMode::DockNone);

  uint tabIndex = 0;
  for(int i=0;i<int(mTabs.Size());++i)
  {
    //Skip preview tab
    if(i == mPreviewTab)
      ++tabIndex;

    TabWidget* tab = mTabs[i];
    
    LayoutResult& result = layouts.PushBack();
    result.Translation = GetTabLocation(tabIndex, tabSize);
    result.Size = tabSize;
    result.PlacedWidget = tab;
    tab->mTitle->SetText(tab->GetOwnedWidget()->GetName());

    // Add a 1 pixel border at the top if the window is floating
    if(floating)
    {
      result.Translation.y += Pixels(1);
      result.Size.y -= Pixels(1);
    }

    ++tabIndex;
  }

  float sizeForTabs = mSize.x - Pixels(20);
  bool animate = !(mSizeForTabs != sizeForTabs);

  AnimateLayout(layouts, animate);
  mSizeForTabs = sizeForTabs;

  mBackground->SetSize(mSize);

  Composite::UpdateTransform();
}

void TabArea::TransferTab(TabWidget* tabToMove, int newIndex, bool select)
{
  TabArea* oldTabControl = tabToMove->mTabArea;
  TabArea* newTabControl = this;

  // Attach the widget to the new area before removing it from
  // the old since removing the last widget will destroy the window

  // Move tab widget
  newTabControl->AttachChildWidget(tabToMove);

  // Move owned widget
  Widget* ownedWidget = tabToMove->GetOwnedWidget();
  newTabControl->mParentWindow->mClientWidget->AttachChildWidget(ownedWidget);

  // Remove (this may destroy the window)
  oldTabControl->InternalRemoveTab(tabToMove);
  tabToMove->mTabArea = this;

  // Update the tab position
  if(newIndex == -1)
    newIndex = mTabs.Size();

  Vec2 tabSize = GetTabSize();
  Vec3 position = GetTabLocation(newIndex, tabSize);

  // Insert at new position
  mTabs.Insert(mTabs.Begin() + newIndex, tabToMove);

  // Lower down so it animates on
  tabToMove->SetTranslation(position + Pixels(0,20,0));
  tabToMove->SetSize(tabSize);

  if(select)
    ChangeSelectedTab(tabToMove);

  mPreviewTab = -1;
}

void TabArea::InternalRemoveTab(TabWidget* tab)
{
  // Erase will return previous valid element or zero
  uint oldTabIndex = EraseValueIndex(mTabs, tab);

  // Tab was above move down
  if(mLastTabIndex > int(oldTabIndex))
    --mLastTabIndex;

  if(mTabs.Empty())
  {
    // No more tabs destroy the window
    mParentWindow->Destroy();
  }
  else
  {
    if(tab->mSelected)
    {
      if( uint(mLastTabIndex) < mTabs.Size())
        ChangeSelectedTab(mTabs[mLastTabIndex]);
      // if it was the last tab
      else if(oldTabIndex == mTabs.Size())
        ChangeSelectedTab(mTabs[oldTabIndex-1]);
    }
  }
}

void TabArea::OnTabDropTest(WindowTabEvent* tabEvent)
{
  tabEvent->TabAreaFound = this;
}

int TabArea::TabIndexAt(MouseEvent* event)
{
  Vec2 localPosition = ToLocal(event->Position);
  Vec2 tabSize = GetTabSize();
  int position = int(localPosition.x / tabSize.x);
  return Math::Clamp(position, 0, int(mTabs.Size()));
}

void TabArea::AddNewTab(Widget* widget, bool selectTab)
{
  TabWidget* tab = new TabWidget(this);
  tab->mTabArea = this;
  tab->mTitle->SetText(widget->GetName());
  tab->SetOwnedWidget(widget);

  Vec2 tabSize = GetTabSize();
  Vec3 position = GetTabLocation(mTabs.Size(), tabSize);

  tab->SetTranslation(position + Pixels(0,20,0));
  tab->SetSize(tabSize);

  this->MarkAsNeedsUpdate();
  this->mParentWindow->MarkAsNeedsUpdate();

  mTabs.PushBack(tab);
  mParentWindow->mClientWidget->AttachChildWidget(widget);

  if(selectTab)
  {
    ChangeSelectedTab(tab);
  }
  else
  {
    tab->GetOwnedWidget()->SetActive(false);
    tab->mSelected = false;
  }
}

void TabArea::OnTabShow(WindowTabEvent* event)
{
  if(TabWidget* tab = TabFromWidget(event->Target))
  {
    ChangeSelectedTab(tab);
  }
}

void TabArea::OnTabFind(WindowTabEvent* event)
{
  if(event->TabWidgetFound != nullptr)
    return;

  forRange(TabWidget* tab, mTabs.All())
  {
    Widget* ownedWidget = tab->GetOwnedWidget();
    if(ownedWidget == nullptr)
      continue;

    ownedWidget->DispatchEvent(Events::TabFind, event);

    bool nameMatch = !event->Name.Empty() && ownedWidget->GetName() == event->Name;
    if(nameMatch)
    {
      event->TabWidgetFound = ownedWidget;
      return;
    }
  }
}



void TabArea::ChangeSelectedTab(TabWidget* toSelect)
{
  //tab is already dead, we'll be cleaned up at the end of the frame
  if(toSelect->GetOwnedWidget() == nullptr)
    return;

  //Update last tab
  mLastTabIndex = mSelectedTabIndex;

  mParentWindow->SetHighlightBorder(false);

  //Deactivate old tabs and find new tab
  int newTabIndex = -1;
  for(uint tabIndex = 0; tabIndex < mTabs.Size(); ++tabIndex)
  {
    TabWidget* tab = mTabs[tabIndex];
    if(tab == toSelect)
    {
      // new tab
      newTabIndex = tabIndex;
    }
    else
    {
      // old tab
      if(Widget* old = tab->GetOwnedWidget())
        old->SetActive(false);

      tab->mSelected = false;
      tab->MarkAsNeedsUpdate();
    }
  }

  mSelectedTabIndex = newTabIndex;
  toSelect->GetOwnedWidget()->SetActive(true);
  toSelect->GetOwnedWidget()->TryTakeFocus();
  toSelect->mSelected = true;
  toSelect->MarkAsNeedsUpdate();

  // Set the highlight of the window for this tab
  mParentWindow->SetHighlightBorder(toSelect->mHighlight, toSelect->mHighlightColor);
}

//------------------------------------------------------------ Window
const float cMinSizeSize = Pixels(24);

ZilchDefineType(Window, builder, type)
{
}

Window::Window(Composite* parent)
  : Composite(parent)
{
  static const String className = "Window";
  mDefSet = mDefSet->GetDefinitionSet(className);
  SetName("Window");

  mClientPadding = GetClientPadding();
  mWindowStyle = WindowStyle::Normal;
  mMinimized = false;
  mLayoutSize = Vec2(0, 0);
  mFloatingSize = Vec2(0, 0);
  mMinSize = Vec2(100, 100);

  mDropShadow = CreateAttached<Element>(cDropShadow);
  mDropShadow->SetInteractive(false);
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetName("WindowBackground");
  mBackgroundBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mBackgroundBorder->SetInteractive(false);

  mWindowWidget = new Composite(this, AttachType::Direct);
  mWindowWidget->SetName(WindowElementsName);
  mTitleBackground = mWindowWidget->CreateAttached<Element>(cWhiteSquare);

  mTitleText = new Text(mWindowWidget, cText);
  mTitleText->SetInteractive(false);
  mGripZones = new GripZones(mWindowWidget, this);

  mClientWidget = new Composite(this, AttachType::Direct);
  mClientWidget->SetName(WindowClientName);
  mClientWidget->SetClipping(true);
  mClientWidget->SetLayout(CreateFillLayout());

  mTitleMover = new Gripper(mWindowWidget, this, DockMode::DockTop);
  mTitleMover->mGripDirection = DockMode::DockFill;

  mCloseButton = mWindowWidget->CreateAttached<Element>(CloseIcon);
  mTabArea = new TabArea(mWindowWidget, this);

  // Create the Highlight border
  mHighlightBorder = CreateAttached<Element>(cBorder);
  mHighlightBorder->SetInteractive(false);
  SetHighlightBorder(false);

  ConnectThisTo(mTitleMover, Events::LeftMouseDrag, ForwardMouseDown);
  ConnectThisTo(mTitleText, Events::LeftMouseDrag, ForwardMouseDown);
  
  ConnectThisTo(mTitleMover, Events::RightMouseDown, RightMouseDownOnTitle);
  ConnectThisTo(mTitleMover, Events::DoubleClick,  DoubleClickOnTitle);
  ConnectThisTo(mCloseButton, Events::LeftClick, MouseClickClose);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  ConnectThisTo(this, Events::CloseWindow, OnCloseWindow);

  ConnectThisTo(this, Events::TabShow, OnTabShow);
  ConnectThisTo(this, Events::TabFind, OnTabFind);

  ConnectThisTo(this, Events::TabDropTest, OnTabDropTest);

  // Connect to events for setting the border highlight
  ConnectThisTo(this, Events::HighlightBorder, OnHighlightBorder);

  ConnectThisTo(this, Events::FocusGainedHierarchy, OnFocusGained);
 }

Window::~Window()
{
  if(mDocker) mDocker->WidgetDestroyed(this);
}

void Window::OnDestroy()
{
  mTabArea->CloseTabs();

  Composite::OnDestroy();
}

bool Window::TakeFocusOverride()
{
  // If the window has tab pass focus to the selected
  // tab widget
  if(Widget* activeTabWidget = mTabArea->GetActiveTabWidget())
    return activeTabWidget->TryTakeFocus();

  // Try to pass the focus to the first client area widget
  if(!mClientWidget->mChildren.Empty())
      return mClientWidget->mChildren.Front().TryTakeFocus();

  return false;
}

Vec2 Window::Measure(LayoutArea& data)
{
  if(mLayoutSize.x == 0)
    return mSize;
  else
    return mLayoutSize;
}

void Window::SetDockMode(DockMode::Enum newDockMode)
{
  if(mCurDockMode == DockMode::DockNone && !mMinimized)
  {
    mFloatingSize = this->GetSize();
    mLayoutSize = this->GetSize();
  }

  if(newDockMode == DockMode::DockNone && mMinimized)
    SetMinimized(false);

  if(newDockMode == DockMode::DockNone)
  {
    AnimateToSize(this, mFloatingSize);
  }

  Composite::SetDockMode(newDockMode);
}

Vec2 Window::GetMinSize()
{
  Thickness totalPadding = GetTotalWindowPadding();
  return Math::Max(mMinSize, ExpandSizeByThickness(totalPadding, mClientWidget->GetMinSize()));
}

void Window::SizeToContents()
{
  this->SetSize(this->GetMinSize());
}

void Window::SetMinimized(bool value)
{
  mMinimized = value;
  if(mMinimized)
  {
    //Turn off the client area
    mClientWidget->SetActive(false);
    //Do not expand on y
    mSizePolicy = SizePolicies(SizePolicy::Auto, SizePolicy::Fixed);
    mLayoutSize = Vec2(mSize.x, cMinSizeSize);

    if(mCurDockMode == DockMode::DockNone)
      mFloatingSize = this->GetSize();

    AnimateToSize(this, mLayoutSize);

    SetHighlightBorder(false);
  }
  else
  {
    mClientWidget->SetActive(true);

    //Do expand on y
    mLayoutSize = mFloatingSize;

    AnimateToSize(this, mFloatingSize);
    mSizePolicy = SizePolicies(SizePolicy::Auto, SizePolicy::Auto);
  }

  this->MarkAsNeedsUpdate();
  this->GetParent()->MarkAsNeedsUpdate();

}

void Window::SetLayout(Layout* layout)
{
  mClientWidget->SetLayout(layout); 
}

void Window::SetHighlightBorder(bool state, Vec4Param color)
{
  mHighlightBorder->SetVisible(state);
  mHighlightBorder->SetColor(color);
}


void Window::OnHighlightBorder(HighlightBorderEvent* e)
{
  if(mMinimized)
    return;

  // Find the tab of the given widget
  TabWidget* tab = mTabArea->TabFromWidget(e->mWidget);
  if(tab)
  {
    // Set the highlight states on the tab
    tab->mHighlight = e->mState;
    tab->mHighlightColor = e->mColor;

    // If the tab is already selected, set the border state
    if(mTabArea->IsTabSelected(tab))
      SetHighlightBorder(e->mState, e->mColor);
  }
}

void Window::ChangeStyle(uint style)
{
  mWindowStyle = (WindowStyle::Enum)style;
  MarkAsNeedsUpdate();
}

void Window::OnTabDropTest(WindowTabEvent* event)
{
  event->WindowFound = this;
}

void Window::OnTabShow(WindowTabEvent* event)
{
  if(event->Handled)
    return;

  if(mTabArea)
    mTabArea->OnTabShow(event);

  if(mDocker)
    mDocker->Show(this);
}

void Window::OnTabFind(WindowTabEvent* event)
{
  if(event->Handled)
    return;

  if(mTabArea)
    mTabArea->OnTabFind(event);

  if(!event->Name.Empty() && event->Name == this->GetName())
    event->WindowFound = this;
}

void Window::OnCloseWindow(WindowTabEvent* event)
{
  if(event->Handled)
    return;

  if(mTabArea)
  {
    event->Handled = true;
    mTabArea->CloseTabWith(event->Target);

    //If no tabs left destroy this window
    if(mTabArea->mTabs.Size()==0)
      this->Destroy();
  }
}

void Window::UpdateTransform()
{
  bool tabbedWindow = mTabArea->mTabs.Size() > 0;

  if(tabbedWindow)
  {
    if(mCloseButton->IsMouseOver())
      mCloseButton->SetColor(WindowUi::TitleBarXHighlight);
    else
      mCloseButton->SetColor(WindowUi::TitleBarXColor);
  }
  else
  {
    if(mCloseButton->IsMouseOver())
      mCloseButton->SetColor(WindowUi::TabXHighlight);
    else
      mCloseButton->SetColor(WindowUi::TabXColor);
  }

  if(GetTransformUpdateState() != TransformUpdateState::LocalUpdate)
  {
    Composite::UpdateTransform();
    return;
  }

  Thickness borderThickness = GetBorderThickness();

  mBackground->SetSize(mSize);
  mBackground->SetColor(WindowUi::BackgroundColor);
  mBackgroundBorder->SetSize(mSize);
  mBackgroundBorder->SetColor(Vec4(0.18f, 0.18f, 0.18f, 1));
  mGripZones->SetSize(mSize);

  // Remove the border
  Rect currentRect = Rect::PointAndSize(Vec2(0,0), mSize);
  currentRect.RemoveThickness(borderThickness);

  if(mWindowStyle == WindowStyle::Normal)
  {
    mWindowWidget->SetActive(true);
    mWindowWidget->SetSize(mSize);

    // Update the client area
    Vec2 titleBarSize = Vec2(currentRect.SizeX, WindowUi::TitleBarHeight);

    Vec2 tabAreaSize = titleBarSize - Vec2(WindowUi::CloseSize, 0);

    mTabArea->SetTranslation(Vec3(currentRect.TopLeft()));
    mTabArea->SetSize(tabAreaSize);

    mTitleBackground->SetTranslation(Vec3(currentRect.TopLeft()) + Pixels(1,0,0));
    mTitleBackground->SetSize(titleBarSize - Vec2(2, 0));

    if(tabbedWindow)
      mTitleBackground->SetColor(WindowUi::TabbedTitleBarColor);
    else
      mTitleBackground->SetColor(WindowUi::TitleBarColor);

    Vec2 titleTextOffset = Vec2(3, 3);

    mTitleText->SetTranslation(Vec3(borderThickness.TopLeft() + titleTextOffset));
    mTitleText->SetSize(tabAreaSize - titleTextOffset);
    mTitleMover->SetSize(Vec2(mSize.x, borderThickness.Top));

    Vec2 closeButtonSize = mCloseButton->GetSize();
    Vec3 closeButtonPos = Vec3(mSize.x - closeButtonSize.x - Pixels(7), Pixels(7), 0);
    mCloseButton->SetTranslation( SnapToPixels(closeButtonPos) );

    //Set up drop shadow
    if(mCurDockMode == DockMode::DockNone)
    {
      mDropShadow->SetActive(true);
      mDropShadow->SetSize(mSize);
      mDropShadow->SetTranslation(Pixels(6, 6, 0));
    }
    else
    {
      mDropShadow->SetActive(false);
    }

    if(mMinimized)
    {
      mDropShadow->SetActive(false);
    }

    // Remove the title bar
    currentRect.RemoveThickness(TitleBarPadding());

    currentRect.RemoveThickness(mClientPadding);

    mClientRect = currentRect;

    // Clamp client size this will cause the child widgets
    const float minWindowSize = 16.0f;
    mClientRect.SizeX = Math::Max(mClientRect.SizeX, minWindowSize);
    mClientRect.SizeY = Math::Max(mClientRect.SizeY, minWindowSize);

    if(!mMinimized)
    {
      PlaceWithRect(mClientRect, mClientWidget);
      mClientRect.RemoveThickness(Thickness(-Pixels(3,3,3,3)));
      PlaceWithRect(mClientRect, mHighlightBorder);
    }
  }
  else
  {
    mWindowWidget->SetActive(false);
    mClientRect.SetSize(mSize);
    mClientRect.SetTranslation(Vec2::cZero);
    PlaceWithRect(currentRect, mClientWidget);
  }

  Composite::UpdateTransform();
}

void Window::SetTitle(StringParam str)
{
  mTitleText->SetText(str);
}

void Window::AttachChildWidget(Widget* widget, AttachType::Enum attachType)
{
  if(attachType == AttachType::Direct)
    Composite::AttachChildWidget(widget);
  else
    mClientWidget->AttachChildWidget(widget);
}

void Window::HideClose()
{
  mCloseButton->SetVisible(false);
}

bool Window::CanClose() const
{
  return mCloseButton->GetActive();
}

void Window::AttachAsTab(Widget* widget, bool selectNewTab)
{
  mTabArea->AddNewTab(widget, selectNewTab);
  mClientWidget->SetLayout(CreateFillLayout());
}

void Window::ForwardMouseDown(MouseEvent* event)
{
  mTitleMover->OnMouseDown(event);
}

void Window::DoubleClickOnTitle(MouseEvent* event)
{
  SetDockMode(DockMode::DockFill);
}

void Window::ToggleMinimized()
{
  mMinimized = !mMinimized;
  this->SetMinimized(mMinimized);
}

void Window::RightMouseDownOnTitle(MouseEvent* event)
{
  ToggleMinimized();
}

void Window::OnKeyDown(KeyboardEvent* event)
{
  if(event->Handled)
    return;

  // Ctrl-Tab and Ctrl-Shift-Tab for tab switch
  if(event->CtrlPressed && event->Key == Keys::Tab)
  {
    event->Handled = true;
    mTabArea->TabSwitch(!event->ShiftPressed);
  }

  // F11 for zooming
  if(event->Key == Keys::F11)
  {
    if(event->ShiftPressed)
    {
      if(mWindowStyle == WindowStyle::Normal)
        mWindowStyle = WindowStyle::NoFrame;
      else
        mWindowStyle = WindowStyle::Normal;
    }

    if(mDocker)
      mDocker->Zoom(this);
  }
}

void Window::OnFocusGained(FocusEvent* event)
{
  // Only move forward if the window is floating
  if(mCurDockMode == DockMode::DockNone)
    this->MoveToFront();
}

void Window::MouseClickClose(MouseEvent* event)
{
  if(mHideOnClose)
  {
    this->SetActive(false);
  }
  else
  {
    this->Destroy();
  }
}

void CloseTabContaining(Widget* widget)
{
  if(widget == nullptr)
    return;

  WindowTabEvent tabEvent;
  tabEvent.Target = widget;
  widget->DispatchBubble(Events::CloseWindow, &tabEvent);
}

Window* GetWindowContaining(Widget* widget)
{
  // Walk up the tree until a widget of type window is found
  BoundType* windowType = ZilchTypeId(Window);

  while(widget != nullptr)
  {
    if(ZilchVirtualTypeId(widget) == windowType)
      return (Window*)widget;
    widget = widget->GetParent();
  }

  return nullptr;
}

void ShowWidget(Widget* widget)
{
  WindowTabEvent tabEvent;
  tabEvent.Target = widget;
  widget->DispatchBubble(Events::TabShow, &tabEvent);
}

}//namespace Zero
