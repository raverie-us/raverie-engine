///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationGraph.hpp
/// Declaration of the AnimationGraph component class.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2011-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------- Animation Graph
/// The AnimationGraph component controls animation for an individual game
/// object. It stores all needed per instance (vs what is shared in the
/// animation resource) manages the current time and enumerates the animation
/// sets. The AnimationGraph can animate multiple child objects and properties
/// enabling bone animation, and other hierarchical animations.
class AnimationGraph : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor / destructor.
  AnimationGraph();
  ~AnimationGraph();

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void SetDefaults() override;

  void ResetAnimationNode();
  void ForceUpdate();

  /// Rate at that the active animations plays relative to space time.
  void SetTimeScale(float scale);
  float GetTimeScale();

  /// Is the animGraph animating?
  bool GetActive(){ return mActive; }
  void SetActive(bool value);

  void SetActiveNode(AnimationNode* node);
  AnimationNode* GetActiveNode();

  bool IsPlayingInGraph(Animation* animation);
  void PrintGraph();
  
  /// Node creation functions.
  BasicAnimation* CreateBasicNode(Animation* animation,
                                  AnimationPlayMode::Enum mode);
  DirectBlend* CreateDirectBlendNode();
  CrossBlend* CreateCrossBlendNode();
  SelectiveNode* CreateSelectiveNode();
  ChainNode* CreateChainNode();

  void SetUpPlayData(Animation* animation, PlayData& playData);

  /// The master List.
  BlendTracks mBlendTracks;

  /// Editor preview functionality.
  void PreviewGraph();
  typedef void (*DebugPreviewFunction)(AnimationGraph*);
  static DebugPreviewFunction mOnPreviewPressed;
  static DebugPreviewFunction mOnGraphCreated;
  u64 mDebugPreviewId;

private:
  friend class ObjectTrack;
  friend class Animator;

  /// Updates the root node on each from and applies it to the object tree.
  void OnUpdate(UpdateEvent* e);
  void ApplyFrame(AnimationFrame& frame);

  /// We need to re-link all objects whenever the meta database has been
  /// modified. This should only ever happen if this object is in the editor.
  void OnMetaModified(MetaTypeEvent* e);

  /// Whether or not the graph is updated.
  bool mActive;

  /// A scalar to the entire animation graph.
  float mTimeScale;

  /// Used to avoid double updates of animation nodes.
  uint mFrameId;

  /// The current root animation node.
  HandleOf<AnimationNode> mActiveNode;

  /// Still around for updater's.
  AnimationPlayMode::Enum mPlayMode;
  HandleOf<Animation> mAnimation;
};

//------------------------------------------------------------- Simple Animation
/// Plays a single animation on Initialize.
class SimpleAnimation : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SimpleAnimation(){}

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Animation getter/setter.
  Animation* GetAnimation();
  void SetAnimation(Animation* animation);

  /// Play mode getter/setter.
  AnimationPlayMode::Enum GetPlayMode();
  void SetPlayMode(AnimationPlayMode::Enum mode);

  /// Play animations directly.
  AnimationNode* PlaySingle(Animation* animation, AnimationPlayMode::Enum playMode);
  AnimationNode* DirectBlend(Animation* animation, float transitionTime, AnimationPlayMode::Enum playMode);
  AnimationNode* CrossBlend(Animation* animation, float transitionTime, AnimationPlayMode::Enum playMode);
  AnimationNode* PlayIsolatedAnimation(Animation* animation, Cog* rootBone, AnimationPlayMode::Enum playMode);
  AnimationNode* ChainAnimation(Animation* animation, AnimationPlayMode::Enum playMode);

private:
  AnimationGraph* mAnimGraph;
  AnimationPlayMode::Enum mPlayMode;
  HandleOf<Animation> mAnimation;
};

}//namespace Zero
