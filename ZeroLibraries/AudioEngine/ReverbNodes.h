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
    // Returns the current wet percentage
    float GetWetPercent();
    // Sets the percentage of output that is filtered
    void SetWetPercent(const float wetPercent);
    // Sets the percentage of filtered output over the specified number of seconds
    void InterpolateWetPercent(const float wetPercent, const float time);

  private:
    ~ReverbNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;

    // The current length of reverb in milliseconds
    float TimeMSec;
    // The current wet percentage
    float WetPercent;
    // Whether the reverb tail has finished
    bool OutputFinished;
    // The filter used for calculations
    typedef Zero::HashMap<ListenerNode*, Reverb*> FilterMapType;
    FilterMapType FiltersPerListener;
  };

}

#endif
