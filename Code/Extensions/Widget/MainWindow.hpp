// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(MainWindowTransformUpdated);
}

class MainWindowTransformEvent : public Event
{
public:
  ZilchDeclareType(MainWindowTransformEvent, TypeCopyMode::ReferenceType);

  MainWindowTransformEvent(Vec2Param oldScreenSize = Vec2::cZero,
                           Vec2Param newScreenSize = Vec2::cZero);

  OsWindow* GetTargetWindow();

  Vec2 mOldScreenSize;
  Vec2 mNewScreenSize;
};

class MainWindow : public RootWidget
{
public:
  ZilchDeclareType(MainWindow, TypeCopyMode::ReferenceType);

  MainWindow();

  void LoadMenu(StringParam menuName);
  void SetTitle(StringParam title);
  void OnDoubleClickTitle(MouseEvent* event);
  Composite* GetPopUp() override;

  void AttachChildWidget(Widget* child, AttachType::Enum attachType) override;
  void UpdateTransform() override;

  void OnClickClose(Event* event);
  void OnClickMax(Event* event);
  void OnClickMin(Event* event);

  Composite* mClientWidget;
  Composite* mMenuArea;
  Element* mBorder;
  Element* mTitleBack;
  GripZones* mSizeGrips;
  Gripper* mTitleGrip;

  Label* mTitleText;
  Composite* mTitleBar;
  Composite* mWindowWidget;
  MenuBar* mMenu;
  Composite* mPopUp;
  Composite* mMainMenu;
  Vec2 mLayoutSize;
};

class OsDocker : public Docker
{
public:
  void Dock(Widget* widget, DockArea::Enum area) override{};
  DockArea::Enum GetDockArea() override
  {
    return DockArea::Floating;
  }
  void Zoom(Widget* widget) override{};
  void WidgetDestroyed(Widget* widget) override;
  WindowBorderArea::Enum GetWindowBorderArea(Widget* widget, DockMode::Enum direction) override;
};

} // namespace Zero
