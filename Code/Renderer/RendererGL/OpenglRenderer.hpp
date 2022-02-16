// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
// These functions must be defined by the platform (as well as CreateRenderer)
// These could be virtual functions on the OpenglRenderer, however there should
// only be one implementation of OpenGL per platform, so we can avoid the
// virtual overhead.
class OpenglRenderer;
extern void zglSetSwapInterval(OpenglRenderer* renderer, int interval);
extern IntVec2 zglGetWindowRenderableSize(OpenglRenderer* renderer);
extern void zglSwapBuffers(OpenglRenderer* renderer);

// Links regarding the portability of GLAPIENTRY:
// http://lists.openscenegraph.org/pipermail/osg-users-openscenegraph.org/2007-October/003023.html
// http://sourceforge.net/p/glew/bugs/227/
typedef void(GLAPIENTRY* UniformFunction)(GLint, GLsizei, const void*);

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
  // This must be called by the derived class after the OpenGL context has been
  // created.
  void Initialize(OsHandle windowHandle, OsHandle deviceContext, OsHandle renderContext, String& error);

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

  void SetVSync(bool vsync) override;

  void GetTextureData(GetTextureDataInfo* info) override;

  void ShowProgress(ShowProgressInfo* info) override;

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
  void SetShader(GLuint shader);

  void DelayedRenderDataDestruction();
  void DestroyRenderData(GlMaterialRenderData* renderData);
  void DestroyRenderData(GlMeshRenderData* renderData);
  void DestroyRenderData(GlTextureRenderData* renderData);

  GLuint GetSampler(u32 samplerSettings);
  void DestroyUnusedSamplers();

  UniformFunction mUniformFunctions[ShaderInputType::Count];

  HashMap<ShaderKey, GlShader> mGlShaders;
  HashMap<ShaderKey, ShaderEntry> mShaderEntries;

  bool mLazyShaderCompilation;

  GLuint mActiveShader;
  GLuint mActiveTexture;
  u64 mActiveMaterial;
  uint mNextTextureSlot;
  uint mNextTextureSlotMaterial;

  float mCurrentLineWidth;
  bool mClipMode;
  Vec4 mCurrentClip;
  BlendSettings mCurrentBlendSettings;

  OsHandle mWindow;
  OsHandle mDeviceContext;
  OsHandle mRenderContext;

  RenderTasks* mRenderTasks;
  RenderQueues* mRenderQueues;
  FrameBlock* mFrameBlock;
  ViewBlock* mViewBlock;
  uint mShaderInputsId;
  String mRenderPassName;

  IntVec2 mViewportSize;

  GLuint mTriangleArray;
  GLuint mTriangleVertex;
  GLuint mTriangleIndex;
  GLuint mLoadingShader;

  GLuint mSingleTargetFbo;
  GLuint mMultiTargetFbo;

  StreamedVertexBuffer mStreamedVertexBuffer;

  Array<GlMaterialRenderData*> mMaterialRenderDataToDestroy;
  Array<GlMeshRenderData*> mMeshRenderDataToDestroy;
  Array<GlTextureRenderData*> mTextureRenderDataToDestroy;

  HashMap<u32, GLuint> mSamplers;
  HashSet<u32> mUnusedSamplers;

  bool mVsync;
};

} // namespace Zero
