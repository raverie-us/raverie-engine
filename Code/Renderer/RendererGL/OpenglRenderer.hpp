// MIT Licensed (see LICENSE.md).
#pragma once
#include "Foundation/Platform/PlatformCommunication.hpp"

namespace Raverie
{
// These functions must be defined by the platform (as well as CreateRenderer)
// These could be virtual functions on the OpenglRenderer, however there should
// only be one implementation of OpenGL per platform, so we can avoid the
// virtual overhead.
class OpenglRenderer;

typedef void(*UniformFunction)(GLint, GLsizei, const void*);

class StreamedVertexBuffer
{
public:
  void Initialize();
  void Destroy();

  void AddVertices(StreamedVertex* vertices, uint count, PrimitiveType::Enum primitiveType);
  void AddVertices(StreamedVertexArray& vertices, uint start, uint count, PrimitiveType::Enum primitiveType);
  void FlushBuffer(bool deactivate);

  uint mBufferSize;
  GLuint mVertexArray;
  GLuint mVertexBuffer;

  uint mCurrentBufferOffset;

  PrimitiveType::Enum mPrimitiveType;
  bool mActive;
};

class GlShader
{
public:
  GLuint mId;
  HashMap<String, GLint> mLocations;
};

class GlMaterialRenderData : public MaterialRenderData
{
public:
};

class GlMeshRenderData : public MeshRenderData
{
public:
  GLuint mVertexBuffer;
  GLuint mIndexBuffer;
  GLuint mVertexArray;
  GLsizei mIndexCount;
  PrimitiveType::Enum mPrimitiveType;
  Array<MeshBone> mBones;
};

class GlTextureRenderData : public TextureRenderData
{
public:
  GLuint mId;
  TextureType::Enum mType;
  TextureFormat::Enum mFormat;
  uint mWidth;
  uint mHeight;
  u32 mSamplerSettings;
};

class OpenglRenderer : public Renderer
{
public:
  OpenglRenderer();
  ~OpenglRenderer();

  // This must be called by the derived class before the OpenGL context has been
  // destroyed.
  void Shutdown();

  void BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float nearPlane, float farPlane) override;
  void BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float nearPlane, float farPlane) override;
  bool YInvertImageData(TextureType::Enum type) override;

  GlMaterialRenderData* CreateMaterialRenderData() override;
  GlMeshRenderData* CreateMeshRenderData() override;
  GlTextureRenderData* CreateTextureRenderData() override;

  void AddMaterial(AddMaterialInfo* info) override;
  void AddMesh(AddMeshInfo* info) override;
  void AddTexture(AddTextureInfo* info) override;
  void RemoveMaterial(MaterialRenderData* data) override;
  void RemoveMesh(MeshRenderData* data) override;
  void RemoveTexture(TextureRenderData* data) override;

  bool GetLazyShaderCompilation() override;
  void SetLazyShaderCompilation(bool isLazy) override;
  void AddShaders(Array<ShaderEntry>& entries, uint forceCompileBatchCount) override;
  void RemoveShaders(Array<ShaderEntry>& entries) override;
  void DeleteShaderByKey(const ShaderKey& shaderKey);

  GLint GetUniformLocation(GlShader* shader, StringParam name);

  void GetTextureData(GetTextureDataInfo* info) override;

  void DoRenderTasks(RenderTasks* renderTasks, RenderQueues* renderQueues) override;

  void DoRenderTaskRange(RenderTaskRange& taskRange);
  void DoRenderTaskClearTarget(RenderTaskClearTarget* task);
  void DoRenderTaskRenderPass(RenderTaskRenderPass* task);
  void DoRenderTaskPostProcess(RenderTaskPostProcess* task);
  void DoRenderTaskBackBufferBlit(RenderTaskBackBufferBlit* task);
  void DoRenderTaskTextureUpdate(RenderTaskTextureUpdate* task);

  void SetRenderTargets(RenderSettings& renderSettings);

  void DrawStatic(ViewNode& viewNode, FrameNode& frameNode);
  void DrawStreamed(ViewNode& viewNode, FrameNode& frameNode);

  void SetShaderParameter(ShaderInputType::Enum inputType, StringParam name, void* data);
  void SetShaderParameterMatrix(StringParam name, Mat3& transform);
  void SetShaderParameterMatrix(StringParam name, Mat4& transform);
  void SetShaderParameterMatrixInv(StringParam name, Mat3& transform);
  void SetShaderParameterMatrixInv(StringParam name, Mat4& transform);
  void SetShaderParameters(FrameBlock* frameBlock, ViewBlock* viewBlock);
  void SetShaderParameters(FrameNode* frameNode, ViewNode* viewNode);
  void SetShaderParameters(IndexRange inputRange, uint& nextTextureSlot);
  void SetShaderParameters(u64 objectId, uint shaderInputsId, uint& nextTextureSlot);

  GlShader* GetShader(ShaderKey& shaderKey);
  void CreateShader(ShaderEntry& entry);
  void CreateShader(StringParam vertexSource, StringParam geometrySource, StringParam pixelSource, GLuint& shader);
  void SetShader(GlShader* shader);

  void DelayedRenderDataDestruction();
  void DestroyRenderData(GlMaterialRenderData* renderData);
  void DestroyRenderData(GlMeshRenderData* renderData);
  void DestroyRenderData(GlTextureRenderData* renderData);

  GLuint GetSampler(u32 samplerSettings);
  void DestroyUnusedSamplers();

  UniformFunction mUniformFunctions[ShaderInputType::Count];

  HashMap<ShaderKey, GlShader*> mGlShaders;
  HashMap<ShaderKey, ShaderEntry> mShaderEntries;

  bool mLazyShaderCompilation = true;

  GLuint mActiveShaderId = 0;
  GlShader* mActiveShader = nullptr;
  GLuint mActiveTexture = 0;
  u64 mActiveMaterial = 0;
  uint mNextTextureSlot = 0;
  uint mNextTextureSlotMaterial = 0;

  float mCurrentLineWidth = 1.0f;
  bool mClipMode = false;
  Vec4 mCurrentClip = Vec4::cZero;
  BlendSettings mCurrentBlendSettings;

  RenderTasks* mRenderTasks = nullptr;
  RenderQueues* mRenderQueues = nullptr;
  FrameBlock* mFrameBlock = nullptr;
  ViewBlock* mViewBlock = nullptr;
  uint mShaderInputsId = 0;
  String mRenderPassName;

  IntVec2 mViewportSize = IntVec2::cZero;

  GLuint mTriangleArray = 0;
  GLuint mTriangleVertex = 0;
  GLuint mTriangleIndex = 0;
  GLuint mLoadingShader = 0;
  GLint mLoadingTextureLoc = 0;
  GLint mLoadingTransformLoc = 0;
  GLint mLoadingUvTransformLoc = 0;
  GLint mLoadingAlphaLoc = 0;


  GLuint mSingleTargetFbo = 0;
  GLuint mMultiTargetFbo = 0;

  StreamedVertexBuffer mStreamedVertexBuffer;

  Array<GlMaterialRenderData*> mMaterialRenderDataToDestroy;
  Array<GlMeshRenderData*> mMeshRenderDataToDestroy;
  Array<GlTextureRenderData*> mTextureRenderDataToDestroy;

  HashMap<u32, GLuint> mSamplers;
  HashSet<u32> mUnusedSamplers;
};

} // namespace Raverie
