import RaverieEngineWorker from "./worker.ts?worker";
import {
  Cursor,
  KeyState,
  Keys,
  MessageCopy,
  MessageInitialize,
  MessageKeyboardButtonChanged,
  MessageMouseButtonChanged,
  MessageMouseMove,
  MessagePaste,
  MessageTextTyped,
  MouseButtons,
  MouseState,
  ToMainMessageType,
  ToWorkerMessageType
} from "./shared";

const parent = document.createElement("div");
parent.style.position = "relative";

const canvas = document.createElement("canvas");
canvas.style.position = "absolute";
canvas.style.backgroundColor = "#000";
canvas.tabIndex = 1;
canvas.width = 1024;
canvas.height = 768;
const offscreenCanvas = canvas.transferControlToOffscreen();

const yieldCanvas = document.createElement("canvas");
yieldCanvas.style.position = "absolute";
yieldCanvas.style.backgroundColor = "transparent";
yieldCanvas.style.display = "none";
yieldCanvas.style.pointerEvents = "none";
yieldCanvas.width = canvas.width;
yieldCanvas.height = canvas.height;
canvas.append(yieldCanvas);
const yieldContext = yieldCanvas.getContext("2d")!;

parent.append(canvas);
parent.append(yieldCanvas);
document.body.append(parent);

let emulatedClipboardText: string | null = null;

const downloadFile = (filename: string, buffer: ArrayBuffer) => {
  const blob = new Blob([buffer]);
  const url  = window.URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = filename;
  a.style.display = 'none';
  a.target = '_blank';
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  window.URL.revokeObjectURL(url);
}

const worker = new RaverieEngineWorker();
worker.addEventListener("message", (event: MessageEvent<ToMainMessageType>) => {
  const data = event.data;
  switch (data.type) {
    case "yieldDraw":
      const imageData = new ImageData(data.pixels, data.width, data.height);
      yieldContext.putImageData(imageData, 0, 0);
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
      case "downloadFile":
        downloadFile(data.filename, data.buffer);
        break;
      case "copyData":
        if (navigator.clipboard) {
          navigator.clipboard.writeText(data.text).catch(() => {
            emulatedClipboardText = data.text;
          });
        } else {
          // In an insecure context, we emulate copy local to the page since we can't set the clipboard immediately
          emulatedClipboardText = data.text;
        }
        break;
  }
});

const workerPostMessage = <T extends ToWorkerMessageType>(message: T, transfer?: Transferable[]) => {
  if (transfer) {
    worker.postMessage(message, transfer);
  } else {
    worker.postMessage(message);
  }
}

workerPostMessage<MessageInitialize>({
  type: "initialize",
  canvas: offscreenCanvas,
  args: new URL(location.href).searchParams.get("args") || ""
}, [offscreenCanvas]);

canvas.addEventListener("mousemove", (event) => {
  const rect = canvas.getBoundingClientRect();
  workerPostMessage<MessageMouseMove>({
    type: "mouseMove",
    x: event.clientX - rect.left,
    y: event.clientY - rect.top,
    dx: event.movementX,
    dy: event.movementY
  });
});

const onMouseButtonChanged = (event: MouseEvent) => {
  const rect = canvas.getBoundingClientRect();
  workerPostMessage<MessageMouseButtonChanged>({
    type: "mouseButtonChanged",
    x: event.clientX - rect.left,
    y: event.clientY - rect.top,
    // Button values line up with MouseButtons enum
    button: event.button as MouseButtons,
    state: (event.type === "mouseup") ? MouseState.Up : MouseState.Down
  });
};

canvas.addEventListener("mousedown", onMouseButtonChanged);
canvas.addEventListener("mouseup", onMouseButtonChanged);

const mapKeyboardKey = (code: string) => {
  switch (code) {
    case "KeyA": return Keys.A;
    case "KeyB": return Keys.B;
    case "KeyC": return Keys.C;
    case "KeyD": return Keys.D;
    case "KeyE": return Keys.E;
    case "KeyF": return Keys.F;
    case "KeyG": return Keys.G;
    case "KeyH": return Keys.H;
    case "KeyI": return Keys.I;
    case "KeyJ": return Keys.J;
    case "KeyK": return Keys.K;
    case "KeyL": return Keys.L;
    case "KeyM": return Keys.M;
    case "KeyN": return Keys.N;
    case "KeyO": return Keys.O;
    case "KeyP": return Keys.P;
    case "KeyQ": return Keys.Q;
    case "KeyR": return Keys.R;
    case "KeyS": return Keys.S;
    case "KeyT": return Keys.T;
    case "KeyU": return Keys.U;
    case "KeyV": return Keys.V;
    case "KeyW": return Keys.W;
    case "KeyY": return Keys.Y;
    case "KeyX": return Keys.X;
    case "KeyZ": return Keys.Z;

    case "Space": return Keys.Space;

    case "Digit0": return Keys.Num0;
    case "Digit1": return Keys.Num1;
    case "Digit2": return Keys.Num2;
    case "Digit3": return Keys.Num3;
    case "Digit4": return Keys.Num4;
    case "Digit5": return Keys.Num5;
    case "Digit6": return Keys.Num6;
    case "Digit7": return Keys.Num7;
    case "Digit8": return Keys.Num8;
    case "Digit9": return Keys.Num9;

    case "BracketLeft": return Keys.LeftBracket;
    case "BracketRight": return Keys.RightBracket;
    case "Comma": return Keys.Comma;

    case "Period": return Keys.Period;
    case "Semicolon": return Keys.Semicolon;
    case "Minus": return Keys.Minus;
    case "Quote": return Keys.Apostrophe;
    case "Slash": return Keys.Slash;
    case "Backslash": return Keys.Backslash;

    case "ArrowUp": return Keys.Up;
    case "ArrowDown": return Keys.Down;
    case "ArrowLeft": return Keys.Left;
    case "ArrowRight": return Keys.Right;

    case "F1": return Keys.F1;
    case "F2": return Keys.F2;
    case "F3": return Keys.F3;
    case "F4": return Keys.F4;
    case "F5": return Keys.F5;
    case "F6": return Keys.F6;
    case "F7": return Keys.F7;
    case "F8": return Keys.F8;
    case "F9": return Keys.F9;
    case "F10": return Keys.F10;
    case "F11": return Keys.F11;
    case "F12": return Keys.F12;

    case "Insert": return Keys.Insert;
    case "Delete": return Keys.Delete;
    case "Backspace": return Keys.Back;
    case "Home": return Keys.Home;
    case "End": return Keys.End;
    case "Backquote": return Keys.Tilde;
    case "Tab": return Keys.Tab;
    case "ShiftLeft": return Keys.Shift;
    case "ShiftRight": return Keys.Shift;
    case "AltLeft": return Keys.Alt;
    case "AltRight": return Keys.Alt;
    case "ControlLeft": return Keys.Control;
    case "ControlRight": return Keys.Control;
    case "CapsLock": return Keys.Capital;
    case "Enter": return Keys.Enter;
    case "Escape": return Keys.Escape;
    case "PageUp": return Keys.PageUp;
    case "PageDown": return Keys.PageDown;
    case "Equal": return Keys.Equal;

    // Numpad
    case "Numpad0": return Keys.NumPad0;
    case "Numpad1": return Keys.NumPad1;
    case "Numpad2": return Keys.NumPad2;
    case "Numpad3": return Keys.NumPad3;
    case "Numpad4": return Keys.NumPad4;
    case "Numpad5": return Keys.NumPad5;
    case "Numpad6": return Keys.NumPad6;
    case "Numpad7": return Keys.NumPad7;
    case "Numpad8": return Keys.NumPad8;
    case "Numpad9": return Keys.NumPad9;
    case "NumpadAdd": return Keys.Add;
    case "NumpadMultiply": return Keys.Multiply;
    case "NumpadSubtract": return Keys.Subtract;
    case "NumpadDivide": return Keys.Divide;
    case "NumpadDecimal": return Keys.Decimal;

    default: return Keys.Unknown;
  }
};

const onKeyboardButtonChanged = (event: KeyboardEvent) => {
  // Ideally we'd prevent all default browser behavior, but doing so supresses events like keypress
  if (event.code === "Tab" || /^F[0-9]{1,2}$/.test(event.code) || (event.ctrlKey && event.code === "KeyF")) {
    event.preventDefault();
  }

  let state = KeyState.Up;
  if (event.type === "keydown") {
    if (event.repeat) {
      state = KeyState.Repeated;
    } else {
      state = KeyState.Down;
    }
  }

  workerPostMessage<MessageKeyboardButtonChanged>({
    type: "keyboardButtonChanged",
    button: mapKeyboardKey(event.code),
    state
  });
}

canvas.addEventListener("keydown", onKeyboardButtonChanged);
canvas.addEventListener("keyup", onKeyboardButtonChanged);

// TODO(trevor): Long term, the engine should tell us when we focus on a text field, what the text is,
// and what position the cursor is at. We can then create an invisible text input, set the text,
// and change to the correct position so that we can get auto-complete and proper mobile support.
canvas.addEventListener("keypress", (event) => {
  workerPostMessage<MessageTextTyped>({
    type: "textTyped",
    rune: event.charCode
  });
});

canvas.addEventListener("contextmenu", (event) => {
  event.preventDefault();
});

const copyCutHandler = (event: ClipboardEvent) => {
  if (document.activeElement === canvas) {
    workerPostMessage<MessageCopy>({
      type: "copy",
      isCut: event.type === "cut"
    });
    event.preventDefault();
  }
};

// These two event handlers don't work on the canvas (only on document) so we must check focus
document.addEventListener("copy", copyCutHandler);
document.addEventListener("paste", (event) => {
  if (document.activeElement === canvas && (event.clipboardData || emulatedClipboardText)) {
    workerPostMessage<MessagePaste>({
      type: "paste",
      text: emulatedClipboardText || event.clipboardData!.getData("text/plain")
    });
    emulatedClipboardText = null;
    event.preventDefault();
  }
});
