// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

static const float cAcceptableLoadtime = 0.15f;

void ExecuteRendererJob(RendererJob* job)
{
  ProfileScopeFunction();
  job->Execute();
}

OsInt RendererThreadMain(void* rendererThreadJobQueue)
{
  RendererThreadJobQueue* jobQueue = (RendererThreadJobQueue*)rendererThreadJobQueue;

  Array<RendererJob*> rendererJobs;

  bool running = true;
  while (running)
  {
    jobQueue->WaitForJobs();

    jobQueue->TakeAllJobs(rendererJobs);
    forRange (RendererJob* job, rendererJobs.All())
      ExecuteRendererJob(job);
    rendererJobs.Clear();

    if (!ThreadingEnabled)
      break;

    running = !jobQueue->ShouldExitThread();
  }

  return 0;
}

void RendererJobQueue::AddJob(RendererJob* rendererJob)
{
  mThreadLock.Lock();
  mRendererJobs.PushBack(rendererJob);
  mThreadLock.Unlock();
}

void RendererJobQueue::TakeAllJobs(Array<RendererJob*>& rendererJobs)
{
  mThreadLock.Lock();
  rendererJobs.Append(mRendererJobs.All());
  mRendererJobs.Clear();
  mThreadLock.Unlock();
}

void RendererThreadJobQueue::AddJob(RendererJob* rendererJob)
{
  RendererJobQueue::AddJob(rendererJob);
  mRendererThreadEvent.Signal();
}

void RendererThreadJobQueue::WaitForJobs()
{
  mRendererThreadEvent.Wait();
}

bool RendererThreadJobQueue::HasJobs()
{
  bool hasJobs;
  mThreadLock.Lock();
  hasJobs = !mRendererJobs.Empty();
  mThreadLock.Unlock();
  return hasJobs;
}

bool RendererThreadJobQueue::ShouldExitThread()
{
  return mExitThread;
}

WaitRendererJob::WaitRendererJob()
{
  mWaitEvent.Initialize();
}

void WaitRendererJob::WaitOnThisJob()
{
  mWaitEvent.Wait();
}

void CreateRendererJob::Execute()
{
  Z::gRenderer = CreateRenderer();
  mWaitEvent.Signal();
}

void DestroyRendererJob::Execute()
{
  delete Z::gRenderer;
  mRendererJobQueue->mExitThread = true;
  mWaitEvent.Signal();
}

void AddMaterialJob::Execute()
{
  Z::gRenderer->AddMaterial(this);
  delete this;
}

void AddMeshJob::Execute()
{
  Z::gRenderer->AddMesh(this);
  delete this;
}

void AddTextureJob::Execute()
{
  Z::gRenderer->AddTexture(this);
  delete this;
}

void RemoveMaterialJob::Execute()
{
  Z::gRenderer->RemoveMaterial(this->mRenderData);
  delete this;
}

void RemoveMeshJob::Execute()
{
  Z::gRenderer->RemoveMesh(this->mRenderData);
  delete this;
}

void RemoveTextureJob::Execute()
{
  Z::gRenderer->RemoveTexture(this->mRenderData);
  delete this;
}

void SetLazyShaderCompilationJob::Execute()
{
  Z::gRenderer->SetLazyShaderCompilation(this->mLazyShaderCompilation);
  delete this;
}

AddShadersJob::AddShadersJob(RendererThreadJobQueue* jobQueue) : RepeatingJob(jobQueue), mForceCompileBatchCount(0)
{
  // Job starts and terminates itself.
  Start();
}

void AddShadersJob::OnExecute()
{
  Z::gRenderer->AddShaders(mShaders, mForceCompileBatchCount);
}

bool AddShadersJob::OnShouldRun()
{
  bool shouldRun = !mShaders.Empty();
  if (!shouldRun)
  {
    Terminate();
    Z::gEngine->has(GraphicsEngine)->mReturnJobQueue->AddJob(this);
  }
  return shouldRun;
}

void AddShadersJob::ReturnExecute()
{
  // Blocking task is assumed only used when mForceCompileBatchCount is not 0.
  // Do not need to send this event otherwise.
  if (mForceCompileBatchCount > 0)
  {
    Event event;
    Z::gEngine->DispatchEvent(Events::BlockingTaskFinish, &event);
  }
  delete this;
}

void RemoveShadersJob::Execute()
{
  Z::gRenderer->RemoveShaders(mShaders);
  delete this;
}

void DoRenderTasksJob::Execute()
{
  Z::gRenderer->DoRenderTasks(mRenderTasks, mRenderQueues);
  mWaitEvent.Signal();
}

void ReturnRendererJob::Execute()
{
  OnExecute();
  Z::gEngine->has(GraphicsEngine)->mReturnJobQueue->AddJob(this);
}

void GetTextureDataJob::OnExecute()
{
  Z::gRenderer->GetTextureData(this);
}

void SaveImageToFileJob::ReturnExecute()
{
  if (mImage == nullptr)
  {
    delete this;
    return;
  }

  SaveToImageJob* job = new SaveToImageJob();
  job->mImage = mImage;
  job->mWidth = mWidth;
  job->mHeight = mHeight;
  job->mFormat = mFormat;
  job->mFilename = mFilename;

  if (mFormat == TextureFormat::RGB32f)
    job->mImageType = ImageSaveFormat::Hdr;
  else
    job->mImageType = ImageSaveFormat::Png;

  Z::gJobs->AddJob(job);

  delete this;
}

RepeatingJob::RepeatingJob(RendererThreadJobQueue* jobQueue) :
    mRendererJobQueue(jobQueue),
    mExecuteDelay(16),
    mStartCount(0),
    mEndCount(0),
    mShouldRun(false),
    mDelayTerminate(false),
    mForceTerminate(false)
{
}

void RepeatingJob::Execute()
{
  if (ShouldRun())
  {
    OnExecute();

    if (ThreadingEnabled)
      Os::Sleep(mExecuteDelay);

    mRendererJobQueue->AddJob(this);
  }
  else
  {
    ++mEndCount;
  }
}

void RepeatingJob::Lock()
{
  mThreadLock.Lock();
}

void RepeatingJob::Unlock()
{
  mThreadLock.Unlock();
}

bool RepeatingJob::ShouldRun()
{
  Lock();
  // Regular logic for if job should run.
  bool running = (OnShouldRun() || mShouldRun);
  // Used to continue running job regardless during engine startup.
  running |= mDelayTerminate;
  // Forces job to end if it is in the queue more than once.
  running &= !(mEndCount < mStartCount - 1);
  // Forces job to end if the engine has to prematurely shut down.
  running &= (mForceTerminate == false);
  Unlock();
  return running;
}

bool RepeatingJob::IsRunning()
{
  Lock();
  bool running = mEndCount < mStartCount;
  Unlock();
  return running;
}

void RepeatingJob::Start()
{
  Lock();
  mShouldRun = true;
  ++mStartCount;
  Unlock();
}

void RepeatingJob::Terminate()
{
  Lock();
  mShouldRun = false;
  Unlock();
}

void RepeatingJob::ForceTerminate()
{
  Lock();
  mForceTerminate = true;
  Unlock();
}

} // namespace Raverie
