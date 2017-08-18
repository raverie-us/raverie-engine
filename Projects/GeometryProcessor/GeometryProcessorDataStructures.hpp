//////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------------ Used for bones
class BoneData
{
public:
  float mBoneWeight;
  byte  mBoneIndex;
};

// bone data is keyed by the vertex index that this data effects
typedef HashMap<size_t, Array<BoneData>> BoneDataMap;

//------------------------------------------------------------------------ Used for meshs
class VertexData
{
public:
  Vec3  mPosition;
  Vec3  mNormal;
  Vec3  mTangent;
  Vec3  mBitangent;
  Vec2  mUV0;
  Vec2  mUV1;
  Vec4  mColor0;
  Vec4  mColor1;
  float mBoneWeights[cMaxBonesWeights];
  byte  mBoneIndices[cMaxBonesWeights];
};

typedef Array<VertexData> VertexArray;
typedef Array<Vec3> VertexPositionArray;
typedef Array<uint> IndexArray;

class MeshData
{
public:
  MeshData() 
  : mHasPosition(false),
    mHasNormal(false),
    mHasTangentBitangent(false),
    mHasUV0(false),
    mHasUV1(false),
    mHasColor0(false),
    mHasColor1(false),
    mHasBones(false)
  {};

  String mMeshName;
  Mat4 mMeshTransform;

  Aabb mAabb;

  FixedVertexDescription mVertexDescription;
  VertexArray mVertexBuffer;
  IndexArray  mIndexBuffer;
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

//------------------------------------------------------------------------ Used for archetypes
class HierarchyData
{
public:
  HierarchyData() : mHasMesh(false), mIsSkeletonRoot(false) {};

  String mParentNodeName;
  // I need the node name despite it also being the key
  String mNodeName;
  // hierarchy structure of the animation nodes
  String mNodePath;
  // must be combined with parent transforms to prior to being used for its updated location
  Mat4 mLocalTransform;
  // all children of this node by name
  Array<String> mChildren;
  // 
  bool mHasMesh;
  String mMeshName;
  // information defining the necessary nodes should bones be present
  String mSkeletonRootNodePath;
  bool mIsSkeletonRoot;
};

// animation data is keyed by node name
typedef HashMap<String, HierarchyData> HierarchyDataMap;

}// namespace Zero