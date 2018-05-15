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

DeclareBitField12(WebBrowserModifiers, CapsLock, Shift, Control, Alt, LeftMouse, MiddleMouse, RightMouse, Command, NumLock, IsKeyPad, IsLeft, IsRight);

// Everything we need to startup and shutdown browsers (singleton)
// Also manages all instances of browsers
class WebBrowserManager : public EventObject
{
public:
  ZilchDeclareType(WebBrowserManager, TypeCopyMode::ReferenceType);

  WebBrowserManager();
  ~WebBrowserManager();
  static WebBrowserManager& GetInstance();
  void OnOsShellUpdate(Event* event);

  // These functions get implemented by the platform

  ///////////////////////////////////////////////////
  // BEGIN PLATFORM
  ///////////////////////////////////////////////////
  void PlatformCreate();
  void PlatformDestroy();
  void PlatformUpdate();
  ///////////////////////////////////////////////////
  // END PLATFORM
  ///////////////////////////////////////////////////
};

extern const String cWebBrowserDefaultUrl;
extern const IntVec2 cWebBrowserDefaultSize;
extern const bool cWebBrowserDefaultTransparent;
extern const Vec4 cWebBrowserDefaultBackgroundColor;
extern const Vec2 cWebBrowserDefaultScrollSpeed;

class WebBrowserSetup : public Object
{
public:
  ZilchDeclareType(WebBrowserSetup, TypeCopyMode::ReferenceType);
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
  ZilchDeclareType(WebBrowser, TypeCopyMode::ReferenceType);

  WebBrowser();
  WebBrowser(const WebBrowserSetup& setup);
  ~WebBrowser();

  static HandleOf<WebBrowser> Create();
  static HandleOf<WebBrowser> Create(const WebBrowserSetup& setup);

  /// Reloads the browser (full forced without cache)
  void Reload();

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

  String mLastSetUrl;
  String mStatus;
  String mTitle;
  Vec2 mScrollSpeed;
  Vec4 mBackgroundColor;
  bool mTransparent;

  ///////////////////////////////////////////////////
  // BEGIN PLATFORM
  ///////////////////////////////////////////////////

  void CreatePlatformBrowser(const WebBrowserSetup& setup);
  void DestroyPlatformBrowser();
  void ResizePlatformBrowser(IntVec2Param size);

  bool GetCanGoForward();
  bool GetCanGoBackward();

  void GoForward();
  void GoBackward();

  bool GetIsLoading();
  void Reload(bool useCache);

  void SetFocus(bool focus);
  bool GetFocus();

  void SetVisible(bool visible);
  bool GetVisible();

  void SetBackgroundColorPlatform(Vec4Param color);
  void SetTransparentPlatform(bool transparent);

  void SetUrlPlatform(StringParam url);
  String GetUrlPlatform();

  void ExecuteScriptFromLocation(StringParam script, StringParam url, int line);

  void SimulateKey(int key, bool down, WebBrowserModifiers::Enum modifiers);
  void SimulateTextTyped(int character, WebBrowserModifiers::Enum modifiers);
  void SimulateMouseMove(IntVec2Param localPosition, WebBrowserModifiers::Enum modifiers);
  void SimulateMouseClick(IntVec2Param localPosition, MouseButtons::Enum button, bool down, WebBrowserModifiers::Enum modifiers);
  void SimulateMouseDoubleClick(IntVec2Param localPosition, MouseButtons::Enum button, WebBrowserModifiers::Enum modifiers);
  void SimulateMouseScroll(IntVec2Param localPosition, Vec2Param delta, WebBrowserModifiers::Enum modifiers);

  ///////////////////////////////////////////////////
  // END PLATFORM
  ///////////////////////////////////////////////////
    
  // Internal
  void Initialize(const WebBrowserSetup& setup);
  void ReInitializePlatformBrowser();
  PixelBuffer mBuffer;
  void* mPlatformBrowser;
};

class WebBrowserEvent : public Event
{
public:
  ZilchDeclareType(WebBrowserEvent, TypeCopyMode::ReferenceType);

  /// The web browser that spawned the event
  WebBrowser* GetWebBrowser();
  HandleOf<WebBrowser> mWebBrowser;
};

class WebBrowserPopupCreateEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(WebBrowserPopupCreateEvent, TypeCopyMode::ReferenceType);

  String mName;
  String mUrl;
};

class WebBrowserCursorEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(WebBrowserCursorEvent, TypeCopyMode::ReferenceType);
  WebBrowserCursorEvent();

  /// The mouse cursor that we should be displaying (for Zero.Mouse.Cursor)
  Cursor::Enum mCursor;
};

class WebBrowserPointQueryEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(WebBrowserPointQueryEvent, TypeCopyMode::ReferenceType);
  WebBrowserPointQueryEvent();

  /// The position on the browser that we're querying for
  IntVec2 mViewPixelPosition;

  /// We must output this position given the view position
  IntVec2 mScreenPixelPosition;

  /// The web browser that was requesting the point query
  WebBrowser* GetWebBrowser();
  HandleOf<WebBrowser> mWebBrowser;
};

class WebBrowserConsoleEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(WebBrowserConsoleEvent, TypeCopyMode::ReferenceType);
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
  ZilchDeclareType(WebBrowserTextEvent, TypeCopyMode::ReferenceType);

  /// The status or title of the browser
  String mText;
};

class WebBrowserUrlEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(WebBrowserUrlEvent, TypeCopyMode::ReferenceType);
  WebBrowserUrlEvent();

  /// The url that we are navigating to
  String mUrl;
  
  /// If we handled the event (cancels the navigation)
  bool mHandled;
};

class WebBrowserDownloadEvent : public WebBrowserEvent
{
public:
  ZilchDeclareType(WebBrowserDownloadEvent, TypeCopyMode::ReferenceType);
  WebBrowserDownloadEvent();

  /// The file path may be filled out by the download handler
  String mFilePath;

  /// If we set this to true, then we cancel the event
  bool mCancel;

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
