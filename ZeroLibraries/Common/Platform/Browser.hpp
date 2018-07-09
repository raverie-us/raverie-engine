///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
DeclareBitField12(BrowserModifiers, CapsLock, Shift, Control, Alt, LeftMouse, MiddleMouse, RightMouse, Command, NumLock, IsKeyPad, IsLeft, IsRight);

DeclareEnum2(BrowserColorFormat, RGBA8, BGRA8);

class BrowserSetup
{
public:
  String mUrl;
  IntVec2 mSize;
  IntVec2 mClientPosition;
  bool mTransparent;
  Math::Vec4 mBackgroundColor;
  Math::Vec2 mScrollSpeed;
};

class BrowserDownload
{
public:
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

class Browser
{
public:
  Browser(const BrowserSetup& setup);
  ~Browser();

  /// Returns true if this browser is implemented as a browser that floats on top of everything.
  /// If the browser is floating, it may not invoke mOnPaint as it may be painted by the operating system.
  static bool IsFloatingOnTop();

  /// Returns if this browser is considered secure which means that it follows browsing standards
  /// for security, such as not allowing cross-origin requests unless explicitly allowed by the server.
  static bool IsSecurityRestricted();

  Math::IntVec2 GetSize();
  void SetSize(Math::IntVec2Param size);

  /// This is only used when the browser 'IsFloatingOnTop'.
  Math::IntVec2 GetClientPosition();
  void SetClientPosition(Math::IntVec2Param clientPosition);

  /// This is only used when the browser 'IsFloatingOnTop'.
  int GetZIndex();
  void SetZIndex(int zindex);

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

  Math::Vec4 GetBackgroundColor();
  void SetBackgroundColor(Math::Vec4Param color);

  bool GetTransparent();
  void SetTransparent(bool transparent);

  String GetStatus();
  String GetTitle();

  Math::Vec2 GetScrollSpeed();
  void SetScrollSpeed(Math::Vec2Param pixelsPerScroll);

  void SetUrl(StringParam url);
  String GetUrl();

  void ExecuteScriptFromLocation(StringParam script, StringParam url, int line);

  void SimulateKey(int key, bool down, BrowserModifiers::Enum modifiers);
  void SimulateTextTyped(int character, BrowserModifiers::Enum modifiers);
  void SimulateMouseMove(Math::IntVec2Param localPosition, BrowserModifiers::Enum modifiers);
  void SimulateMouseClick(Math::IntVec2Param localPosition, MouseButtons::Enum button, bool down, BrowserModifiers::Enum modifiers);
  void SimulateMouseDoubleClick(Math::IntVec2Param localPosition, MouseButtons::Enum button, BrowserModifiers::Enum modifiers);
  void SimulateMouseScroll(Math::IntVec2Param localPosition, Math::Vec2Param delta, BrowserModifiers::Enum modifiers);

  // This must be called before any browsers are created
  static void PlatformCreate();
  static void PlatformDestroy();
  static void PlatformUpdate();

  // Warning, the size may not match the last size that was sent since the data could be latent.
  // Caution to be taken to clamp the dirty rectangles within your buffer size.
  // This method may never be called if this browser 'IsFloatingOnTop'.
  void(*mOnPaint)(BrowserColorFormat::Enum format, const byte* data, Math::IntVec2Param bufferSize, const Array<IntRect>& dirtyRectangles, Browser* browser);

  void(*mOnPopup)(StringParam name, StringParam url, Browser* browser);
  void(*mOnPointQuery)(Math::IntVec2Param browserPixelPosition, Math::IntVec2* monitorPixelPositionOut, Browser* browser);
  void(*mOnConsoleMessage)(StringParam message, StringParam source, int line, bool* handledOut, Browser* browser);
  void(*mOnStatusChanged)(StringParam text, Browser* browser);
  void(*mOnTitleChanged)(StringParam text, Browser* browser);
  void(*mOnUrlChanged)(StringParam url, bool* handledOut, Browser* browser);
  void(*mOnCursorChanged)(Cursor::Enum cursor, Browser* browser);
  void(*mOnDownloadStarted)(BrowserDownload& download, bool* cancelOut, Browser* browser);
  void(*mOnDownloadUpdated)(const BrowserDownload& download, bool* cancelOut, Browser* browser);

  void* mUserData;

  String mLastSetUrl;
  String mStatus;
  String mTitle;
  Math::Vec2 mScrollSpeed;
  Math::Vec4 mBackgroundColor;
  bool mTransparent;
  Math::IntVec2 mSize;
  Math::IntVec2 mClientPosition;
  int mZIndex;
  bool mVisible;

  OsHandle mHandle;
};

// The implementation of this may remain empty for platforms that do not require a sub-process.
class BrowserSubProcess
{
public:
  static const bool IsRequired;
  static int Execute();
};

} // namespace Zero
