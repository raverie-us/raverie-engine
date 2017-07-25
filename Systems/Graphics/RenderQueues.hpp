#pragma once

namespace Zero
{

class StreamedVertex
{
public:
  StreamedVertex() {}
  StreamedVertex(Vec3 position, Vec2 uv, Vec4 color, Vec2 uvAux = Vec2::cZero)
    : mPosition(position), mUv(uv), mColor(color), mUvAux(uvAux) {}

  Vec3 mPosition;
  Vec2 mUv;
  Vec4 mColor;
  Vec2 mUvAux;
};

class FrameNode
{
public:
  void Extract(FrameBlock& frameBlock);
  // Use for Extract only
  GraphicalEntry* mGraphicalEntry;

  RenderingType::Enum mRenderingType;
  CoreVertexType::Enum mCoreVertexType;

  // Render data
  MaterialRenderData* mMaterialRenderData;
  MeshRenderData* mMeshRenderData;
  TextureRenderData* mTextureRenderData;

  // View independent transforms
  Mat4 mLocalToWorld;
  Mat3 mLocalToWorldNormal;

  Vec3 mObjectWorldPosition;

  IndexRange mBoneMatrixRange;
  IndexRange mIndexRemapRange;
  IndexRange mShaderInputRange;

  // temporary, needed by ui
  Vec4 mClip;
  // temp, needed by gizmo debug draw
  float mBorderThickness;
};

class ViewNode
{
public:
  void Extract(ViewBlock& viewBlock, FrameBlock& frameBlock);
  // Use for Extract only
  GraphicalEntry* mGraphicalEntry;

  int mFrameNodeIndex;

  // View dependent transforms
  Mat4 mLocalToView;
  Mat3 mLocalToViewNormal;
  Mat4 mLocalToPerspective;

  PrimitiveType::Enum mStreamedVertexType;
  uint mStreamedVertexStart;
  uint mStreamedVertexCount;
};

class FrameBlock
{
public:
  Array<FrameNode> mFrameNodes;
  RenderQueues* mRenderQueues;

  // Space data
  float mFrameTime;
  float mLogicTime;
};

class ViewBlock
{
public:
  Array<ViewNode> mViewNodes;
  Array<IndexRange> mRenderGroupRanges;

  // View transforms
  Mat4 mWorldToView;
  Mat4 mViewToPerspective;
  Mat4 mZeroPerspectiveToApiPerspective;

  float mNearPlane;
  float mFarPlane;
  Vec2 mViewportSize;
  Vec2 mInverseViewportSize;

  // Used by DebugGraphical
  Vec3 mEyePosition;
  Vec3 mEyeDirection;
  Vec3 mEyeUp;
  float mFieldOfView;
  float mOrthographicSize;
  bool mOrthographic;

  // For Graphicals to track Camera specific data
  CogId mCameraId;
};

class RenderQueues
{
public:
  void Clear();

  void AddStreamedLineRect(ViewNode& viewNode, Vec3 pos0, Vec3 pos1, Vec2 uv0, Vec2 uv1, Vec4 color, Vec2 uvAux0 = Vec2(0, 0), Vec2 uvAux1 = Vec2(1, 1));
  void AddStreamedQuad(ViewNode& viewNode, Vec3 pos0, Vec3 pos1, Vec2 uv0, Vec2 uv1, Vec4 color, Vec2 uvAux0 = Vec2(0, 0), Vec2 uvAux1 = Vec2(1, 1));
  void AddStreamedQuadTiled(ViewNode& viewNode, Vec3 pos0, Vec3 pos1, Vec2 uv0, Vec2 uv1, Vec4 color, Vec2 tileSize, Vec2 uvAux0 = Vec2(0, 0), Vec2 uvAux1 = Vec2(1, 1));
  void AddStreamedQuadNineSliced(ViewNode& viewNode, Vec3 pos0, Vec3 pos1, Vec2 uv0, Vec2 uv1, Vec4 color, Vec4 posSlices, Vec4 uvSlices, Vec2 uvAux0 = Vec2(0, 0), Vec2 uvAux1 = Vec2(1, 1));

  void AddStreamedQuadView(ViewNode& viewNode, Vec3 pos[4], Vec2 uv0, Vec2 uv1, Vec4 color);

  Array<FrameBlock> mFrameBlocks;
  Array<ViewBlock> mViewBlocks;
  Array<StreamedVertex> mStreamedVertices;

  uint mSkinningBufferVersion;
  Array<Mat4> mSkinningBuffer;
  Array<uint> mIndexRemapBuffer;

  /// This was for the old Ui system to add custom shader inputs. It's only set when rendering widgets.
  RenderTasks* mRenderTasks;
};

} // namespace Zero
