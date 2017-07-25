///////////////////////////////////////////////////////////////////////////////
///
/// \file Window.hpp
/// Declaration of the Window widget class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  /// Test dropping a tap
  DeclareEvent(TabDropTest);
  /// Drop a tab
  DeclareEvent(TabDrop);
  /// Find a window or Tab
  DeclareEvent(TabFind);
  // Show a tab or window
  DeclareEvent(TabShow);
  /// Close the window that Contains the object
  DeclareEvent(CloseWindow);
  /// Can this window or tab be closed?
  DeclareEvent(CloseCheck);
  /// Sent on the owned widget of a tab when the contents of the widget
  /// have been modified (or saved)
  DeclareEvent(TabModified);
  /// Name Change
  DeclareEvent(NamedChanged);
  /// When sent on the window composite, it will create a highlight around the
  /// edges of the window, only if the tab owning the specified widget in the 
  /// HighlightBorderEvent is selected.
  /// For example: If you send this event on the property grid, it will bubble up
  /// to the window and only highlight the window if the tab owning
  /// the property grid is selected.
  DeclareEvent(HighlightBorder);
  /// An event for tab widgets to query their owned widgets if they are modified and need to be saved
  /// It returns whether or not they need to display a CODA along with the appropriate message for
  /// the resource type being queried
  DeclareEvent(QueryModifiedSave);
  /// Confirmation event that tells the owned widget to save its changes
  DeclareEvent(ConfirmModifiedSave);
}

/// Describes the border to be created around a window.
class HighlightBorderEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  /// Whether or not the border is visible.
  bool mState;

  /// The color of the border.
  Vec4 mColor;

  /// The highlight is only active when the tab owning this widget is selected.
  Widget* mWidget;
};

class Gripper;
class TabWidget;
class TabArea;
class WindowTabEvent;
class Window;
class GripZones;
class MessageBoxEvent;

//------------------------------------------------------------- Window Tab Event
class WindowTabEvent : public HandleableEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  WindowTabEvent()
  {
    TabAreaFound = nullptr;
    TabWidgetFound = nullptr;
    WindowFound = nullptr;
    Target = nullptr;
  }

  String Name;
  Handle SearchObject;

  Widget* Target;
  TabArea* TabAreaFound;
  Widget* TabWidgetFound;
  Window* WindowFound;
};

class TabModifiedEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  TabModifiedEvent(bool modified) : Modified(modified) {}
  bool Modified;
};

class QueryModifiedSaveEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  QueryModifiedSaveEvent() : Modified(false) {};
  bool Modified;
  String Title;
  String Message;
};

//------------------------------------------------------------------- Multi Dock
class MultiDock;
class Window;

class TabWidget : public Composite
{
public:
  typedef TabWidget ZilchSelf;

  TabWidget(Composite* parent);

  void LockTab();
  bool UnLocked();

  void UpdateTransform() override;

  void OnClickClose(Event* event);
  void OnMouseDown(MouseEvent* event);
  void OnMouseEnter(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);
  void OnMouseDrag(MouseEvent* event);
  void OnRightClick(Event* event);
  void OnCloseAllOtherTabs(Event* event);
  void OnOwnedWidgetModified(TabModifiedEvent* e);
  void OnOwnedChangedFocus(FocusEvent* event);
  void OnNewWindow(Event* event);

  Widget* GetOwnedWidget();
  void SetOwnedWidget(Widget* widget);

  Text* mTitle;
  Element* mClose;
  Element* mBackground;
  bool mSelected;
  /// If the highlight on the window should be displayed for this tab.
  bool mHighlight;
  /// The color of the highlight for this tab.
  Vec4 mHighlightColor;
  TabArea* mTabArea;

private:
  friend class TabArea;
  HandleOf<Widget> mOwned;
};

// TabArea Manages Tabs on a Window
class TabArea : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TabArea(Composite* parent, Window* window);

  void AddNewTab(Widget* widget, bool selectTab = true);
  void TransferTab(TabWidget* tab, int newIndex=-1, bool select=false);
  void CloseTabs();
  void CloseAllOtherTabs(TabWidget* tab);
  void RequestCloseTab(TabWidget* tab);
  void CloseTab(TabWidget* tab);
  void CloseTabWith(Widget* widget);
  void SelectTabWith(Widget* widget);
  void LockTabs();
  void TabSwitch(bool forwards);
  void ChangeSelectedTab(TabWidget* tab);
  void InternalRemoveTab(TabWidget* tab);
  int TabIndexAt(MouseEvent* mouse);
  Vec2 GetTabSize();
  Vec3 GetTabLocation(uint index, Vec2 tabSize);
  TabWidget* TabFromWidget(Widget* widget);
  bool IsTabSelected(TabWidget* tab);
  Widget* GetActiveTabWidget();
  void ConfirmationOfDestructiveAction(TabWidget* tab, StringParam title, StringParam message);
  void CodaResponse(MessageBoxEvent* event);
  
  // Widget Interface
  void UpdateTransform() override;
  
  // Events
  void OnFindWindow(WindowTabEvent* event);
  void OnTabDropTest(WindowTabEvent* drop);
  void OnRightMouseDown(MouseEvent* event);
  void OnMouseDown(MouseEvent* event);
  void OnTabFind(WindowTabEvent* event);
  void OnTabShow(WindowTabEvent* event);

  //Tabs
  Array<TabWidget*> mTabs;
  TabWidget* mCodaTab;
  // Currently Selected Tab
  int mSelectedTabIndex;
  // Last Selected Tab
  int mLastTabIndex;
  // Preview Tab for Tab Drag
  int mPreviewTab;
  // Size for each tab
  float mSizeForTabs;

  Window* mParentWindow;
  Widget* mBackground;
};

//----------------------------------------------------------------------- Window

DeclareEnum3(WindowStyle, Normal, NoFrame, Small);
Thickness GetTotalWindowPadding();

///Window is composite widget with a title bar and sizers. 
class Window : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Window(Composite* parent);
  ~Window();

  //Set the title of the window.
  void SetTitle(StringParam newTitle);
  void ChangeStyle(uint style);

  //Composite interface
  void AttachChildWidget(Widget* widget, AttachType::Enum attachType) override;
  void AttachAsTab(Widget* widget, bool selectNewTab = true);
  void SetLayout(Layout* layout);

  /// Border highlighting.
  void SetHighlightBorder(bool state, Vec4Param color = Vec4(1,0,0,1));
  void OnHighlightBorder(HighlightBorderEvent* event);

  /// How much room is there available for the client (windows borders removed).
  Vec2 GetClientSize() { return mClientRect.GetSize(); }

  //Set if the window closes or hide when the close
  //button is clicked.
  void SetMinimized(bool value);
  void HideClose();
  bool CanClose() const;

  TabArea* mTabArea;
  void ToggleMinimized();

  //Widget Interface
  void UpdateTransform() override;
  Vec2 Measure(LayoutArea& data) override;
  void SetDockMode(DockMode::Enum dock) override;
  Vec2 GetMinSize() override;
  void OnDestroy() override;
  bool TakeFocusOverride() override;
  void SizeToContents() override;

  void ForwardMouseDown(MouseEvent* event);

//protected:
  //Events
  void OnTabDropTest(WindowTabEvent* event);
  void OnCloseWindow(WindowTabEvent* event);
  void RightMouseDownOnTitle(MouseEvent* event);
  void MouseClickClose(MouseEvent* event);
  void DoubleClickOnTitle(MouseEvent* event);
  void OnTabFind(WindowTabEvent* event);
  void OnTabShow(WindowTabEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  void OnFocusGained(FocusEvent* event);
  WindowStyle::Enum mWindowStyle;
  bool mMinimized;
  Rect mClientRect;
  Vec2 mFloatingSize;
  Vec2 mLayoutSize;

  //Child widgets
  Gripper* mTitleMover;
  GripZones* mGripZones;
  Composite* mClientWidget;
  Composite* mWindowWidget;

  //Display Objects
  Element* mCloseButton;
  Element* mDropShadow;
  Element* mTitleBackground;
  Element* mBackground;
  Element* mBackgroundBorder;
  Text* mTitleText;
  Element* mHighlightBorder;
  Thickness mClientPadding;
};

void CloseTabContaining(Widget* widget);
Window* GetWindowContaining(Widget* widget);
void ShowWidget(Widget* widget);

namespace WindowUi
{
DeclareTweakable(Vec4,  BackgroundColor);
}

}//namespace Zero
