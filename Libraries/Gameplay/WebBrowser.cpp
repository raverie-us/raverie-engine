// MIT Licensed (see LICENSE.md).
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
} // namespace Events

// WebBrowserEvent
ZilchDefineType(WebBrowserEvent, builder, type)
{
  ZilchBindGetterProperty(WebBrowser);
}

WebBrowser* WebBrowserEvent::GetWebBrowser()
{
  return mWebBrowser;
}

// WebBrowserPopupCreateEvent
ZilchDefineType(WebBrowserPopupCreateEvent, builder, type)
{
  ZilchBindFieldProperty(mName);
  ZilchBindFieldProperty(mUrl);
}

// WebBrowserCursorEvent
ZilchDefineType(WebBrowserCursorEvent, builder, type)
{
  ZilchBindFieldProperty(mCursor);
}

WebBrowserCursorEvent::WebBrowserCursorEvent()
{
  mCursor = Cursor::Arrow;
}

// WebBrowserPointQueryEvent
ZilchDefineType(WebBrowserPointQueryEvent, builder, type)
{
  ZilchBindFieldProperty(mBrowserPixelPosition);
  ZilchBindFieldProperty(mMonitorPixelPosition);
}

WebBrowserPointQueryEvent::WebBrowserPointQueryEvent()
{
  mBrowserPixelPosition = IntVec2::cZero;
  mMonitorPixelPosition = IntVec2::cZero;
}

// WebBrowserConsoleEvent
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

// WebBrowserTextEvent
ZilchDefineType(WebBrowserTextEvent, builder, type)
{
  ZilchBindFieldProperty(mText);
}

// WebBrowserUrlEvent
ZilchDefineType(WebBrowserUrlEvent, builder, type)
{
  ZilchBindFieldProperty(mUrl);
  ZilchBindFieldProperty(mHandled);
}

WebBrowserUrlEvent::WebBrowserUrlEvent()
{
  mHandled = false;
}

// WebBrowserDownloadEvent
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

// WebBrowserManager
ZilchDefineType(WebBrowserManager, builder, type)
{
}

WebBrowserManager::WebBrowserManager() : mInitializedPlatform(false)
{
  ConnectThisTo(Z::gEngine, Events::OsShellUpdate, OnOsShellUpdate);
}

WebBrowserManager::~WebBrowserManager()
{
  if (mInitializedPlatform)
    Browser::PlatformDestroy();
}

void WebBrowserManager::OnOsShellUpdate(Event* event)
{
  if (mInitializedPlatform)
    Browser::PlatformUpdate();
}

void WebBrowserManager::EnsurePlatformInitailized()
{
  if (mInitializedPlatform)
    return;

  mInitializedPlatform = true;
  Browser::PlatformCreate();
}

// WebBrowserSetup
ZilchDefineType(WebBrowserSetup, builder, type)
{
  ZilchBindFieldProperty(mUrl);
  ZilchBindFieldProperty(mSize);
  ZilchBindFieldProperty(mTransparent);
  ZilchBindFieldProperty(mBackgroundColor);
  ZilchBindFieldProperty(mScrollSpeed);

  ZilchBindDestructor();
  ZilchBindDefaultConstructor();

  type->CreatableInScript = true;
}

const String cWebBrowserDefaultUrl(Urls::cUserWebBrowserDefault);
const IntVec2 cWebBrowserDefaultSize(1024, 1024);
const bool cWebBrowserDefaultTransparent(false);
const Vec4 cWebBrowserDefaultBackgroundColor(1.0f);
const Vec2 cWebBrowserDefaultScrollSpeed(100, 100);

WebBrowserSetup::WebBrowserSetup(StringParam url,
                                 IntVec2Param size,
                                 IntVec2Param clientPosition,
                                 bool transparent,
                                 Vec4Param backgroundColor,
                                 Vec2Param scrollSpeed)
{
  mUrl = url;
  mSize = size;
  mClientPosition = clientPosition;
  mTransparent = transparent;
  mBackgroundColor = backgroundColor;
  mScrollSpeed = scrollSpeed;
}

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

  ZilchBindOverloadedMethod(Create, ZilchStaticOverload(HandleOf<WebBrowser>));
  ZilchBindOverloadedMethod(Create, ZilchStaticOverload(HandleOf<WebBrowser>, const WebBrowserSetup&));

  ZilchBindGetterSetterProperty(Size);
  ZilchBindGetterSetterProperty(ClientPosition);
  ZilchBindGetterSetterProperty(ZIndex);
  ZilchBindGetterProperty(IsFloatingOnTop);
  ZilchBindGetterProperty(IsSecurityRestricted);

  ZilchBindGetterProperty(Texture);
  ZilchBindGetterProperty(Status);
  ZilchBindGetterProperty(Title);
  ZilchBindGetterSetterProperty(ScrollSpeed);

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
  ZilchBindGetterSetterProperty(BackgroundColor);
  ZilchBindGetterSetterProperty(Transparent);

  ZilchBindMethod(ExecuteScript);
  ZilchBindMethod(ExecuteScriptFromLocation);

  ZilchBindMethod(SimulateKey);
  ZilchBindMethod(SimulateTextTyped);
  ZilchBindMethod(SimulateMouseMove);
  ZilchBindMethod(SimulateMouseClick);
  ZilchBindMethod(SimulateMouseDoubleClick);
  ZilchBindMethod(SimulateMouseScroll);
}

WebBrowser::WebBrowser() : WebBrowser(WebBrowserSetup())
{
}

BrowserSetup ConvertSetupAndEnsurePlatformInitialized(const WebBrowserSetup& setup)
{
  WebBrowserManager::GetInstance()->EnsurePlatformInitailized();

  BrowserSetup browserSetup;
  browserSetup.mUrl = setup.mUrl;
  browserSetup.mSize = setup.mSize;
  browserSetup.mClientPosition = setup.mClientPosition;
  browserSetup.mTransparent = setup.mTransparent;
  browserSetup.mBackgroundColor = setup.mBackgroundColor;
  browserSetup.mScrollSpeed = setup.mScrollSpeed;
  return browserSetup;
}

WebBrowser::WebBrowser(const WebBrowserSetup& setup) : mBrowser(ConvertSetupAndEnsurePlatformInitialized(setup))
{
  mBrowser.mUserData = this;

  mBrowser.mOnPaint = &OnPaint;
  mBrowser.mOnPopup = &OnPopup;
  mBrowser.mOnPointQuery = &OnPointQuery;
  mBrowser.mOnConsoleMessage = &OnConsoleMessage;
  mBrowser.mOnStatusChanged = &OnStatusChanged;
  mBrowser.mOnTitleChanged = &OnTitleChanged;
  mBrowser.mOnUrlChanged = &OnUrlChanged;
  mBrowser.mOnCursorChanged = &OnCursorChanged;
  mBrowser.mOnDownloadStarted = &OnDownloadStarted;
  mBrowser.mOnDownloadUpdated = &OnDownloadUpdated;

  mBuffer.Resize(setup.mSize.x, setup.mSize.y, true, true, ToByteColor(mBrowser.GetBackgroundColor()));
}

HandleOf<WebBrowser> WebBrowser::Create()
{
  return new WebBrowser();
}

HandleOf<WebBrowser> WebBrowser::Create(const WebBrowserSetup& setup)
{
  return new WebBrowser(setup);
}

WebBrowser::~WebBrowser()
{
}

bool WebBrowser::GetIsFloatingOnTop()
{
  return Browser::IsFloatingOnTop();
}

bool WebBrowser::GetIsSecurityRestricted()
{
  return Browser::IsSecurityRestricted();
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

  mBuffer.Resize(size.x, size.y, true, true, ToByteColor(mBrowser.GetBackgroundColor()));

  mBrowser.SetSize(size);
}

IntVec2 WebBrowser::GetSize()
{
  return IntVec2(mBuffer.Width, mBuffer.Height);
}

Math::IntVec2 WebBrowser::GetClientPosition()
{
  return mBrowser.GetClientPosition();
}

void WebBrowser::SetClientPosition(Math::IntVec2Param clientPosition)
{
  mBrowser.SetClientPosition(clientPosition);
}

int WebBrowser::GetZIndex()
{
  return mBrowser.GetZIndex();
}

void WebBrowser::SetZIndex(int zindex)
{
  mBrowser.SetZIndex(zindex);
}

Texture* WebBrowser::GetTexture()
{
  return mBuffer.Image;
}

Vec4 WebBrowser::GetBackgroundColor()
{
  return mBrowser.GetBackgroundColor();
}

void WebBrowser::SetBackgroundColor(Vec4Param color)
{
  return mBrowser.SetBackgroundColor(color);
}

bool WebBrowser::GetTransparent()
{
  return mBrowser.GetTransparent();
}

void WebBrowser::SetTransparent(bool transparent)
{
  return mBrowser.SetTransparent(transparent);
}

void WebBrowser::SetUrl(StringParam url)
{
  return mBrowser.SetUrl(url);
}

String WebBrowser::GetUrl()
{
  return mBrowser.GetUrl();
}

String WebBrowser::GetStatus()
{
  return mBrowser.GetStatus();
}
String WebBrowser::GetTitle()
{
  return mBrowser.GetTitle();
}

Vec2 WebBrowser::GetScrollSpeed()
{
  return mBrowser.GetScrollSpeed();
}

void WebBrowser::SetScrollSpeed(Vec2Param pixelsPerScroll)
{
  return mBrowser.SetScrollSpeed(pixelsPerScroll);
}

bool WebBrowser::GetCanGoForward()
{
  return mBrowser.GetCanGoForward();
}

bool WebBrowser::GetCanGoBackward()
{
  return mBrowser.GetCanGoBackward();
}

void WebBrowser::GoForward()
{
  return mBrowser.GoForward();
}

void WebBrowser::GoBackward()
{
  return mBrowser.GoBackward();
}

bool WebBrowser::GetIsLoading()
{
  return mBrowser.GetIsLoading();
}

void WebBrowser::Reload()
{
  Reload(true);
}

void WebBrowser::Reload(bool useCache)
{
  return mBrowser.Reload(useCache);
}

void WebBrowser::SetFocus(bool focus)
{
  return mBrowser.SetFocus(focus);
}

bool WebBrowser::GetFocus()
{
  return mBrowser.GetFocus();
}

void WebBrowser::SetVisible(bool visible)
{
  return mBrowser.SetVisible(visible);
}

bool WebBrowser::GetVisible()
{
  return mBrowser.GetVisible();
}

void WebBrowser::ExecuteScriptFromLocation(StringParam script, StringParam url, int line)
{
  return mBrowser.ExecuteScriptFromLocation(script, url, line);
}

void WebBrowser::SimulateKey(int key, bool down, BrowserModifiers::Enum modifiers)
{
  return mBrowser.SimulateKey(key, down, modifiers);
}

void WebBrowser::SimulateTextTyped(int character, BrowserModifiers::Enum modifiers)
{
  return mBrowser.SimulateTextTyped(character, modifiers);
}

void WebBrowser::SimulateMouseMove(IntVec2Param localPosition, BrowserModifiers::Enum modifiers)
{
  return mBrowser.SimulateMouseMove(localPosition, modifiers);
}

void WebBrowser::SimulateMouseClick(IntVec2Param localPosition,
                                    MouseButtons::Enum button,
                                    bool down,
                                    BrowserModifiers::Enum modifiers)
{
  return mBrowser.SimulateMouseClick(localPosition, button, down, modifiers);
}

void WebBrowser::SimulateMouseDoubleClick(IntVec2Param localPosition,
                                          MouseButtons::Enum button,
                                          BrowserModifiers::Enum modifiers)
{
  return mBrowser.SimulateMouseDoubleClick(localPosition, button, modifiers);
}

void WebBrowser::SimulateMouseScroll(IntVec2Param localPosition, Vec2Param delta, BrowserModifiers::Enum modifiers)
{
  return mBrowser.SimulateMouseScroll(localPosition, delta, modifiers);
}

void WebBrowser::OnPaint(BrowserColorFormat::Enum format,
                         const byte* data,
                         Math::IntVec2Param bufferSize,
                         const Array<IntRect>& dirtyRectangles,
                         Browser* browser)
{
  WebBrowser* webBrowser = (WebBrowser*)browser->mUserData;

  int srcX = bufferSize.x;
  int srcY = bufferSize.y;

  int dstX = (int)webBrowser->mBuffer.Width;
  int dstY = (int)webBrowser->mBuffer.Height;

  int minX = Math::Min(srcX, dstX);
  int minY = Math::Min(srcY, dstY);

  ErrorIf(format != BrowserColorFormat::BGRA8, "This was only coded for BGRA8 at the moment");

  ByteColor* dst = webBrowser->mBuffer.Data;
  ByteColor* src = (ByteColor*)data;

  for (size_t i = 0; i < dirtyRectangles.Size(); ++i)
  {
    const IntRect& rect = dirtyRectangles[i];

    int maxX = Math::Min(minX, rect.X + rect.SizeX);
    int maxY = Math::Min(minY, rect.Y + rect.SizeY);

    for (int y = rect.Y; y < maxY; ++y)
    {
      for (int x = rect.X; x < maxX; ++x)
      {
        ByteColor& dstColor = dst[x + y * dstX];
        ByteColor& srcColor = src[x + y * srcX];

        dstColor = srcColor;

        // Change from BGRA to RGBA
        byte* dstBytes = (byte*)&dstColor;
        Math::Swap(dstBytes[0], dstBytes[2]);
      }
    }
  }

  webBrowser->mBuffer.Upload();
}

void WebBrowser::OnPopup(StringParam name, StringParam url, Browser* browser)
{
  WebBrowser* webBrowser = (WebBrowser*)browser->mUserData;

  WebBrowserPopupCreateEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mName = name;
  toSend.mUrl = url;
  webBrowser->DispatchEvent(Events::WebBrowserPopup, &toSend);
}

void WebBrowser::OnPointQuery(Math::IntVec2Param browserPixelPosition,
                              Math::IntVec2* monitorPixelPositionOut,
                              Browser* browser)
{
  WebBrowser* webBrowser = (WebBrowser*)browser->mUserData;

  WebBrowserPointQueryEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mBrowserPixelPosition = browserPixelPosition;
  webBrowser->DispatchEvent(Events::WebBrowserPointQuery, &toSend);

  *monitorPixelPositionOut = toSend.mMonitorPixelPosition;
}

void WebBrowser::OnConsoleMessage(StringParam message, StringParam source, int line, bool* handledOut, Browser* browser)
{
  WebBrowser* webBrowser = (WebBrowser*)browser->mUserData;

  WebBrowserConsoleEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mMessage = message;
  toSend.mSource = source;
  toSend.mLine = line;
  webBrowser->DispatchEvent(Events::WebBrowserConsoleMessage, &toSend);
  *handledOut = toSend.mHandled;
}

void WebBrowser::OnStatusChanged(StringParam text, Browser* browser)
{
  WebBrowser* webBrowser = (WebBrowser*)browser->mUserData;

  WebBrowserTextEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mText = text;
  webBrowser->DispatchEvent(Events::WebBrowserStatusChanged, &toSend);
}

void WebBrowser::OnTitleChanged(StringParam text, Browser* browser)
{
  WebBrowser* webBrowser = (WebBrowser*)browser->mUserData;

  WebBrowserTextEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mText = text;
  webBrowser->DispatchEvent(Events::WebBrowserTitleChanged, &toSend);
}

void WebBrowser::OnUrlChanged(StringParam url, bool* handledOut, Browser* browser)
{
  WebBrowser* webBrowser = (WebBrowser*)browser->mUserData;

  WebBrowserUrlEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mUrl = url;
  webBrowser->DispatchEvent(Events::WebBrowserUrlChanged, &toSend);
  *handledOut = toSend.mHandled;
}

void WebBrowser::OnCursorChanged(Cursor::Enum cursor, Browser* browser)
{
  WebBrowser* webBrowser = (WebBrowser*)browser->mUserData;

  WebBrowserCursorEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mCursor = cursor;
  webBrowser->DispatchEvent(Events::WebBrowserCursorChanged, &toSend);
}

void WebBrowser::OnDownloadStarted(BrowserDownload& download, bool* cancelOut, Browser* browser)
{
  WebBrowser* webBrowser = (WebBrowser*)browser->mUserData;

  WebBrowserDownloadEvent toSend;
  toSend.mWebBrowser = webBrowser;

  toSend.mFilePath = download.mFilePath;
  toSend.mIsInProgress = download.mIsInProgress;
  toSend.mIsComplete = download.mIsComplete;
  toSend.mCurrentSpeed = download.mCurrentSpeed;
  toSend.mReceivedBytes = download.mReceivedBytes;
  toSend.mTotalBytes = download.mTotalBytes;
  toSend.mId = download.mId;
  toSend.mUrl = download.mUrl;
  toSend.mOriginalUrl = download.mOriginalUrl;
  toSend.mSuggestedFileName = download.mSuggestedFileName;
  toSend.mContentDisposition = download.mContentDisposition;
  toSend.mMimeType = download.mMimeType;

  webBrowser->DispatchEvent(Events::WebBrowserDownloadStarted, &toSend);

  download.mFilePath = toSend.mFilePath;
  *cancelOut = toSend.mCancel;
}

void WebBrowser::OnDownloadUpdated(const BrowserDownload& download, bool* cancelOut, Browser* browser)
{
  WebBrowser* webBrowser = (WebBrowser*)browser->mUserData;

  WebBrowserDownloadEvent toSend;
  toSend.mWebBrowser = webBrowser;

  toSend.mFilePath = download.mFilePath;
  toSend.mIsInProgress = download.mIsInProgress;
  toSend.mIsComplete = download.mIsComplete;
  toSend.mCurrentSpeed = download.mCurrentSpeed;
  toSend.mReceivedBytes = download.mReceivedBytes;
  toSend.mTotalBytes = download.mTotalBytes;
  toSend.mId = download.mId;
  toSend.mUrl = download.mUrl;
  toSend.mOriginalUrl = download.mOriginalUrl;
  toSend.mSuggestedFileName = download.mSuggestedFileName;
  toSend.mContentDisposition = download.mContentDisposition;
  toSend.mMimeType = download.mMimeType;

  webBrowser->DispatchEvent(Events::WebBrowserDownloadUpdated, &toSend);

  *cancelOut = toSend.mCancel;
}

} // namespace Zero
