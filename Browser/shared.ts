export interface MessageCanvas {
  type: "canvas";
  canvas: OffscreenCanvas;
}

export type ToWorkerMessageType = MessageCanvas;

export interface MessageYieldDraw {
  type: "yieldDraw";
  pixels: Uint8ClampedArray;
  width: number;
  height: number;
}

export interface MessageYieldComplete {
  type: "yieldComplete";
}

export type ToMainMessageType = MessageYieldDraw | MessageYieldComplete;
