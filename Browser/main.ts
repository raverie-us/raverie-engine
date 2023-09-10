import RaverieEngineWorker from "./index.ts?worker";
import { ToMainMessageType } from "./shared";

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
yieldCanvas.width = canvas.width;
yieldCanvas.height = canvas.height;
canvas.append(yieldCanvas);
const ctx = yieldCanvas.getContext("2d")!;

parent.append(canvas);
parent.append(yieldCanvas);
document.body.append(parent);

const worker = new RaverieEngineWorker();
worker.addEventListener("message", (event: MessageEvent<ToMainMessageType>) => {
  switch (event.data.type) {
    case "yieldDraw":
      const data = new ImageData(event.data.pixels, event.data.width, event.data.height);
      ctx.putImageData(data, 0, 0);
      yieldCanvas.style.display = "block";
      break;
    case "yieldComplete":
      yieldCanvas.style.display = "none";
      break;
  }
});

worker.postMessage({canvas: offscreenCanvas, type: 'canvas'}, [offscreenCanvas]);
