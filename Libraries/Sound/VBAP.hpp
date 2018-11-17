///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
//------------------------------------------------------------------------------------- Speaker Info

class SpeakerInfo
{
public:
  SpeakerInfo(float radians, const Math::Vec2& vector, SpeakerInfo* neighbor, unsigned index);

  // Creates the matrix between this speaker and its left neighbor
  void CreateMatrix();

  // The angle of this speaker
  float mAngleRadians;
  // The vector to this speaker
  Math::Vec2 mVector;
  // Pointer to the speaker's left neighbor
  SpeakerInfo* mLeftNeighbor;
  // The index of the channel used by this speaker
  unsigned mChannelIndex;
  // The matrix for this speaker and its neighbor
  Math::Mat2 mNeighborMatrix;
  // Will be true if the matrix was successfully inverted
  bool mInverted;
};

//--------------------------------------------------------------------------------------------- VBAP

class VBAP
{
public:
  VBAP();
  ~VBAP();

  // Initializes for the specified number of channels
  void Initialize(unsigned channels);
  // Fills out the gain values per channel for the specified source direction
  // The outputGains buffer is assumed to be MaxChannels long
  void ComputeGains(Math::Vec2 sourceVec, const float sourceExtent, float* outputGains);
  // Returns the number of channels that were initialized
  unsigned GetNumberOfChannels();

private:
  // Calculates gain for a specified speaker
  void UpdateSpeaker(SpeakerInfo* speaker, float* outputGains, float minRadians, float maxRadians,
    float leftAngle, float thisAngle);
  // Calculates the panning for speakers which are 180 degrees apart
  float Calculate180Panning(SpeakerInfo* speaker, Math::Vec2& source, float gain);

  unsigned mNumberOfChannels;
  Array<SpeakerInfo*> mSpeakers;
};

} // namespace Zero
