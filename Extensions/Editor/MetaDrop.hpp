///////////////////////////////////////////////////////////////////////////////
///
/// \file MetaDrop.hpp
/// 
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
  DeclareEvent(MetaDrop);
  DeclareEvent(MetaDropTest);
  DeclareEvent(MetaDropUpdate);
}

class ViewportMouseEvent;

//-------------------------------------------------------------- Meta Drop Event
/// MetaDropEvent for dropping MetaObjects
class MetaDropEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MetaDropEvent(MouseEvent* e);

  Handle GetObject();
  Handle GetMouseOverObject();
  MouseEvent* GetMouseEvent();
  ViewportMouseEvent* GetViewportMouseEvent();

  // Signal that the drop has been handled
  bool Handled;
  // Marks that this meta drop is invalid
  // (so a message can be displayed to the user)
  bool Failed;
  // In Testing Mode fill if the drop is valid
  // set handled and the result but do not drop
  bool Testing;
  Vec2 Position;
  // Object Being Dropped
  Handle Instance;
  Handle MouseOverObject;
  // Result String to display as tool tip
  String Result;
  // For dropping property values
  Property* Property;

  MouseEvent* mMouseEvent;
  ViewportMouseEvent* mViewportMouseEvent;

  // Used for custom placement of tooltips while dragging
  bool mUseTooltipPlacement;
  ToolTipPlacement mToolTipPlacement;
};

//------------------------------------------------------------Meta Drag
class ToolTip;

/// MetaDrag for implementing MetaDrops. Will dispatch MetaDropEvent when
/// mouse is released.
class MetaDrag : public MouseManipulation
{
public:
  typedef MetaDrag ZilchSelf;
  MetaDrag(Mouse* mouse, Composite* owner, HandleParam object);
  ~MetaDrag();
  
  // Add an object to the drop. Each object will be dropped individually.
  void AddObject(HandleParam instance);

  //MouseManipulation Interface
  void OnMouseMove(MouseEvent* event) override;
  void OnMouseUp(MouseEvent* event) override;
  void OnMouseUpdate(MouseEvent* event) override;

  //Events
  void OnKeyDown(KeyboardEvent* event);
private:
  Array<Handle> mObjects;
  ToolTip* mToolTip;
  Vec4 mBorderColor;
};

}//namespace Zero
