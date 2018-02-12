///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef REVERBNODES_H
#define REVERBNODES_H

namespace Audio
{
  //------------------------------------------------------------------------------------ Reverb Node

  class Reverb;

  // A simple reverb filter
  class ReverbNode : public SimpleCollapseNode
  {
  public:
    ReverbNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded = false);

    // Returns the current reverb length
    float GetTime();
    // Sets the length of the reverb tail in milliseconds
    void SetTime(const float timeInMSec);
    // Returns the current wet level (0 - 1.0f)
    float GetWetLevel();
    // Sets the fraction of output that has the reverb filter applied to it (0 - 1.0f)
    void SetWetLevel(const float wetLevel);
    // Sets the wet level value over the specified number of seconds
    void InterpolateWetLevel(const float newWetLevel, const float time);

  private:
    ~ReverbNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;

    // The current length of reverb in milliseconds
    float TimeMSec;
    // The current wet level (0 - 1.0f)
    float WetLevelValue;
    // Whether the reverb tail has finished
    bool OutputFinished;
    // The filter used for calculations
    typedef Zero::HashMap<ListenerNode*, Reverb*> FilterMapType;
    FilterMapType FiltersPerListener;
  };

}

#endif
