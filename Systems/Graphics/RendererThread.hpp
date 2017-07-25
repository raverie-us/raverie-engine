#pragma once

namespace Zero
{

OsInt RendererThreadMain(void* rendererThreadJobQueue);

class RendererJob
{
public:
  virtual ~RendererJob() {}
  virtual void Execute() = 0;
};

class RendererJobQueue
{
public:
  void AddJob(RendererJob* rendererJob);
  void TakeAllJobs(Array<RendererJob*>& rendererJobs);

  ThreadLock mThreadLock;
  Array<RendererJob*> mRendererJobs;
};

class RendererThreadJobQueue : public RendererJobQueue
{
public:
  void AddJob(RendererJob* rendererJob);
  void WaitForJobs();
  bool HasJobs();
  bool ShouldExitThread();

  OsEvent mRendererThreadEvent;
  bool mExitThread;
};

class WaitRendererJob : public RendererJob
{
public:
  WaitRendererJob();
  // Wait function called by main thread to wait on this job
  void WaitOnThisJob();
  OsEvent mWaitEvent;
};

class CreateRendererJob : public WaitRendererJob
{
public:
  void Execute() override;

  OsHandle mMainWindowHandle;
  String mError;
};

class DestroyRendererJob : public WaitRendererJob
{
public:
  void Execute() override;

  RendererThreadJobQueue* mRendererJobQueue;
};

class AddMaterialJob : public RendererJob
{
public:
  void Execute() override;

  MaterialRenderData* mRenderData;
  String mCompositeName;
  ResourceId mMaterialId;
};

class AddMeshJob : public RendererJob
{
public:
  void Execute() override;

  MeshRenderData* mRenderData;
  uint mVertexSize;
  uint mVertexCount;
  byte* mVertexData;
  uint mIndexSize;
  uint mIndexCount;
  byte* mIndexData;
  Array<VertexAttribute> mVertexAttributes;
  PrimitiveType::Enum mPrimitiveType;
  Array<MeshBone> mBones;
};

class AddTextureJob : public RendererJob
{
public:
  void Execute() override;

  TextureRenderData* mRenderData;
  uint mWidth;
  uint mHeight;
  uint mMipCount;
  uint mTotalDataSize;
  MipHeader* mMipHeaders;
  byte* mImageData;

  TextureType::Enum mType;
  TextureFormat::Enum mFormat;
  TextureCompression::Enum mCompression;
  TextureAddressing::Enum mAddressingX;
  TextureAddressing::Enum mAddressingY;
  TextureFiltering::Enum mFiltering;
  TextureCompareMode::Enum mCompareMode;
  TextureCompareFunc::Enum mCompareFunc;
  TextureAnisotropy::Enum mAnisotropy;
  TextureMipMapping::Enum mMipMapping;

  bool mSubImage;
  uint mXOffset;
  uint mYOffset;
};

class RemoveMaterialJob : public RendererJob
{
public:
  void Execute() override;

  MaterialRenderData* mRenderData;
};

class RemoveMeshJob : public RendererJob
{
public:
  void Execute() override;

  MeshRenderData* mRenderData;
};

class RemoveTextureJob : public RendererJob
{
public:
  void Execute() override;

  TextureRenderData* mRenderData;
};

class AddShadersJob : public RendererJob
{
public:
  void Execute() override;

  Array<ShaderEntry> mShaders;
  bool mForceCompile;
};

class RemoveShadersJob : public RendererJob
{
public:
  void Execute() override;

  Array<ShaderEntry> mShaders;
};

class SetVSyncJob : public RendererJob
{
public:
  void Execute() override;

  bool mVSync;
};

class DoRenderTasksJob : public WaitRendererJob
{
public:
  void Execute() override;

  RenderTasks* mRenderTasks;
  RenderQueues* mRenderQueues;
};

class ReturnRendererJob : public RendererJob
{
public:
  void Execute() override;
  virtual void OnExecute() = 0;
  virtual void ReturnExecute() = 0;
};

class GetTextureDataJob : public ReturnRendererJob
{
public:
  void OnExecute() override;

  TextureRenderData* mRenderData;
  TextureFormat::Enum mFormat;
  uint mWidth;
  uint mHeight;
  byte* mImage;
};

class SaveImageToFileJob : public GetTextureDataJob
{
public:
  void ReturnExecute() override;

  String mFilename;
};

// Adds itself back into the job queue until terminated
class RepeatingJob : public RendererJob
{
public:
  RepeatingJob(RendererThreadJobQueue* jobQueue);

  void Execute() override;
  virtual void OnExecute() = 0;
  virtual bool OnShouldRun() {return false;}

  void Lock();
  void Unlock();
  bool ShouldRun();
  bool IsRunning();
  void Start();
  void Terminate();
  void ForceTerminate();

  ThreadLock mThreadLock;
  RendererThreadJobQueue* mRendererJobQueue;
  uint mExecuteDelay;
  uint mStartCount;
  uint mEndCount;
  bool mShouldRun;
  bool mDelayTerminate;
  bool mForceTerminate;
};

class ShowProgressJob : public RepeatingJob
{
public:
  ShowProgressJob(RendererThreadJobQueue* jobQueue);

  void OnExecute() override;
  bool OnShouldRun() override;

  TextureRenderData* mLoadingTexture;
  TextureRenderData* mLogoTexture;
  TextureRenderData* mWhiteTexture;
  TextureRenderData* mSplashTexture;
  uint mLogoFrameSize;
  float mCurrentPercent;
  float mTargetPercent;
  uint mProgressWidth;
  TextureRenderData* mFontTexture;
  Array<StreamedVertex> mProgressText;
  bool mSplashMode;
  float mSplashFade;
  Timer mTimer;
};

} // namespace Zero
