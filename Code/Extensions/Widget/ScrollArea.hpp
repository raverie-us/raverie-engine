// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

namespace Events
{
DeclareEvent(ScrollUpdated);
}

class BaseScrollArea;

/// Scroll bar class.
class ScrollBar : public Composite
{
public:
  typedef ScrollBar RaverieSelf;

  ScrollBar(BaseScrollArea* scrollparent, uint orientation);
  ~ScrollBar();

  void MouseDownSlider(MouseEvent* event);
  void MouseEnterSlider(MouseEvent* event);
  void MouseExitSlider(MouseEvent* event);

  void MouseDownUp(MouseEvent* event);
  void MouseEnterUp(MouseEvent* event);
  void MouseExitUp(MouseEvent* event);

  void MouseDownDown(MouseEvent* event);
  void MouseEnterDown(MouseEvent* event);
  void MouseExitDown(MouseEvent* event);

  void OnRightMouseUp(MouseEvent* event);

public:
  uint mOrientation;
  bool mVisible;
  bool mSliderVisible;
  bool mDoNotShow;
  BaseScrollArea* mScrollParent;

  Element* mUp;   // up or left
  Element* mDown; // down or right
  Element* mSlider;
  Element* mBackground;
};

DeclareEnum3(ScrollUpdate, Auto, ScrollBar, External);

/// ScrollArea is a composite widget with scroll bars.
class BaseScrollArea : public Composite
{
public:
  RaverieDeclareType(BaseScrollArea, TypeCopyMode::ReferenceType);

  BaseScrollArea(Composite* parent, bool modernStyle = false);
  ~BaseScrollArea();

  void DisableScrollBar(uint axis);
  bool IsScrollBarVisible(uint axis)
  {
    return mScrollBar[axis]->mVisible;
  }

  /// Set the scrolled percentage (0 to 1)
  void SetScrolledPercentage(Vec2 scrollPos);
  Vec2 GetScrolledPercentage();

  /// Set the scroll position offset (0 to scroll room)
  /// This will clamp values outside the range
  void SetScrolledOffset(Vec2 scrollPos, bool animate = false);
  Vec2 GetScrolledOffset();

  /// Scroll the area defined as min to max into view
  void ScrollAreaToView(Vec2 min, Vec2 max, bool animate = false);

  /// Current offset of the client area
  Vec2 GetClientOffset()
  {
    return mClientOffset;
  }
  void SetClientOffset(Vec2Param offset)
  {
    mClientOffset = offset;
    MarkAsNeedsUpdate();
  }

  /// Rect of the client area
  Vec4 GetClientArea();

  /// Visible Client Area scroll area size with scroll bars removed
  virtual Vec2 GetClientVisibleSize()
  {
    return mVisibleSize;
  }

  void ScrollPercent(Vec2 additivePercentage);
  void ScrollPixels(Vec2 additivePixels);

  void OnMouseScroll(MouseEvent* event);
  void SetScrolledPercentageInternal(Vec2 scrollPercentage, ScrollUpdate::Enum updateType, bool generateMessages);
  void SetScrolledOffsetInternal(Vec2Param scrollOffset, ScrollUpdate::Enum updateType, bool generateMessages, bool animate = false);
  // Changing the client area size
  virtual Vec2 GetClientSize() = 0;
  virtual void SetClientSize(Vec2 newSize) = 0;

  virtual void UpdateArea(ScrollUpdate::Enum type) = 0;

  void UpdateTransform() override;

  bool IsModernStyle();
  Vec2 GetSliderRoom();
  ScrollBar* GetVerticalScrollBar();
  float GetScrollBarWidth();

  void SetScrollWellSize(int axis, float size);
  void SetScrollSliderSize(int axis, float size);
  void SetScrollSliderOffset(int axis, float offset);

  Vec2 ClampClientOffset(Vec2Param offset);
  void UpdateScrollBars();

  bool mModernStyle;
  float mScrollSpeedScalar;

protected:
  ScrollBar* mScrollBar[2];

  Vec2 mScrollSlideRoom;
  Vec2 mVisibleSize;
  Vec2 mClientOffset;
  Vec2 mAnimatingToClientOffset;

private:
  Vec2 mScrollWellSize;
  Vec2 mSliderSize;
  Vec2 mSliderOffset;
};

// Scrollable area. Attach objects to the Client Composite;
class ScrollArea : public BaseScrollArea
{
public:
  RaverieDeclareType(ScrollArea, TypeCopyMode::ReferenceType);

  ScrollArea(Composite* composite, bool modernStyle = false);

  Composite* GetClientWidget();
  Spacer* GetBackground()
  {
    return mBackground;
  }

  void SetClientSize(Vec2 newSize) override;
  Vec2 GetClientSize() override;

  void AttachChildWidget(Widget* widget, AttachType::Enum attachType) override;
  void UpdateTransform() override;
  bool TakeFocusOverride() override;

  void UpdateArea(ScrollUpdate::Enum type);
  float GetScrollBarSize();

  Spacer* mBackground;
  Composite* mClientArea;
  Composite* mClipWidget;
};

} // namespace Raverie
