///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------------- Layout Proxy
//******************************************************************************
LayoutProxy::LayoutProxy(Widget* widgetToProxy) :
  Composite(widgetToProxy->GetParent())
{
  // We're going to attach to the widgets parent
  Composite* parent = widgetToProxy->GetParent();

  // We were already inserted in our base constructor, so remove ourself
  parent->mChildren.Erase(this);

  // Re-Insert where the widget we're proxying is
  parent->mChildren.InsertBefore(widgetToProxy, this);

  // Attach the proxy to ourself
  AttachChildWidget(widgetToProxy);

  // Copy some props
  SetTranslation(widgetToProxy->GetTranslation());
  SetSize(widgetToProxy->GetSize());
  mSizePolicy = widgetToProxy->mSizePolicy;
  mHorizontalAlignment = widgetToProxy->mHorizontalAlignment;
  mVerticalAlignment = widgetToProxy->mVerticalAlignment;
  SetNotInLayout(widgetToProxy->mNotInLayout);
  // Move the proxy to the same position
  widgetToProxy->SetTranslation(Vec3::cZero);

  mProxy = widgetToProxy;

  mMaintainChildSize = true;
}

//******************************************************************************
void LayoutProxy::UpdateTransform()
{
  Widget* proxy = mProxy;

  // Manually update the size of the child if specified.
  if(proxy)
  {
    if(mMaintainChildSize)
      proxy->SetSize(mSize);
  }
  else
  {
    // Destroy ourself if the proxy was destroyed to avoid dangling proxies
    this->Destroy();
  }

  Composite::UpdateTransform();
}

//******************************************************************************
Vec2 LayoutProxy::GetMinSize()
{
  if(Widget* proxy = mProxy)
    return proxy->GetMinSize();
  return mSize;
}

//******************************************************************************
LayoutProxy* CreateLayoutProxy(Widget* widget)
{
  return new LayoutProxy(widget);
}

//******************************************************************************
LayoutProxy* ProxyAndAnimateIn(Widget* widget, Vec3Param startPos, float translateTime,
                       float fadeInTime, float delay)
{
  // Make a proxy so we can translate the widget manually
  LayoutProxy* proxy = CreateLayoutProxy(widget);

  // Animate it in
  widget->SetTranslation(startPos);
  widget->SetColor(Vec4(1,1,1,0));

  ActionSequence* seq = new ActionSequence(widget);
  seq->Add(new ActionDelay(delay));

  ActionGroup* group = new ActionGroup();
  group->Add(MoveWidgetAction(widget, Vec3::cZero, translateTime));
  group->Add(Fade(widget, Vec4(1), fadeInTime));

  seq->Add(group);

  return proxy;
}

} //namespace Zero
