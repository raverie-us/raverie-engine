///////////////////////////////////////////////////////////////////////////////
///
/// \file Tool.hpp
/// Declaration of the Tool classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Viewport;
class KeyboardEvent;
class MouseEvent;
class PropertyView;
class Editor;
class FocusEvent;
class Composite;
class DisplaySpace;
class Command;
class EditorViewport;

//------------------------------------------------------------- Selection Result
struct SelectionResult
{
  SelectionResult(Cog* hit, Vec3 position, Vec3 normal)
    : Object(hit), Position(position), Normal(normal)
  {}

  Cog* Object;
  Vec3 Position;
  Vec3 Normal;
};

// Tool Events
namespace Events
{
  DeclareEvent(ToolActivate);
  DeclareEvent(ToolDeactivate);
  DeclareEvent(ToolDraw);
}

namespace Tags
{
  DeclareTag(Tool);
}

class ViewportTextWidget : public Text
{
  ByteColor mTextNonHoverColor;
  ByteColor mTextHoverColor;

public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef ViewportTextWidget ZilchSelf;

  ViewportTextWidget(Composite* parent) : Text(parent, "Text")
  {
    mTextNonHoverColor = 0xFFFFFFFF;
    mTextHoverColor = 0xFF3489EC;

    mFont = FontManager::GetDefault()->GetRenderFont(40);

    ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
    ConnectThisTo(this, Events::MouseExit, OnMouseExit);
  }

  void OnMouseEnter(MouseEvent* e)
  {
    SetColor(ToFloatColor(mTextHoverColor));
  }

  void OnMouseExit(MouseEvent* e)
  {
    SetColor(ToFloatColor(mTextNonHoverColor));
  }

  void UpdateTransform() override
  {
    Vec2 size = GetSize();
    Vec2 viewportSize = mParent->GetSize();

    Vec2 pos = viewportSize / 2.0f - size / 2.0f;

    SetTranslation(Math::ToVector3(pos, 0));
    Text::UpdateTransform();
  }
};

//------------------------------------------------------------------------- Tool
class Tool : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  static Component* GetOrCreateEditComponent(BoundType* meta, StringParam defaultName, StringParam defaultArchetype, CogId& lastEdited, bool canCreate = true);
  static ViewportTextWidget* CreateViewportTextWidget(StringParam text);

  virtual void Initialize() {}
  virtual void Activate() {}
  virtual void Deactivate() {}
  virtual String ToString(bool shortFormat = false) const { return ZilchVirtualTypeId(this)->Name; }

  virtual bool KeyDown(Viewport* viewport, KeyboardEvent* event) { return false; }
  virtual bool KeyUp(Viewport* viewport, KeyboardEvent* event) { return false; }
  virtual bool MouseDown(Viewport* viewport, MouseEvent* event) { return false; }
  virtual bool MouseUp(Viewport* viewport, MouseEvent* event) { return false; }
  
  virtual bool RightMouseDown(Viewport* viewport, MouseEvent* event) { return false; }
  virtual bool RightMouseUp(Viewport* viewport, MouseEvent* event) { return false; }

  virtual bool MouseDoubleClick(Viewport* viewport, MouseEvent* event) { return false; }

  virtual void MouseMoveOnViewport(Viewport* viewport, MouseEvent* event) {}
  virtual void MouseUpdateOnViewport(Viewport* viewport, MouseEvent* event) {}
  virtual void Draw() {}

  virtual bool MouseScroll(Viewport* viewport, MouseEvent* event) { return false; }
  virtual void FocusLost(Viewport* viewport, FocusEvent* event) {}

  virtual void StartDrag(Viewport* viewport, Vec2Param mouseStart) {}
  virtual bool MouseDragMovement( Viewport* viewport, Vec2Param mouseStart, Vec2Param newPosition) { return false; }
  virtual void MouseDragUpdate(Viewport* viewport, Vec2Param mouseStart, Vec2Param newPosition){};
  virtual void EndDrag(Viewport* viewport) {}

  /// Returned value should be the single ui composite attached to toolControl
  virtual Composite* AddCustomUI(Composite* toolControl) { return nullptr; }
  virtual bool NeedsPropertyGrid() { return false; }

  //Dragging
  void BeginDrag(Viewport* viewport);
  Editor* mEditor;
  Command* mToolCommand;
};

//----------------------------------------------------- Collider Ray Cast Result
class Collider;
struct ColliderRayCastResult
{
  Collider* HitCollider;
  Vec3 PositionInWorld;
  Vec3 DirectionInWorld;
  Vec3 PositionOnObject;
};

class SelectTool;
struct BaseCastFilter;

//Ray cast using the current options on the selection tool.
SelectionResult EditorRayCast(Viewport* viewport, Vec2 mousePosition);
void ColliderRayCast(Viewport* viewport, Vec2Param mousePosition,
                     ColliderRayCastResult& result, BaseCastFilter* filter = nullptr);
void BeginSelectDrag(EditorViewport* viewport, MouseEvent* event, SelectTool* tool);

}//namespace Zero
