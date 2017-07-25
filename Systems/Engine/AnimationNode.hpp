///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2011-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Forward Declarations.
class AnimationGraph;
class AnimationGraphEvent;
class AnimationNode;
class Animation;
class PoseNode;

DeclareEnum3(AnimationPlayMode, PlayOnce, Loop, Pingpong);
DeclareEnum2(AnimationDirection, Forward, Backward);

/// Blend between two looping animation like walk to run
AnimationNode* BuildCrossBlend(AnimationGraph* animGraph, AnimationNode* a, 
                               AnimationNode* b, float transitionTime);

/// Blend from the current animation position to the beginning of the next.
AnimationNode* BuildDirectBlend(AnimationGraph* animGraph, AnimationNode* a, 
                                AnimationNode* b, float transitionTime);

AnimationNode* BuildSelectiveNode(AnimationGraph* t, AnimationNode* a, 
                                  AnimationNode* b, Cog* rootBone);

// Rename to Sequence
AnimationNode* BuildChainNode(AnimationGraph* t, AnimationNode* a, AnimationNode* b);

/// Base animation node.
AnimationNode* BuildBasic(AnimationGraph* animGraph, Animation* animation, 
                          float t, AnimationPlayMode::Enum playMode);

//------------------------------------------------------------------ Blend Track
struct BlendTrack
{
  uint Index;
  Property* Property;
  Handle Object;
};

typedef HashMap<String, BlendTrack*> BlendTracks;

//----------------------------------------------------- Property Track Play Data
///Data needed for each track to play
struct PropertyTrackPlayData
{
  PropertyTrackPlayData() : mBlend(NULL) {}

  BlendTrack* mBlend;
  //Current key frame of the animation track
  uint mKeyframeIndex;
  //Component this track animates.
  Object* mComponent;
};

class ObjectTrack;

//------------------------------------------------------------------ Blend Track
//Data needed for an object track
struct ObjectTrackPlayData
{
  ObjectTrackPlayData() {}
  //Object being animated.
  CogId ObjectHandle;
  //Per sub track data for this track.
  Array<PropertyTrackPlayData> mSubTrackPlayData;
  ///Track
  ObjectTrack* Track;
};

//--------------------------------------------------------- Animation Frame Data
struct AnimationFrameData
{
  AnimationFrameData() : Active(false) {}
  bool Active;
  Any Value;
};

//-------------------------------------------------------------- Animation Frame
struct AnimationFrame
{
  Array<AnimationFrameData> Tracks;
};

typedef Array<ObjectTrackPlayData> PlayData;

//--------------------------------------------------------------- Animation Node
DeclareEnum2(AnimationNodeState, Running, Finished);

//Node in animation graph
class AnimationNode : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  typedef Array<AnimationGraphEvent*>& EventList;

  /// Constructor / destructor.
  AnimationNode();
  virtual ~AnimationNode(){}

  /// Used when the meta database has changed.
  virtual void ReLinkAnimations(){}

  virtual AnimationNode* Update(AnimationGraph* animGraph, float dt,
                                uint frameId, EventList eventsToSend) = 0;
  virtual AnimationNode* Clone(){return NULL;}
  virtual bool IsPlayingInNode(StringParam animName)=0;
  virtual void PrintNode(uint tabs)=0;
  virtual bool IsActive(){return mTime <= mDuration;}

  virtual String GetDisplayName();
  
  AnimationNode* GetParent();

  /// Collapses all children to a pose node on the next Update.
  void CollapseToPose();

  /// The duration of the node.
  void SetDuration(float duration);
  float GetDuration();

  /// Time getter/setter.
  void SetTime(float time);
  float GetTime();

  /// A value between [0-1].
  void SetNormalizedTime(float normalizedTime);
  float GetNormalizedTime();

  /// If this node has already been updated, we shouldn't do anything.
  bool HasUpdatedThisFrame(uint frameId);

  /// Whether or not this node has ever been updated.
  bool HasUpdatedAtLeastOnce();

  /// Whether or not the node is currently paused.
  bool mPaused;

  /// Whether or not to collapse to a pose node when finished playing.
  bool mCollapseToPoseOnFinish;

  AnimationFrame mFrameData;

protected:
  /// The current time in the node.
  float mTime;

  /// The duration of the node.
  float mDuration;

  /// A scalar to dt when updating the node.
  float mTimeScale;

  /// Whether or not to, on next update, collapse to a pose node.
  bool mCollapseToPose;

  /// We've already been updated for this frame id.
  uint mUpdatedFrameId;

  AnimationNode* mParent;
};

//-------------------------------------------------------------------- Pose Node
class PoseNode : public AnimationNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  PoseNode(AnimationFrame& pose);

  /// AnimationNode Interface.
  AnimationNode* Update(AnimationGraph* animGraph, float dt,
                        uint frameId, EventList eventsToSend) override;
  virtual bool IsPlayingInNode(StringParam animName){return false;}
  virtual void PrintNode(uint tabs);
};

//-------------------------------------------------------------- Basic Animation
/// This node simply plays a single animation.
class BasicAnimation : public AnimationNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  BasicAnimation();
  BasicAnimation(AnimationGraph* animGraph);
  BasicAnimation(AnimationGraph* animGraph, Animation* animation, float t,
                 AnimationPlayMode::Enum playMode);  

  /// AnimationNode Interface.
  void ReLinkAnimations() override;
  AnimationNode* Update(AnimationGraph* animGraph, float dt,
                        uint frameId, EventList eventsToSend) override;
  void UpdateFrame(AnimationGraph* animGraph);
  AnimationNode* Clone() override;
  bool IsPlayingInNode(StringParam animName) override;
  void PrintNode(uint tabs) override;
  String GetDisplayName() override;

  /// The current animation playing.
  Animation* GetAnimation();
  void SetAnimation(Animation* animation);

  /// Loops until this count hit, or indefinitely if -1.
  int mLoopCount;

  /// The current play mode.
  AnimationPlayMode::Enum mPlayMode;

protected:
  /// The animGraph object in which this node is attached to.
  HandleOf<AnimationGraph> mAnimGraph;

  /// The animation being played.
  HandleOf<Animation> mAnimation;

  /// Used for the ping-pong play mode.
  float mDirection;

  PlayData mPlayData;
};

//------------------------------------------------------------------- Dual Blend
/// This node is an interface for animation nodes that deal with 
/// multiple animations. It's templated for the clone function.
template <typename DerivedType>
class DualBlend : public AnimationNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef DualBlend<DerivedType> self_type;

  /// Constructors.
  DualBlend();
  ~DualBlend();

  /// AnimationNode Interface.
  void ReLinkAnimations() override;
  virtual String GetName()=0;
  AnimationNode* Clone() override;
  AnimationNode* CollapseToA(AnimationGraph* animGraph, uint frameId, EventList eventsToSend);
  AnimationNode* CollapseToB(AnimationGraph* animGraph, uint frameId, EventList eventsToSend);
  bool IsPlayingInNode(StringParam animName) override;

  /// Left node.
  void SetFrom(AnimationNode* node);
  AnimationNode* GetFrom();

  /// Right Node
  void SetTo(AnimationNode* node);
  AnimationNode* GetTo();

  /// If two nodes are pointing at this node, we need to properly hand them
  /// both the correct node when we collapse. If null, it should return itself.
  HandleOf<AnimationNode> mLastReturned;
  HandleOf<AnimationNode> mA;
  HandleOf<AnimationNode> mB;
};

//----------------------------------------------------------------- Direct Blend
/// Blends directly between the two animations (the animations do not continue
/// to play while the blend node is in affect).
class DirectBlend : public DualBlend<DirectBlend>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  DirectBlend();

  /// AnimationNode Interface.
  String GetName() override;
  AnimationNode* Update(AnimationGraph* animGraph, float dt,
                        uint frameId, EventList eventsToSend) override;
  void PrintNode(uint tabs) override;
};

// Normalized CrossBlend vs Basic
//------------------------------------------------------------------ Cross Blend
DeclareEnum2(AnimationBlendType,
             // Does no cadence matching.
             Standard,
             // Does a basic cadence matching.
             Normalized);

DeclareEnum2(AnimationBlendMode,
             // Automatically interpolates the blend and collapsed when
             // the duration is met.
             Auto,
             // The time is not updated and is up to
             // the user to change the blend time.
             Manual);

/// Blends between the two animations while continuing to play both individual
/// animations.
class CrossBlend : public DualBlend<CrossBlend>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  CrossBlend();

  /// AnimationNode Interface.
  String GetName() override;
  AnimationNode* Update(AnimationGraph* animGraph, float dt,
                        uint frameId, EventList eventsToSend) override;
  void PrintNode(uint tabs) override;

  /// Updates the time of the 'To' animation to sync the cadence with the
  /// current position in the 'From' animation.
  /// Sync right percentage to left percentage.
  void SyncCadence();

  /// Updates the blend time and time scales of the children based on the
  /// given values. Min represents the 'From' timescale and the max represents
  /// the 'To' timescale.
  /// When current is at min, you're running at the 'From' animations timescale,
  /// when current is at max, you're running at the 'To' animations timescale.
  /// When current is outside the boundaries, timescale will be based on
  /// the closest animations timescale.
  void SetNormalizedTimeScale(float min, float max, float current);

  float mTimeScaleFrom, mTimeScaleTo;
  AnimationBlendType::Enum mType;
  AnimationBlendMode::Enum mMode;
};

//--------------------------------------------------------------- Selective Node
class SelectiveNode : public DualBlend<SelectiveNode>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  SelectiveNode();

  /// AnimationNode Interface.
  String GetName() override;
  AnimationNode* Update(AnimationGraph* animGraph, float dt,
                        uint frameId, EventList eventsToSend) override;
  AnimationNode* Clone() override;
  void PrintNode(uint tabs) override;

  void SetRoot(Cog* root);
  Cog* GetRoot();

  CogId mRoot;
  HashSet<uint> mSelectiveBones;
};

//------------------------------------------------------------------- Chain Node
class ChainNode : public DualBlend<ChainNode>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  ChainNode();

  String GetName() override;
  AnimationNode* Update(AnimationGraph* animGraph, float dt,
                        uint frameId, EventList eventsToSend) override;
  bool IsPlayingInNode(StringParam animName) override;
  void PrintNode(uint tabs) override;
};

}// namespace Zero

#include "AnimationNode.inl"
