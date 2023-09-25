import { AudioConstants, ToAudioMessageType, ToWorkerMessageType } from "./shared";

const concat = (a: Float32Array, b: Float32Array) => {
  var c = new Float32Array(a.length + b.length);
  c.set(a, 0);
  c.set(b, a.length);
  return c;
}

export interface MessageAudioWorkletInitialize {
  audioPort: MessagePort;
}

class RaverieAudioWorklet extends AudioWorkletProcessor {
  private frames = new Float32Array();

  public constructor() {
    super();
    console.log("constructed RaverieAudioWorklet");
    this.port.addEventListener("message", (event: MessageEvent<MessageAudioWorkletInitialize>) => {
      console.log("Got audio initialization", event.data);

      event.data.audioPort.addEventListener("message", (event: MessageEvent<ToAudioMessageType>) => {
        // TODO(trevor): Not working, just for test
        console.log("Got audio data");
        this.frames = event.data.samplesPerChannel;
      });
    });
  }

  public process(inputs: Float32Array[][], outputs: Float32Array[][], parameters: Record<string, Float32Array>): boolean {
    if (outputs.length !== 1) {
      throw new Error("Expected only one output");
    }
    const output = outputs[0];

    if (output.length !== AudioConstants.Channels) {
      throw new Error(`Expected ${AudioConstants.Channels} audio channels`);
    }

    const lChannel = output[0];
    const rChannel = output[1];

    if (lChannel.length !== rChannel.length) {
      throw new Error("Expected channels to be of the same length");
    }

    const framesCount = lChannel.length;

    for (let i = 0; i < framesCount; ++i) {
      lChannel[i] = this.frames[i * 2 + 0] || 0;
      lChannel[i] = this.frames[i * 2 + 1] || 0;
    }

    console.log("Processed audio data", framesCount);
    this.frames = this.frames.subarray(framesCount);
    return true;
  }
}

registerProcessor("raverie-audio", RaverieAudioWorklet);