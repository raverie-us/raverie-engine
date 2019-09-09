// MIT Licensed (see LICENSE.md).

#pragma once

namespace Zero
{

// Recording Node

/// Records audio generated by its input SoundNodes into a WAV file
class RecordingNode : public SimpleCollapseNode
{
public:
  ZilchDeclareType(RecordingNode, TypeCopyMode::ReferenceType);

  RecordingNode(StringParam name, unsigned ID);
  ~RecordingNode();

  /// The name of the output file that will be created, including the full path.
  /// Do not include the file extension.
  String GetFileName();
  void SetFileName(String& fileName);
  /// Starts writing all audio input to a file.
  void StartRecording();
  /// Stops writing data and closes the file.
  void StopRecording();
  /// When true, recording is paused, and can be resumed by setting to false
  bool GetPaused();
  void SetPaused(bool paused);
  /// When false, audio data will be saved in a buffer and written to the file
  /// when StopRecording is called. When true, data will be written to the file
  /// constantly during every update frame, and nothing will be saved.
  bool GetStreamToDisk();
  void SetStreamToDisk(bool stream);

private:
  bool GetOutputSamples(BufferType* outputBuffer,
                        const unsigned numberOfChannels,
                        ListenerNode* listener,
                        const bool firstRequest) override;
  void WriteBuffer(Zero::Array<float>* buffer, unsigned numberOfChannels);

  Zero::String mFileName;
  Zero::File mFileStream;
  unsigned mSamplesRecorded;
  const float cMaxValue;
  ThreadedInt mRecording;
  ThreadedInt mPaused;
  unsigned mChannels;
  bool mStreaming;
  Zero::Array<float> mSavedSamples;

  struct WavHeader
  {
    char riff_chunk[4];
    unsigned chunk_size;
    char wave_fmt[4];
    char fmt_chunk[4];
    unsigned fmt_chunk_size;
    unsigned short audio_format;
    unsigned short number_of_channels;
    unsigned sampling_rate;
    unsigned bytes_per_second;
    unsigned short bytes_per_sample;
    unsigned short bits_per_sample;
    char data_chunk[4];
    unsigned data_chunk_size;
  };
};

// Save Audio Node

/// Saves audio from its input SoundNodes and then plays it. All audio from
/// inputs is passed to outputs.
class SaveAudioNode : public SimpleCollapseNode
{
public:
  ZilchDeclareType(SaveAudioNode, TypeCopyMode::ReferenceType);

  SaveAudioNode(StringParam name, unsigned ID);

  /// When true, audio from input SoundNodes will be saved. Setting this to true
  /// will remove any existing saved audio before saving more.
  bool GetSaveAudio();
  void SetSaveAudio(bool save);
  /// Plays the saved audio.
  void PlaySavedAudio();
  /// Stops playing the saved audio.
  void StopPlaying();
  /// Removes all currently saved audio.
  void ClearSavedAudio();

private:
  bool GetOutputSamples(BufferType* outputBuffer,
                        const unsigned numberOfChannels,
                        ListenerNode* listener,
                        const bool firstRequest) override;
  void ClearSavedAudioThreaded();

  Threaded<bool> mSaveData;
  Threaded<bool> mPlayData;
  BufferType mSavedSamplesThreaded;
  size_t mPlaybackIndexThreaded;
};

} // namespace Zero
