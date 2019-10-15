// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Forward Declarations
class KeyFrame;
class TrackNode;
class RichAnimation;
class Animation;

typedef u32 TrackId;
typedef u32 KeyFrameId;
// First 32 bits are the track id, second 32 bits are the frame id
typedef u64 TrackKeyFrameId;

const TrackId cInvalidTrackId = (TrackId)-1;
const KeyFrameId cInvalidKeyFrameId = (KeyFrameId)-1;

// Events
namespace Events
{
// Sent on the individual tracks and onto the RichAnimation.
DeclareEvent(KeyFrameAdded);
DeclareEvent(KeyFrameModified);
DeclareEvent(KeyFrameDeleted);

// Sent on the rich animation whenever a track is added / removed.
DeclareEvent(TrackAdded);
DeclareEvent(TrackDeleted);

// Sent on the AnimationData when any data is modified on the animation.
DeclareEvent(AnimationModified);
} // namespace Events

// Helpers
String ComponentNameFromPath(StringParam propertyPath);
String PropertyNameFromPath(StringParam propertyPath);
String GetPropertyPath(HandleParam object, Property* property);
String GetObjectPath(Cog* object, Cog* root);

/// Sent when a key frame is added, modified, or deleted.
class KeyFrameEvent : public Event
{
public:
  ZilchDeclareType(KeyFrameEvent, TypeCopyMode::ReferenceType);
  KeyFrame* mKeyFrame;
};

/// Sent when an animation track is added or deleted.
class TrackEvent : public Event
{
public:
  ZilchDeclareType(TrackEvent, TypeCopyMode::ReferenceType);
  TrackNode* mTrack;
};

class KeyFrame : public EventObject
{
public:
  /// Constructors.
  KeyFrame();
  KeyFrame(float time, AnyParam value, KeyFrameId id, TrackNode* parent);

  /// Serializes the data to the given stream.
  void Serialize(Serializer& stream);

  /// Duplicates this key frame at the given time and returns the new key frame.
  KeyFrame* Duplicate(float newTime);

  /// Time.
  void SetTime(float time);
  float GetTime();

  /// Value.
  void SetValue(AnyParam value);
  Any GetValue();

  /// Tangents.
  void SetTangentIn(Vec2Param tangent);
  Vec2 GetTangentIn();
  void SetTangentOut(Vec2Param tangent);
  Vec2 GetTangentOut();
  void SetTangents(Vec2Param tangentIn, Vec2Param tangentOut);

  Vec2 GetGraphPosition();

  /// The track that this key frame belongs to.
  TrackNode* GetParentTrack();

  /// Destroys the key frame.
  void Destroy();

  /// Bubbles the given event to the track and the rich animation.
  void DispatchBubble(StringParam eventName, Event* e);

  /// Flags used by the curve editor.
  u32 mEditorFlags;

private:
  friend class TrackNode;

  void Modified();

  /// A unique id for this key frame
  KeyFrameId Id;

  /// The time in the animation this key frame occurs at.
  float mTime;

  /// The value at this time.
  Any mValue;

  /// Tangents to describe the curve.
  Vec2 mTangentIn, mTangentOut;

  /// The property track we belong to.
  TrackNode* mParent;
};

DeclareEnum5(TrackType, Object, Component, Property, SubProperty, Invalid);

class TrackNode : public EventObject
{
public:
  ZilchDeclareType(TrackNode, TypeCopyMode::ReferenceType);
  typedef ArrayMultiMap<float, KeyFrame*> KeyFrames;

  /// Constructors.
  TrackNode();
  TrackNode(StringParam name,
            StringParam path,
            TrackType::Enum type,
            BoundType* targetMeta,
            TrackNode* parent,
            RichAnimation* richAnim);
  ~TrackNode();

  /// Serialized the track to the given stream.
  void Serialize(Serializer& stream);
  /// Only needs to be called on things that were constructed using
  /// the default constructor.
  void Initialize(RichAnimation* richAnimation, TrackNode* parent);

  void Modified();

  /// Renames the track. Can only be used on Object Tracks.
  void Rename(StringParam newName, Status& status);

  /// Whether or not this is the root track.
  bool IsRoot();

  /// Samples the track. This is an invalid operation if it's not a property
  /// or sub property track.
  Any SampleTrack(float t);

  /// Samples the current value on the object. This function will search
  /// down from the time line object to find the correct object instance.
  Any SampleObject(Cog* animGraphObject);

  /// Checks to see if the track is valid for the given object. This will
  /// return the track that makes this invalid. For example, if this node is
  /// a component track, but its parent (the object track) is invalid, it
  /// will return the object track.
  TrackNode* IsValid(Cog* animGraphObject, Status& status);

  /// Returns whether or not this track or any parent track is disabled.
  bool IsDisabled();

  /// If it's an object track, it will return GetCog.
  /// If it's a Component track, it will return GetComponent.
  /// If it's a Property or SubProperty track, it will return GetProperty.
  ObjPtr GetObject(Cog* animGraphObject);

  /// Returns the owning Cog of this track. Valid for all track types.
  Cog* GetCog(Cog* animGraphObject);
  /// Returns the owning component of this track. Valid for (Sub)Property and
  /// Component tracks.
  Component* GetComponent(Cog* animGraphObject);
  /// Returns the property of this track. Valid for (Sub)Property tracks.
  Property* GetProperty(Cog* animGraphObject);

  /// Returns the owning Object track of this track. Valid for all track types.
  TrackNode* GetObjectTrack();
  /// Returns the owning Component track of this track. Valid for
  /// (Sub)Property and Component tracks.
  TrackNode* GetComponentTrack();
  /// Returns the Property track of this track. Valid for (Sub)Property tracks.
  TrackNode* GetPropertyTrack();

  /// Attempts to find a key frame at the given time.
  KeyFrames::valueRange FindKeyFrames(float time);

  /// Creates a key at the given time with the given value.
  KeyFrame* CreateKeyFrame(float time, AnyParam value);

  /// Updates the key frame at the given time. If there is no key frame,
  /// at the given time, a new one will be created.
  void UpdateOrCreateKeyAtTime(float time, AnyParam value);

  /// Erase all key frames from this track.
  void ClearKeyFrames();

  /// Fills out the given array with key times all key frames in this track.
  void GetKeyTimes(Array<float>& keyTimes);

  /// Adds the given child.
  void AddChild(TrackNode* child);

  /// Bubbles the given event to the track and the rich animation.
  void DispatchBubble(StringParam eventName, Event* e);

  /// Bakes the key frames to the given array.
  typedef Pair<float, Any> KeyEntry;
  void BakeKeyFrames(Array<KeyEntry>& keyFrames);

  /// Invalidates the baked curve of this and all children tracks.
  void InvalidateBakedCurve();

  /// Destroys this track, all key frames associated with it, and all children
  /// tracks.
  void Destroy();

public:
  friend class KeyFrame;

  /// Marks the key frame as modified and clears the baked curve.
  void Modified(KeyFrame* keyFrame);

  /// Bakes the curve (if not already baked).
  void BakePiecewiseFunction();

  /// Deletes the given key frame.
  void KeyFrameDeleted(KeyFrame* keyFrame);

  /// A unique Id for this track.
  TrackId Id;

  /// The type of the track.
  TrackType::Enum Type;

  /// The name of the track.
  String Name;
  String Path;

  /// Disables the track and all children tracks.
  bool mDisabled;

  /// The parent track.
  TrackNode* Parent;

  /// Meta of the track if valid. This is only set on
  /// property and component tracks.
  BoundTypeHandle mTargetMeta;

  /// Children tracks. If this track is a property track and it has children,
  /// they will be sub-property tracks. Sub property tracks are guaranteed
  /// to be in vector order (x,y,z), making it indexable.
  Array<TrackNode*> Children;

  BoundTypeHandle mPropertyType;

  /// All key frames in this track. If this is a property track of a vector
  /// type, the key frames will be stored in the Children tracks (one for
  /// each element of the vector). When sampling or baking out the key frames,
  /// it will query the Children tracks.
  KeyFrames mKeyFrames;

  /// The rich animation we belong to.
  RichAnimation* mRichAnimation;

  /// We want to store the baked curve and only rebuild the curve
  /// when the track has been modified.
  Math::PiecewiseFunction mBakedCurve;

  /// Color used to draw the curve. This is used solely by the editor.
  Vec4 mDisplayColor;
};

class RichAnimation : public EventObject
{
public:
  ZilchDeclareType(RichAnimation, TypeCopyMode::ReferenceType);

  /// Constructor.
  RichAnimation();
  ~RichAnimation();

  /// ContentComponent Interface.
  void Serialize(Serializer& stream);
  void Initialize();

  /// Bakes the RichAnimation to the Animation.
  void BakeToAnimation(Animation* animation);

  /// Finds a child object with the given path.
  TrackNode* GetObjectTrack(TrackNode* root, StringTokenRange pathRange, String currPath, bool createNew = true);
  TrackNode* GetObjectTrack(StringRange path, bool createNew = true);
  TrackNode* GetPropertyTrack(TrackNode* objectTrack, StringParam path, BoundType* targetMeta, bool createNew = true);

  /// Searches the direct children of the given track
  TrackNode* GetDirectChildTrack(TrackNode* parent, StringParam path);
  TrackNode*
  GetComponentTrack(TrackNode* objectTrack, StringParam component, BoundType* targetMeta, bool createNew = true);

  TrackNode* GetPropertyTrack(
      Cog* object, Cog* animGraphObject, BoundType* componentType, StringParam propertyName, bool createNew = true);

  /// Returns a key frame from the given id.
  KeyFrame* GetKeyFrame(TrackKeyFrameId id);

  /// Searches the tree for the maximum key time.
  void UpdateDuration();

  /// Adds the given track to be tracked.
  void RegisterTrack(TrackNode* track);

  /// Sample tolerance.
  void SetSampleTolerance(float tolerance);
  float GetSampleTolerance();

  /// Sends events for all changed key frames.
  void Flush();

  class range
  {
  public:
    range(TrackNode* root);

    /// Range Interface.
    TrackNode* Front();
    void PopFront();
    bool Empty();
    range& All()
    {
      return *this;
    }

  private:
    /// Reentrant pre-order n-dimensional tree traversal.
    TrackNode* NextTrack(TrackNode* track);

    /// Finds the next sibling of the given track. Returns null if it's
    /// the last child of its parent.
    TrackNode* NextSibling(TrackNode* track);

    TrackNode* mCurrent;
  };

  /// Returns a range that walks all the tracks (pre-order traversal).
  range allTracks();

private:
  friend class KeyFrame;
  friend class TrackNode;

  /// Recursive destructor.
  void DestroyRecursive(TrackNode* root);

  /// Removes the given track.
  void DestroyTrack(TrackNode* track);

  /// Marks the given key frame as modified. Once Flush is called, an event
  /// will get sent out saying that it was modified. This allows for multiple
  /// modifications to a single key frame with only one event being sent out.
  void Modified(KeyFrame* keyFrame);
  void SendModifiedEvent();

public:
  /// Amount of error accepted / number of units off.
  float mSampleTolerance;

  /// The root node of the animation tree.
  TrackNode* mRoot;

  /// The duration of the animation.
  float mDuration;

  /// Used to look up tracks by a unique id.
  HashMap<u64, TrackNode*> mTrackMap;

private:
  /// Used to give tracks unique Id's.
  u32 mCurrTrackId;

  /// Used so that we aren't sending out multiple modified events when
  /// making multiple modifications at once.
  bool mModified;

  /// Key frames that have been modified and need events sent out for each.
  /// This makes it so that we don't get multiple modified events for
  /// a single key frame.
  HashSet<KeyFrame*> mDirtyKeyFrames;
};

class RichAnimationBuilder : public DirectBuilderComponent
{
public:
  ZilchDeclareType(RichAnimationBuilder, TypeCopyMode::ReferenceType);

  RichAnimationBuilder();

  /// ContentComponent Interface.
  void Initialize(ContentComposition* item) override;
  void Serialize(Serializer& stream) override;
  void BuildContent(BuildOptions& buildOptions) override;

  Archetype* GetPreviewArchetype();
  void SetPreviewArchetype(Archetype* archetype);

  String mPreviewArchetype;
};

} // namespace Zero
