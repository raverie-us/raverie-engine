// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(MainWindowTransformUpdated);
}

class MainWindowTransformEvent : public Event
{
public:
  RaverieDeclareType(MainWindowTransformEvent, TypeCopyMode::ReferenceType);

  MainWindowTransformEvent(Vec2Param oldScreenSize = Vec2::cZero, Vec2Param newScreenSize = Vec2::cZero);

  OsWindow* GetTargetWindow();

  Vec2 mOldScreenSize;
  Vec2 mNewScreenSize;
};

class MainWindow : public RootWidget
{
public:
  RaverieDeclareType(MainWindow, TypeCopyMode::ReferenceType);

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

  Label* mTitleText;
  Composite* mTitleBar;
  Composite* mWindowWidget;
  MenuBar* mMenu;
  Composite* mPopUp;
  Composite* mMainMenu;
  Vec2 mLayoutSize;
};

class MainDocker : public Docker
{
public:
  void Dock(Widget* widget, DockArea::Enum area) override{};
  DockArea::Enum GetDockArea() override
  {
    return DockArea::Floating;
  }
  void Zoom(Widget* widget) override{};
  void WidgetDestroyed(Widget* widget) override;
};

} // namespace Raverie
