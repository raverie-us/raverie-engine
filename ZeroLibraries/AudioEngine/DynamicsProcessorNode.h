///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef DYNAMICSNODE_H
#define DYNAMICSNODE_H

namespace Audio
{
  //------------------------------------------------------------------------ Dynamics Processor Node

  class DynamicsProcessor;

  enum ProcessorTypes { Compressor, Limiter, Expander, Gate };

  class DynamicsProcessorNode : public SimpleCollapseNode
  {
  public:
    DynamicsProcessorNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface *extInt, const bool threaded = false);

    // Returns the current input gain in decibels
    float GetInputGain();
    // Sets the input gain value in decibels
    void SetInputGain(const float gainDB);
    // Returns the current threshold value in decibels
    float GetThreshold();
    // Sets the threshold value in decibels
    void SetThreshold(const float thresholdDB);
    // Returns the current attack time in milliseconds
    float GetAttackMSec();
    // Sets the attack time in milliseconds
    void SetAttackMSec(const float attack);
    // Returns the current release time in milliseconds
    float GetReleaseMSec();
    // Sets the release time in milliseconds
    void SetReleaseMsec(const float release);
    // Returns the current ratio of the filter
    float GetRatio();
    // Sets the ratio of the filter
    void SetRatio(const float ratio);
    // Returns the current output gain in decibels
    float GetOutputGain();
    // Sets the output gain in decibels
    void SetOutputGain(const float gainDB);
    // Returns the current knee width of the filter
    float GetKneeWidth();
    // Sets the knee width of the filter
    void SetKneeWidth(const float kneeWidth);
    // Returns the current filter type
    ProcessorTypes GetType();
    // Sets the filter type
    void SetType(const ProcessorTypes type);

  private:
    ~DynamicsProcessorNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;

    // Filter for calculations
    DynamicsProcessor *Filter;
    // Input gain value in decibels
    float InputGainDB;
    // Threshold value in decibels
    float ThresholdDB;
    // Attack time in milliseconds
    float AttackMSec;
    // Release time in milliseconds
    float ReleaseMSec;
    // Filter ratio value
    float Ratio;
    // Output gain in decibels
    float OutputGainDB;
    // Filter knee width value
    float KneeWidth;
    // Processor type
    ProcessorTypes ProcessorType;
  };

}

#endif
