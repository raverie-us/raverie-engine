///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //------------------------------------------------------------------------ Dynamics Processor Node

  //************************************************************************************************
  DynamicsProcessorNode::DynamicsProcessorNode(Zero::Status& status, Zero::StringParam name, 
      const unsigned ID, ExternalNodeInterface *extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    Filter(nullptr),
    InputGainDB(0),
    ThresholdDB(0),
    AttackMSec(20),
    ReleaseMSec(1000),
    Ratio(1),
    OutputGainDB(0),
    KneeWidth(0),
    ProcessorType(Compressor)
  {
    if (!Threaded)
    {
      SetSiblingNodes(new DynamicsProcessorNode(status, name, ID, extInt, true), status);
    }
    else
    {
      Filter = new DynamicsProcessor();
    }
  }

  //************************************************************************************************
  DynamicsProcessorNode::~DynamicsProcessorNode()
  {
    if (Filter)
      delete Filter;
  }

  //************************************************************************************************
  float DynamicsProcessorNode::GetInputGain()
  {
    return InputGainDB;
  }

  //************************************************************************************************
  void DynamicsProcessorNode::SetInputGain(const float gainDB)
  {
    InputGainDB = gainDB;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&DynamicsProcessorNode::SetInputGain,
        (DynamicsProcessorNode*)GetSiblingNode(), gainDB));
    }
    else if (Filter)
      Filter->SetInputGain(gainDB);
  }

  //************************************************************************************************
  float DynamicsProcessorNode::GetThreshold()
  {
    return ThresholdDB;
  }

  //************************************************************************************************
  void DynamicsProcessorNode::SetThreshold(const float thresholdDB)
  {
    ThresholdDB = thresholdDB;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&DynamicsProcessorNode::SetThreshold,
      (DynamicsProcessorNode*)GetSiblingNode(), thresholdDB));
    else if (Filter)
      Filter->SetThreshold(thresholdDB);
  }

  //************************************************************************************************
  float DynamicsProcessorNode::GetAttackMSec()
  {
    return AttackMSec;
  }

  //************************************************************************************************
  void DynamicsProcessorNode::SetAttackMSec(const float attack)
  {
    AttackMSec = attack;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&DynamicsProcessorNode::SetAttackMSec,
      (DynamicsProcessorNode*)GetSiblingNode(), attack));
    else if (Filter)
      Filter->SetAttackMSec(attack);
  }

  //************************************************************************************************
  float DynamicsProcessorNode::GetReleaseMSec()
  {
    return ReleaseMSec;
  }

  //************************************************************************************************
  void DynamicsProcessorNode::SetReleaseMsec(const float release)
  {
    ReleaseMSec = release;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&DynamicsProcessorNode::SetReleaseMsec,
      (DynamicsProcessorNode*)GetSiblingNode(), release));
    else if (Filter)
      Filter->SetReleaseMsec(release);
  }

  //************************************************************************************************
  float DynamicsProcessorNode::GetRatio()
  {
    return Ratio;
  }

  //************************************************************************************************
  void DynamicsProcessorNode::SetRatio(const float ratio)
  {
    Ratio = ratio;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&DynamicsProcessorNode::SetRatio,
      (DynamicsProcessorNode*)GetSiblingNode(), ratio));
    else if (Filter)
      Filter->SetRatio(ratio);
  }

  //************************************************************************************************
  float DynamicsProcessorNode::GetOutputGain()
  {
    return OutputGainDB;
  }

  //****************************************************************************
  void DynamicsProcessorNode::SetOutputGain(const float gainDB)
  {
    OutputGainDB = gainDB;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&DynamicsProcessorNode::SetOutputGain,
      (DynamicsProcessorNode*)GetSiblingNode(), gainDB));
    else if (Filter)
      Filter->SetOutputGain(gainDB);
  }

  //************************************************************************************************
  float DynamicsProcessorNode::GetKneeWidth()
  {
    return KneeWidth;
  }

  //************************************************************************************************
  void DynamicsProcessorNode::SetKneeWidth(const float knee)
  {
    KneeWidth = knee;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&DynamicsProcessorNode::SetKneeWidth,
      (DynamicsProcessorNode*)GetSiblingNode(), knee));
    else if (Filter)
      Filter->SetKneeWidth(knee);
  }

  //************************************************************************************************
  Audio::ProcessorTypes DynamicsProcessorNode::GetType()
  {
    return ProcessorType;
  }

  //************************************************************************************************
  void DynamicsProcessorNode::SetType(const ProcessorTypes type)
  {
    ProcessorType = type;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&DynamicsProcessorNode::SetType,
      (DynamicsProcessorNode*)GetSiblingNode(), type));
    else if (Filter)
      Filter->SetType((DynamicsProcessor::ProcessorTypes)type);
  }

  //************************************************************************************************
  bool DynamicsProcessorNode::GetOutputSamples(BufferType *outputBuffer, const unsigned numberOfChannels,
    ListenerNode *listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    // Apply filter
    Filter->ProcessBuffer(InputSamples.Data(), InputSamples.Data(), outputBuffer->Data(),
      numberOfChannels, bufferSize);

    AddBypass(outputBuffer);

    return true;
  }

}