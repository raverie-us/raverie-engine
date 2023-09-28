import { RaverieEngine } from "./main";
import RaverieEngineWorkerUrl from "./worker.ts?worker&url";
import RaverieEngineWasmUrl from "../Build/Active/Code/Editor/RaverieEditor/RaverieEditor.wasm?url";

const url = new URL(location.href);
const args = url.searchParams.get("args") || "";
const playGame = url.searchParams.get("play") === "" || Boolean(url.searchParams.get("play"));

const uint8ArrayToBase64 = (bytes: Uint8Array): string => {
  let binary = "";
  const len = bytes.byteLength;
  for (let i = 0; i < len; ++i) {
    binary += String.fromCharCode(bytes[i]);
  }
  return window.btoa(binary);
}

const base64ToUint8Buffer = (base64: string): Uint8Array => {
  const binary = window.atob(base64);
  const bytes: number[] = [];
  const len = binary.length;
  for (let i = 0; i < len; ++i) {
    bytes.push(binary[i].charCodeAt(0));
  }
  return new Uint8Array(bytes);
}

const projectArchiveKey = "projectArchive";
const projectArchiveStr = playGame ? null : localStorage.getItem(projectArchiveKey);
const projectArchive = projectArchiveStr
  ? base64ToUint8Buffer(projectArchiveStr)
  : undefined;

const builtContentArchiveKey = "builtContentArchive";
const builtContentArchiveStr = playGame ? localStorage.getItem(builtContentArchiveKey) : null;
const builtContentArchive = builtContentArchiveStr
  ? base64ToUint8Buffer(builtContentArchiveStr)
  : undefined;

const raverieEngine = new RaverieEngine({
  parent: document.body,
  workerUrl: RaverieEngineWorkerUrl,
  wasmUrl: RaverieEngineWasmUrl,
  args,
  projectArchive,
  builtContentArchive
});

raverieEngine.canvas.focus();
raverieEngine.addEventListener("projectSave", (event) => {
  try {
    localStorage.setItem(projectArchiveKey, uint8ArrayToBase64(new Uint8Array(event.projectArchive)));
  } catch (err) {
    console.warn("Failed to save project archive:", err);
  }

  try {
    localStorage.setItem(builtContentArchiveKey, uint8ArrayToBase64(new Uint8Array(event.builtContentArchive)));
  } catch (err) {
    console.warn("Failed to save built content archive:", err);
  }
});
