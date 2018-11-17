///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

//----------------------------------------------------------------------------------- Recording Node

//**************************************************************************************************
ZilchDefineType(RecordingNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(FileName);
  ZilchBindMethod(StartRecording);
  ZilchBindMethod(StopRecording);
  ZilchBindGetterSetter(Paused);
  ZilchBindGetterSetter(StreamToDisk);
}

//**************************************************************************************************
RecordingNode::RecordingNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mFileName("RecordedOutput.wav"),
  mSamplesRecorded(0),
  cMaxValue((float)((1 << 15) - 1)),
  mRecording(cFalse),
  mPaused(cFalse),
  mChannels(0),
  mStreaming(cTrue)
{

}

//**************************************************************************************************
RecordingNode::~RecordingNode()
{
  StopRecording();
}

//**************************************************************************************************
String RecordingNode::GetFileName()
{
  return mFileName;
}

//**************************************************************************************************
void RecordingNode::SetFileName(String& fileName)
{
  mFileName = String::Format("%s.wav", fileName.c_str());
}

//**************************************************************************************************
void RecordingNode::StartRecording()
{
  if (mRecording.Get() == cTrue)
    return;

  mFileStream.Open(mFileName.c_str(), FileMode::Write, FileAccessPattern::Sequential);
  if (mFileStream.IsOpen())
  {
    mSamplesRecorded = 0;

    WavHeader header = { 0 };
    mFileStream.Write(reinterpret_cast<byte*>(&header), sizeof(header));

    mRecording.Set(cTrue);
  }
}

//**************************************************************************************************
void RecordingNode::StopRecording()
{
  if (mRecording.Get() == cFalse)
    return;

  mRecording.Set(cFalse);

  if (!mFileStream.IsOpen())
    return;

  WavHeader header =
  {
    { 'R', 'I', 'F', 'F' },
    36 + (mSamplesRecorded * 2),
    { 'W', 'A', 'V', 'E' },
    { 'f', 'm', 't', ' ' },
    16,                                       // fmt chunk size
    1,                                        // audio format
    (unsigned short)mChannels,                // number of channels
    cSystemSampleRate,                        // sampling rate
    cSystemSampleRate * mChannels * 16 / 8,   // bytes per second
    2 * 16 / 8,                               // bytes per sample
    16,                                       // bits per sample
    { 'd', 'a', 't', 'a' },
    mSamplesRecorded * 2
  };

  mFileStream.Seek(0);
  mFileStream.Write(reinterpret_cast<byte*>(&header), sizeof(header));

  // If samples are being saved, write them to the file
  if (!mStreaming)
  {
    forRange(float& sample, mSavedSamples.All())
    {
      // Convert from float to short
      short shortValue = (short)(sample * cMaxValue);

      mFileStream.Write(reinterpret_cast<byte*>(&shortValue), sizeof(short));
    }

    mSavedSamples.Clear();
  }

  mFileStream.Close();
}

//**************************************************************************************************
bool RecordingNode::GetPaused()
{
  return mPaused.Get() == cTrue;
}

//**************************************************************************************************
void RecordingNode::SetPaused(bool paused)
{
  if (paused)
    mPaused.Set(cTrue);
  else
    mPaused.Set(cFalse);
}

//**************************************************************************************************
bool RecordingNode::GetStreamToDisk()
{
  return mStreaming;
}

//**************************************************************************************************
void RecordingNode::SetStreamToDisk(bool stream)
{
  mStreaming = stream;
}

//**************************************************************************************************
bool RecordingNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
  ListenerNode* listener, const bool firstRequest)
{
  // Get input
  bool isThereOutput = AccumulateInputSamples(outputBuffer->Size(), numberOfChannels, listener);

  // If there is input data, move input to output buffer
  if (isThereOutput)
    mInputSamplesThreaded.Swap(*outputBuffer);

  // If we are recording, not paused, this is the first time input was requested, and we still
  // have a sibling node, create a task to write the data to the file
  if (mRecording.Get() == cTrue && mPaused.Get() == cFalse && firstRequest)
  {
    // If there was no input data, set the buffer to zero
    if (!isThereOutput)
      memset(outputBuffer->Data(), 0, sizeof(float) * outputBuffer->Size());

    Zero::Array<float>* buffer = new Zero::Array<float>(*outputBuffer);
    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&RecordingNode::WriteBuffer, this, buffer,
      numberOfChannels), this);
  }

  return isThereOutput;
}

//**************************************************************************************************
void RecordingNode::WriteBuffer(Zero::Array<float>* buffer, unsigned numberOfChannels)
{
  if (mRecording.Get() == cFalse || !mFileStream.IsOpen())
  {
    delete buffer;
    return;
  }

  mChannels = numberOfChannels;

  if (mStreaming)
  {
    forRange(float& sample, buffer->All())
    {
      short shortValue = (short)(sample * cMaxValue);

      mFileStream.Write(reinterpret_cast<byte*>(&shortValue), sizeof(short));
      ++mSamplesRecorded;
    }
  }
  else
  {
    mSavedSamples.Append(buffer->All());

    mSamplesRecorded += buffer->Size();
  }

  delete buffer;
}

//---------------------------------------------------------------------------------- Save Audio Node

//**************************************************************************************************
ZilchDefineType(SaveAudioNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(SaveAudio);
  ZilchBindMethod(PlaySavedAudio);
  ZilchBindMethod(StopPlaying);
  ZilchBindMethod(ClearSavedAudio);
}

//**************************************************************************************************
SaveAudioNode::SaveAudioNode(StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mSaveData(false),
  mPlayData(false),
  mPlaybackIndexThreaded(0)
{

}

//**************************************************************************************************
bool SaveAudioNode::GetSaveAudio()
{
  return mSaveData.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void SaveAudioNode::SetSaveAudio(bool save)
{
  if (save)
  {
    Z::gSound->Mixer.AddTask(CreateFunctor(&SaveAudioNode::ClearSavedAudioThreaded, this), this);
    mSaveData.Set(true, AudioThreads::MainThread);
  }
  else
    mSaveData.Set(false, AudioThreads::MixThread);
}

//**************************************************************************************************
void SaveAudioNode::PlaySavedAudio()
{
  mPlayData.Set(true, AudioThreads::MainThread);
}

//**************************************************************************************************
void SaveAudioNode::StopPlaying()
{
  mPlayData.Set(false, AudioThreads::MainThread);
}

//**************************************************************************************************
void SaveAudioNode::ClearSavedAudio()
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&SaveAudioNode::ClearSavedAudioThreaded, this), this);
}

//**************************************************************************************************
bool SaveAudioNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
  ListenerNode* listener, const bool firstRequest)
{
  // Get input data
  bool isInputData = AccumulateInputSamples(outputBuffer->Size(), numberOfChannels, listener);

  // If there is input data and we are saving, append the samples to the buffer
  if (mSaveData.Get(AudioThreads::MixThread) && isInputData)
    AppendToBuffer(&mSavedSamplesThreaded, mInputSamplesThreaded, 0, outputBuffer->Size());

  // If there is input data, move it to the output buffer
  if (isInputData)
    outputBuffer->Swap(mInputSamplesThreaded);

  // Check if we are playing saved data
  if (mPlayData.Get(AudioThreads::MixThread))
  {
    // The samples to copy can't be more than the samples available
    size_t samplesToCopy = Math::Min(outputBuffer->Size(), mSavedSamplesThreaded.Size()
      - mPlaybackIndexThreaded);

    // If there's no input data, copy the saved samples into the output buffer
    if (!isInputData)
    {
      memcpy(outputBuffer->Data(), mSavedSamplesThreaded.Data() + mPlaybackIndexThreaded,
        sizeof(float) * samplesToCopy);

      // If there are extra samples in the output buffer, set them all to zero
      if (samplesToCopy < outputBuffer->Size())
      {
        memset(outputBuffer->Data() + (outputBuffer->Size() - samplesToCopy), 0,
          outputBuffer->Size() - samplesToCopy);
      }
    }
    // If there is input data, add the saved samples to the existing data
    else
    {
      for (size_t i = 0; i < samplesToCopy; ++i)
        (*outputBuffer)[i] += mSavedSamplesThreaded[mPlaybackIndexThreaded + i];
    }

    // Move the playback index forward
    mPlaybackIndexThreaded += samplesToCopy;
    // Check if we've reached the end of the saved data
    if (mPlaybackIndexThreaded >= mSavedSamplesThreaded.Size())
    {
      mPlayData.Set(false, AudioThreads::MixThread);
      mPlaybackIndexThreaded = 0;
    }

    // Mark that we do have valid data to return
    isInputData = true;
  }

  return isInputData;
}

//**************************************************************************************************
void SaveAudioNode::ClearSavedAudioThreaded()
{
  mSavedSamplesThreaded.Clear();
  mPlaybackIndexThreaded = 0;
}

} // namespace Zero
