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

export interface MessageCanvas {
  type: "canvas";
  canvas: OffscreenCanvas;
}

export interface MessageMouseMove {
  type: "mouseMove";
  x: number;
  y: number;
  dx: number;
  dy: number;
}

export interface MessageMouseDown {
  type: "mouseDown";
  x: number;
  y: number;
  button: MouseButtons;
}

export interface MessageMouseUp {
  type: "mouseUp";
  x: number;
  y: number;
  button: MouseButtons;
}

export type ToWorkerMessageType = MessageCanvas | MessageMouseMove | MessageMouseDown | MessageMouseUp;

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

export type ToMainMessageType = MessageYieldDraw | MessageYieldComplete | MessageMouseTrap | MessageMouseSetCursor;
