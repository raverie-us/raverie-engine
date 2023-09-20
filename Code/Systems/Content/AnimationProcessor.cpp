// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

AnimationProcessor::AnimationProcessor(AnimationBuilder* animationBuilder,
                                       HierarchyDataMap& hierarchyData,
                                       AnimationNodeRedirectMap& animationRedirectMap) :
    mBuilder(animationBuilder),
    mHierarchyDataMap(hierarchyData),
    mAnimationRedirectMap(animationRedirectMap)
{
}

AnimationProcessor::~AnimationProcessor()
{
}

void AnimationProcessor::ExtractAndProcessAnimationData(const aiScene* scene)
{
  GeometryImport* geoImport = mBuilder->mOwner->has(GeometryImport);
  Mat4 transform = geoImport->mTransform;
  Quat changeOfBasis = Math::ToQuaternion(geoImport->mChangeOfBasis);

  aiAnimation** animations = scene->mAnimations;
  size_t numAnimations = scene->mNumAnimations;

  for (size_t animIndex = 0; animIndex < numAnimations; ++animIndex)
  {
    aiAnimation* sceneAnimation = animations[animIndex];

    // Start our animation data for this scenes animation
    AnimationData animationData;
    animationData.AnimationName = CleanAssetName(sceneAnimation->mName.C_Str());

    float ticksPerSecond = (float)sceneAnimation->mTicksPerSecond != 0 ? (float)sceneAnimation->mTicksPerSecond : 1.f;
    animationData.AnimationDuration = (float)sceneAnimation->mDuration / ticksPerSecond;
    animationData.FramesPerSecond = ticksPerSecond;

    // Collect each of the animations channel data (Raverie Object Tracks)
    aiNodeAnim** sceneAnimationChannels = sceneAnimation->mChannels;
    size_t numChannels = sceneAnimation->mNumChannels;
    for (size_t channelIndex = 0; channelIndex < numChannels; ++channelIndex)
    {
      SceneTrack trackData;

      aiNodeAnim* sceneChannelNode = sceneAnimationChannels[channelIndex];
      String name = CleanAssetName(sceneChannelNode->mNodeName.C_Str());

      // Check if the animation has been corrected and collapsed into a
      // different node
      if (mAnimationRedirectMap.ContainsKey(name))
        name = mAnimationRedirectMap.FindValue(name, String());
      // Some animations have been removed completely if they were just taking a
      // node out of bind pose and used as a local transform on a collapsed node
      // when animating
      else if (!mHierarchyDataMap.ContainsKey(name))
        continue;

      // Entire node hierarchy path to this particular animation node
      HierarchyData& node = mHierarchyDataMap[name];

      trackData.FullPath = node.mNodePath;

      // These variables are used to decompose the transform matrix to extract
      // the scale for animation correction
      Vec3 translation;
      Mat3 rotation;
      Vec3 preScaleCorrection;
      Vec3 postScaleCorrection;

      // Animation correction matrices for collapsed pivot nodes
      Mat4 preAnimationCorrection = Mat4::cIdentity;
      Mat4 postAnimationCorrection = Mat4::cIdentity;
      if (node.mIsAnimatedPivot)
      {
        preAnimationCorrection = node.mPreAnimationCorrection;
        postAnimationCorrection = node.mPostAnimationCorrection;
      }

      // Collect all the channels position, rotation, and scale keys
      size_t numPositionKeys = sceneChannelNode->mNumPositionKeys;
      for (size_t i = 0; i < numPositionKeys; i++)
      {
        PositionKey positionKey = AssimpToZeroPositionKey(sceneChannelNode->mPositionKeys[i]);
        positionKey.Keytime /= ticksPerSecond;

        // Animation Correction if pivots were collapsed
        if (node.mIsAnimatedPivot)
        {
          Mat4 translateTransform;
          positionKey.Position =
              Math::TransformPoint(preAnimationCorrection * postAnimationCorrection, positionKey.Position);
        }

        positionKey.Position = Math::TransformPoint(transform, positionKey.Position);
        trackData.PositionKeys.PushBack(positionKey);
      }

      size_t numRotationKeys = sceneChannelNode->mNumRotationKeys;
      for (size_t i = 0; i < numRotationKeys; i++)
      {
        RotationKey rotationKey = AssimpToZeroRotationKey(sceneChannelNode->mRotationKeys[i]);
        rotationKey.Keytime /= ticksPerSecond;

        // Animation Correction if pivots were collapsed
        if (node.mIsAnimatedPivot)
        {
          rotationKey.Rotation =
              ToQuaternion(preAnimationCorrection) * rotationKey.Rotation * ToQuaternion(postAnimationCorrection);
        }

        rotationKey.Rotation = changeOfBasis * rotationKey.Rotation * changeOfBasis.Inverted();
        trackData.RotationKeys.PushBack(rotationKey);
      }

      size_t numScalingKeys = sceneChannelNode->mNumScalingKeys;
      for (size_t i = 0; i < numScalingKeys; i++)
      {
        ScalingKey scalingKey = AssimpToZeroScalingKey(sceneChannelNode->mScalingKeys[i]);
        scalingKey.Keytime /= ticksPerSecond;

        // Animation Correction if pivots were collapsed
        if (node.mIsAnimatedPivot)
        {
          Mat4::Decompose(preAnimationCorrection, translation, rotation, preScaleCorrection);
          Mat4::Decompose(postAnimationCorrection, translation, rotation, postScaleCorrection);
          scalingKey.Scale = preScaleCorrection * scalingKey.Scale * postScaleCorrection;
        }

        trackData.ScalingKeys.PushBack(scalingKey);
      }

      animationData.ObjectTracks.PushBack(trackData);
    }
    mAnimationDataArray.PushBack(animationData);
  }
}

// Utility templates for getting sub-tracks for animation clips
template <typename KeyType>
bool TransformKeyLessThan(const KeyType& keyA, const KeyType& keyB)
{
  return keyA.Keytime < keyB.Keytime;
}

template <typename KeyType>
void* GetKeyArray(SceneTrack& sceneTrack)
{
  return nullptr;
}

template <>
void* GetKeyArray<PositionKey>(SceneTrack& sceneTrack)
{
  return (void*)&sceneTrack.PositionKeys;
}

template <>
void* GetKeyArray<RotationKey>(SceneTrack& sceneTrack)
{
  return (void*)&sceneTrack.RotationKeys;
}

template <>
void* GetKeyArray<ScalingKey>(SceneTrack& sceneTrack)
{
  return (void*)&sceneTrack.ScalingKeys;
}

template <typename KeyType>
void GetClipTrack(SceneTrack& sceneTrack, SceneTrack& clipTrack, float startTime, float endTime)
{
  Array<KeyType>* sceneKeys = (Array<KeyType>*)GetKeyArray<KeyType>(sceneTrack);
  Array<KeyType>* clipKeys = (Array<KeyType>*)GetKeyArray<KeyType>(clipTrack);

  // Find begin and end range for sub-track.
  KeyType startKey = {startTime};
  KeyType endKey = {endTime};
  typename Array<KeyType>::iterator lower =
      UpperBound(sceneKeys->All(), startKey, TransformKeyLessThan<KeyType>).Begin();
  typename Array<KeyType>::iterator upper = UpperBound(sceneKeys->All(), endKey, TransformKeyLessThan<KeyType>).Begin();

  // Guarantee that at least one key is assigned.
  if (lower == upper)
  {
    if (lower != sceneKeys->Begin())
      --lower;
    else if (upper != sceneKeys->End())
      ++upper;
  }

  clipKeys->Assign(lower, upper);

  // Rebase key times to the first key.
  float offset = clipKeys->Front().Keytime;
  forRange (KeyType& key, clipKeys->All())
    key.Keytime -= offset;
}

void AnimationProcessor::ExportAnimationData(String outputPath)
{
  size_t numAnimations = mAnimationDataArray.Size();
  if (numAnimations == 0)
    return;

  Array<GeometryResourceEntry> entries;

  if (mBuilder->mClips.Size() != 0)
  {
    Array<String> usedClipNames;

    forRange (AnimationClip& clip, mBuilder->mClips.All())
    {
      if ((size_t)clip.mAnimationIndex >= mAnimationDataArray.Size())
        continue;

      AnimationData& animData = mAnimationDataArray[clip.mAnimationIndex];

      String name = BuildString(mBuilder->Name, "_", CleanAssetName(clip.mName));

      // Ignore attempts to make duplicate names.
      if (usedClipNames.Contains(name))
        continue;
      usedClipNames.PushBack(name);

      String animationFileName = BuildString(name, ".anim.bin");
      String outputFileName = FilePath::Combine(outputPath, animationFileName);
      ChunkFileWriter writer;
      writer.Open(outputFileName);

      float startTime = clip.mStartFrame / animData.FramesPerSecond;
      float endTime = clip.mEndFrame / animData.FramesPerSecond;

      endTime = Math::Clamp(endTime, 0.0f, animData.AnimationDuration);
      startTime = Math::Clamp(startTime, 0.0f, endTime);

      AnimationHeader animHeader;
      animHeader.mAnimationDuration = endTime - startTime;
      animHeader.mNumTracks = animData.ObjectTracks.Size();
      writer.Write(animHeader);

      for (size_t trackIndex = 0; trackIndex < animHeader.mNumTracks; ++trackIndex)
      {
        SceneTrack& sceneTrack = animData.ObjectTracks[trackIndex];

        SceneTrack clipTrack;
        clipTrack.FullPath = sceneTrack.FullPath;
        GetClipTrack<PositionKey>(sceneTrack, clipTrack, startTime, endTime);
        GetClipTrack<RotationKey>(sceneTrack, clipTrack, startTime, endTime);
        GetClipTrack<ScalingKey>(sceneTrack, clipTrack, startTime, endTime);

        u32 objectTrackStart = writer.StartChunk(ObjectTrackChunk);
        ObjectTrackHeader trackHeader;
        trackHeader.mNumPositionKeys = clipTrack.PositionKeys.Size();
        trackHeader.mNumRotationKeys = clipTrack.RotationKeys.Size();
        trackHeader.mNumScalingKeys = clipTrack.ScalingKeys.Size();
        writer.Write(trackHeader);
        writer.Write(clipTrack.FullPath);

        for (size_t i = 0; i < trackHeader.mNumPositionKeys; ++i)
          writer.Write(clipTrack.PositionKeys[i]);

        for (size_t i = 0; i < trackHeader.mNumRotationKeys; ++i)
          writer.Write(clipTrack.RotationKeys[i]);

        for (size_t i = 0; i < trackHeader.mNumScalingKeys; ++i)
          writer.Write(clipTrack.ScalingKeys[i]);

        writer.EndChunk(objectTrackStart);
      }
    }
  }
  else
  {
    for (size_t animIndex = 0; animIndex < numAnimations; ++animIndex)
    {
      AnimationData& animData = mAnimationDataArray[animIndex];

      String name = mBuilder->Name;
      if (numAnimations > 1)
        name = BuildString(name, "_", animData.AnimationName);

      String animationFileName = BuildString(name, ".anim.bin");
      String outputFileName = FilePath::Combine(outputPath, animationFileName);
      ChunkFileWriter writer;
      writer.Open(outputFileName);

      AnimationHeader animHeader;
      animHeader.mAnimationDuration = animData.AnimationDuration;
      animHeader.mNumTracks = animData.ObjectTracks.Size();

      writer.Write(animHeader);

      for (size_t trackIndex = 0; trackIndex < animHeader.mNumTracks; ++trackIndex)
      {
        SceneTrack& sceneTrack = animData.ObjectTracks[trackIndex];

        u32 objectTrackStart = writer.StartChunk(ObjectTrackChunk);
        ObjectTrackHeader trackHeader;
        trackHeader.mNumPositionKeys = sceneTrack.PositionKeys.Size();
        trackHeader.mNumRotationKeys = sceneTrack.RotationKeys.Size();
        trackHeader.mNumScalingKeys = sceneTrack.ScalingKeys.Size();
        writer.Write(trackHeader);
        writer.Write(sceneTrack.FullPath);

        for (size_t i = 0; i < trackHeader.mNumPositionKeys; ++i)
          writer.Write(sceneTrack.PositionKeys[i]);

        for (size_t i = 0; i < trackHeader.mNumRotationKeys; ++i)
          writer.Write(sceneTrack.RotationKeys[i]);

        for (size_t i = 0; i < trackHeader.mNumScalingKeys; ++i)
          writer.Write(sceneTrack.ScalingKeys[i]);

        writer.EndChunk(objectTrackStart);
      }
    }
  }

  mBuilder->mAnimations = entries;
}

} // namespace Raverie
