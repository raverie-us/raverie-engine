// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
/// For detecting Intel drivers to handle driver bugs.
extern bool gIntelGraphics;

DeclareBitField2(FileDialogFlags, MultiSelect, Folder);

static const IntVec2 cMinimumMonitorSize(800, 600);

struct FileDialogFilter
{
  FileDialogFilter();
  FileDialogFilter(StringParam filter);
  FileDialogFilter(StringParam description, StringParam filter);

  String mDescription;
  // i.e. "*.fbx"
  String mFilter;
};

typedef void (*FileDialogCallback)(Array<String>& files, void* userData);

/// FileDialogConfig is used to configure the Open File Dialog and the Save File
/// Dialog.
struct FileDialogInfo
{
  FileDialogInfo();
  void AddFilter(StringParam description, StringParam filter);

  String Title;
  String StartingDirectory;
  Array<FileDialogFilter> mSearchFilters;
  // Should not include '.'
  String mDefaultSaveExtension;
  String DefaultFileName;
  FileDialogFlags::Type Flags;
  FileDialogCallback mCallback;
  void* mUserData;

  /// The resulting files from an open or save operation.
  /// This will be empty if the user cancelled or the operation failed.
  /// This should be cleared on every new call to opening a dialog.
  Array<String> mFiles;
};

extern cstr KeyNames[];

namespace Keys
{
// KeyCodes for keys A - Z same as capital ASCII characters 'A' - 'Z'
// KeyCodes for keys 0 - 9 same as ASCII number characters '0' - '9'

enum Enum
{
  // Key not mapped
  Unknown = 0,

  // Letters
  A = 'A',
  B = 'B',
  C = 'C',
  D = 'D',
  E = 'E',
  F = 'F',
  G = 'G',
  H = 'H',
  I = 'I',
  J = 'J',
  K = 'K',
  L = 'L',
  M = 'M',
  N = 'N',
  O = 'O',
  P = 'P',
  Q = 'Q',
  R = 'R',
  S = 'S',
  T = 'T',
  U = 'U',
  V = 'V',
  W = 'W',
  Y = 'Y',
  X = 'X',
  Z = 'Z',

  Space = ' ',

  // Numbers
  Num0 = '0',
  Num1 = '1',
  Num2 = '2',
  Num3 = '3',
  Num4 = '4',
  Num5 = '5',
  Num6 = '6',
  Num7 = '7',
  Num8 = '8',
  Num9 = '9',

  // Symbols
  LeftBracket = '[',
  RightBracket = ']',
  Comma = ',',
  Period = '.',
  Semicolon = ';',
  Minus = '-',
  Apostrophe = '\'',
  Slash = '/',
  Backslash = '\\',

  // Arrow Keys
  Up = 128,
  Down,
  Left,
  Right,

  // Function Keys
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,

  // Special Keys
  Insert,
  Delete,
  Back,
  Home,
  End,
  Tilde,
  Tab,
  Shift,
  Alt,
  Control,
  Capital,
  Enter,
  Escape,
  PageUp,
  PageDown,
  Equal,

  // Num Pad
  NumPad0,
  NumPad1,
  NumPad2,
  NumPad3,
  NumPad4,
  NumPad5,
  NumPad6,
  NumPad7,
  NumPad8,
  NumPad9,
  Add,
  Multiply,
  Subtract,
  Divide,
  Decimal,

  None,

  KeyMax,
  Size
};
} // namespace Keys

/// As the extra mouse buttons are typically Back and Forward they have been
/// named accordingly.
DeclareEnum6(MouseButtons, Left, Right, Middle, XOneBack, XTwoForward, None);

/// Standard Mouse Cursors
DeclareEnum11(Cursor, Arrow, Wait, Cross, SizeNWSE, SizeNESW, SizeWE, SizeNS, SizeAll, TextBeam, Hand, Invisible);

// Usb Standard Usage Pages
namespace UsbUsagePage
{
const uint GenericDesktop = 0x01;
}

// Usb Standard usage ids
// for GenericDesktop page
namespace UsbUsage
{
const uint Pointer = 0x01;
const uint Mouse = 0x02;
const uint Reserved = 0x03;
const uint Joystick = 0x04;
const uint Gamepad = 0x05;
const uint Keyboard = 0x06;
const uint Keypad = 0x07;
const uint MultiAxis = 0x08;
const uint Tablet = 0x09;

// 0A-2F Reserved

const uint X = 0x30;
const uint Y = 0x31;
const uint Z = 0x32;
const uint Rx = 0x33;
const uint Ry = 0x34;
const uint Rz = 0x35;
const uint Slider = 0x36;
const uint Dial = 0x37;
const uint Wheel = 0x38;
const uint HatSwitch = 0x39;
const uint CountedBuffer = 0x3A;
const uint ByteCount = 0x3B;
const uint MotionWakeup = 0x3C;
const uint Start = 0x3D;
const uint Select = 0x3E;
const uint Vx = 0x40;
const uint Vy = 0x41;
const uint Vz = 0x42;
const uint Vbrx = 0x43;
const uint Vbry = 0x44;
const uint Vbrz = 0x45;
const uint Vno = 0x46;
const uint FeatureNotification = 0x47;
const uint SystemControl = 0x81;
const uint SystemPowerDown = 0x81;
const uint SystemSleep = 0x82;
const uint SystemWake = 0x83;
const uint SystemContextMenu = 0x84;
const uint SystemMainMenu = 0x85;
const uint SystemAppMenu = 0x86;
const uint SystemMenuHelp = 0x87;
const uint SystemMenuExit = 0x88;
const uint SystemMenuSelect = 0x89;
const uint SystemMenuRight = 0x8A;
const uint SystemMenuLeft = 0x8B;
const uint SystemMenuUp = 0x8C;
const uint SystemMenuDown = 0x8D;
const uint SystemColdRestart = 0x8E;
const uint SystemWarmRestart = 0x8F;
const uint DpadUp = 0x90;
const uint DpadDown = 0x91;
const uint DpadRight = 0x92;
const uint DpadLeft = 0x93;
} // namespace UsbUsage

ZeroShared HashMap<uint, String>& GetUsageNames();

class ZeroShared PlatformAxis
{
public:
  String mName;
  uint mOffset;
  uint mSize;
  uint mMin;
  uint mMax;
  uint mUsbUsage; // 0 if this is not available
  bool mCanBeDisabled;
};

class ZeroShared PlatformButton
{
public:
  String mName;
  uint mOffset;
  uint mBit;
};

class ZeroShared PlatformInputDevice
{
public:
  OsHandle mDeviceHandle;
  Guid mGuid;
  String mName;
  Array<PlatformAxis> mAxes;
  Array<PlatformButton> mButtons;
};

class ShellWindow;
class PlatformInputDevice;

class ClipboardData
{
public:
  bool mHasText = false;
  String mText;
  bool mHasImage = false;
  Image mImage;
};

/// Monitor coordinates means a position anywhere on the desktop relative to the
/// primary monitor's top left corner. Client coordinates means a position
/// within a window (not including title bar if the window has a border). Sizes
/// are in pixels and are independent of Monitor or Client coordinates (because
/// Monitor to Client only translates)

/// Contains any code to pump operating system messages and return information
/// about the shell.
class Shell
{
public:
  Shell();
  ~Shell();

  /// Name of the Shell's operating system.
  String GetOsName();

  /// OS specific line-scroll setting when using the mouse scroll wheel.
  uint GetScrollLineCount();

  /// Get the monitor rectangle for the primary monitor (in monitor
  /// coordinates).
  IntRect GetPrimaryMonitorRectangle();

  /// Get the monitor size for the primary monitor (in monitor coordinates).
  IntVec2 GetPrimaryMonitorSize();

  /// Get the pixel color at the mouse position.
  ByteColor GetColorAtMouse();

  /// Set a region that the cursor cannot leave (in monitor coordinates).
  void SetMonitorCursorClip(const IntRect& monitorRectangle);

  /// Release the cursor for being trapped within a clip region.
  void ClearMonitorCursorClip();

  /// Get the current mouse cursor (returns Arrow if the cursor is unknown).
  Cursor::Enum GetMouseCursor();

  /// Set a region that the cursor cannot leave.
  void SetMouseCursor(Cursor::Enum cursor);

  void SetMonitorCursorPosition(Math::IntVec2Param monitorPosition);
  Math::IntVec2 GetMonitorCursorPosition();

  /// Checks if a key is down
  bool IsKeyDown(Keys::Enum key);

  /// Checks if a mouse button is down
  bool IsMouseDown(MouseButtons::Enum button);

  /// Get an image of the entire desktop (primary monitor).
  bool GetPrimaryMonitorImage(Image* image);

  /// Show the file open dialog. Results are returned via mCallback or mFiles.
  void OpenFile(FileDialogInfo& config);

  /// Show the save file dialog. Results are returned via mCallback or mFiles.
  void SaveFile(FileDialogInfo& config);

  /// Message box used for critical failures.
  void ShowMessageBox(StringParam title, StringParam message);

  /// Pumps operation system events which will result in callbacks being called.
  /// Note that this may also update ShellWindows.
  void Update();

  /// Scan for input new devices and store them. The shell must have
  /// created a main window for this to work properly on all platforms.
  const Array<PlatformInputDevice>& ScanInputDevices();

  // Internals

  /// The current cursor (defaults to Arrow).
  Cursor::Enum mCursor;

  /// A window whose flag is passed as 'MainWindow' (there is only one allowed)
  ShellWindow* mMainWindow;
  Array<ShellWindow*> mWindows;
  Array<PlatformInputDevice> mInputDevices;

  static Shell* sInstance;

  void (*mOnCopy)(ClipboardData&, bool cut, Shell* shell);
  void (*mOnPaste)(const ClipboardData& data, Shell* shell);

  void* mUserData;

  ZeroDeclarePrivateData(Shell, 128);
};

/// Flags used to control the behavior of an ShellWindow
DeclareBitField7(WindowStyleFlags,
                 // Is the window visible. This is 'NotVisible' instead of
                 // visible so that the default is visible.
                 NotVisible,
                 // Main window
                 MainWindow,
                 // Does the window appear on the task bar
                 OnTaskBar,
                 // Does the window have a title bar area
                 TitleBar,
                 // Does this window have resizable borders
                 Resizable,
                 // Does this window have a close button
                 Close,
                 // Window has client area only
                 ClientOnly);

/// The state of the window for minimizing / maximizing
DeclareEnum5(WindowState,
             // Shrink the window to the taskbar
             Minimized,
             // Expand the window to fill the current desktop
             Maximized,
             // Arbitrarily sized window
             Windowed,
             // Window covers everything including the taskbar and is minimized
             // without focus
             Fullscreen,
             // Restore window to previous state before being minimizing
             Restore);

/// Border of the window for manipulation
DeclareEnum10(WindowBorderArea, Title, TopLeft, Top, TopRight, Left, Right, BottomLeft, Bottom, BottomRight, None);

DeclareEnum3(ProgressType, Normal, Indeterminate, None);

class ShellWindow
{
public:
  ShellWindow(Shell* shell,
              StringParam windowName,
              Math::IntVec2Param clientSize,
              Math::IntVec2Param monitorClientPos,
              ShellWindow* parentWindow,
              WindowStyleFlags::Enum flags,
              WindowState::Enum state);

  /// Destroy will be called automatically here.
  ~ShellWindow();

  /// Free the window and all resources. All calls to the
  /// window are invalid after this. This is automatically called
  /// in the destructor.
  void Destroy();

  /// Get the rectangle of the window's client area (in monitor coordinates).
  IntRect GetMonitorClientRectangle();
  void SetMonitorClientRectangle(const IntRect& monitorRect);

  /// Position of the window's top-left client area in monitor coordinates.
  IntVec2 GetMonitorClientPosition();
  void SetMonitorClientPosition(Math::IntVec2Param monitorPosition);

  /// The size of the window's client area.
  IntVec2 GetClientSize();
  void SetClientSize(Math::IntVec2Param clientSize);

  /// The minimum size of the window (in pixels).
  IntVec2 GetMinClientSize();
  void SetMinClientSize(Math::IntVec2Param minClientSize);

  /// Get the rectangle of the window's bordered area (in monitor coordinates).
  IntRect GetMonitorBorderedRectangle();
  void SetMonitorBorderedRectangle(const IntRect& monitorRect);

  /// Position of the window's top-left bordered area in monitor coordinates.
  IntVec2 GetMonitorBorderedPosition();
  void SetMonitorBorderedPosition(Math::IntVec2Param monitorPosition);

  /// The size of the window including the border (in pixels).
  IntVec2 GetBorderedSize();
  void SetBorderedSize(Math::IntVec2Param borderedSize);

  /// Get the parent window of this window. Null if there is no parent.
  ShellWindow* GetParent();

  /// Convert monitor coordinates to client coordinates.
  IntVec2 MonitorToClient(Math::IntVec2Param monitorPosition);

  /// Convert monitor coordinates to bordered coordinates.
  IntVec2 MonitorToBordered(Math::IntVec2Param monitorPosition);

  /// Convert client coordinates to monitor coordinates
  IntVec2 ClientToMonitor(Math::IntVec2Param clientPosition);

  /// Convert client coordinates to bordered coordinates
  IntVec2 ClientToBordered(Math::IntVec2Param clientPosition);

  /// Convert bordered coordinates to monitor coordinates
  IntVec2 BorderedToMonitor(Math::IntVec2Param borderedPosition);

  /// Convert bordered coordinates to client coordinates
  IntVec2 BorderedToClient(Math::IntVec2Param borderedPosition);

  /// Style flags control border style, title bar, and other features.
  WindowStyleFlags::Enum GetStyle();
  void SetStyle(WindowStyleFlags::Enum style);

  /// Is the window visible on the desktop?
  bool GetVisible();
  void SetVisible(bool visible);

  /// Set the title of window displayed in the title bar.
  String GetTitle();
  void SetTitle(StringParam title);

  /// State of the Window, Set state to Minimize, Maximize, or Restore the
  /// window
  WindowState::Enum GetState();
  void SetState(WindowState::Enum windowState);

  /// When the mouse is captured it will only allow mouse events to be sent to
  /// this window (no other operating system windows or other windows we've
  /// created).
  void SetMouseCapture(bool capture);
  bool GetMouseCapture();

  /// Try to take Focus and bring the window to the foreground.
  /// The OS may prevent this from working if this app is not the foreground
  /// app.
  void TakeFocus();
  /// Does this window have focus?
  bool HasFocus();

  /// Get an image from window.
  bool GetImage(Image* image);

  /// Sends a message to close the window (the window is not destroyed).
  void Close();

  /// Resize or move the window using the default OS method.
  void ManipulateWindow(WindowBorderArea::Enum area);

  /// Returns the last value to SetProgress (default 0).
  float GetProgress();

  /// Sets the progress visible to the operating system (for example, when
  /// downloading a file).
  void SetProgress(ProgressType::Enum progressType, float progress);

  /// Fix the window (specifically for Intel related Windowing issues with
  /// OpenGL)
  void PlatformSpecificFixup();

  /// If this window has it's own buttons, then we may not need to draw our own.
  bool HasOwnMinMaxExitButtons();

  /// The window has been requested to close.
  void (*mOnClose)(ShellWindow* window);

  /// The window has been activated or deactivated.
  void (*mOnFocusChanged)(bool activated, ShellWindow* window);

  /// Files have been drag-dropped onto this window.
  void (*mOnMouseDropFiles)(Math::IntVec2Param clientPosition, const Array<String>& files, ShellWindow* window);

  /// An update that is called in cases where the window may be frozen (such as
  /// dragging on the Windows OS).
  void (*mOnFrozenUpdate)(ShellWindow* window);

  /// Occurs when the window is resized (may occur even if the size is the same
  /// and should be protected against).
  void (*mOnClientSizeChanged)(Math::IntVec2Param clientSize, ShellWindow* window);

  void (*mOnMinimized)(ShellWindow* window);
  void (*mOnRestored)(ShellWindow* window);

  /// Callback for when a keycode is translated into text.
  void (*mOnTextTyped)(Rune rune, ShellWindow* window);

  /// Callback for when a keyboard key is pressed.
  void (*mOnKeyDown)(Keys::Enum key, uint osKey, bool repeated, ShellWindow* window);

  /// Callback for when a keyboard key is released.
  void (*mOnKeyUp)(Keys::Enum key, uint osKey, ShellWindow* window);

  void (*mOnMouseDown)(Math::IntVec2Param clientPosition, MouseButtons::Enum button, ShellWindow* window);
  void (*mOnMouseUp)(Math::IntVec2Param clientPosition, MouseButtons::Enum button, ShellWindow* window);
  void (*mOnMouseMove)(Math::IntVec2Param clientPosition, ShellWindow* window);
  void (*mOnMouseScrollX)(Math::IntVec2Param clientPosition, float scrollAmount, ShellWindow* window);
  void (*mOnMouseScrollY)(Math::IntVec2Param clientPosition, float scrollAmount, ShellWindow* window);

  /// Called when any hardware devices change.
  void (*mOnDevicesChanged)(ShellWindow* window);

  /// Called when a mouse moves (raw input without pointer balistics applied).
  void (*mOnRawMouseChanged)(Math::IntVec2Param movement, ShellWindow* window);

  /// Called when the window is asking if a position should result in dragging
  /// or resizing the window.
  WindowBorderArea::Enum (*mOnHitTest)(Math::IntVec2Param clientPosition, ShellWindow* window);

  /// Called when an input device is updated.
  void (*mOnInputDeviceChanged)(
      PlatformInputDevice& device, uint buttons, const Array<uint>& axes, const DataBlock& data, ShellWindow* window);

  /// Userdata used for all callbacks.
  void* mUserData;

  // Internals

  /// The shell we were created from.
  Shell* mShell;

  /// The operating system specific handle to the window.
  OsHandle mHandle;

  /// Minimum size of a window. Should be initialized to IntVec2(10, 10).
  IntVec2 mMinClientSize;

  /// Should be initialized to the passed in clientSize.
  /// Only should be used to track the last set client size (detect changes).
  IntVec2 mClientSize;

  /// If this window has captured the mouse.
  bool mCapture;

  /// Should be initialized to (-1, -1)
  /// Only should be used to track the last set mouse position (detect changes).
  IntVec2 mClientMousePosition;

  BitField<WindowStyleFlags::Enum> mStyle;

  ShellWindow* mParent;

  float mProgress;

  ZeroDeclarePrivateData(ShellWindow, 64);
};

} // namespace Zero
