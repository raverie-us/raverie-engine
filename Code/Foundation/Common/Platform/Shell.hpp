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

DeclareEnum3(ProgressType, Normal, Indeterminate, None);

DeclareEnum17(GamepadRawButton,
  A,
  B,
  X,
  Y,
  LeftShoulder,
  RightShoulder,
  LeftTrigger,
  RightTrigger,
  Back,
  Start,
  LeftThumb,
  RightThumb,
  DpadUp,
  DpadDown,
  DpadLeft,
  DpadRight,
  Center
);

DeclareEnum4(GamepadRawAxis,
  LeftThumbX,
  LeftThumbY,
  RightThumbX,
  RightThumbY
);

class GamepadRawButtonState {
public:
  bool mPressed = false;
  bool mTouched = false;
  float mValue = 0.0f;
};

class GamepadRawAxisState {
public:
  float mValue = 0.0f;
};

class GamepadRawState
{
public:
  GamepadRawButtonState& GetOrCreateButton(uint32_t buttonIndex);
  GamepadRawAxisState& GetOrCreateAxis(uint32_t axisIndex);

  bool mConnected = false;
  String mId;
  Array<GamepadRawButtonState> mButtons;
  Array<GamepadRawAxisState> mAxes;
};

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

  /// Sends a message to close the window (the window is not destroyed).
  void Close();

  void SetProgress(const char* textOrNull, float percent);

  GamepadRawState& GetOrCreateGamepad(uint32_t gamepadIndex);

  // Internals

  /// The current cursor (defaults to Arrow).
  Cursor::Enum mCursor;
  bool mMouseState[MouseButtons::Size] = {false};
  bool mKeyState[Keys::Size] = {false};
  IntVec2 mClientSize = IntVec2::cZero;
  static IntVec2 sInitialClientSize;
  static bool sInitialFocused;

  // We don't limit this to 4 since we use this for joysticks too
  Array<GamepadRawState> mGamepads;

  // If the mouse is currently trapped (not visible and centered on the window).
  bool mMouseTrapped = false;

  bool mHasFocus = false;
  bool mInitialLoadingComplete = false;

  static Shell* sInstance;
  
};

} // namespace Zero
