// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class KeyboardEvent;
class HandleableEvent;

namespace Events
{
DeclareEvent(ValueChanged);
} // namespace Events

void TabJump(Widget* widget, KeyboardEvent* event);
void FindNextFocus(Widget* widget, FocusDirection::Enum direction);

// A widget that fills up space
class Spacer : public Widget
{
public:
  RaverieDeclareType(Spacer, TypeCopyMode::ReferenceType);

  Spacer(Composite* parent);
  Spacer(Composite* parent, SizePolicy::Enum policy, Vec2Param size);
  ~Spacer();
  Vec2 mMinSize;
  Vec2 GetMinSize() override;
  void SetSize(Vec2 size);
};

class Splitter : public Composite
{
public:
  RaverieDeclareType(Splitter, TypeCopyMode::ReferenceType);

  Splitter(Composite* parent);
  ~Splitter();

  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  void OnLeftMouseDown(MouseEvent* event);
  void OnLeftMouseUp(MouseEvent* event);
  void OnMouseMove(MouseEvent* event);
  void OnMouseEnter(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);

  bool mDragging;
  /// Reverses the logic for who to expand/contract.
  /// Easiest way to deal with different layouts at the moment.
  bool mReverse;
  /// The axis we're resizing on (x or y axis).
  SizeAxis::Enum mAxis;

  Element* mBackground;

  /// The position of the mouse when it was clicked down.
  Vec2 mMouseStart;
  /// The size of the left widget when the mouse was clicked.
  Vec2 mLeftStartSize;
  Vec2 mLeftStartMinSize;
  /// The size of the right widget when the mouse was clicked.
  Vec2 mRightStartSize;
  Vec2 mRightStartMinSize;
};

} // namespace Raverie
