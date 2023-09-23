// Keep in sync with Shell.hpp
export enum Cursor {
  Arrow,
  Wait,
  Cross,
  SizeNWSE,
  SizeNESW,
  SizeWE,
  SizeNS,
  SizeAll,
  TextBeam,
  Hand,
  Invisible,
};

// Keep in sync with Shell.hpp
export enum MouseButtons {
  Left,
  Right,
  Middle,
  XOneBack,
  XTwoForward,
  None
}

// Keep in sync with Shell.hpp
export enum Keys
{
  // Key not mapped
  Unknown = 0,

  // Letters
  A = 'A'.charCodeAt(0),
  B = 'B'.charCodeAt(0),
  C = 'C'.charCodeAt(0),
  D = 'D'.charCodeAt(0),
  E = 'E'.charCodeAt(0),
  F = 'F'.charCodeAt(0),
  G = 'G'.charCodeAt(0),
  H = 'H'.charCodeAt(0),
  I = 'I'.charCodeAt(0),
  J = 'J'.charCodeAt(0),
  K = 'K'.charCodeAt(0),
  L = 'L'.charCodeAt(0),
  M = 'M'.charCodeAt(0),
  N = 'N'.charCodeAt(0),
  O = 'O'.charCodeAt(0),
  P = 'P'.charCodeAt(0),
  Q = 'Q'.charCodeAt(0),
  R = 'R'.charCodeAt(0),
  S = 'S'.charCodeAt(0),
  T = 'T'.charCodeAt(0),
  U = 'U'.charCodeAt(0),
  V = 'V'.charCodeAt(0),
  W = 'W'.charCodeAt(0),
  Y = 'Y'.charCodeAt(0),
  X = 'X'.charCodeAt(0),
  Z = 'Z'.charCodeAt(0),

  Space = ' '.charCodeAt(0),

  // Numbers
  Num0 = '0'.charCodeAt(0),
  Num1 = '1'.charCodeAt(0),
  Num2 = '2'.charCodeAt(0),
  Num3 = '3'.charCodeAt(0),
  Num4 = '4'.charCodeAt(0),
  Num5 = '5'.charCodeAt(0),
  Num6 = '6'.charCodeAt(0),
  Num7 = '7'.charCodeAt(0),
  Num8 = '8'.charCodeAt(0),
  Num9 = '9'.charCodeAt(0),

  // Symbols
  LeftBracket = '['.charCodeAt(0),
  RightBracket = ']'.charCodeAt(0),
  Comma = '.charCodeAt(0),'.charCodeAt(0),
  Period = '.'.charCodeAt(0),
  Semicolon = ';'.charCodeAt(0),
  Minus = '-'.charCodeAt(0),
  Apostrophe = '\''.charCodeAt(0),
  Slash = '/'.charCodeAt(0),
  Backslash = '\\'.charCodeAt(0),

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
}

export enum MouseState {
  Up,
  Down,
}

export enum KeyState {
  Up,
  Down,
  Repeated,
}

// Should match AudioIOInterface.hpp
export enum AudioConstants {
  SampleRate = 44100,
  Channels = 2,
}

export interface MessageInitialize {
  type: "initialize";
  canvas: OffscreenCanvas;
  args: string;
  focused: boolean;
  projectArchive: Uint8Array | null;
  builtContentArchive: Uint8Array | null;
}

export interface MessageMouseMove {
  type: "mouseMove";
  clientX: number;
  clientY: number;
  dx: number;
  dy: number;
}

export interface MessageMouseScroll {
  type: "mouseScroll";
  clientX: number;
  clientY: number;
  scrollX: number;
  scrollY: number;
}

export interface MessageMouseButtonChanged {
  type: "mouseButtonChanged";
  clientX: number;
  clientY: number;
  button: MouseButtons;
  state: MouseState;
}

export interface MessageKeyboardButtonChanged {
  type: "keyboardButtonChanged";
  button: Keys;
  state: KeyState;
}

export interface MessageTextTyped {
  type: "textTyped";
  rune: number;
}

export interface MessageCopy {
  type: "copy";
  isCut: boolean;
}

export interface MessagePaste {
  type: "paste";
  text: string;
}

export interface MessagePartFile {
  fileName: string;
  buffer: ArrayBuffer;
}

export interface MessageFilesDropped {
  type: "filesDropped";
  clientX: number;
  clientY: number;
  files: MessagePartFile[];
}

export interface MessageOpenFileDialogFinish {
  type: "openFileDialogFinish";
  dialog: number;
  files: MessagePartFile[];
}

export interface MessageSizeChanged {
  type: "sizeChanged";
  clientWidth: number;
  clientHeight: number;
}

export interface MessageFocusChanged {
  type: "focusChanged";
  focused: boolean;
}

export interface MessageGamepadConnectionChanged {
  type: "gamepadConnectionChanged";
  gamepadIndex: number;
  id: string;
  connected: boolean;
}

export interface MessageGamepadButtonChanged {
  type: "gamepadButtonChanged";
  gamepadIndex: number;
  buttonIndex: number;
  pressed: boolean;
  touched: boolean;
  value: number;
}

export interface MessageGamepadAxisChanged {
  type: "gamepadAxisChanged";
  gamepadIndex: number;
  axisIndex: number;
  value: number;
}

export interface MessageAudioPort {
  type: "audioPort";
  port: MessagePort;
}

export interface MessageAudioOutputRequest {
  type: "audioOutputRequest";
  id: number;
  framesRequested: number;
}

export type ToWorkerMessageType =
  MessageInitialize |
  MessageMouseMove |
  MessageMouseScroll |
  MessageMouseButtonChanged |
  MessageKeyboardButtonChanged |
  MessageTextTyped |
  MessageCopy |
  MessagePaste |
  MessageFilesDropped |
  MessageOpenFileDialogFinish |
  MessageSizeChanged |
  MessageFocusChanged |
  MessageGamepadConnectionChanged |
  MessageGamepadButtonChanged |
  MessageGamepadAxisChanged |
  MessageAudioOutputRequest |
  MessageAudioPort;

export interface MessageYieldDraw {
  type: "yieldDraw";
  pixels: Uint8ClampedArray;
  width: number;
  height: number;
}

export interface MessageYieldComplete {
  type: "yieldComplete";
}

export interface MessageMouseTrap {
  type: "mouseTrap";
  value: boolean;
}

export interface MessageMouseSetCursor {
  type: "mouseSetCursor";
  cursor: Cursor;
}

export interface MessageDownloadFile {
  type: "downloadFile";
  filename: string;
  buffer: ArrayBuffer;
}

export interface MessageCopyData {
  type: "copyData";
  text: string;
}

export interface MessageOpenFileDialog {
  type: "openFileDialog";
  dialog: number;
  multiple: boolean;
  accept: string;
}

export interface MessageProgressUpdate {
  type: "progressUpdate";
  text: string | null;
  percent: number;
}

export interface MessageProjectSave {
  type: "projectSave";
  name: string;
  projectArchive: ArrayBuffer;
  builtContentArchive: ArrayBuffer;
}

export interface MessageOpenUrl {
  type: "openUrl";
  url: string;
}

export interface MessageGamepadVibrate {
  type: "gamepadVibrate";
  gamepadIndex: number;
  duration: number;
  intensity: number;
}

export interface MessageAudioOutput {
  type: "audioOutput";
  samplesPerChannel: Float32Array;
  id: number;
}

export type ToMainMessageType =
  MessageYieldDraw |
  MessageYieldComplete |
  MessageMouseTrap |
  MessageMouseSetCursor |
  MessageDownloadFile |
  MessageCopyData |
  MessageOpenFileDialog |
  MessageProgressUpdate |
  MessageProjectSave |
  MessageOpenUrl |
  MessageGamepadVibrate |
  MessageAudioOutput;

  export type ToAudioMessageType =
    MessageAudioOutput;
