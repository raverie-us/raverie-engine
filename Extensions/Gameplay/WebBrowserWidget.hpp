///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class WebBrowserWidget : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  WebBrowserWidget(Composite* composite, const WebBrowserSetup& setup);

  void OnBackPressed(ObjectEvent* event);
  void OnForwardPressed(ObjectEvent* event);
  void OnReloadPressed(ObjectEvent* event);
  void OnAddressTextSubmit(ObjectEvent* event);

  void OnWebBrowserPointQuery(WebBrowserPointQueryEvent* event);
  void OnWebBrowserUrlChanged(WebBrowserUrlEvent* event);
  void OnWebBrowserCursorChanged(WebBrowserCursorEvent* event);
  void OnWebBrowserTitleChanged(WebBrowserTextEvent* event);
  void OnWebBrowserStatusChanged(WebBrowserTextEvent* event);
  void OnWebBrowserConsoleMessage(WebBrowserConsoleEvent* event);

  void OnFocusGained(FocusEvent* event);
  void OnFocusLost(FocusEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  void OnKeyUp(KeyboardEvent* event);
  void OnTextTyped(KeyboardTextEvent* event);
  void OnMouseMove(MouseEvent* event);
  void OnMouseDown(MouseEvent* event);
  void OnMouseDoubleClick(MouseEvent* event);
  void OnMouseUp(MouseEvent* event);
  void OnMouseScroll(MouseEvent* event);
  
  void UpdateTransform() override;

  void OnEngineUpdate(UpdateEvent* event);

  Composite* mAddressBar;
  IconButton* mBack;
  IconButton* mForward;
  IconButton* mReload;
  TextBox* mAddressText;
  TextBox* mStatusBar;
  TextureView* mBrowserView;
  WebBrowser* mBrowser;
};

} // namespace Zero
