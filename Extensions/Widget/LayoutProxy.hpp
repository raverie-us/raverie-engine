///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------------------- Layout Proxy
/// Used to allow custom manipulation of widgets while still remaining in 
/// a layout (ie. widgets sliding into position).
class LayoutProxy : public Composite
{
public:
  /// Constructor.
  LayoutProxy(Widget* widgetToProxy);

  /// Composite Interface.
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  /// Because the child is no longer in a layout, it won't be re-sized if 
  /// the proxy is resized. So if this is set to true, the child will
  /// be manually sized to the proxies size on UpdateTransform.
  bool mMaintainChildSize;

  /// A handle to the widget we're proxying. If it ever goes invalid, the
  /// proxy should be destroyed.
  HandleOf<Widget> mProxy;
};

/// Proxies the given widget.
LayoutProxy* CreateLayoutProxy(Widget* widget);

/// Proxies and animates the given widget in.
LayoutProxy* ProxyAndAnimateIn(Widget* widget, Vec3Param startPos,
                       float translateTime, float fadeInTime, float delay);

} //namespace Zero
