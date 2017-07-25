///////////////////////////////////////////////////////////////////////////////
///
/// \file MainWindow.hpp
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class MainWindow : public RootWidget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MainWindow(OsWindow* window);

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
  IconButton* mClose;
  IconButton* mMax;
  IconButton* mMin;
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
  void Dock(Widget* widget, DockArea::Enum area) override {};
  DockArea::Enum GetDockArea() override { return DockArea::Floating; }
  void Zoom(Widget* widget) override {};
  void Show(Widget* widget) override;
  void WidgetDestroyed(Widget* widget) override;
  bool StartManipulation(Widget* widget, DockMode::Enum direction) override;
};

}//namespace Zero
