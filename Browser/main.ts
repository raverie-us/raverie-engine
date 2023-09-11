import RaverieEngineWorker from "./index.ts?worker";
import { Cursor, MessageCanvas, MessageMouseDown, MessageMouseMove, MessageMouseUp, MouseButtons, ToMainMessageType } from "./shared";

const parent = document.createElement("div");
parent.style.position = "relative";

const canvas = document.createElement("canvas");
canvas.style.position = "absolute";
canvas.style.backgroundColor = "#000";
canvas.width = 800;
canvas.height = 600;
const offscreenCanvas = canvas.transferControlToOffscreen();

const yieldCanvas = document.createElement("canvas");
yieldCanvas.style.position = "absolute";
yieldCanvas.style.backgroundColor = "#000";
yieldCanvas.style.display = "none";
yieldCanvas.style.pointerEvents = "none";
yieldCanvas.width = canvas.width;
yieldCanvas.height = canvas.height;
canvas.append(yieldCanvas);
const ctx = yieldCanvas.getContext("2d")!;

parent.append(canvas);
parent.append(yieldCanvas);
document.body.append(parent);

const worker = new RaverieEngineWorker();
worker.addEventListener("message", (event: MessageEvent<ToMainMessageType>) => {
  const data = event.data;
  switch (data.type) {
    case "yieldDraw":
      const imageData = new ImageData(data.pixels, data.width, data.height);
      ctx.putImageData(imageData, 0, 0);
      yieldCanvas.style.display = "block";
      break;
    case "yieldComplete":
      yieldCanvas.style.display = "none";
      break;
    case "mouseTrap":
      if (data.value) {
        canvas.requestPointerLock();
      } else if (document.pointerLockElement === canvas) {
        document.exitPointerLock();
      }
      break;
    case "mouseSetCursor":
      switch (data.cursor) {
        case Cursor.Arrow:
          canvas.style.cursor = "default";
          break;
        case Cursor.Wait:
          canvas.style.cursor = "wait";
          break;
        case Cursor.Cross:
          canvas.style.cursor = "crosshair";
          break;
        case Cursor.SizeNWSE:
          canvas.style.cursor = "nwse-resize";
          break;
        case Cursor.SizeNESW:
          canvas.style.cursor = "nesw-resize";
          break;
        case Cursor.SizeWE:
          canvas.style.cursor = "ew-resize";
          break;
        case Cursor.SizeNS:
          canvas.style.cursor = "ns-resize";
          break;
        case Cursor.SizeAll:
          canvas.style.cursor = "all-scroll";
          break;
        case Cursor.TextBeam:
          canvas.style.cursor = "text";
          break;
        case Cursor.Hand:
          canvas.style.cursor = "pointer";
          break;
        case Cursor.Invisible:
          canvas.style.cursor = "none";
          break;
      }
      break;
  }
});

const canvasMessage: MessageCanvas = {
  type: "canvas",
  canvas: offscreenCanvas
};
worker.postMessage(canvasMessage, [offscreenCanvas]);

canvas.addEventListener("mousemove", (event) => {
  const rect = canvas.getBoundingClientRect();
  const toSend: MessageMouseMove = {
    type: "mouseMove",
    x: event.clientX - rect.left,
    y: event.clientY - rect.top,
    dx: event.movementX,
    dy: event.movementY
  };
  worker.postMessage(toSend);
});

canvas.addEventListener("mousedown", (event) => {
  const rect = canvas.getBoundingClientRect();
  const toSend: MessageMouseDown = {
    type: "mouseDown",
    x: event.clientX - rect.left,
    y: event.clientY - rect.top,
    // Button values line up with MouseButtons enum
    button: event.button as MouseButtons
  };
  worker.postMessage(toSend);
});

canvas.addEventListener("mouseup", (event) => {
  const rect = canvas.getBoundingClientRect();
  const toSend: MessageMouseUp = {
    type: "mouseUp",
    x: event.clientX - rect.left,
    y: event.clientY - rect.top,
    // Button values line up with MouseButtons enum
    button: event.button as MouseButtons
  };
  worker.postMessage(toSend);
});
