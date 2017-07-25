///////////////////////////////////////////////////////////////////////////////
///
/// \file Composite.hpp
/// Declaration of the Composite Widget class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

typedef InList<Widget, &Widget::mWidgetLink> WidgetList;

//-------------------------------------------------------------------- Composite
typedef WidgetList::range WidgetListRange;

///Composite is a widget that Contains children.
///Base class for all widgets that have children.
class Composite : public Widget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Composite(Composite* parent, AttachType::Enum attachType = AttachType::Normal);
  ~Composite();

  void OnDestroy() override;

  //Layout of child widgets
  Layout* GetLayout() { return mLayout; }
  virtual void SetLayout(Layout* layout);
  Vec2 Measure(LayoutArea& data) override;
  Vec2 GetMinSize() override;
  void SetMinSize(Vec2 newMin);

  //Get children
  WidgetListRange GetChildren() { return mChildren.All(); }
  Composite* GetSelfAsComposite() override { return this; }
  void ChangeRoot(RootWidget* newRoot) override;

  /// Destroys all children of this composite.
  void DestroyChildren();

  //Composite interface

  //Attach a child widget
  virtual void AttachChildWidget(Widget* child, AttachType::Enum attachType =  AttachType::Normal);

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  //Widget interface
  void UpdateTransform() override;
  void DispatchDown(StringParam eventId, Event* event) override;

  //How much size is there for the client. For the 
  //standard composite this is the size of the composite.
  virtual Vec2 GetClientVisibleSize() { return mSize; }
  virtual void DoLayout();

  WidgetList mChildren;
  Widget* HitTest(Vec2 location, Widget* skip) override;

protected:
  friend class Widget;
  Layout* mLayout;
  Vec2 mMinSize;

private:
  static void InternalDetach(Composite* parent, Widget* child);
  static void InternalAttach(Composite* parent, Widget* child);

};

DeclareEnum3(UiTraversal, DirectDescendantsOnly, DepthFirst, BreadthFirst);

// Find any child widget by class
Widget* FindWidgetByName(StringParam name, UiTraversal::Enum traversalType, size_t index, Widget* parent);

//-------------------------------------------------------------------------------- Colored Composite
class ColoredComposite : public Composite
{
public:
  ColoredComposite(Composite* parent, Vec4Param color, AttachType::Enum attachType = AttachType::Normal);
  void UpdateTransform() override;

  Element* mBackground;
};


}//namespace Zero
