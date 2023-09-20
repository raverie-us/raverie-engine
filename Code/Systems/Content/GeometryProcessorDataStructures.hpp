// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// for bones
class BoneData
{
public:
  float mBoneWeight;
  byte mBoneIndex;
};

// bone data is keyed by the vertex index that this data effects
typedef HashMap<size_t, Array<BoneData>> BoneDataMap;

// for meshs
const size_t cMaxBonesWeights = 4;

class VertexData
{
public:
  Vec3 mPosition;
  Vec3 mNormal;
  Vec3 mTangent;
  Vec3 mBitangent;
  Vec2 mUV0;
  Vec2 mUV1;
  Vec4 mColor0;
  Vec4 mColor1;
  float mBoneWeights[cMaxBonesWeights];
  byte mBoneIndices[cMaxBonesWeights];
};

typedef Array<VertexData> VertexArray;
typedef Array<Vec3> VertexPositionArray;
typedef Array<uint> IndexArray;

class MeshData
{
public:
  MeshData() : mHasPosition(false), mHasNormal(false), mHasTangentBitangent(false), mHasUV0(false), mHasUV1(false), mHasColor0(false), mHasColor1(false), mHasBones(false){};

  String mMeshName;
  String mPhysicsMeshName;
  Mat4 mMeshTransform;

  Aabb mAabb;

  FixedVertexDescription mVertexDescription;
  VertexArray mVertexBuffer;
  IndexArray mIndexBuffer;
  Array<MeshBone> mBones;

  bool mHasPosition;
  bool mHasNormal;
  bool mHasTangentBitangent;
  bool mHasUV0;
  bool mHasUV1;
  bool mHasColor0;
  bool mHasColor1;
  bool mHasBones;
};

typedef HashMap<uint, MeshData> MeshDataMap;

// for archetypes
class HierarchyData
{
public:
  HierarchyData() :
      mLocalTransform(Mat4::cIdentity),
      mHasMesh(false),
      mIsSkeletonRoot(false),
      mIsPivot(false),
      mIsAnimatedPivot(false),
      mPreAnimationCorrection(Mat4::cIdentity),
      mPostAnimationCorrection(Mat4::cIdentity),
      mAnimationNode(nullptr){};

  String mParentNodeName;
  // The node name is needed for data processing despite it also being the key
  String mNodeName;
  // Hierarchy structure of the animation nodes
  String mNodePath;
  // Must be combined with parent transforms to prior to being used for its
  // updated location
  Mat4 mLocalTransform;
  // All children of this node by name
  Array<String> mChildren;

  bool mHasMesh;
  String mMeshName;
  String mPhysicsMeshName;
  // Information defining the necessary nodes should bones be present
  String mSkeletonRootNodePath;
  bool mIsSkeletonRoot;

  // Flag to mark if a node is a pivot for consideration when collapsing pivots
  // is enabled
  bool mIsPivot;
  // Flag to mark whether a node is an animated pivot and may needs its
  // animations corrected
  bool mIsAnimatedPivot;
  // Combined transforms of removed nodes for correcting animations
  Mat4 mPreAnimationCorrection;
  Mat4 mPostAnimationCorrection;
  // This nodes animation node data, only used when collapsing pivots
  aiNodeAnim* mAnimationNode;
};

// animation data is keyed by node name
typedef HashMap<String, HierarchyData> HierarchyDataMap;

} // namespace Raverie
