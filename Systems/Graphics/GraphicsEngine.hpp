#pragma once

namespace Zero
{

DeclareEnum3(UniqueCompositeOp, Add, Remove, Modify);

namespace Events
{
  DeclareEvent(ShaderInputsModified);
}

class ShaderInputsEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  BoundType* mType;
};

class ShaderMetaProperty
{
public:
  String mMetaPropertyName;
  String mFragmentName;
  String mInputName;
};

/// Interface for static utilities of the Graphics engine.
class GraphicsStatics
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Information about the active graphics hardware.
  static GraphicsDriverSupport* GetDriverSupport();
};

class TextureToFile
{
public:
  TextureToFile() {}
  TextureToFile(HandleOf<Texture> texture, StringParam filename) : mTexture(texture), mFilename(filename) {}
  HandleOf<Texture> mTexture;
  String mFilename;
};

/// System object for graphics.
class GraphicsEngine : public System
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GraphicsEngine();
  ~GraphicsEngine();

  cstr GetName() override;
  void Initialize(SystemInitializer& initializer) override;
  void Update() override;

  void OnEngineShutdown(Event* event);

  void AddSpace(GraphicsSpace* space);
  void RemoveSpace(GraphicsSpace* space);

  // Show loading progress
  void StartProgress(Event* event);
  void UpdateProgress(ProgressEvent* event);
  void EndProgress(Event* event);
  void OnProjectLoaded(ObjectEvent* event);
  void OnNoProjectLoaded(Event* event);
  void SetSplashscreenLoading();
  void EndProgressDelayTerminate();

  void OnProjectCogModified(Event* event);
  void SetVerticalSync(bool verticalSync);

  uint GetRenderGroupCount();
  void UpdateRenderGroups();

  void CheckTextureYInvert(Texture* texture);

  // Methods for making RendererJobs
  void AddRendererJob(RendererJob* rendererJob);
  void CreateRenderer(OsHandle mainWindowHandle);
  void DestroyRenderer();
  void AddMaterial(Material* material);
  void AddMesh(Mesh* mesh);
  void AddTexture(Texture* texture, bool subImage = false, uint xOffset = 0, uint yOffset = 0);
  void RemoveMaterial(Material* material);
  void RemoveMesh(Mesh* mesh);
  void RemoveTexture(Texture* texture);

  void OnRenderGroupAdded(ResourceEvent* event);
  void OnRenderGroupRemoved(ResourceEvent* event);

  void OnMaterialAdded(ResourceEvent* event);
  void OnMaterialModified(ResourceEvent* event);
  void OnMaterialRemoved(ResourceEvent* event);

  void OnZilchFragmentAdded(ResourceEvent* event);
  void OnZilchFragmentModified(ResourceEvent* event);
  void OnZilchFragmentRemoved(ResourceEvent* event);

  void OnMeshAdded(ResourceEvent* event);
  void OnMeshModified(ResourceEvent* event);
  void OnMeshRemoved(ResourceEvent* event);

  void OnTextureAdded(ResourceEvent* event);
  void OnTextureModified(ResourceEvent* event);
  void OnTextureRemoved(ResourceEvent* event);

  void OnResourcesAdded(ResourceEvent* event);
  void OnResourcesRemoved(ResourceEvent* event);

  void AddComposite(Material* material);
  void RemoveComposite(StringParam compositeName);

  Shader* GetOrCreateShader(StringParam coreVertex, StringParam composite, StringParam renderPass, ShaderMap& shaderMap);
  void FindShadersToCompile(Array<String>& coreVertexRange, Array<String>& compositeRange, Array<String>& renderPassRange, ShaderSetMap& testMap, uint index, ShaderSet& shaders);
  void FindShadersToRemove(Array<String>& elementRange, ShaderSetMap& testMap, ShaderSet& shaders);

  void AddToShaderMaps(ShaderSet& shaders);
  void RemoveFromShaderMaps(ShaderSet& shaders);
  void RemoveFromShaderMap(ShaderSetMap& shaderMap, StringParam elementName, Shader* shader);

  void ForceCompileAllShaders();

  void ProcessModifiedScripts(LibraryRef library);

  ZilchFragmentType::Enum GetFragmentType(MaterialBlock* materialBlock);

  HandleOf<RenderTarget> GetRenderTarget(uint width, uint height, TextureFormat::Enum format, SamplerSettings samplerSettings = SamplerSettings());
  HandleOf<RenderTarget> GetRenderTarget(HandleOf<Texture> texture);
  void ClearRenderTargets();

  void WriteTextureToFile(HandleOf<Texture> texture, StringParam filename);

  void ModifiedFragment(ZilchFragmentType::Enum type, StringParam name);
  void RemovedFragment(ZilchFragmentType::Enum type, StringParam name);

  // ResourceLibraries don't know how to compile fragment libraries, so we do it here.
  void OnCompileZilchFragments(ZilchCompileFragmentEvent* event);
  void OnScriptsCompiledPrePatch(ZilchCompileEvent* event);
  void OnScriptsCompiledCommit(ZilchCompileEvent* event);
  void OnScriptsCompiledPostPatch(ZilchCompileEvent* event);
  void OnScriptCompilationFailed(Event* event);
  
  void UpdateUniqueComposites(Material* material, UniqueCompositeOp::Enum uniqueCompositeOp);

  void CompileShaders();

  typedef InList<GraphicsSpace, &GraphicsSpace::EngineLink> GraphicsSpaceList;
  GraphicsSpaceList mSpaces;

  // Render data
  RenderQueues mRenderQueues[2];
  RenderTasks mRenderTasks[2];
  // GraphicsEngine writes to back, Renderer reads from front
  RenderQueues* mRenderQueuesBack;
  RenderTasks* mRenderTasksBack;
  RenderQueues* mRenderQueuesFront;
  RenderTasks* mRenderTasksFront;

  bool mNewLibrariesCommitted;

  uint mFrameCounter;

  uint mRenderGroupCount;
  bool mUpdateRenderGroupCount;

  HandleOf<Cog> mProjectCog;
  bool mVerticalSync;

  bool mEngineShutdown;

  RenderTargetManager mRenderTargetManager;

  ZilchShaderGenerator* mShaderGenerator;

  Array<Resource*> mRenderGroups;

  Array<Material*> mAddedMaterials;
  Array<Material*> mAddedMaterialsForComposites;
  Array<RenderGroup*> mAddedRenderGroups;

  // Renderer thread
  Thread mRendererThread;
  RendererThreadJobQueue* mRendererJobQueue;
  DoRenderTasksJob* mDoRenderTasksJob;
  RendererJobQueue* mReturnJobQueue;
  ShowProgressJob* mShowProgressJob;

  // Shader building
  ShaderMap mCompositeShaders;
  ShaderMap mPostProcessShaders;

  ShaderSetMap mShaderCoreVertexMap;
  ShaderSetMap mShaderCompositeMap;
  ShaderSetMap mShaderRenderPassMap;

  HashMap<String, UniqueComposite> mUniqueComposites;

  HashSet<String> mModifiedComposites;
  HashSet<String> mRemovedComposites;

  Array<String> mModifiedFragmentFiles;
  Array<String> mRemovedFragmentFiles;

  Array<String> mModifiedCoreVertex;
  Array<String> mModifiedRenderPass;
  Array<String> mModifiedPostProcess;

  Array<String> mRemovedCoreVertex;
  Array<String> mRemovedRenderPass;
  Array<String> mRemovedPostProcess;

  // Map of meta properties with the ShaderInput attribute
  HashMap< String, Array<ShaderMetaProperty> > mComponentShaderProperties;

  Array<TextureToFile> mDelayedTextureToFile;
};

class SaveToPngJob : public Job
{
public:
  int Execute() override;
  byte* mImage;
  uint mWidth;
  uint mHeight;
  uint mBitDepth;
  String mFilename;
};

class SaveToHdrJob : public Job
{
public:
  int Execute() override;
  byte* mImage;
  uint mWidth;
  uint mHeight;
  String mFilename;
};

} // namespace Zero
