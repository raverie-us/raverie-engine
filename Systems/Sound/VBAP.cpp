///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{
const float degreesToRadians = Math::cTwoPi / 360.0f;
const float FrontSpeakerRadians = 90.0f * degreesToRadians;
const float SideSpeakerRadians = 110.0f * degreesToRadians;
const float BackSpeakerRadians = 145.0f * degreesToRadians;

float GetNormalizeValue(float* values, unsigned count)
{
  float value = 0.0f;
  for (unsigned i = 0; i < count; ++i)
    value += values[i] * values[i];
  return Math::Sqrt(value);
}

float GetNormalizeValue(const Math::Vec2& vector)
{
  float value = vector.x * vector.x + vector.y * vector.y;
  return Math::Sqrt(value);
}

//----------------------------------------------------------------------------------- Speaker Info

//************************************************************************************************
SpeakerInfo::SpeakerInfo(float radians, const Math::Vec2& vector, SpeakerInfo* neighbor, unsigned index) :
  mAngleRadians(radians),
  mVector(vector),
  mLeftNeighbor(neighbor),
  mChannelIndex(index)
{
  if (Math::Abs(mVector.x) < 0.0001f)
    mVector.x = 0.0f;
  if (Math::Abs(mVector.y) < 0.0001f)
    mVector.y = 0.0f;
}

//************************************************************************************************
void SpeakerInfo::CreateMatrix()
{
  ErrorIf(!mLeftNeighbor, "Neighbor pointer was not set");

  mNeighborMatrix = Math::Mat2(mVector.x, mLeftNeighbor->mVector.x, mVector.y, mLeftNeighbor->mVector.y);
  mInverted = Math::Mat2::SafeInvert(mNeighborMatrix);
}

//------------------------------------------------------------------------------------------- VBAP

//************************************************************************************************
VBAP::VBAP() :
  mNumberOfChannels(1)
{

}

//************************************************************************************************
VBAP::~VBAP()
{
  forRange(SpeakerInfo* info, mSpeakers.All())
    delete info;
}

//************************************************************************************************
void VBAP::Initialize(unsigned channels)
{
  // Make sure the channels value is within range
  if (channels > 8)
    channels = 8;

  mNumberOfChannels = channels;

  // Remove any current speaker data
  forRange(SpeakerInfo* info, mSpeakers.All())
    delete info;
  mSpeakers.Clear();

  Math::Mat2 rotationMatrix;
  Math::Vec2 normalVec(1.0f, 0.0f);

  if (channels == 1)
  {
    // One channel is handled as a special case, don't need to add any speakers
    return;
  }
  else if (channels == 2)
  {
    // Front left
    rotationMatrix.Rotate(-FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      nullptr, 0));
    // Front right
    rotationMatrix.Rotate(FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[0], 1));
  }
  else if (channels == 3)
  {
    // Front left
    rotationMatrix.Rotate(-FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      nullptr, 0));
    // Center
    mSpeakers.PushBack(new SpeakerInfo(0, Math::Vec2(1.0f, 0.0f), mSpeakers[0], 2));
    // Front right
    rotationMatrix.Rotate(FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[1], 1));
  }
  else if (channels == 4)
  {
    // Front left
    rotationMatrix.Rotate(-FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      nullptr, 0));
    // Front right
    rotationMatrix.Rotate(FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[0], 1));
    // Back right
    rotationMatrix.Rotate(BackSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(BackSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[1], 3));
    // Back left
    rotationMatrix.Rotate(-BackSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-BackSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[2], 2));
  }
  else if (channels == 5)
  {
    // Side left
    rotationMatrix.Rotate(-SideSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-SideSpeakerRadians, rotationMatrix.Transform(normalVec),
      nullptr, 3));
    // Front left
    rotationMatrix.Rotate(-FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[0], 0));
    // Center
    mSpeakers.PushBack(new SpeakerInfo(0, Math::Vec2(1.0f, 0.0f), mSpeakers[1], 2));
    // Front right
    rotationMatrix.Rotate(FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[2], 1));
    // Side right
    rotationMatrix.Rotate(SideSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(SideSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[3], 4));
  }
  else if (channels == 6)
  {
    // Side left
    rotationMatrix.Rotate(-SideSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-SideSpeakerRadians, rotationMatrix.Transform(normalVec),
      nullptr, 4));
    // Front left
    rotationMatrix.Rotate(-FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[0], 0));
    // Center
    mSpeakers.PushBack(new SpeakerInfo(0, Math::Vec2(1.0f, 0.0f), mSpeakers[1], 2));
    // Front right
    rotationMatrix.Rotate(FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[2], 1));
    // Side right
    rotationMatrix.Rotate(SideSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(SideSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[3], 5));
  }
  else if (channels == 7)
  {
    // Back left
    rotationMatrix.Rotate(-BackSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-BackSpeakerRadians, rotationMatrix.Transform(normalVec),
      nullptr, 5));
    // Side left
    rotationMatrix.Rotate(-SideSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-SideSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[0], 3));
    // Front left
    rotationMatrix.Rotate(-FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[1], 0));
    // Center
    mSpeakers.PushBack(new SpeakerInfo(0, Math::Vec2(1.0f, 0.0f), mSpeakers[2], 2));
    // Front right
    rotationMatrix.Rotate(FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[3], 1));
    // Side right
    rotationMatrix.Rotate(SideSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(SideSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[4], 4));
    // Back right
    rotationMatrix.Rotate(BackSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(BackSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[5], 6));
  }
  else if (channels == 8)
  {
    // Back left
    rotationMatrix.Rotate(-BackSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-BackSpeakerRadians, rotationMatrix.Transform(normalVec),
      nullptr, 6));
    // Side left
    rotationMatrix.Rotate(-SideSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-SideSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[0], 4));
    // Front left
    rotationMatrix.Rotate(-FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(-FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[1], 0));
    // Center
    mSpeakers.PushBack(new SpeakerInfo(0, Math::Vec2(1.0f, 0.0f), mSpeakers[2], 2));
    // Front right
    rotationMatrix.Rotate(FrontSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(FrontSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[3], 1));
    // Side right
    rotationMatrix.Rotate(SideSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(SideSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[4], 5));
    // Back right
    rotationMatrix.Rotate(BackSpeakerRadians);
    mSpeakers.PushBack(new SpeakerInfo(BackSpeakerRadians, rotationMatrix.Transform(normalVec),
      mSpeakers[5], 7));
  }

  // Left neighbor of the first speaker is always the last speaker
  mSpeakers[0]->mLeftNeighbor = mSpeakers[mSpeakers.Size() - 1];

  // Create the matrix for each speaker pair
  forRange(SpeakerInfo* speaker, mSpeakers.All())
    speaker->CreateMatrix();
}

//************************************************************************************************
void VBAP::ComputeGains(Math::Vec2 sourceVec, const float sourceExtent, float* gainsOutput)
{
  memset(gainsOutput, 0, sizeof(float) * AudioConstants::cMaxChannels);

  // One speaker, no calculations needed
  if (mNumberOfChannels == 1)
  {
    gainsOutput[0] = 1.0f;
    return;
  }

  // If the source is directly above or below the listener, treat it as being in front
  if (sourceVec == Math::Vec2(0.0f, 0.0f))
    sourceVec.x = 1.0f;

  // Get the angle to the source
  float sourceRadians = Math::Vec2::AngleBetween(sourceVec, Math::Vec2(1.0f, 0.0f));
  // If it's on the left, make the angle match the speaker angles
  if (sourceVec.y > 0)
    sourceRadians = -sourceRadians;

  // Get the min and max extent angles
  float halfExtentRadians = sourceExtent * degreesToRadians * 0.5f;
  float minRadians = sourceRadians - halfExtentRadians;
  float maxRadians = sourceRadians + halfExtentRadians;

  // Get the speaker gain values
  forRange(SpeakerInfo* speaker, mSpeakers.All())
  {
    ErrorIf(!speaker->mLeftNeighbor, "Speaker neighbor was not set");

    // In this case the angles are within the normal range
    if (speaker->mAngleRadians >= 0.0f || speaker->mLeftNeighbor->mAngleRadians <= 0.0f)
      UpdateSpeaker(speaker, gainsOutput, minRadians, maxRadians, speaker->mLeftNeighbor->mAngleRadians,
        speaker->mAngleRadians);
    // Otherwise, try adjusting both angles. The function performs the check for the correct angle value.
    else
    {
      UpdateSpeaker(speaker, gainsOutput, minRadians, maxRadians,
        speaker->mLeftNeighbor->mAngleRadians, speaker->mAngleRadians + Math::cTwoPi);
      UpdateSpeaker(speaker, gainsOutput, minRadians, maxRadians,
        speaker->mLeftNeighbor->mAngleRadians - Math::cTwoPi, speaker->mAngleRadians);
    }
  }
}

//************************************************************************************************
unsigned VBAP::GetNumberOfChannels()
{
  return mNumberOfChannels;
}

//************************************************************************************************
void VBAP::UpdateSpeaker(SpeakerInfo* speaker, float* outputGains, float minRadians,
  float maxRadians, float leftAngle, float thisAngle)
{
  // Make sure the sound is audible between these speakers
  if (thisAngle < minRadians || leftAngle > maxRadians)
    return;

  // Find the angle range of the sound between the two speakers
  float fromAngle = Math::Max(leftAngle, minRadians);
  float toAngle = Math::Min(thisAngle, maxRadians);
  if (fromAngle > toAngle)
    Math::Swap(fromAngle, toAngle);

  // Direction to source for this pair is the midpoint of the angle range
  float sourceDirection = fromAngle + ((toAngle - fromAngle) * 0.5f);

  // Gain is the percentage of angle range occupied by the source
  float gain = (toAngle - fromAngle) / (thisAngle - leftAngle);

  // If point source, allow full volume
  if (minRadians == maxRadians)
    gain = 1.0f;

  Math::Vec2 speakerGains(0.0f, 0.0f);

  // Make sure the inverse exists
  if (speaker->mInverted)
  {
    // Get the scaled vector to the source
    Math::Vec2 scaledSource(gain * Math::Cos(sourceDirection), gain * Math::Sin(sourceDirection));

    // Use the scaled source vector to get the individual speaker gains
    speakerGains = Math::Mat2::Multiply(speaker->mNeighborMatrix, scaledSource);
  }
  // In this case the speakers are 180 degrees apart
  else
  {
    // Get the scaled vector to the source
    Math::Vec2 source(Math::Cos(sourceDirection), Math::Sin(sourceDirection));

    // Calculate gains separately
    speakerGains.x = Calculate180Panning(speaker, source, gain);
    speakerGains.y = Calculate180Panning(speaker->mLeftNeighbor, source, gain);
  }

  // Keep the gain values positive
  if (speakerGains.x < 0.0f && speakerGains.y < 0.0f)
  {
    speakerGains *= -1.0f;
    Math::Swap(speakerGains.x, speakerGains.y);
  }
  else if (speakerGains.x < 0.0f)
    speakerGains.x = 0.0f;
  else if (speakerGains.y < 0.0f)
    speakerGains.y = 0.0f;

  // Normalize the gain values
  float multiplier = 1.0f;
  float normalize = GetNormalizeValue(speakerGains);
  if (normalize != 0.0f)
    multiplier = gain / normalize;

  // Add the gain values to the appropriate channels
  outputGains[speaker->mChannelIndex] += speakerGains.x * multiplier;
  outputGains[speaker->mLeftNeighbor->mChannelIndex] += speakerGains.y * multiplier;
}

//************************************************************************************************
float VBAP::Calculate180Panning(SpeakerInfo* speaker, Math::Vec2& source, float gain)
{
  float dotValue = speaker->mVector.Dot(Math::Vec2(source - speaker->mVector));
  float length = (dotValue * speaker->mVector).Length();
  float speakerDistance = 2.0f; // speakers are on unit circle, 180 degrees apart
  float speakerGain = 1.0f - (length / speakerDistance);
  return speakerGain * gain;
}

} // namespace Zero
