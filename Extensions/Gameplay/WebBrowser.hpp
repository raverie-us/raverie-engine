///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(WebBrowserPopup);
  DeclareEvent(WebBrowserPointQuery);
  DeclareEvent(WebBrowserConsoleMessage);
  DeclareEvent(WebBrowserStatusChanged);
  DeclareEvent(WebBrowserTitleChanged);
  DeclareEvent(WebBrowserUrlChanged);
  DeclareEvent(WebBrowserCursorChanged);
  DeclareEvent(WebBrowserDownloadStarted);
  DeclareEvent(WebBrowserDownloadUpdated);
}

class UpdateEvent;
class WebBrowser;

// Everything we need to startup and shutdown browsers (singleton)
// Also manages all instances of browsers
class WebBrowserManager : public ExplicitSingleton<WebBrowserManager, EventObject>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  WebBrowserManager();
  ~WebBrowserManager();
  static WebBrowserManager& GetInstance();
  void OnOsShellUpdate(Event* event);
};

extern const String cWebBrowserDefaultUrl;
extern const IntVec2 cWebBrowserDefaultSize;
extern const bool cWebBrowserDefaultTransparent;
extern const Vec4 cWebBrowserDefaultBackgroundColor;
extern const Vec2 cWebBrowserDefaultScrollSpeed;

class WebBrowserSetup : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  WebBrowserSetup
  (
    StringParam url = cWebBrowserDefaultUrl,
    IntVec2Param size = cWebBrowserDefaultSize,
    bool transparent = cWebBrowserDefaultTransparent,
    Vec4Param backgroundColor = cWebBrowserDefaultBackgroundColor,
    Vec2Param scrollSpeed = cWebBrowserDefaultScrollSpeed
  );

  String mUrl;
  IntVec2 mSize;
  bool mTransparent;
  Vec4 mBackgroundColor;
  Vec2 mScrollSpeed;
};

class WebBrowser : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  WebBrowser();
  WebBrowser(const WebBrowserSetup& setup);
  ~WebBrowser();

  static HandleOf<WebBrowser> Create();
  static HandleOf<WebBrowser> Create(const WebBrowserSetup& setup);

  void ExecuteScript(StringParam script);

  void SetSize(IntVec2Param size);
  IntVec2 GetSize();

  Texture* GetTexture();

  /// The background color of the browser when no CSS background is specified or when pages are loading.
  /// Note that changing this MAY cause the browser page to refresh/reinitialize.
  Vec4 GetBackgroundColor();
  void SetBackgroundColor(Vec4Param color);

  /// Whether the browser renderer allows transparency when no CSS background is specified or when pages are loading.
  /// Note that changing this MAY cause the browser page to refresh/reinitialize.
  bool GetTransparent();
  void SetTransparent(bool transparent);

  void SetUrl(StringParam url);
  String GetUrl();

  String GetStatus();
  String GetTitle();

  Vec2 GetScrollSpeed();
  void SetScrollSpeed(Vec2Param pixelsPerScroll);

  bool GetCanGoForward();
  bool GetCanGoBackward();

  void GoForward();
  void GoBackward();

  bool GetIsLoading();

  /// Reloads the browser (full forced without cache)
  void Reload();
  void Reload(bool useCache);

  void SetFocus(bool focus);
  bool GetFocus();

  void SetVisible(bool visible);
  bool GetVisible();

  void ExecuteScriptFromLocation(StringParam script, StringParam url, int line);

  void SimulateKey(int key, bool down, BrowserModifiers::Enum modifiers);
  void SimulateTextTyped(int character, BrowserModifiers::Enum modifiers);
  void SimulateMouseMove(IntVec2Param localPosition, BrowserModifiers::Enum modifiers);
  void SimulateMouseClick(IntVec2Param localPosition, MouseButtons::Enum button, bool down, BrowserModifiers::Enum modifiers);
  void SimulateMouseDoubleClick(IntVec2Param localPosition, MouseButtons::Enum button, BrowserModifiers::Enum modifiers);
  void SimulateMouseScroll(IntVec2Param localPosition, Vec2Param delta, BrowserModifiers::Enum modifiers);

  // Internal
  PixelBuffer mBuffer;
  Browser mBrowser;

private:
  static void OnPaint(BrowserColorFormat::Enum format, const byte* data, Math::IntVec2Param bufferSize, const Array<IntRect>& dirtyRectangles, Browser* browser);
  static void OnPopup(StringParam name, StringParam url, Browser* browser);
  static void OnPointQuery(Math::IntVec2Param browserPixelPosition, Math::IntVec2* monitorPixelPositionOut, Browser* browser);
  static void OnConsoleMessage(StringParam message, StringParam source, int line, bool* handledOut, Browser* browser);
  static void OnStatusChanged(StringParam text, Browser* browser);
  static void OnTitleChanged(StringParam text, Browser* browser);
  static void OnUrlChanged(StringParam url, bool* handledOut, Browser* browser);
  static void OnCursorChanged(Cursor::Enum cursor, Browser* browser);
  static void OnDownloadStarted(BrowserDownload& download, bool* cancelOut, Browser* browser);
  static void OnDownloadUpdated(const BrowserDownload& download, bool* cancelOut, Browser* browser);
};

class WebBrowserEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// The web browser that spawned the event
  WebBrowser* GetWebBrowser();
  HandleOf<WebBrowser> mWebBrowser;
};

class WebBrowserPopupCreateEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String mName;
  String mUrl;
};

class WebBrowserCursorEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  WebBrowserCursorEvent();

  /// The mouse cursor that we should be displaying (for Zero.Mouse.Cursor)
  Cursor::Enum mCursor;
};

class WebBrowserPointQueryEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  WebBrowserPointQueryEvent();

  /// The position on the browser that we're querying for
  IntVec2 mBrowserPixelPosition;

  /// We must output this position given the view position
  IntVec2 mMonitorPixelPosition;
};

class WebBrowserConsoleEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  WebBrowserConsoleEvent();

  /// The console message that was written
  String mMessage;

  /// A unique identifier where the console message originated from
  String mSource;

  /// The line that the console message originated from (relative to the Source)
  int mLine;

  /// Set to true to indicate that you handled the event
  bool mHandled;
};

class WebBrowserTextEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// The status or title of the browser
  String mText;
};

class WebBrowserUrlEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  WebBrowserUrlEvent();

  /// The url that we are navigating to
  String mUrl;
  
  /// If we handled the event (cancels the navigation)
  bool mHandled;
};

class WebBrowserDownloadEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  WebBrowserDownloadEvent();

  /// If we set this to true, then we cancel the event
  bool mCancel;

  /// The file path may be filled out by the download handler
  String mFilePath;

  bool mIsInProgress;
  bool mIsComplete;
  long long mCurrentSpeed;
  long long mReceivedBytes;
  long long mTotalBytes;
  int mId;
  String mUrl;
  String mOriginalUrl;
  String mSuggestedFileName;
  String mContentDisposition;
  String mMimeType;
};

} // namespace Zero
