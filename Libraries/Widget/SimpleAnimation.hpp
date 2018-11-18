///////////////////////////////////////////////////////////////////////////////
///
/// \file SimpleAnimation.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

inline Action* MoveWidgetAction(Widget* widget, Vec3 position, float time)
{
  return AnimatePropertyGetSet(Widget, Translation, 
    Ease::Quad::Out, widget, time, 
    position);
}

inline Action* SizeWidgetAction(Widget* widget, Vec2 size, float time)
{
  return AnimatePropertyGetSet(Widget, Size, Ease::Quad::Out, widget, time, size);
}

inline Action* MoveAndSizeWidgetAction(Widget* widget, Vec3 position, Vec2 size, float time)
{
  ActionGroup* group = new ActionGroup();

  group->Add(MoveWidgetAction(widget, position, time));

  group->Add(SizeWidgetAction(widget, size, time));

  return group;
}

inline Action* Fade(Widget* widget, Vec4 color, float t)
{
  return AnimatePropertyGetSet(Widget, Color, Ease::Quad::Out, widget, t, 
                               color);
}

inline Action* DestroyAction(Widget* widget)
{
  return new CallAction<Widget, &Widget::Destroy>(widget);
}

inline void AnimateTo(Widget* widget, Vec3Param position, Vec2Param size, float time = 0.3f)
{
  widget->GetActions()->Cancel();
  Action* group = MoveAndSizeWidgetAction(widget, position, size, time);
  widget->GetActions()->Add(group, ActionExecuteMode::FrameUpdate);
}

inline void AnimateToSize(Widget* widget, Vec2Param size, float time = 0.3f)
{
  widget->GetActions()->Cancel();
  Action* group = SizeWidgetAction(widget, size, time);
  widget->GetActions()->Add(group, ActionExecuteMode::FrameUpdate);
}

inline Vec3 GetCenterPosition(Vec2Param parentSize, Vec2Param childSize)
{
  Vec3 offsetCenter = ToVector3(parentSize);
  offsetCenter *= 0.5f;
  offsetCenter = offsetCenter - ToVector3(childSize) * 0.5f;
  return SnapToPixels(offsetCenter);
}

inline Vec3 GetCenterPosition(Composite* parent, Widget* child)
{
  return GetCenterPosition(parent->GetSize(), child->GetSize()); 
}

inline void CenterToWindow(Composite* parent, Widget* child, bool animate)
{
  Vec3 offsetCenter = GetCenterPosition(parent, child);

  Vec2 childSize = child->GetSize();

  if(animate)
    AnimateTo( child,  offsetCenter, childSize);
  else
  {
    child->SetSize(childSize);
    child->SetTranslation(offsetCenter);
  }
}

void AnimateLayout(Array<LayoutResult>& Results, bool animate);

}