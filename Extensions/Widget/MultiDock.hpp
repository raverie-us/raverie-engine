///////////////////////////////////////////////////////////////////////////////
///
/// \file MultiDock.hpp
/// Declaration of the Multi Dock classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class MultiDock;
class Window;

//----------------------------------------------------------------- Multi-Docker
class MultiDocker : public Docker
{
public:
  friend class MultiDock;
  friend class DockMovingManipulation;

  MultiDocker(Widget* widget, MultiDock* owner);

  // Docker interface
  DockArea::Enum GetDockArea() override {return Area;}
  void Dock(Widget* widget, DockArea::Enum mode) override;
  void Zoom(Widget* widget) override;
  void WidgetDestroyed(Widget* widget) override;
  bool StartManipulation(Widget* widget, DockMode::Enum direction) override;
  void Show(Widget* widget) override;

  bool Zoomed;
  MultiDock* DockOwner;
  Vec2 FlexSize;
  Rect PreDockSize;
  DockArea::Enum Area;
  int DockIndex;
  uint DockShell;
};

//----------------------------------------------------------------- Docking Area

struct DockingArea
{
  friend class MultiDock;
  DockingArea();
  DockArea::Enum Area;
  uint DockShell;
  float FlexRatio;
  Vec2 DockSize;
  Array<Widget*> Widgets;
};

struct DockingShell
{
  DockingArea DockAreas[DockArea::Count];
};

//------------------------------------------------------------------- Multi-Dock

///Multi dock widget is a composite widget that supports dockable children.
class MultiDock : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MultiDock(Composite* parent);
  ~MultiDock();

  Window* AddWidget(Widget* widget, LayoutInfo& info);
  Window* GetCenterWindow();

  void AttachChildWidget(Widget* widget, AttachType::Enum attachType) override;
  void UpdateTransform() override; 

  void SetExploded(bool exploded, bool animate);
  void CancelZoom();

  void ComputeAndApplyLayout(bool animate);
  void ComputeLayout(Array<LayoutResult>& results);

  Array<DockingShell> DockingShells;
  DockingArea SpecialAreas[3];
  HandleOf<Window> CenterWindow;
  HandleOf<Widget> mZoomed;
  bool mExploded;
  Vec2 mAnimateSize;
};

}//namespace Zero
