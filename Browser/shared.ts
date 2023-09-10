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

export type ToWorkerMessageType = MessageCanvas | MessageMouseMove;

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
  cursor: number;
}

export type ToMainMessageType = MessageYieldDraw | MessageYieldComplete | MessageMouseTrap | MessageMouseSetCursor;
