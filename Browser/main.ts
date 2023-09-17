import RaverieEngineWorker from "./worker.ts?worker";
import {
  Cursor,
  KeyState,
  Keys,
  MessageCopy,
  MessageFilesDropped,
  MessageFocusChanged,
  MessageInitialize,
  MessageKeyboardButtonChanged,
  MessageMouseButtonChanged,
  MessageMouseMove,
  MessageMouseScroll,
  MessageOpenFileDialogFinish,
  MessagePartFile,
  MessagePaste,
  MessageSizeChanged,
  MessageTextTyped,
  MouseButtons,
  MouseState,
  ToMainMessageType,
  ToWorkerMessageType
} from "./shared";

const parent = document.createElement("div");
parent.style.position = "relative";
parent.style.width = "100%";
parent.style.height = "100%";
document.body.append(parent);

const canvas = document.createElement("canvas");
canvas.style.position = "absolute";
canvas.style.width = "100%";
canvas.style.height = "100%";
canvas.style.backgroundColor = "#000";
canvas.style.outline = "none";
canvas.tabIndex = 1;
const initialRect = parent.getBoundingClientRect();
canvas.width = initialRect.width;
canvas.height = initialRect.height;
parent.append(canvas);
const offscreenCanvas = canvas.transferControlToOffscreen();

const yieldCanvas = document.createElement("canvas");
yieldCanvas.style.position = "absolute";
yieldCanvas.style.backgroundColor = "transparent";
yieldCanvas.style.display = "none";
yieldCanvas.style.pointerEvents = "none";
yieldCanvas.width = canvas.width;
yieldCanvas.height = canvas.height;
parent.append(yieldCanvas);
const yieldContext = yieldCanvas.getContext("2d")!;

const input = document.createElement("input");
input.type = "file";
parent.append(input);
let currentDialog: number | null = null;

const checkFocus = () => document.hasFocus() && document.visibilityState == "visible";
let focused = checkFocus();


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
          console.log("Copy Data Below:");
          console.log(emulatedClipboardText);
        }
        break;
      case "openFileDialog":
        input.accept = data.accept;
        input.multiple = data.multiple;
        currentDialog = data.dialog;
        input.click();
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

input.addEventListener("change", async (event) => {
  if (currentDialog) {
    const files: MessagePartFile[] = [];
    if (input.files) {
      for (const file of input.files) {
        files.push({
          fileName: file.name,
          buffer: await file.arrayBuffer()
        });
      }
    }
    console.log(files);

    workerPostMessage<MessageOpenFileDialogFinish>({
      type: "openFileDialogFinish",
      dialog: currentDialog,
      files
    });
    currentDialog = null;
  }
});

workerPostMessage<MessageInitialize>({
  type: "initialize",
  canvas: offscreenCanvas,
  args: new URL(location.href).searchParams.get("args") || "",
  focused,
}, [offscreenCanvas]);

canvas.addEventListener("mousemove", (event) => {
  const rect = canvas.getBoundingClientRect();
  workerPostMessage<MessageMouseMove>({
    type: "mouseMove",
    clientX: event.clientX - rect.left,
    clientY: event.clientY - rect.top,
    dx: event.movementX,
    dy: event.movementY
  });
});

canvas.addEventListener("wheel", (event) => {
  const SCROLL_SCALE = 1 / 60;
  const rect = canvas.getBoundingClientRect();
  workerPostMessage<MessageMouseScroll>({
    type: "mouseScroll",
    clientX: event.clientX - rect.left,
    clientY: event.clientY - rect.top,
    scrollX: event.deltaX * SCROLL_SCALE,
    scrollY: -event.deltaY * SCROLL_SCALE,
  });
  event.preventDefault();
}, {passive: false});

const mapMouseButton = (button: number): MouseButtons => {
  switch (button) {
    case 0: return MouseButtons.Left;
    case 1: return MouseButtons.Middle;
    case 2: return MouseButtons.Right;
    case 3: return MouseButtons.XOneBack;
    case 4: return MouseButtons.XTwoForward;
  }
  throw new Error("Unhandled mouse button");
}

const onMouseButtonChanged = (event: MouseEvent) => {
  const rect = canvas.getBoundingClientRect();
  workerPostMessage<MessageMouseButtonChanged>({
    type: "mouseButtonChanged",
    clientX: event.clientX - rect.left,
    clientY: event.clientY - rect.top,
    button: mapMouseButton(event.button),
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
  // We also prevent Ctrl+Key (except for CXV for copy,cut,paste)
  if (event.code === "Tab" || /^F[0-9]{1,2}$/.test(event.code) || (event.ctrlKey && /^Key[ABDEFGHIJKLMNOPQRSTUWYZ]$/gm.test(event.code))) {
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

const dropFiles = async (dataTransfer: DataTransfer, clientX: number, clientY: number) => {
  const files: MessagePartFile[] = [];
  for (const file of dataTransfer.files) {
    files.push({
      fileName: file.name,
      buffer: await file.arrayBuffer()
    });
  }

  const rect = canvas.getBoundingClientRect();
  workerPostMessage<MessageFilesDropped>({
    type: "filesDropped",
    clientX,
    clientY,
    files
  });
}

// These two event handlers don't work on the canvas (only on document) so we must check focus
document.addEventListener("copy", copyCutHandler);
document.addEventListener("paste", (event) => {
  if (document.activeElement === canvas) {
    let dataTransfer = event.clipboardData;
    if (emulatedClipboardText) {
      dataTransfer = new DataTransfer();
      dataTransfer.setData("text/plain", emulatedClipboardText);
      emulatedClipboardText = null;
    }

    if (dataTransfer) {
      event.preventDefault();
      
      if (dataTransfer.files.length > 0) {
        // Pretend to drop the file at 0,0
        dropFiles(dataTransfer, 0, 0);
      } else {
        workerPostMessage<MessagePaste>({
          type: "paste",
          text: emulatedClipboardText || dataTransfer.getData("text/plain")
        });
      }
    }
  }
});

// We have to prevent default on dragover otherwise it opens the file with it's usual behavior
canvas.addEventListener("dragover", (event) => {
  event.preventDefault();
});

canvas.addEventListener("drop", (event) => {
  event.preventDefault();
  if (event.dataTransfer) {
    const rect = canvas.getBoundingClientRect();
    dropFiles(event.dataTransfer, event.clientX - rect.left, event.clientY - rect.top);
  }
});

const updateFocus = () => {
  const hasFocus = checkFocus();
  if (focused !== hasFocus) {
    focused = hasFocus;

    workerPostMessage<MessageFocusChanged>({
      type: "focusChanged",
      focused
    });
  }
}

window.addEventListener("focus", updateFocus);
window.addEventListener("blur", updateFocus);
document.addEventListener("visibilitychange", updateFocus);

const resizeObserver = new ResizeObserver((entries) => {
  for (const entry of entries) {
    workerPostMessage<MessageSizeChanged>({
      type: "sizeChanged",
      clientWidth: entry.contentRect.width,
      clientHeight: entry.contentRect.height
    });
  }
});

resizeObserver.observe(parent);

