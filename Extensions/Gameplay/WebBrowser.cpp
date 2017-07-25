///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(WebBrowserPopup);
  DefineEvent(WebBrowserPointQuery);
  DefineEvent(WebBrowserConsoleMessage);
  DefineEvent(WebBrowserStatusChanged);
  DefineEvent(WebBrowserTitleChanged);
  DefineEvent(WebBrowserUrlChanged);
  DefineEvent(WebBrowserCursorChanged);
  DefineEvent(WebBrowserDownloadStarted);
  DefineEvent(WebBrowserDownloadUpdated);
}

//------------------------------------------------------------------ WebBrowserEvent
ZilchDefineType(WebBrowserEvent, builder, type)
{
  ZilchBindGetterProperty(WebBrowser);
}

WebBrowser* WebBrowserEvent::GetWebBrowser()
{
  return mWebBrowser;
}

//------------------------------------------------------------------ WebBrowserPopupCreateEvent
ZilchDefineType(WebBrowserPopupCreateEvent, builder, type)
{
  ZilchBindFieldProperty(mName);
  ZilchBindFieldProperty(mUrl);
}

//------------------------------------------------------------------ WebBrowserCursorEvent
ZilchDefineType(WebBrowserCursorEvent, builder, type)
{
  ZilchBindFieldProperty(mCursor);
}

WebBrowserCursorEvent::WebBrowserCursorEvent()
{
  mCursor = Cursor::Arrow;
}

//------------------------------------------------------------------ WebBrowserPointQueryEvent
ZilchDefineType(WebBrowserPointQueryEvent, builder, type)
{
  ZilchBindFieldProperty(mViewPixelPosition);
  ZilchBindFieldProperty(mScreenPixelPosition);
}

WebBrowserPointQueryEvent::WebBrowserPointQueryEvent()
{
  mViewPixelPosition = IntVec2::cZero;
  mScreenPixelPosition = IntVec2::cZero;
}

WebBrowser* WebBrowserPointQueryEvent::GetWebBrowser()
{
  return mWebBrowser;
}

//------------------------------------------------------------------ WebBrowserConsoleEvent
ZilchDefineType(WebBrowserConsoleEvent, builder, type)
{
  ZilchBindFieldProperty(mMessage);
  ZilchBindFieldProperty(mSource);
  ZilchBindFieldProperty(mLine);
}

WebBrowserConsoleEvent::WebBrowserConsoleEvent()
{
  mLine = 0;
  mHandled = false;
}

//------------------------------------------------------------------ WebBrowserTextEvent
ZilchDefineType(WebBrowserTextEvent, builder, type)
{
  ZilchBindFieldProperty(mText);
}

//------------------------------------------------------------------ WebBrowserUrlEvent
ZilchDefineType(WebBrowserUrlEvent, builder, type)
{
  ZilchBindFieldProperty(mUrl);
  ZilchBindFieldProperty(mHandled);
}

WebBrowserUrlEvent::WebBrowserUrlEvent()
{
  mHandled = false;
}

//------------------------------------------------------------------ WebBrowserDownloadEvent
ZilchDefineType(WebBrowserDownloadEvent, builder, type)
{
  ZilchBindFieldProperty(mFilePath);
  ZilchBindFieldProperty(mCancel);
  ZilchBindFieldProperty(mIsInProgress);
  ZilchBindFieldProperty(mIsComplete);
  ZilchBindFieldProperty(mCurrentSpeed);
  ZilchBindFieldProperty(mReceivedBytes);
  ZilchBindFieldProperty(mTotalBytes);
  ZilchBindFieldProperty(mId);
  ZilchBindFieldProperty(mUrl);
  ZilchBindFieldProperty(mOriginalUrl);
  ZilchBindFieldProperty(mSuggestedFileName);
  ZilchBindFieldProperty(mContentDisposition);
  ZilchBindFieldProperty(mMimeType);
}

WebBrowserDownloadEvent::WebBrowserDownloadEvent()
{
  mIsInProgress = false;
  mIsComplete = false;
  mCurrentSpeed = 0;
  mReceivedBytes = 0;
  mTotalBytes = 0;
  mId = 0;
  mCancel = false;
}

//------------------------------------------------------------------ WebBrowserManager
ZilchDefineType(WebBrowserManager, builder, type)
{
}

WebBrowserManager::WebBrowserManager()
{
  PlatformCreate();
  ConnectThisTo(Z::gEngine, Events::EngineUpdate, OnEngineUpdate);
}

WebBrowserManager::~WebBrowserManager()
{
  PlatformDestroy();
}

WebBrowserManager& WebBrowserManager::GetInstance()
{
  static WebBrowserManager instance;
  return instance;
}

void WebBrowserManager::OnEngineUpdate(UpdateEvent* event)
{
  PlatformUpdate();
}

//------------------------------------------------------------------ WebBrowserSetup
ZilchDefineType(WebBrowserSetup, builder, type)
{
  ZilchBindFieldProperty(mUrl);
  ZilchBindFieldProperty(mSize);
  ZilchBindFieldProperty(mTransparent);
  ZilchBindFieldProperty(mBackgroundColor);
  ZilchBindFieldProperty(mScrollSpeed);

  ZilchBindDestructor();
  ZilchBindDefaultConstructor();
}

const String cWebBrowserDefaultUrl("http://www.google.com");
const IntVec2 cWebBrowserDefaultSize(512, 512);
const bool cWebBrowserDefaultTransparent(true);
const Vec4 cWebBrowserDefaultBackgroundColor(1.0f);
const Vec2 cWebBrowserDefaultScrollSpeed(100, 100);

WebBrowserSetup::WebBrowserSetup(StringParam url, IntVec2Param size, bool transparent, Vec4Param backgroundColor, Vec2Param scrollSpeed)
{
  mUrl = url;
  mSize = size;
  mTransparent = transparent;
  mBackgroundColor = backgroundColor;
  mScrollSpeed = scrollSpeed;
}

//------------------------------------------------------------------ WebBrowser
ZilchDefineType(WebBrowser, builder, type)
{
  ZeroBindEvent(Events::WebBrowserPopup, WebBrowserPopupCreateEvent);
  ZeroBindEvent(Events::WebBrowserPointQuery, WebBrowserPointQueryEvent);
  ZeroBindEvent(Events::WebBrowserConsoleMessage, WebBrowserConsoleEvent);
  ZeroBindEvent(Events::WebBrowserStatusChanged, WebBrowserTextEvent);
  ZeroBindEvent(Events::WebBrowserTitleChanged, WebBrowserTextEvent);
  ZeroBindEvent(Events::WebBrowserUrlChanged, WebBrowserUrlEvent);
  ZeroBindEvent(Events::WebBrowserCursorChanged, WebBrowserCursorEvent);
  ZeroBindEvent(Events::WebBrowserDownloadStarted, WebBrowserDownloadEvent);
  ZeroBindEvent(Events::WebBrowserDownloadUpdated, WebBrowserDownloadEvent);

  ZilchBindDestructor();
  ZilchBindDefaultConstructor();
  ZilchBindConstructor(WebBrowserSetup&);
  type->CreatableInScript = true;

  ZilchBindGetterSetterProperty(Size);
  ZilchBindGetterSetterProperty(Texture);
  ZilchBindFieldProperty(mStatus);
  ZilchBindFieldProperty(mTitle);
  ZilchBindFieldProperty(mScrollSpeed);

  ZilchBindGetterSetterProperty(Url);
  ZilchBindGetterProperty(CanGoForward);
  ZilchBindGetterProperty(CanGoBackward);

  ZilchBindMethodProperty(GoForward);
  ZilchBindMethodProperty(GoBackward);

  ZilchBindGetterProperty(IsLoading);
  ZilchBindOverloadedMethod(Reload, ZilchInstanceOverload(void));
  ZilchBindOverloadedMethod(Reload, ZilchInstanceOverload(void, bool));

  ZilchBindGetterSetterProperty(Focus);
  ZilchBindGetterSetterProperty(Visible);

  ZilchBindMethod(ExecuteScript);
  ZilchBindMethod(ExecuteScriptFromLocation);

  ZilchBindMethod(SimulateKey);
  ZilchBindMethod(SimulateTextTyped);
  ZilchBindMethod(SimulateMouseMove);
  ZilchBindMethod(SimulateMouseClick);
  ZilchBindMethod(SimulateMouseDoubleClick);
  ZilchBindMethod(SimulateMouseScroll);
}

WebBrowser::WebBrowser()
{
  WebBrowserSetup setup;
  Initialize(setup);
}

WebBrowser::WebBrowser(const WebBrowserSetup& setup)
{
  Initialize(setup);
}

void WebBrowser::Initialize(const WebBrowserSetup& setup)
{
  // Make sure the singleton is initialized
  WebBrowserManager::GetInstance();

  mScrollSpeed = setup.mScrollSpeed;
  mBackgroundColor = setup.mBackgroundColor;

  mBuffer.Resize(setup.mSize.x, setup.mSize.y, true, true, ToByteColor(mBackgroundColor));
  
  CreatePlatformBrowser(setup);
}

WebBrowser::~WebBrowser()
{
  DestroyPlatformBrowser();
}

void WebBrowser::ExecuteScript(StringParam script)
{
  ExecuteScriptFromLocation(script, String(), 0);
}

void WebBrowser::SetSize(IntVec2Param size)
{
  if (size.x < 0 || size.y < 0)
  {
    DoNotifyException("WebBrowser", "Cannot set the size to a negative number");
    return;
  }

  if (size == GetSize())
    return;

  mBuffer.Resize(size.x, size.y, true, true, ToByteColor(mBackgroundColor));

  ResizePlatformBrowser(size);
}

IntVec2 WebBrowser::GetSize()
{
  return IntVec2(mBuffer.Width, mBuffer.Height);
}

Texture* WebBrowser::GetTexture()
{
  return mBuffer.Image;
}

void WebBrowser::SetTexture(Texture* tex)
{
  mBuffer.Image = tex;
}

void WebBrowser::Reload()
{
  Reload(true);
}

} // namespace Zero
