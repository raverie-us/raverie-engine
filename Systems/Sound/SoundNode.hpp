///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

namespace Events
{

DeclareEvent(AudioInterpolationDone);
DeclareEvent(SoundNodeDisconnected);
DeclareEvent(SoundListenerRemoved);

} // namespace Events

class ListenerNode;
class SoundEvent;

//--------------------------------------------------------------------------------------- Sound Node

class SoundNode : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundNode(StringParam name, int ID, bool listenerDependent, bool generator);
  virtual ~SoundNode();

  /// Adds the passed in node to this node's inputs.
  void AddInputNode(SoundNode* node);
  /// Removes the node passed in as a parameter from this node's inputs. 
  void RemoveInputNode(SoundNode* node);
  /// Inserts the passed in node before this node in the signal path, placing it 
  /// between this node and any nodes which were connected to this node as inputs.
  void InsertNodeBefore(SoundNode* node);
  /// Inserts the passed in node after this node in the signal path, placing it 
  /// between this node and any nodes which were connected to this node's output.
  void InsertNodeAfter(SoundNode* node);
  /// Replaces this node in the graph with the node passed in as a parameter. 
  /// This node will be deleted when it is no longer referenced. 
  void ReplaceWith(SoundNode* node);
  /// Removes the connections between this node and all of its input nodes. 
  void RemoveAllInputs();
  /// Removes the connections between this node and all of its output nodes, disconnecting this node from the graph. 
  /// If this node has no inputs it will be deleted when no longer referenced.
  void RemoveAllOutputs();
  /// Removes this node from the graph by disconnecting it from all inputs and outputs 
  /// and attaching the input nodes to the output nodes, keeping the rest of the graph intact. 
  /// This node will be deleted when it is no longer referenced.
  void RemoveAndAttachInputsToOutputs();
  /// If true, this node will automatically remove itself from the graph when its last input node is removed.
  bool GetAutoCollapse();
  void SetAutoCollapse(bool willCollapse);
  /// Will be true if this node has any input nodes. 
  bool GetHasInputs();
  /// Will be true if this node has any output nodes.
  bool GetHasOutputs();
  /// The number of input nodes that are currently attached to this node.
  int GetInputCount();
  /// The number of output nodes that are currently attached to this node.
  int GetOutputCount();
  /// DEPRECATED The BypassValue property should be used instead.
  float GetBypassPercent();
  void SetBypassPercent(float percent);
  /// The percentage of output (0 to 1.0) that should skip whatever processing the node does.
  float GetBypassValue();
  void SetBypassValue(float value);
  
// Internals
  // The ID given to this sound node when it was constructed
  const unsigned cNodeID;
  // The name given to this sound node when it was constructed
  const String cName;
  // A buffer to hold the output of all input nodes
  BufferType mInputSamplesThreaded;

  typedef Array<HandleOf<SoundNode>> NodeListType;

  virtual void DisconnectThisAndAllInputs();
  void DispatchEventFromMixThread(const String eventID);
  void WarningFromMixThread(const String title, const String message);

  // Returns a pointer to the array of all sound node inputs
  const NodeListType* GetInputs(AudioThreads::Enum whichThread);
  // Returns a pointer to the array of all sound node outputs
  const NodeListType* GetOutputs(AudioThreads::Enum whichThread);
  // Return true if this sound node is currently outputting audio data
  bool HasAudibleOutput();
  // Should be implemented by nodes if they keep track of data per listeners
  virtual void RemoveListenerThreaded(SoundEvent* event) {}
  // Returns the sum of all volumes from outputs. Will return 0.0 by default. The output
  // node will return 1.0. Nodes which modify volume should implement this function
  // and multiply their volume with the return value.
  virtual float GetVolumeChangeFromOutputsThreaded();  
  // Handles getting the output from the sound node
  bool Evaluate(BufferType* outputBuffer, const unsigned numberOfChannels, ListenerNode* listener);
  // Adds the output from all input nodes to the InputSamples buffer
  bool AccumulateInputSamples(const unsigned howManySamples, const unsigned numberOfChannels,
    ListenerNode* listener);
  // Uses the BypassValue to add a portion of the InputSamples buffer to the passed-in buffer
  void AddBypassThreaded(BufferType* outputBuffer);

  void AddInputNodeThreaded(HandleOf<SoundNode> newNode);
  void RemoveInputNodeThreaded(HandleOf<SoundNode> node);

private:
  // If false, this node's output should not be saved into the MixedOutput buffer
  bool mOkayToSaveThreaded;
  // Array of nodes to use as inputs
  NodeListType mInputs[2];
  // Array of nodes using this node as input
  NodeListType mOutputs[2];
  // If true, currently in process of getting output
  bool mInProcessThreaded;
  // Version number of the mixed output
  unsigned mMixedVersionThreaded;
  // Saved output for a mix version
  BufferType mMixedOutputThreaded;
  // Number of channels in the mixed output
  unsigned mNumMixedChannelsThreaded;
  // The listener used for the mixed output
  ListenerNode* mMixedListenerThreaded;
  // If true, node will collapse when all inputs are removed
  ThreadedInt mWillCollapse;
  // If true, this node had valid audio output during the last mix
  ThreadedInt mValidOutputLastMix;
  // If true, this node has listener-specific data (such as an emitter)
  bool mListenerDependentThreaded;
  // The fraction of output that shouldn't be affected by this node (0 - 1.0)
  Threaded<float> mBypassValue;
  // If true, this is a node which generates audio
  bool mGeneratorThreaded;

  // Must be implemented to provide the output of this sound node
  virtual bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) = 0;
  // Called on the non-threaded node when the last input is removed
  virtual void CollapseNode() {}
};  

//----------------------------------------------------------------------------- Simple Collapse Node

// Class to derive from for simple collapse functionality
class SimpleCollapseNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SimpleCollapseNode(Zero::StringParam name, unsigned ID, bool listenerDependent, bool generator) :
    SoundNode(name, ID, listenerDependent, generator)
  {}
  virtual ~SimpleCollapseNode() {}

  // Will disconnect and remove this node when all inputs are removed
  void CollapseNode() override;

};

//-------------------------------------------------------------------------------------- Output Node

// Node used by audio system for final output
class OutputNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  OutputNode();

  float GetVolumeChangeFromOutputsThreaded() override;

  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;

};

//------------------------------------------------------------------------------------- Combine Node

class CombineNode : public SimpleCollapseNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CombineNode(Zero::StringParam name, unsigned ID);
  virtual ~CombineNode() {}

private:
  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;

};

//--------------------------------------------------------------------------- Combine And Pause Node

class CombineAndPauseNode : public SimpleCollapseNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CombineAndPauseNode(Zero::StringParam name, unsigned ID);
  virtual ~CombineAndPauseNode() {}

  bool GetPaused();
  void SetPaused(const bool paused);
  bool GetMuted();
  void SetMuted(bool muted);

private:
  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;
  void SetPausedThreaded(const bool paused);
  void SetMutedThreaded(const bool muted);

  ThreadedInt mPaused;
  bool mPausingThreaded;
  ThreadedInt mMuted;
  bool mMutingThreaded;
  InterpolatingObject VolumeInterpolator;
  bool mInterpolatingThreaded;
};

} // namespace Zero
