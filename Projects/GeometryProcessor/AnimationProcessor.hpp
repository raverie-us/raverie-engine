//////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct SceneTrack
{
  String FullPath;
  Array<PositionKey> PositionKeys;
  Array<RotationKey> RotationKeys;
  Array<ScalingKey>  ScalingKeys;
};

struct AnimationData
{
  String AnimationName;
  float AnimationDuration;
  float FramesPerSecond;
  Array<SceneTrack> ObjectTracks;
};

// animation data is keyed by its name
typedef Array<AnimationData> AnimationDataArray;

class AnimationProcessor
{
public:
  AnimationProcessor(AnimationBuilder* animationBuilder, HierarchyDataMap& hierarchyData);
  ~AnimationProcessor();

  void ExtractAndProcessAnimationData(const aiScene* scene);
  void ExportAnimationData(String outputPath);

  PositionKey AssimpToZeroPositionKey(aiVectorKey positionKey);
  RotationKey AssimpToZeroRotationKey(aiQuatKey  rotationKey);
  ScalingKey  AssimpToZeroScalingKey(aiVectorKey scalingKey);

  AnimationBuilder* mBuilder;
  HierarchyDataMap& mHierarchyDataMap;
  AnimationDataArray mAnimationDataArray;
};

}// namespace Zero
