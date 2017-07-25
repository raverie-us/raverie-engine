///////////////////////////////////////////////////////////////////////////////
///
/// \file Animation.hpp
/// Declaration of the Animation resource and support classes.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2011-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Delimiter for paths to object tracks.
const char cAnimationPathDelimiter = '/';
const cstr cAnimationPathDelimiterStr = "/";

struct TrackParams;
class PropertyTrack;

const uint ObjectTrackChunk = 'trak';

class AnimationHeader
{
public:
  float mAnimationDuration;
  size_t mNumTracks;
};

class ObjectTrackHeader
{
public:
  size_t mNumPositionKeys;
  size_t mNumRotationKeys;
  size_t mNumScalingKeys;
};

struct PositionKey
{
  float Keytime;
  Vec3  Position;
};

struct RotationKey
{
  float Keytime;
  Quat  Rotation;
};

struct ScalingKey
{
  float Keytime;
  Vec3  Scale;
};

//A track for a object in the animation system.
class ObjectTrack
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor / destructor
  ObjectTrack();
  ~ObjectTrack();

  void Serialize(Serializer& stream);

  void UpdateFrame(PlayData& playData, TrackParams& params, AnimationFrame& frame);

  /// Name will be "Transform.Translation" or "Light.Color"
  PropertyTrack* GetPropertyTrack(StringParam name);

  void AddPropertyTrack(PropertyTrack* track);

  String GetLocalName(){return FullPath;}

  uint ObjectTrackId;
  String FullPath;

//private:
  uint mTrackCount;
  PropertyTrackList PropertyTracks;

  /// Intrusive link for the animation
  IntrusiveLink(ObjectTrack, link);
};
typedef InList<ObjectTrack> ObjectTrackList;

class AnimationGraph;

/// The Animation resource class is used to store animations of game objects
/// and their properties.
class Animation : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Animation();
  ~Animation();

  /// Resource Interface.
  void Save(StringParam filename) override;
  void Serialize(Serializer& stream);
  void Initialize(){}

  /// Duration of the animation in seconds 
  float GetDuration(){return mDuration;}

  ObjectTrackList ObjectTracks;
  void UpdateFrame(PlayData& playData, TrackParams& params, AnimationFrame& frame);
  ObjectTrack* GetObjectTrack(StringParam fullPath);
  float mDuration;
  uint mNumberOfTracks;
  //Clear for reload
  void Unload() override;
};

//------------------------------------------------------------ Animation Manager
class AnimationManager : public ResourceManager
{
public:
  DeclareResourceManager(AnimationManager, Animation);

  AnimationManager(BoundType* resourceType);
};

}//namespace Zero
