// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
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

  Array<FileDialogFilter> mSearchFilters;
  // Should not include '.'
  String mDefaultSaveExtension;
  String DefaultFileName;
  bool mMultiple = false;
  FileDialogCallback mCallback;
  void* mUserData;

  /// The resulting files from an open or save operation.
  /// This will be empty if the user cancelled or the operation failed.
  /// This should be cleared on every new call to opening a dialog.
  Array<String> mFiles;
};

DeclareEnum3(KeyState, Up, Down, Repeated);

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

DeclareEnum2(MouseState, Up, Down);

/// Standard Mouse Cursors (keep in sync with TypeScript/platforms)
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

HashMap<uint, String>& GetUsageNames();

class PlatformAxis
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

class PlatformButton
{
public:
  String mName;
  uint mOffset;
  uint mBit;
};

class PlatformInputDevice
{
public:
  OsHandle mDeviceHandle;
  Guid mGuid;
  String mName;
  Array<PlatformAxis> mAxes;
  Array<PlatformButton> mButtons;
};

/// Border of the window for manipulation
DeclareEnum10(WindowBorderArea, Title, TopLeft, Top, TopRight, Left, Right, BottomLeft, Bottom, BottomRight, None);

DeclareEnum3(ProgressType, Normal, Indeterminate, None);

class PlatformInputDevice;

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

  /// Get the pixel color at the mouse position.
  ByteColor GetColorAtMouse();

  /// Get the current mouse cursor (returns Arrow if the cursor is unknown).
  Cursor::Enum GetMouseCursor();

  /// Set a region that the cursor cannot leave.
  void SetMouseCursor(Cursor::Enum cursor);

  /// Checks if a key is down
  bool IsKeyDown(Keys::Enum key);

  /// Checks if a mouse button is down
  bool IsMouseDown(MouseButtons::Enum button);

  /// Show the file open dialog. Results are returned via mCallback or mFiles.
  void OpenFile(FileDialogInfo& config);

  /// Message box used for critical failures.
  void ShowMessageBox(StringParam title, StringParam message);

  /// Scan for input new devices and store them. The shell must have
  /// created a main window for this to work properly on all platforms.
  const Array<PlatformInputDevice>& ScanInputDevices();
  
  /// The size of the window's client area.
  IntVec2 GetClientSize();

  /// When the mouse is captured it will only allow mouse events to be sent to
  /// this window (no other operating system windows or other windows we've
  /// created).
  void SetMouseCapture(bool capture);
  bool GetMouseCapture();

  /// Locks the mouse to prevent it from moving.
  bool GetMouseTrap();
  void SetMouseTrap(bool trapped);

  /// Does this window have focus?
  bool HasFocus();

  /// Get an image from window.
  bool GetImage(Image* image);

  /// Sends a message to close the window (the window is not destroyed).
  void Close();

  /// The window has been activated or deactivated.
  //void (*mOnFocusChanged)(bool activated);

  /// Occurs when the window is resized (may occur even if the size is the same
  /// and should be protected against).
  //void (*mOnClientSizeChanged)(Math::IntVec2Param clientSize);

  /// Called when any hardware devices change.
  //void (*mOnDevicesChanged)();

  /// Called when the window is asking if a position should result in dragging
  /// or resizing the window.
  //WindowBorderArea::Enum (*mOnHitTest)(Math::IntVec2Param clientPosition);

  /// Called when an input device is updated.
  //void (*mOnInputDeviceChanged)(
  //    PlatformInputDevice& device, uint buttons, const Array<uint>& axes, const DataBlock& data);

  // Internals

  /// The current cursor (defaults to Arrow).
  Cursor::Enum mCursor;
  bool mMouseState[MouseButtons::Size] = {false};
  bool mKeyState[Keys::Size] = {false};
  IntVec2 mClientSize = IntVec2::cZero;
  static IntVec2 sInitialClientSize;

  Array<PlatformInputDevice> mInputDevices;

  // If the mouse is currently trapped (not visible and centered on the window).
  bool mMouseTrapped = false;

  static Shell* sInstance;
  
};

} // namespace Zero
