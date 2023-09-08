// MIT Licensed (see LICENSE.md).

#pragma once

namespace Zero
{

OsInt RendererThreadMain(void* rendererThreadJobQueue);
void ExecuteRendererJob(RendererJob* job);

class RendererJob
{
public:
  virtual ~RendererJob()
  {
  }
  virtual void Execute() = 0;
  virtual void ReturnExecute()
  {
  }
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

// Adds itself back into the job queue until terminated
class RepeatingJob : public RendererJob
{
public:
  RepeatingJob(RendererThreadJobQueue* jobQueue);

  void Execute() override;
  virtual void OnExecute() = 0;
  virtual bool OnShouldRun()
  {
    return false;
  }

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

class AddMaterialJob : public RendererJob, public AddMaterialInfo
{
public:
  void Execute() override;
};

class AddMeshJob : public RendererJob, public AddMeshInfo
{
public:
  void Execute() override;
};

class AddTextureJob : public RendererJob, public AddTextureInfo
{
public:
  void Execute() override;
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

class SetLazyShaderCompilationJob : public RendererJob
{
public:
  void Execute() override;

  bool mLazyShaderCompilation;
};

class AddShadersJob : public RepeatingJob
{
public:
  AddShadersJob(RendererThreadJobQueue* jobQueue);

  void OnExecute() override;
  bool OnShouldRun() override;
  void ReturnExecute() override;

  // All processed entries must be removed so that job knows when to terminate.
  Array<ShaderEntry> mShaders;
  // If non 0, specifies how many shaders to compile per execute.
  uint mForceCompileBatchCount;
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
};

class GetTextureDataJob : public ReturnRendererJob, public GetTextureDataInfo
{
public:
  void OnExecute() override;
};

class SaveImageToFileJob : public GetTextureDataJob
{
public:
  void ReturnExecute() override;

  String mFilename;
};

class ShowProgressJob : public RepeatingJob, public ShowProgressInfo
{
public:
  ShowProgressJob(RendererThreadJobQueue* jobQueue);

  void OnExecute() override;
  bool OnShouldRun() override;

  // Capture the state of whatever our progress
  // values are and give it to the renderer.
  void ShowCurrentProgress();
};

} // namespace Zero
