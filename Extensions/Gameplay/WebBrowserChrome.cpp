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
  DefineEvent(ChromePopup);
}

ZilchDefineType(ChromePopupEvent, builder, type)
{

}
//------------------------------------------------------------------ Chrome
// We need to keep this global object alive
CefRefPtr<Chrome> gPlatform;
ZilchDefineType(Chrome, builder, type)
{
}

Chrome::Chrome() :
  mCreatedWebBrowser(nullptr)
{
  ConnectThisTo(this, Events::ChromePopup, OnChromePopup);
}

Chrome::~Chrome()
{
}

void Chrome::OnChromePopup(ChromePopupEvent* event)
{
  WebBrowser* webBrowser = gPlatform->GetWebBrowser(event->mBrowser);
  if (webBrowser == nullptr)
    return;

  WebBrowserPopupCreateEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mName = event->mTargetFrameName;
  toSend.mUrl = event->mTargetUrl;
  webBrowser->DispatchEvent(Events::WebBrowserPopup, &toSend);
}

ChromeBrowserData& Chrome::GetWebBrowserData(WebBrowser* webBrowser)
{
  CEF_REQUIRE_UI_THREAD();
  int id = (int)(size_t)webBrowser->mPlatformBrowser;
  return mIdToData[id];
}

ChromeBrowserData& Chrome::GetWebBrowserData(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();
  int id = browser->GetIdentifier();
  return mIdToData[id];
}

WebBrowser* Chrome::GetWebBrowser(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();
  int id = browser->GetIdentifier();
  WebBrowser* webBrowser = mIdToData[id].mWebBrowser;
  return webBrowser;
}

CefRefPtr<CefBrowser> Chrome::GetCefBrowser(WebBrowser* webBrowser)
{
  CEF_REQUIRE_UI_THREAD();
  int id = (int)(size_t)webBrowser->mPlatformBrowser;
  CefRefPtr<CefBrowser> browser = mIdToData[id].mCefBrowser;
  return browser;
}

bool Chrome::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect & rect)
{
  WebBrowser* webBrowser = GetWebBrowser(browser);
  IntVec2 size = IntVec2(512, 512);

  if (webBrowser)
    size = webBrowser->GetSize();

  rect.x = 0;
  rect.y = 0;
  rect.width = size.x;
  rect.height = size.y;
  return true;
}

void Chrome::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
{
  WebBrowser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr)
    return;

  int srcX = width;
  int srcY = height;

  int dstX = (int)webBrowser->mBuffer.Width;
  int dstY = (int)webBrowser->mBuffer.Height;

  int minX = Math::Min(srcX, dstX);
  int minY = Math::Min(srcY, dstY);

  ByteColor* dst = webBrowser->mBuffer.Data;
  ByteColor* src = (ByteColor*)buffer;

  for (size_t i = 0; i < dirtyRects.size(); ++i)
  {
    const CefRect& rect = dirtyRects[i];
    
    int maxX = Math::Min(minX, rect.x + rect.width);
    int maxY = Math::Min(minY, rect.y + rect.height);

    for (int y = rect.y; y < maxY; ++y)
    {
      for (int x = rect.x; x < maxX; ++x)
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

bool Chrome::GetScreenPoint
(
  CefRefPtr<CefBrowser> browser,
  int viewX,
  int viewY,
  int& screenX,
  int& screenY
)
{
  CEF_REQUIRE_UI_THREAD();

  WebBrowser* webBrowser = gPlatform->GetWebBrowser(browser);
  if (webBrowser == nullptr)
  {
    screenX = 0;
    screenY = 0;
    return true;
  }

  WebBrowserPointQueryEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mViewPixelPosition = IntVec2(viewX, viewY);
  webBrowser->DispatchEvent(Events::WebBrowserPointQuery, &toSend);

  screenX = toSend.mScreenPixelPosition.x;
  screenY = toSend.mScreenPixelPosition.y;
  return true;
}

void Chrome::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& custom_cursor_info)
{
  WebBrowser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr)
    return;

  WebBrowserCursorEvent toSend;
  toSend.mWebBrowser = webBrowser;

  switch (type)
  {
    case CT_POINTER:
      toSend.mCursor = Cursor::Arrow;
      break;
    case CT_CROSS:
      toSend.mCursor = Cursor::Cross;
      break;
    case CT_HAND:
      toSend.mCursor = Cursor::Hand;
      break;
    case CT_IBEAM:
    case CT_VERTICALTEXT:
      toSend.mCursor = Cursor::TextBeam;
      break;
    case CT_WAIT:
      toSend.mCursor = Cursor::Wait;
      break;
    case CT_WESTRESIZE:
    case CT_EASTRESIZE:
    case CT_EASTWESTRESIZE:
    case CT_COLUMNRESIZE:
      toSend.mCursor = Cursor::SizeWE;
      break;
    case CT_NORTHRESIZE:
    case CT_SOUTHRESIZE:
    case CT_NORTHSOUTHRESIZE:
    case CT_ROWRESIZE:
      toSend.mCursor = Cursor::SizeNS;
      break;
    case CT_NORTHEASTRESIZE:
    case CT_SOUTHWESTRESIZE:
      toSend.mCursor = Cursor::SizeNESW;
      break;
    case CT_NORTHWESTRESIZE:
    case CT_SOUTHEASTRESIZE:
      toSend.mCursor = Cursor::SizeNWSE;
      break;
    case CT_NORTHEASTSOUTHWESTRESIZE:
    case CT_NORTHWESTSOUTHEASTRESIZE:
      toSend.mCursor = Cursor::SizeAll;
      break;
    case CT_MIDDLEPANNING:
    case CT_EASTPANNING:
    case CT_NORTHPANNING:
    case CT_NORTHEASTPANNING:
    case CT_NORTHWESTPANNING:
    case CT_SOUTHPANNING:
    case CT_SOUTHEASTPANNING:
    case CT_SOUTHWESTPANNING:
    case CT_WESTPANNING:
      toSend.mCursor = Cursor::Cross;
      break;
    case CT_MOVE:
      toSend.mCursor = Cursor::SizeAll;
      break;
    case CT_NONE:
      toSend.mCursor = Cursor::Invisible;
      break;
    case CT_NODROP:
    case CT_NOTALLOWED:
      toSend.mCursor = Cursor::Cross;
      break;

    case CT_HELP:
    case CT_CELL:
    case CT_CONTEXTMENU:
    case CT_ALIAS:
    case CT_PROGRESS:
    case CT_COPY:
    case CT_ZOOMIN:
    case CT_ZOOMOUT:
    case CT_GRAB:
    case CT_GRABBING:
    case CT_CUSTOM:
      break;
  }

  webBrowser->DispatchEvent(Events::WebBrowserCursorChanged, &toSend);
}

bool Chrome::OnBeforePopup
(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  const CefString& target_url,
  const CefString& target_frame_name,
  CefLifeSpanHandler::WindowOpenDisposition target_disposition,
  bool user_gesture,
  const CefPopupFeatures& popupFeatures,
  CefWindowInfo& windowInfo,
  CefRefPtr<CefClient>& client,
  CefBrowserSettings& settings,
  bool* no_javascript_access
)
{
  ChromePopupEvent* toSend = new ChromePopupEvent();
  toSend->mBrowser = browser;
  toSend->mFrame = frame;
  toSend->mTargetUrl = target_url.ToString().c_str();
  toSend->mTargetFrameName = target_frame_name.ToString().c_str();
  toSend->mTargetDisposition = target_disposition;
  toSend->mUserGesture = user_gesture;
  toSend->mPopupFeatures = popupFeatures;
  toSend->mWindowInfo = windowInfo;
  toSend->mClient = client;
  toSend->mSettings = settings;

  Z::gDispatch->Dispatch(ThreadContext::Main, gPlatform.get(), Events::ChromePopup, toSend);
  return true;
}

void Chrome::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();

  int id = browser->GetIdentifier();

  ChromeBrowserData& data = gPlatform->mIdToData[id];
  data.mWebBrowser = mCreatedWebBrowser;
  data.mCefBrowser = browser;
  data.mFocus = false;
  data.mVisible = true;
}

bool Chrome::DoClose(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();

  // Allow the close. For windowed browsers this will result in the OS close
  // event being sent.
  return false;
}

void Chrome::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();
}

void Chrome::OnLoadError(CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame,
  ErrorCode errorCode,
  const CefString& errorText,
  const CefString& failedUrl)
{
  CEF_REQUIRE_UI_THREAD();

  // Don't display an error for downloaded files
  if (errorCode == ERR_ABORTED)
    return;

  // Display a load error message
  std::stringstream ss;
  ss << "<html><body bgcolor=\"white\">"
    "<h2>Failed to load URL " << std::string(failedUrl) <<
    " with error " << std::string(errorText) << " (" << errorCode <<
    ").</h2></body></html>";
  frame->LoadString(ss.str(), failedUrl);
}

void Chrome::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
  WebBrowser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr)
    return;

  // This is really gross, but chrome entirely uses wide chars (16) and they have a conversion to std::string
  webBrowser->mTitle = title.ToString().c_str();

  WebBrowserTextEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mText = webBrowser->mTitle;
  webBrowser->DispatchEvent(Events::WebBrowserTitleChanged, &toSend);
}

void Chrome::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
{
  WebBrowser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr)
    return;

  WebBrowserTextEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mText = value.ToString().c_str();
  webBrowser->DispatchEvent(Events::WebBrowserStatusChanged, &toSend);
}

bool Chrome::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
  WebBrowser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr)
    return false;

  WebBrowserConsoleEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mMessage = message.ToString().c_str();
  toSend.mSource = source.ToString().c_str();
  toSend.mLine = line;
  webBrowser->DispatchEvent(Events::WebBrowserConsoleMessage, &toSend);
  return toSend.mHandled;
}

bool Chrome::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect)
{
  WebBrowser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr)
    return false;
  
  WebBrowserUrlEvent toSend;
  toSend.mWebBrowser = webBrowser;
  toSend.mUrl = request->GetURL().ToString().c_str();
  webBrowser->DispatchEvent(Events::WebBrowserUrlChanged, &toSend);
  return toSend.mHandled;
}

String GetPotentialFileName(StringParam url)
{
  StringRange lastIndex = url.FindLastOf('=');
  if (lastIndex.Empty())
    lastIndex = url.FindLastOf('?');
  if (lastIndex.Empty())
    lastIndex = url.FindLastOf('/');

  if (!lastIndex.Empty())
  {
    // Move past the single character
    String subString = url.SubString(lastIndex.End(), url.End());
    return BlockingWebRequest::UrlParamDecode(subString);
  }

  return String();
}

void FillDownloadEvent(WebBrowserDownloadEvent* event, CefRefPtr<CefDownloadItem> item)
{
  String url = item->GetURL().ToString().c_str();
  String originalUrl = item->GetOriginalUrl().ToString().c_str();

  String suggestedFileName = item->GetSuggestedFileName().ToString().c_str();

  if (suggestedFileName.Empty())
  {
    suggestedFileName = GetPotentialFileName(url);
    if (suggestedFileName.Empty())
      suggestedFileName = GetPotentialFileName(originalUrl);
    if (suggestedFileName.Empty())
      suggestedFileName = "Download.txt";
  }

  String fullPath = item->GetFullPath().ToString().c_str();
  if (fullPath.Empty())
    fullPath = FilePath::Combine(GetUserDocumentsDirectory(), suggestedFileName);

  event->mFilePath = fullPath;
  event->mIsInProgress = item->IsInProgress();
  event->mIsComplete = item->IsComplete();
  event->mCurrentSpeed = item->GetCurrentSpeed();
  event->mReceivedBytes = item->GetReceivedBytes();
  event->mTotalBytes = item->GetTotalBytes();
  event->mId = (int)item->GetId();
  event->mUrl = url;
  event->mOriginalUrl = originalUrl;
  event->mSuggestedFileName = suggestedFileName;
  event->mContentDisposition = item->GetContentDisposition().ToString().c_str();
  event->mMimeType = item->GetMimeType().ToString().c_str();
}

void Chrome::OnBeforeDownload
(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefDownloadItem> download_item,
  const CefString& suggested_name,
  CefRefPtr<CefBeforeDownloadCallback> callback
)
{
  if (download_item->IsValid() == false)
    return;

  WebBrowser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr)
    return;

  mIdToDownload[(int)download_item->GetId()] = download_item;

  WebBrowserDownloadEvent toSend;
  toSend.mWebBrowser = webBrowser;
  FillDownloadEvent(&toSend, download_item);
  webBrowser->DispatchEvent(Events::WebBrowserDownloadStarted, &toSend);

  if (!toSend.mCancel)
    callback->Continue(toSend.mFilePath.c_str(), false);
}

void Chrome::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)
{
  if (download_item->IsValid() == false)
    return;

  WebBrowser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr)
    return;

  // The OnBeforeDownload event had not yet triggered, just ignore this update
  if (mIdToDownload.ContainsKey(download_item->GetId()) == false)
    return;

  WebBrowserDownloadEvent toSend;
  toSend.mWebBrowser = webBrowser;
  FillDownloadEvent(&toSend, download_item);
  webBrowser->DispatchEvent(Events::WebBrowserDownloadUpdated, &toSend);

  bool finished = toSend.mCancel || download_item->IsCanceled() || download_item->IsComplete();

  if (toSend.mCancel)
    callback->Cancel();

  if (finished)
    mIdToDownload.Erase(download_item->GetId());
}

void WebBrowserManager::PlatformCreate()
{
  CefMainArgs args;
  CefSettings settings;

  settings.windowless_rendering_enabled = 1;
  settings.persist_session_cookies = 1;
  settings.persist_user_preferences = 1;

  CefString(&settings.cache_path) = GetTemporaryDirectory().c_str();

  // We do this so we don't get the 'out of date' chrome suggestions
  // This is mainly because the latest builds of CEF are unstable on Windows
  CefString(&settings.user_agent) = "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/9999.0.9999.0 Safari/537.36";
  CefString(&settings.browser_subprocess_path).FromASCII("ChromeSubProcess.exe");

  CefInitialize(args, settings, nullptr, nullptr);

  // This MUST be created after 'CefInitialize' because it uses Cef primitives
  gPlatform = new Chrome();
}

void WebBrowserManager::PlatformDestroy()
{
  CEF_REQUIRE_UI_THREAD();

  typedef Pair<int, ChromeBrowserData> IdToCefBrowser;
  forRange(IdToCefBrowser& pair, gPlatform->mIdToData.All())
  {
    if (pair.second.mCefBrowser != nullptr)
    {
      CefRefPtr<CefBrowserHost> host = pair.second.mCefBrowser->GetHost();
      if (host != nullptr)
        host->CloseBrowser(true);
    }
  }

  gPlatform->mIdToData.Clear();

  // This MUST be done here, because it frees the gPlatform object
  gPlatform = nullptr;

  // We apparently have to do one more iteration of work after force closing all browsers
  CefDoMessageLoopWork();
  CefShutdown();
}

void WebBrowserManager::PlatformUpdate()
{
  CEF_REQUIRE_UI_THREAD();

  // We must prevent Zero from handling windows messages since an infinite recursion can occur
  // This is primarily because the WM_TIMER message updates the engine recursively (when dragging a window)
  // and Chrome can send windows messages itself
  Z::gEnableOsWindowProcedure = false;
  CefDoMessageLoopWork();
  Z::gEnableOsWindowProcedure = true;
}

void WebBrowser::CreatePlatformBrowser(const WebBrowserSetup& setup)
{
  CEF_REQUIRE_UI_THREAD();
  CefWindowInfo window;
  window.SetAsWindowless(0, setup.mTransparent);

  CefBrowserSettings browserSettings;
  browserSettings.windowless_frame_rate = 60;
  browserSettings.webgl = STATE_DISABLED;
  browserSettings.background_color = ToByteColor(setup.mBackgroundColor);
  
  gPlatform->mCreatedWebBrowser = this;
  CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(window, gPlatform, setup.mUrl.c_str(), browserSettings, nullptr);
  gPlatform->mCreatedWebBrowser = nullptr;

  mPlatformBrowser = (void*)(size_t)browser->GetIdentifier();
}

void WebBrowser::DestroyPlatformBrowser()
{
  if (gPlatform == nullptr)
    return;

  CefRefPtr<CefBrowser> browser = gPlatform->GetCefBrowser(this);
  if (browser == nullptr)
    return;

  browser->GetHost()->CloseBrowser(true);

  int id = browser->GetIdentifier();
  gPlatform->mIdToData.Erase(id);
}

void WebBrowser::ResizePlatformBrowser(IntVec2Param size)
{
  gPlatform->GetCefBrowser(this)->GetHost()->WasResized();
}

bool WebBrowser::GetCanGoForward()
{
  return gPlatform->GetCefBrowser(this)->CanGoForward();
}

bool WebBrowser::GetCanGoBackward()
{
  return gPlatform->GetCefBrowser(this)->CanGoBack();
}

void WebBrowser::GoForward()
{
  return gPlatform->GetCefBrowser(this)->GoForward();
}

void WebBrowser::GoBackward()
{
  return gPlatform->GetCefBrowser(this)->GoBack();
}

bool WebBrowser::GetIsLoading()
{
  return gPlatform->GetCefBrowser(this)->IsLoading();
}

void WebBrowser::Reload(bool useCache)
{
  if (useCache)
    return gPlatform->GetCefBrowser(this)->Reload();
  else
    return gPlatform->GetCefBrowser(this)->ReloadIgnoreCache();
}

void WebBrowser::SetFocus(bool focus)
{
  ChromeBrowserData& data = gPlatform->GetWebBrowserData(this);
  data.mFocus = focus;

  data.mCefBrowser->GetHost()->SendFocusEvent(focus);
}

bool WebBrowser::GetFocus()
{
  return gPlatform->GetWebBrowserData(this).mFocus;
}

void WebBrowser::SetVisible(bool visible)
{
  ChromeBrowserData& data = gPlatform->GetWebBrowserData(this);
  data.mVisible = visible;

  CefRefPtr<CefBrowserHost> host = data.mCefBrowser->GetHost();
  host->SetWindowVisibility(visible);

  if (visible)
    host->SetWindowlessFrameRate(1);
  else
    host->SetWindowlessFrameRate(60);
}

bool WebBrowser::GetVisible()
{
  return gPlatform->GetWebBrowserData(this).mVisible;
}

void WebBrowser::SetUrl(StringParam url)
{
  gPlatform->GetCefBrowser(this)->GetMainFrame()->LoadURL(url.c_str());
}

String WebBrowser::GetUrl()
{
  return gPlatform->GetCefBrowser(this)->GetMainFrame()->GetURL().ToString().c_str();
}

void WebBrowser::ExecuteScriptFromLocation(StringParam script, StringParam url, int line)
{
  gPlatform->GetCefBrowser(this)->GetMainFrame()->ExecuteJavaScript(script.c_str(), url.c_str(), line);
}

void WebBrowser::SimulateKey(int key, bool down, WebBrowserModifiers::Enum modifiers)
{
  CefKeyEvent toSend;
  if (down)
    toSend.type = KEYEVENT_KEYDOWN;
  else
    toSend.type = KEYEVENT_KEYUP;
  toSend.modifiers = (uint32_t)modifiers;
  toSend.windows_key_code = key;
  toSend.native_key_code = key;
  toSend.is_system_key = 0;
  toSend.character = key;
  toSend.unmodified_character = key;

  gPlatform->GetCefBrowser(this)->GetHost()->SendKeyEvent(toSend);
}

void WebBrowser::SimulateTextTyped(int character, WebBrowserModifiers::Enum modifiers)
{
  CefKeyEvent toSend;
  toSend.type = KEYEVENT_CHAR;
  toSend.modifiers = (uint32_t)modifiers;
  toSend.windows_key_code = character;
  toSend.native_key_code = character;
  toSend.is_system_key = 0;
  toSend.character = character;
  toSend.unmodified_character = character;

  gPlatform->GetCefBrowser(this)->GetHost()->SendKeyEvent(toSend);
}

void WebBrowser::SimulateMouseMove(IntVec2Param localPosition, WebBrowserModifiers::Enum modifiers)
{
  CefMouseEvent toSend;
  toSend.modifiers = (uint32_t)modifiers;
  toSend.x = localPosition.x;
  toSend.y = localPosition.y;
  gPlatform->GetCefBrowser(this)->GetHost()->SendMouseMoveEvent(toSend, false);
}

cef_mouse_button_type_t GetButton(MouseButtons::Enum button)
{
  switch (button)
  {
  case MouseButtons::Left:
    return MBT_LEFT;
  case MouseButtons::Right:
    return MBT_RIGHT;
  case MouseButtons::Middle:
  default:
    return MBT_MIDDLE;
  }
}

void WebBrowser::SimulateMouseClick(IntVec2Param localPosition, MouseButtons::Enum button, bool down, WebBrowserModifiers::Enum modifiers)
{
  CefRefPtr<CefBrowser> browser = gPlatform->GetCefBrowser(this);

  if (button == MouseButtons::XOneBack && down == false)
  {
    browser->GoBack();
    return;
  }

  if (button == MouseButtons::XTwoForward && down == false)
  {
    browser->GoForward();
    return;
  }

  CefMouseEvent toSend;
  toSend.modifiers = (uint32_t)modifiers;
  toSend.x = localPosition.x;
  toSend.y = localPosition.y;
  browser->GetHost()->SendMouseClickEvent(toSend, GetButton(button), !down, 1);
}

void WebBrowser::SimulateMouseDoubleClick(IntVec2Param localPosition, MouseButtons::Enum button, WebBrowserModifiers::Enum modifiers)
{
  CefRefPtr<CefBrowser> browser = gPlatform->GetCefBrowser(this);

  CefMouseEvent toSend;
  toSend.modifiers = (uint32_t)modifiers;
  toSend.x = localPosition.x;
  toSend.y = localPosition.y;
  browser->GetHost()->SendMouseClickEvent(toSend, GetButton(button), true, 2);
  browser->GetHost()->SendMouseClickEvent(toSend, GetButton(button), false, 2);
}

void WebBrowser::SimulateMouseScroll(IntVec2Param localPosition, Vec2Param delta, WebBrowserModifiers::Enum modifiers)
{
  Vec2 deltaScroll = mScrollSpeed * delta;

  CefMouseEvent toSend;
  toSend.modifiers = (uint32_t)modifiers;
  toSend.x = localPosition.x;
  toSend.y = localPosition.y;
  gPlatform->GetCefBrowser(this)->GetHost()->SendMouseWheelEvent(
    toSend, (int)Math::Round(deltaScroll.x), (int)Math::Round(deltaScroll.y));
}

} // namespace Zero
