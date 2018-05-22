///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#undef GetNextSibling
#undef GetFirstChild

// NOTE: This should only ever be included by one cpp (implementation generally hidden)
#include <include/cef_app.h>
#include <include/cef_browser.h>
#include <include/cef_client.h>

#include <include/base/cef_bind.h>
#include <include/cef_app.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/wrapper/cef_helpers.h>
#include <include/cef_browser.h>
#include <include/cef_browser_process_handler.h>
#include <include/wrapper/cef_helpers.h>
#include <include/cef_parser.h>

#pragma comment(lib, "libcef.lib")

#if defined(ZeroDebug)
#pragma comment(lib, "libcef_dll_wrapper_debug.lib")
#else
#pragma comment(lib, "libcef_dll_wrapper_release.lib")
#endif

namespace Zero
{

class ChromeBrowserData
{
public:
  Browser* mWebBrowser;
  CefRefPtr<CefBrowser> mCefBrowser;
  bool mFocus;
  bool mVisible;
};

class ChromePopupInfo
{
public:
  CefRefPtr<CefBrowser> mBrowser;
  CefRefPtr<CefFrame> mFrame;
  String mTargetUrl;
  String mTargetFrameName;
  CefLifeSpanHandler::WindowOpenDisposition mTargetDisposition;
  bool mUserGesture;
  CefPopupFeatures mPopupFeatures;
  CefWindowInfo mWindowInfo;
  CefRefPtr<CefClient> mClient;
  CefBrowserSettings mSettings;
};

class Chrome :
  public CefClient,
  public CefDisplayHandler,
  public CefLifeSpanHandler,
  public CefLoadHandler,
  public CefRenderHandler,
  public CefRequestHandler,
  public CefDownloadHandler
{
public:
  Chrome();
  ~Chrome();

  // CefClient Interface
  CefRefPtr<CefDisplayHandler>  GetDisplayHandler()   override { return this; }
  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler()  override { return this; }
  CefRefPtr<CefLoadHandler>     GetLoadHandler()      override { return this; }
  CefRefPtr<CefRenderHandler>   GetRenderHandler()    override { return this; }
  CefRefPtr<CefRequestHandler>  GetRequestHandler()   override { return this; }
  CefRefPtr<CefDownloadHandler> GetDownloadHandler()  override { return this; }
  
  // CefRenderHandler Interface
  bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect & rect) override;
  void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList & dirtyRects, const void * buffer, int width, int height) override;
  bool GetScreenPoint
  (
    CefRefPtr<CefBrowser> browser,
    int viewX,
    int viewY,
    int& screenX,
    int& screenY
  ) override;
  void OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursor, CursorType type, const CefCursorInfo& custom_cursor_info) override;

  // CefDisplayHandler Interface
  void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;
  void OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value) override;
  bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line) override;

  // CefLifeSpanHandler Interface
  bool OnBeforePopup
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
  ) override;
  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  bool DoClose(CefRefPtr<CefBrowser> browser) override;
  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefLoadHandler Interface
  void OnLoadError(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    ErrorCode errorCode,
    const CefString& errorText,
    const CefString& failedUrl) override;

  // CefRequestHandler Interface
  bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect);

  // CefDownloadHandler Interface
  void OnBeforeDownload
  (
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDownloadItem> download_item,
    const CefString& suggested_name,
    CefRefPtr<CefBeforeDownloadCallback> callback
  ) override;
  void OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback) override;

  void OnChromePopup(ChromePopupInfo* event);

  // Maps from internal platform browsers to our Browser
  ChromeBrowserData& GetWebBrowserData(CefRefPtr<CefBrowser> browser);
  ChromeBrowserData& GetWebBrowserData(Browser* webBrowser);
  Browser* GetWebBrowser(CefRefPtr<CefBrowser> browser);
  CefRefPtr<CefBrowser> GetCefBrowser(Browser* webBrowser);
  HashMap<int, ChromeBrowserData> mIdToData;
  HashMap<int, CefRefPtr<CefDownloadItem> > mIdToDownload;

  // We are currently creating a CefBrowser for this Browser
  Browser* mCreatedWebBrowser;

  // Popups are added on another thread and must be locked / pumped on an update.
  ThreadLock mPopupsLock;
  Array<ChromePopupInfo> mPopups;

  IMPLEMENT_REFCOUNTING(Chrome);
};

//------------------------------------------------------------------ Chrome
// We need to keep this global object alive
CefRefPtr<Chrome> gPlatform;

Chrome::Chrome() :
  mCreatedWebBrowser(nullptr)
{
}

Chrome::~Chrome()
{
}

void Chrome::OnChromePopup(ChromePopupInfo* event)
{
  Browser* webBrowser = gPlatform->GetWebBrowser(event->mBrowser);
  if (webBrowser == nullptr || !webBrowser->mOnPopup)
    return;

  String name = event->mTargetFrameName;
  if (name.Empty())
  {
    name = CefString(&event->mWindowInfo.window_name).ToString().c_str();

    if (name.Empty())
      name = "Popup";
  }
  
  webBrowser->mOnPopup(name, event->mTargetUrl, webBrowser);
}

ChromeBrowserData& Chrome::GetWebBrowserData(Browser* webBrowser)
{
  CEF_REQUIRE_UI_THREAD();
  int id = (int)(size_t)webBrowser->mHandle;
  return mIdToData[id];
}

ChromeBrowserData& Chrome::GetWebBrowserData(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();
  int id = browser->GetIdentifier();
  return mIdToData[id];
}

Browser* Chrome::GetWebBrowser(CefRefPtr<CefBrowser> browser)
{
  CEF_REQUIRE_UI_THREAD();
  int id = browser->GetIdentifier();
  Browser* webBrowser = mIdToData[id].mWebBrowser;
  return webBrowser;
}

CefRefPtr<CefBrowser> Chrome::GetCefBrowser(Browser* webBrowser)
{
  CEF_REQUIRE_UI_THREAD();
  int id = (int)(size_t)webBrowser->mHandle;
  CefRefPtr<CefBrowser> browser = mIdToData[id].mCefBrowser;
  return browser;
}

bool Chrome::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect & rect)
{
  Browser* webBrowser = GetWebBrowser(browser);
  IntVec2 size = IntVec2(1024, 1024);

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
  CEF_REQUIRE_UI_THREAD();

  Browser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr || !webBrowser->mOnPaint)
    return;

  Array<IntRect> rects;
  for (size_t i = 0; i < dirtyRects.size(); ++i)
  {
    const CefRect& dirtyRect = dirtyRects[i];
    IntRect& rect = rects.PushBack();
    rect.X = dirtyRect.x;
    rect.Y = dirtyRect.y;
    rect.SizeX = dirtyRect.width;
    rect.SizeY = dirtyRect.height;
  }

  webBrowser->mOnPaint(BrowserColorFormat::BGRA8, (const byte*)buffer, IntVec2(width, height), rects, webBrowser);
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

  Browser* webBrowser = gPlatform->GetWebBrowser(browser);
  if (webBrowser == nullptr || !webBrowser->mOnPointQuery)
  {
    screenX = 0;
    screenY = 0;
    return true;
  }

  IntVec2 monitorPos = IntVec2::cZero;
  webBrowser->mOnPointQuery(IntVec2(viewX, viewY), &monitorPos, webBrowser);
  screenX = monitorPos.x;
  screenY = monitorPos.y;
  return true;
}

void Chrome::OnCursorChange(CefRefPtr<CefBrowser> browser, CefCursorHandle cursorHandle, CursorType type, const CefCursorInfo& custom_cursor_info)
{
  CEF_REQUIRE_UI_THREAD();

  Browser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr || !webBrowser->mOnCursorChanged)
    return;

  Cursor::Enum cursor = Cursor::Arrow;
  switch (type)
  {
    case CT_POINTER:
      cursor = Cursor::Arrow;
      break;
    case CT_CROSS:
      cursor = Cursor::Cross;
      break;
    case CT_HAND:
      cursor = Cursor::Hand;
      break;
    case CT_IBEAM:
    case CT_VERTICALTEXT:
      cursor = Cursor::TextBeam;
      break;
    case CT_WAIT:
      cursor = Cursor::Wait;
      break;
    case CT_WESTRESIZE:
    case CT_EASTRESIZE:
    case CT_EASTWESTRESIZE:
    case CT_COLUMNRESIZE:
      cursor = Cursor::SizeWE;
      break;
    case CT_NORTHRESIZE:
    case CT_SOUTHRESIZE:
    case CT_NORTHSOUTHRESIZE:
    case CT_ROWRESIZE:
      cursor = Cursor::SizeNS;
      break;
    case CT_NORTHEASTRESIZE:
    case CT_SOUTHWESTRESIZE:
      cursor = Cursor::SizeNESW;
      break;
    case CT_NORTHWESTRESIZE:
    case CT_SOUTHEASTRESIZE:
      cursor = Cursor::SizeNWSE;
      break;
    case CT_NORTHEASTSOUTHWESTRESIZE:
    case CT_NORTHWESTSOUTHEASTRESIZE:
      cursor = Cursor::SizeAll;
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
      cursor = Cursor::Cross;
      break;
    case CT_MOVE:
      cursor = Cursor::SizeAll;
      break;
    case CT_NONE:
      cursor = Cursor::Invisible;
      break;
    case CT_NODROP:
    case CT_NOTALLOWED:
      cursor = Cursor::Cross;
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

  webBrowser->mOnCursorChanged(cursor, webBrowser);
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
  mPopupsLock.Lock();
  ChromePopupInfo& toSend = mPopups.PushBack();
  toSend.mBrowser = browser;
  toSend.mFrame = frame;
  toSend.mTargetUrl = target_url.ToString().c_str();
  toSend.mTargetFrameName = target_frame_name.ToString().c_str();
  toSend.mTargetDisposition = target_disposition;
  toSend.mUserGesture = user_gesture;
  toSend.mPopupFeatures = popupFeatures;
  toSend.mWindowInfo = windowInfo;
  toSend.mClient = client;
  toSend.mSettings = settings;
  mPopupsLock.Unlock();
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
  CEF_REQUIRE_UI_THREAD();

  Browser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr || !webBrowser->mOnTitleChanged)
    return;

  // This is really gross, but chrome entirely uses wide chars (16) and they have a conversion to std::string
  webBrowser->mTitle = title.ToString().c_str();

  webBrowser->mOnTitleChanged(webBrowser->mTitle, webBrowser);
}

void Chrome::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
{
  CEF_REQUIRE_UI_THREAD();

  Browser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr || !webBrowser->mOnStatusChanged)
    return;

  webBrowser->mStatus = value.ToString().c_str();

  webBrowser->mOnStatusChanged(webBrowser->mStatus, webBrowser);
}

bool Chrome::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
  CEF_REQUIRE_UI_THREAD();

  Browser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr || !webBrowser->mOnConsoleMessage)
    return false;

  bool handled = false;
  webBrowser->mOnConsoleMessage(message.ToString().c_str(), source.ToString().c_str(), line, &handled, webBrowser);
  return handled;
}

bool Chrome::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect)
{
  CEF_REQUIRE_UI_THREAD();

  Browser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr || !webBrowser->mOnUrlChanged)
    return false;

  bool handled = false;
  webBrowser->mOnUrlChanged(request->GetURL().ToString().c_str(), &handled, webBrowser);

  // Check to see if the scheme is supported. If not, don't handle it.
  CefURLParts urlParts;
  if (CefParseURL(request->GetURL(), urlParts))
  {
    String scheme = String(CefString(&urlParts.scheme).ToString().c_str()).ToLower();

    // This list was gleaned from cef_scheme.h and posts on the CEF forum
    static const String cHttp("http");
    static const String cHttps("https");
    static const String cFile("file");
    static const String cFtp("ftp");
    static const String cAbout("about");
    static const String cViewCache("view-cache");

    // If the scheme is not one of the above, then respond as if we handled the url
    bool isUnknownScheme =
      scheme != cHttp &&
      scheme != cHttps &&
      scheme != cFile &&
      scheme != cFtp &&
      scheme != cAbout &&
      scheme != cViewCache;

    // Always handle the event in the case of an unknown scheme
    // otherwise the browser will bring the user to an error page
    if (isUnknownScheme)
      return true;
  }

  return handled;
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
    return UrlParamDecode(subString);
  }

  return String();
}

void FillDownload(BrowserDownload* download, CefRefPtr<CefDownloadItem> item)
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

  download->mFilePath = fullPath;
  download->mIsInProgress = item->IsInProgress();
  download->mIsComplete = item->IsComplete();
  download->mCurrentSpeed = item->GetCurrentSpeed();
  download->mReceivedBytes = item->GetReceivedBytes();
  download->mTotalBytes = item->GetTotalBytes();
  download->mId = (int)item->GetId();
  download->mUrl = url;
  download->mOriginalUrl = originalUrl;
  download->mSuggestedFileName = suggestedFileName;
  download->mContentDisposition = item->GetContentDisposition().ToString().c_str();
  download->mMimeType = item->GetMimeType().ToString().c_str();
}

void Chrome::OnBeforeDownload
(
  CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefDownloadItem> download_item,
  const CefString& suggested_name,
  CefRefPtr<CefBeforeDownloadCallback> callback
)
{
  CEF_REQUIRE_UI_THREAD();

  if (download_item->IsValid() == false)
    return;

  Browser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr || !webBrowser->mOnDownloadStarted)
    return;

  mIdToDownload[(int)download_item->GetId()] = download_item;

  BrowserDownload download;
  FillDownload(&download, download_item);

  bool cancel = false;
  webBrowser->mOnDownloadStarted(download, &cancel, webBrowser);

  if (!cancel)
    callback->Continue(download.mFilePath.c_str(), false);
}

void Chrome::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)
{
  CEF_REQUIRE_UI_THREAD();

  if (download_item->IsValid() == false)
    return;

  Browser* webBrowser = GetWebBrowser(browser);
  if (webBrowser == nullptr || !webBrowser->mOnDownloadUpdated)
    return;

  // The OnBeforeDownload event had not yet triggered, just ignore this update
  if (mIdToDownload.ContainsKey(download_item->GetId()) == false)
    return;

  BrowserDownload download;
  FillDownload(&download, download_item);

  bool cancel = false;
  webBrowser->mOnDownloadUpdated(download, &cancel, webBrowser);

  bool finished = cancel || download_item->IsCanceled() || download_item->IsComplete();

  if (cancel)
    callback->Cancel();

  if (finished)
    mIdToDownload.Erase(download_item->GetId());
}

void Browser::PlatformCreate()
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

void Browser::PlatformDestroy()
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

void Browser::PlatformUpdate()
{
  CEF_REQUIRE_UI_THREAD();

  CefDoMessageLoopWork();
  
  gPlatform->mPopupsLock.Lock();
  forRange(ChromePopupInfo& popupInfo, gPlatform->mPopups)
    gPlatform->OnChromePopup(&popupInfo);
  gPlatform->mPopups.Clear();
  gPlatform->mPopupsLock.Unlock();
}

Browser::Browser(const BrowserSetup& setup) :
  mUserData(nullptr),
  mLastSetUrl(setup.mUrl),
  mScrollSpeed(setup.mScrollSpeed),
  mBackgroundColor(setup.mBackgroundColor),
  mTransparent(setup.mTransparent),
  mSize(setup.mSize)
{
  CefWindowInfo window;
  window.SetAsWindowless(0, setup.mTransparent);

  CefBrowserSettings browserSettings;
  browserSettings.windowless_frame_rate = 60;
  browserSettings.webgl = STATE_DISABLED;

  // Change from RGBA to BGRA (which is what Chrome uses)
  ByteColor backgroundColor = ToByteColor(setup.mBackgroundColor);
  byte* dstBytes = (byte*)&backgroundColor;
  Math::Swap(dstBytes[0], dstBytes[2]);

  browserSettings.background_color = backgroundColor;
  
  // These globals are how we pass parameters to the created browser
  gPlatform->mCreatedWebBrowser = this;
  CefRefPtr<CefBrowser> browser = CefBrowserHost::CreateBrowserSync(window, gPlatform, setup.mUrl.c_str(), browserSettings, nullptr);
  gPlatform->mCreatedWebBrowser = nullptr;

  mHandle = (void*)(size_t)browser->GetIdentifier();
}

Browser::~Browser()
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

Math::IntVec2 Browser::GetSize()
{
  return mSize;
}

void Browser::SetSize(Math::IntVec2Param size)
{
  mSize = size;
  gPlatform->GetCefBrowser(this)->GetHost()->WasResized();
}

bool Browser::GetCanGoForward()
{
  return gPlatform->GetCefBrowser(this)->CanGoForward();
}

bool Browser::GetCanGoBackward()
{
  return gPlatform->GetCefBrowser(this)->CanGoBack();
}

void Browser::GoForward()
{
  return gPlatform->GetCefBrowser(this)->GoForward();
}

void Browser::GoBackward()
{
  return gPlatform->GetCefBrowser(this)->GoBack();
}

bool Browser::GetIsLoading()
{
  return gPlatform->GetCefBrowser(this)->IsLoading();
}

void Browser::Reload(bool useCache)
{
  if (useCache)
    return gPlatform->GetCefBrowser(this)->Reload();
  else
    return gPlatform->GetCefBrowser(this)->ReloadIgnoreCache();
}

void Browser::SetFocus(bool focus)
{
  ChromeBrowserData& data = gPlatform->GetWebBrowserData(this);
  data.mFocus = focus;

  data.mCefBrowser->GetHost()->SendFocusEvent(focus);
}

bool Browser::GetFocus()
{
  return gPlatform->GetWebBrowserData(this).mFocus;
}

void Browser::SetVisible(bool visible)
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

void ReInitializeBrowser(Browser* browser)
{
  BrowserSetup setup;
  setup.mUrl = browser->GetUrl();
  setup.mSize = browser->GetSize();
  setup.mTransparent = browser->mTransparent;
  setup.mBackgroundColor = browser->mBackgroundColor;
  setup.mScrollSpeed = browser->mScrollSpeed;
  browser->~Browser();
  new (browser) Browser(setup);
}

Math::Vec4 Browser::GetBackgroundColor()
{
  return mBackgroundColor;
}

void Browser::SetBackgroundColor(Math::Vec4Param color)
{
  if (color == mBackgroundColor)
    return;

  // Reinitialize and re-fill out the mBackgroundColor color on WebBrowserSetup
  mBackgroundColor = color;
  ReInitializeBrowser(this);
}

bool Browser::GetTransparent()
{
  return mTransparent;
}

void Browser::SetTransparent(bool transparent)
{
  if (transparent == mTransparent)
    return;

  // Reinitialize and re-fill out the mTransparent color on WebBrowserSetup
  mTransparent = transparent;
  ReInitializeBrowser(this);
}

String Browser::GetStatus()
{
  return mStatus;
}

String Browser::GetTitle()
{
  return mTitle;
}

Math::Vec2 Browser::GetScrollSpeed()
{
  return mScrollSpeed;
}

void Browser::SetScrollSpeed(Math::Vec2Param pixelsPerScroll)
{
  mScrollSpeed = pixelsPerScroll;
}

bool Browser::GetVisible()
{
  return gPlatform->GetWebBrowserData(this).mVisible;
}

void Browser::SetUrl(StringParam url)
{
  // CEF Asserts if you give it an empty url...
  if (url.Empty())
    return;

  mLastSetUrl = url;
  gPlatform->GetCefBrowser(this)->GetMainFrame()->LoadURL(url.c_str());
}

String Browser::GetUrl()
{
  String url = gPlatform->GetCefBrowser(this)->GetMainFrame()->GetURL().ToString().c_str();

  if (!url.Empty())
    return url;

  return mLastSetUrl;
}

void Browser::ExecuteScriptFromLocation(StringParam script, StringParam url, int line)
{
  gPlatform->GetCefBrowser(this)->GetMainFrame()->ExecuteJavaScript(script.c_str(), url.c_str(), line);
}

void Browser::SimulateKey(int key, bool down, BrowserModifiers::Enum modifiers)
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

void Browser::SimulateTextTyped(int character, BrowserModifiers::Enum modifiers)
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

void Browser::SimulateMouseMove(Math::IntVec2Param localPosition, BrowserModifiers::Enum modifiers)
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

void Browser::SimulateMouseClick(Math::IntVec2Param localPosition, MouseButtons::Enum button, bool down, BrowserModifiers::Enum modifiers)
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

void Browser::SimulateMouseDoubleClick(Math::IntVec2Param localPosition, MouseButtons::Enum button, BrowserModifiers::Enum modifiers)
{
  CefRefPtr<CefBrowser> browser = gPlatform->GetCefBrowser(this);

  CefMouseEvent toSend;
  toSend.modifiers = (uint32_t)modifiers;
  toSend.x = localPosition.x;
  toSend.y = localPosition.y;
  browser->GetHost()->SendMouseClickEvent(toSend, GetButton(button), true, 2);
  browser->GetHost()->SendMouseClickEvent(toSend, GetButton(button), false, 2);
}

void Browser::SimulateMouseScroll(Math::IntVec2Param localPosition, Math::Vec2Param delta, BrowserModifiers::Enum modifiers)
{
  Math::Vec2 deltaScroll = mScrollSpeed * delta;

  CefMouseEvent toSend;
  toSend.modifiers = (uint32_t)modifiers;
  toSend.x = localPosition.x;
  toSend.y = localPosition.y;
  gPlatform->GetCefBrowser(this)->GetHost()->SendMouseWheelEvent(
    toSend, (int)Math::Round(deltaScroll.x), (int)Math::Round(deltaScroll.y));
}

} // namespace Zero
