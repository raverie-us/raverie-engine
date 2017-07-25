#pragma once

namespace Zero
{

// Links regarding the portability of GLAPIENTRY:
// http://lists.openscenegraph.org/pipermail/osg-users-openscenegraph.org/2007-October/003023.html
// http://sourceforge.net/p/glew/bugs/227/
typedef void (GLAPIENTRY *UniformFunction)(GLint, GLsizei, const void*);

class GlShader
{
public:
  GLuint mId;
};

class GlMaterialRenderData : public MaterialRenderData
{
public:
  String mCompositeName;
  ResourceId mResourceId;
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

typedef Pair<String, String> StringPair;
typedef Pair<String, StringPair> ShaderKey;

class OpenglRenderer : public Renderer
{
public:
  OpenglRenderer();
  ~OpenglRenderer();

  void BuildOrthographicTransform(Mat4Ref matrix, float size, float aspect, float near, float far) override;
  void BuildPerspectiveTransform(Mat4Ref matrix, float fov, float aspect, float near, float far) override;
  bool YInvertImageData(TextureType::Enum type) override;

  void CreateRenderData(Material* material) override;
  void CreateRenderData(Mesh* mesh) override;
  void CreateRenderData(Texture* texture) override;

  void AddMaterial(AddMaterialJob* job) override;
  void AddMesh(AddMeshJob* job) override;
  void AddTexture(AddTextureJob* job) override;
  void RemoveMaterial(RemoveMaterialJob* job) override;
  void RemoveMesh(RemoveMeshJob* job) override;
  void RemoveTexture(RemoveTextureJob* job) override;

  void AddShaders(AddShadersJob* job) override;
  void RemoveShaders(RemoveShadersJob* job) override;

  void SetVSync(SetVSyncJob* job) override;
  
  void GetTextureData(GetTextureDataJob* job) override;

  void ShowProgress(ShowProgressJob* job) override;

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

  String mCoreVertexTypeNames[CoreVertexType::Count];

  HashMap<ShaderKey, GlShader> mGlShaders;
  HashMap<ShaderKey, ShaderEntry> mShaderEntries;

  GLuint mActiveShader;
  GLuint mActiveTexture;
  ResourceId mActiveMaterial;
  uint mNextTextureSlot;

  float mCurrentLineWidth;
  bool mClipMode;
  Vec4 mCurrentClip;

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
};

} // namespace Zero
