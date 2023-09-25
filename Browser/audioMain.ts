import {
  AudioConstants,
  MessageAudioOutputRequest,
  ToAudioMessageType,
  ToWorkerAudioMessageType
} from "./shared";
import RaverieAudioWorklet from "./audioWorklet?url";
import { MessageAudioWorkletInitialize } from "./audioWorklet";

// TODO(trevor): The worklet is a work in progress
const ENABLE_AUDIO_WORKLET = false;

const ENABLE_AUDIO = false;

export class RaverieAudio {
  public readonly workerPort: MessagePort;
  private readonly audioPort: MessagePort;

  private started = false;

  public constructor() {
    const channel = new MessageChannel();
    this.workerPort = channel.port1;
    this.audioPort = channel.port2;
  }

  // This should only be called by a user action (can be called more than once)
  public async start() {
    if (this.started || !ENABLE_AUDIO) {
      return;
    }
    this.started = true;

    const audioContext = new AudioContext({
      sampleRate: AudioConstants.SampleRate,
      latencyHint: "interactive"
    });

    if (ENABLE_AUDIO_WORKLET && "audioWorklet" in AudioContext.prototype) {
      await audioContext.audioWorklet.addModule(RaverieAudioWorklet);
      
      const raverieAudio = new AudioWorkletNode(audioContext, "raverie-audio");
      const initMessage: MessageAudioWorkletInitialize = {
        audioPort: this.audioPort
      };
      raverieAudio.port.postMessage(initMessage, [this.audioPort]);

      console.log("Created audio worklet");
      raverieAudio.connect(audioContext.destination);
    } else {
      const workerPostMessage = <T extends ToWorkerAudioMessageType>(message: T) => {
        this.audioPort.postMessage(message);
      };

      let audioSamplesPerChannel = new Float32Array();
      let audioRequestId = 0;
      let audioResponseId = 0;
      const audioRequestsAhead = 3;
      const audioMissedMultiplier = 4;
      const audioScriptNode = audioContext.createScriptProcessor(512, 0, 2);
      audioScriptNode.onaudioprocess = (event) => {
        const outputBuffer = event.outputBuffer;
    
        if (outputBuffer.numberOfChannels !== AudioConstants.Channels) {
          throw new Error(`Unexpected number of channels ${outputBuffer.numberOfChannels}`);
        }
    
        const lChannel = outputBuffer.getChannelData(0);
        const rChannel = outputBuffer.getChannelData(1);
    
        if (lChannel.length !== rChannel.length) {
          throw new Error("Expected channels to be of the same length");
        }
    
        const needSampleCount = lChannel.length;
        const haveSampleCount = Math.floor(audioSamplesPerChannel.length / 2);
        const missedSamples = needSampleCount > haveSampleCount;
    
        if (audioRequestId - audioResponseId < audioRequestsAhead) {
          workerPostMessage<MessageAudioOutputRequest>({
            type: "audioOutputRequest",
            framesRequested: missedSamples
              ? needSampleCount * audioMissedMultiplier
              : needSampleCount,
            id: audioRequestId
          });
          ++audioRequestId;
    
          if (missedSamples) {
            console.log("Missed samples (main)", needSampleCount - audioSamplesPerChannel.length, "audioRequestId", audioRequestId, "audioResponseId", audioResponseId);
          }
        }
        
        const samplesWeCanRead = Math.min(needSampleCount, haveSampleCount);
    
        for (let i = 0; i < samplesWeCanRead; ++i) {
          lChannel[i] = audioSamplesPerChannel[i * 2 + 0] || 0;
          rChannel[i] = audioSamplesPerChannel[i * 2 + 1] || 0;
        }
    
        audioSamplesPerChannel = audioSamplesPerChannel.slice(samplesWeCanRead * AudioConstants.Channels);
      };
      audioScriptNode.connect(audioContext.destination);

      this.audioPort.onmessage = (event: MessageEvent<ToAudioMessageType>) => {
        const data = event.data;
        audioResponseId = data.id;
        const newAudioFrames = new Float32Array(audioSamplesPerChannel.length + data.samplesPerChannel.length);
        newAudioFrames.set(audioSamplesPerChannel, 0);
        newAudioFrames.set(data.samplesPerChannel, audioSamplesPerChannel.length);
        audioSamplesPerChannel = newAudioFrames;
      };
    }
  }
}
