#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(ShaderInputsModified);
}

ZilchDefineType(ShaderInputsEvent, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

ZilchDefineType(GraphicsStatics, builder, type)
{
  ZilchBindGetter(DriverSupport);
}

GraphicsDriverSupport* GraphicsStatics::GetDriverSupport()
{
  return &Z::gRenderer->mDriverSupport;
}

System* CreateGraphicsSystem()
{
  return new GraphicsEngine();
}

Memory::Pool* Shader::sPool = nullptr;

//--------------------------------------------------------------- GraphicsEngine
ZilchDefineType(GraphicsEngine, builder, type)
{
}

GraphicsEngine::GraphicsEngine()
  : mRenderGroupCount(0)
  , mUpdateRenderGroupCount(false)
  , mNewLibrariesCommitted(false)
{
  mEngineShutdown = false;
}

GraphicsEngine::~GraphicsEngine()
{
  ShaderSettingsLibrary::GetInstance().ClearLibrary();
  ShaderSettingsLibrary::Destroy();

  DestroyRenderer();
  mRendererThread.WaitForCompletion();
  delete mDoRenderTasksJob;
  delete mRendererJobQueue;
  delete mShaderGenerator;

  delete mReturnJobQueue;

  delete mShowProgressJob;

  // Call clear functions for anything that has to be manually destructed
  // Must be done after renderer is destroyed
  mRenderTasksBack->Clear();
  mRenderQueuesBack->Clear();
  mRenderTasksFront->Clear();
  mRenderQueuesFront->Clear();
}

cstr GraphicsEngine::GetName()
{
  return "Graphics";
}

void GraphicsEngine::Initialize(SystemInitializer& initializer)
{
  // This needs to be initialized only once and multiple shader generators might be created
  ShaderSettingsLibrary::InitializeInstance();

  // Need to get translator or mode from Renderer
  mShaderGenerator = CreateZilchShaderGenerator();

  ConnectThisTo(Z::gEngine, Events::EngineShutdown, OnEngineShutdown);

  ConnectThisTo(Z::gEngine, Events::LoadingStart, StartProgress);
  ConnectThisTo(Z::gEngine, Events::LoadingProgress, UpdateProgress);
  ConnectThisTo(Z::gEngine, Events::LoadingFinish, EndProgress);
  ConnectThisTo(Z::gEngine, Events::ProjectLoaded, OnProjectLoaded);
  ConnectThisTo(Z::gEngine, Events::NoProjectLoaded, OnNoProjectLoaded);

  ConnectThisTo(Z::gResources, Events::ResourcesLoaded, OnResourcesAdded);
  ConnectThisTo(Z::gResources, Events::ResourcesUnloaded, OnResourcesRemoved);

  // Event connections for ResourceManagers
  ConnectThisTo(RenderGroupManager::GetInstance(), Events::ResourceAdded, OnRenderGroupAdded);
  ConnectThisTo(RenderGroupManager::GetInstance(), Events::ResourceRemoved, OnRenderGroupRemoved);

  ConnectThisTo(MaterialManager::GetInstance(), Events::ResourceAdded, OnMaterialAdded);
  ConnectThisTo(MaterialManager::GetInstance(), Events::ResourceModified, OnMaterialModified);
  ConnectThisTo(MaterialManager::GetInstance(), Events::ResourceRemoved, OnMaterialRemoved);

  ConnectThisTo(ZilchFragmentManager::GetInstance(), Events::ResourceAdded, OnZilchFragmentAdded);
  ConnectThisTo(ZilchFragmentManager::GetInstance(), Events::ResourceModified, OnZilchFragmentModified);
  ConnectThisTo(ZilchFragmentManager::GetInstance(), Events::ResourceRemoved, OnZilchFragmentRemoved);

  ConnectThisTo(MeshManager::GetInstance(), Events::ResourceAdded, OnMeshAdded);
  ConnectThisTo(MeshManager::GetInstance(), Events::ResourceModified, OnMeshModified);
  ConnectThisTo(MeshManager::GetInstance(), Events::ResourceRemoved, OnMeshRemoved);

  ConnectThisTo(TextureManager::GetInstance(), Events::ResourceAdded, OnTextureAdded);
  ConnectThisTo(TextureManager::GetInstance(), Events::ResourceModified, OnTextureModified);
  ConnectThisTo(TextureManager::GetInstance(), Events::ResourceRemoved, OnTextureRemoved);

  ConnectThisTo(ZilchManager::GetInstance(), Events::CompileZilchFragments, OnCompileZilchFragments);
  ConnectThisTo(ZilchManager::GetInstance(), Events::ScriptsCompiledPrePatch, OnScriptsCompiledPrePatch);
  ConnectThisTo(ZilchManager::GetInstance(), Events::ScriptsCompiledCommit, OnScriptsCompiledCommit);
  ConnectThisTo(ZilchManager::GetInstance(), Events::ScriptsCompiledPostPatch, OnScriptsCompiledPostPatch);
  ConnectThisTo(ZilchManager::GetInstance(), Events::ScriptCompilationFailed, OnScriptCompilationFailed);

  ParticleList::Memory = new Memory::Pool("Particles", Memory::GetRoot(), sizeof(Particle), 1024);
  Shader::sPool = new Memory::Pool("Shaders", Memory::GetRoot(), sizeof(Shader), 1024);

  mFrameCounter = 0;

  // Set pointers for swapping render data
  mRenderQueuesBack = &mRenderQueues[0];
  mRenderTasksBack = &mRenderTasks[0];
  mRenderQueuesFront = &mRenderQueues[1];
  mRenderTasksFront = &mRenderTasks[1];

  // Renderer thread setup
  mRendererJobQueue = new RendererThreadJobQueue();
  mRendererJobQueue->mRendererThreadEvent.Initialize();
  mRendererJobQueue->mExitThread = false;

  mDoRenderTasksJob = new DoRenderTasksJob();
  mDoRenderTasksJob->mWaitEvent.Signal();

  mReturnJobQueue = new RendererJobQueue();

  mShowProgressJob = new ShowProgressJob(mRendererJobQueue);
  mShowProgressJob->mDelayTerminate = true;

  mRendererThread.Initialize(RendererThreadMain, mRendererJobQueue, "RendererThread");
  ErrorIf(mRendererThread.IsValid() == false, "RendererThread failed to initialize.");
  mRendererThread.Resume();

  mVerticalSync = true;
}

void GraphicsEngine::Update()
{
  // Do not try to run rendering while this job is going.
  if (mShowProgressJob->IsRunning())
    return;

  ProfileScopeTree("GraphicsSystem", "Engine", Color::Blue);

  // Run all return jobs from renderer
  Array<RendererJob*> returnJobs;
  mReturnJobQueue->TakeAllJobs(returnJobs);
  forRange (RendererJob* job, returnJobs.All())
  {
    ReturnRendererJob* returnJob = (ReturnRendererJob*)job;
    returnJob->ReturnExecute();
  }

  ++mFrameCounter;

  mRenderTasksBack->Clear();
  mRenderTasksBack->mShaderInputsVersion = mFrameCounter;

  mRenderQueuesBack->Clear();
  mRenderQueuesBack->mSkinningBufferVersion = mFrameCounter;

  // UpdateRenderGroups can happen at the beginning of update if broadphase is done within this update function
  UpdateRenderGroups();

  {
    ProfileScopeTree("FrameUpdate", "Graphics", Color::SpringGreen);
    float frameDt = Z::gEngine->has(TimeSystem)->mEngineDt;
    forRange (GraphicsSpace& space, mSpaces.All())
      space.OnFrameUpdate(frameDt);
  }

  {
    ProfileScopeTree("RenderTasksUpdate", "Graphics", Color::LimeGreen);
    forRange (GraphicsSpace& space, mSpaces.All())
      space.RenderTasksUpdate(*mRenderTasksBack);
  }

  {
    ProfileScopeTree("RenderQueuesUpdate", "Graphics", Color::LawnGreen);
    forRange (GraphicsSpace& space, mSpaces.All())
      space.RenderQueuesUpdate(*mRenderTasksBack, *mRenderQueuesBack);

    Sort(mRenderTasksBack->mRenderTaskRanges.All());
  }

  {
    ProfileScopeTree("UiRenderUpdate", "Graphics", Color::DarkOliveGreen);
    // add ui render task range after sorting so that everything else renders before it
    Event event;
    DispatchEvent("UiRenderUpdate", &event);
  }

  {
    ProfileScopeTree("WaitOnRenderer", "Graphics", Color::Bisque);
    // cannot run another RenderTasks job unless the last one is done
    mDoRenderTasksJob->WaitOnThisJob();
  }

  Swap(mRenderTasksBack, mRenderTasksFront);
  Swap(mRenderQueuesBack, mRenderQueuesFront);

  // pass everything to the renderer, all rendering happens on this job
  mDoRenderTasksJob->mRenderTasks = mRenderTasksFront;
  mDoRenderTasksJob->mRenderQueues = mRenderQueuesFront;
  AddRendererJob(mDoRenderTasksJob);

  // Add job for texture data after render tasks job so that
  // textures being written to in render tasks will have the expected data
  forRange (TextureToFile& toFile, mDelayedTextureToFile.All())
  {
    SaveImageToFileJob* toFileJob = new SaveImageToFileJob();
    toFileJob->mRenderData = toFile.mTexture->mRenderData;
    toFileJob->mFilename = toFile.mFilename;
    AddRendererJob(toFileJob);
  }
  mDelayedTextureToFile.Clear();

  // Release textures that were not reused this frame
  mRenderTargetManager.ClearUnusedTextures();

  // when new RenderGroups are added or removed, it could happen after broadphase used them to organize visible objects
  // but before that data is used to render, and the renderer needs RenderGroup values to match still
  // so, any management or changes to RenderGroup id's should happen here after the whole frame and rendering are completed
  // if done at end of update, add and remove events need to defer their operations until here
  //UpdateRenderGroups();

  gDebugDraw->ClearObjects();
}

void GraphicsEngine::OnEngineShutdown(Event* event)
{
  mRenderTargetManager.Shutdown();

  // Clear all shaders
  HashSet<Shader*> shadersToRemove;
  shadersToRemove.Append(mCompositeShaders.Values());
  shadersToRemove.Append(mPostProcessShaders.Values());

  if (shadersToRemove.Empty() == false)
  {
    RemoveShadersJob* removeShadersJob = new RemoveShadersJob();

    forRange (Shader* shader, shadersToRemove.All())
    {
      ShaderEntry entry(shader);
      removeShadersJob->mShaders.PushBack(entry);

      mCompositeShaders.Erase(shader->mName);
      Shader::sPool->DeallocateType(shader);
    }

    AddRendererJob(removeShadersJob);

    mCompositeShaders.Clear();
    mPostProcessShaders.Clear();
    mShaderCoreVertexMap.Clear();
    mShaderCompositeMap.Clear();
    mShaderRenderPassMap.Clear();
  }

  mEngineShutdown = true;
  mShowProgressJob->ForceTerminate();
  while (mRendererJobQueue->HasJobs());
}

void GraphicsEngine::AddSpace(GraphicsSpace* space)
{
  mSpaces.PushBack(space);
}

void GraphicsEngine::RemoveSpace(GraphicsSpace* space)
{
  mSpaces.Erase(space);
}

void GraphicsEngine::StartProgress(Event* event)
{
  if (mEngineShutdown)
    return;

  Texture* loadingTexture = TextureManager::FindOrNull("ZeroLoading");
  Texture* logoTexture = TextureManager::FindOrNull("ZeroLogoAnimated");
  Texture* whiteTexture = TextureManager::FindOrNull("White");
  Texture* splashTexture = TextureManager::FindOrNull("ZeroSplash");
  if (loadingTexture == nullptr || logoTexture == nullptr || whiteTexture == nullptr || splashTexture == nullptr)
    return;

  mShowProgressJob->Lock();
  mShowProgressJob->mLoadingTexture = loadingTexture->mRenderData;
  mShowProgressJob->mLogoTexture = logoTexture->mRenderData;
  mShowProgressJob->mWhiteTexture = whiteTexture->mRenderData;
  mShowProgressJob->mSplashTexture = splashTexture->mRenderData;
  mShowProgressJob->mLogoFrameSize = 128;
  mShowProgressJob->mCurrentPercent = 0.0f;
  mShowProgressJob->mTargetPercent = 0.0f;
  mShowProgressJob->mProgressWidth = loadingTexture->mWidth;
  mShowProgressJob->mProgressText.Clear();
  mShowProgressJob->Unlock();

  mShowProgressJob->Start();
  mRendererJobQueue->AddJob(mShowProgressJob);
}

void GraphicsEngine::UpdateProgress(ProgressEvent* event)
{
  if (mEngineShutdown)
    return;

  Font* font = FontManager::FindOrNull("NotoSans-Regular");
  if (font == nullptr)
    return;

  RenderFont* renderFont = font->GetRenderFont(16);

  String progressText = BuildString(event->Operation, " ", event->CurrentTask, " ", event->ProgressLine);
  FontProcessorVertexArray fontProcessor(Vec4(1.0f));
  AddTextRange(fontProcessor, renderFont, progressText, Vec2::cZero, TextAlign::Left, Vec2(1.0f), Vec2((float)mShowProgressJob->mProgressWidth, (float)renderFont->mFontHeight), true);

  mShowProgressJob->Lock();
  mShowProgressJob->mTargetPercent = event->Percentage;
  mShowProgressJob->mFontTexture = renderFont->mTexture->mRenderData;
  mShowProgressJob->mProgressText = fontProcessor.mVertices;
  mShowProgressJob->Unlock();
}

void GraphicsEngine::EndProgress(Event* event)
{
  if (mEngineShutdown)
    return;

  mShowProgressJob->Terminate();
}

void GraphicsEngine::OnProjectLoaded(ObjectEvent* event)
{
  if (mProjectCog.IsNotNull())
    DisconnectAll(mProjectCog, this);

  mProjectCog = (Cog*)event->GetSource();
  ConnectThisTo(*mProjectCog, Events::ObjectModified, OnProjectCogModified);
  OnProjectCogModified(event);

  EndProgressDelayTerminate();
}

void GraphicsEngine::OnNoProjectLoaded(Event* event)
{
  EndProgressDelayTerminate();
}

void GraphicsEngine::EndProgressDelayTerminate()
{
  // Allows job to terminate after startup completes for the first time.
  mShowProgressJob->Lock();
  mShowProgressJob->mDelayTerminate = false;
  mShowProgressJob->Unlock();

  // Block until job completes.
  // Important for exports to not run engine update until job fully exits.
  while (mShowProgressJob->IsRunning())
    Os::Sleep(mShowProgressJob->mExecuteDelay);
}

void GraphicsEngine::SetSplashscreenLoading()
{
  mShowProgressJob->mSplashMode = true;
}

void GraphicsEngine::OnProjectCogModified(Event* event)
{
  if (FrameRateSettings* frameRate = mProjectCog.has(FrameRateSettings))
    SetVerticalSync(frameRate->mVerticalSync && !frameRate->mLimitFrameRate);
  else
    SetVerticalSync(false);
}

void GraphicsEngine::SetVerticalSync(bool verticalSync)
{
  if (verticalSync == mVerticalSync)
    return;

  mVerticalSync = verticalSync;

  SetVSyncJob* setVSyncJob = new SetVSyncJob();
  setVSyncJob->mVSync = mVerticalSync;
  AddRendererJob(setVSyncJob);
}

uint GraphicsEngine::GetRenderGroupCount()
{
  return mRenderGroupCount;
}

void GraphicsEngine::UpdateRenderGroups()
{
  if (mUpdateRenderGroupCount)
  {
    mRenderGroups.Clear();
    RenderGroupManager::GetInstance()->EnumerateResources(mRenderGroups);

    mRenderGroupCount = mRenderGroups.Size();

    for (uint i = 0; i < mRenderGroupCount; ++i)
    {
      RenderGroup* renderGroup = (RenderGroup*)mRenderGroups[i];
      renderGroup->mSortId = i;
    }

    mUpdateRenderGroupCount = false;
  }
}

void GraphicsEngine::CheckTextureYInvert(Texture* texture)
{
  // Check for Y-invert
  // Some Api's expect byte 0 to be the bottom left pixel, in Zero byte 0 is the top left
  // Have to Y-invert because sampling from a rendered target must also work correctly
  // Uv coordinate correction from Zero to Api is done by the shader translation of texture samples
  if (!Z::gRenderer->YInvertImageData(texture->mType))
    return;

  // All incoming image data from Zero should be a color format and/or block compressed
  if (texture->mImageData && IsColorFormat(texture->mFormat))
  {
    if (texture->mCompression == TextureCompression::None)
    {
      for (uint i = 0; i < texture->mMipCount; ++i)
      {
        MipHeader* mipHeader = texture->mMipHeaders + i;
        byte* mipData = texture->mImageData + mipHeader->mDataOffset;
        YInvertNonCompressed(mipData, mipHeader->mWidth, mipHeader->mHeight, GetPixelSize(texture->mFormat));
      }
    }
    else
    {
      for (uint i = 0; i < texture->mMipCount; ++i)
      {
        MipHeader* mipHeader = texture->mMipHeaders + i;
        byte* mipData = texture->mImageData + mipHeader->mDataOffset;
        YInvertBlockCompressed(mipData, mipHeader->mWidth, mipHeader->mHeight, mipHeader->mDataSize, texture->mCompression);
      }
    }
  }
}

void GraphicsEngine::AddRendererJob(RendererJob* rendererJob)
{
  mRendererJobQueue->AddJob(rendererJob);
}

void GraphicsEngine::CreateRenderer(OsHandle mainWindowHandle)
{
  CreateRendererJob* rendererJob = new CreateRendererJob();
  rendererJob->mMainWindowHandle = mainWindowHandle;
  AddRendererJob(rendererJob);
  rendererJob->WaitOnThisJob();

  if (rendererJob->mError.Empty() == false)
    FatalEngineError(rendererJob->mError.c_str());

  delete rendererJob;
}

void GraphicsEngine::DestroyRenderer()
{
  DestroyRendererJob* rendererJob = new DestroyRendererJob();
  rendererJob->mRendererJobQueue = mRendererJobQueue;
  AddRendererJob(rendererJob);
  rendererJob->WaitOnThisJob();
  delete rendererJob;
}

void GraphicsEngine::AddMaterial(Material* material)
{
  Z::gRenderer->CreateRenderData(material);
  AddMaterialJob* rendererJob = new AddMaterialJob();
  rendererJob->mRenderData = material->mRenderData;
  rendererJob->mCompositeName = material->mCompositeName;
  rendererJob->mMaterialId = material->mResourceId;

  AddRendererJob(rendererJob);
}

void GraphicsEngine::AddMesh(Mesh* mesh)
{
  Z::gRenderer->CreateRenderData(mesh);
  AddMeshJob* rendererJob = new AddMeshJob();
  rendererJob->mRenderData = mesh->mRenderData;

  VertexBuffer* vertices = &mesh->mVertices;
  IndexBuffer* indices = &mesh->mIndices;

  rendererJob->mPrimitiveType = mesh->mPrimitiveType;

  rendererJob->mVertexSize = 0;
  rendererJob->mVertexCount = 0;
  rendererJob->mVertexData = nullptr;
  rendererJob->mIndexSize = indices->mIndexSize;
  rendererJob->mIndexCount = 0;
  rendererJob->mIndexData = nullptr;

  if (vertices->mFixedDesc.mVertexSize != 0)
  {
    rendererJob->mVertexSize = vertices->mFixedDesc.mVertexSize;
    rendererJob->mVertexCount = vertices->mDataSize / vertices->mFixedDesc.mVertexSize;

    uint vertexDataSize = rendererJob->mVertexSize * rendererJob->mVertexCount;
    rendererJob->mVertexData = new byte[vertexDataSize];
    memcpy(rendererJob->mVertexData, vertices->mData, vertexDataSize);

    rendererJob->mVertexAttributes.Reserve(8);
    for (uint i = 0; i < FixedVertexDescription::sMaxElements; ++i)
    {
      if (vertices->mFixedDesc.mAttributes[i].mSemantic == VertexSemantic::None)
        break;
      rendererJob->mVertexAttributes.PushBack(vertices->mFixedDesc.mAttributes[i]);
    }
  }

  rendererJob->mIndexSize = indices->mIndexSize;
  rendererJob->mIndexCount = indices->mIndexCount;

  if (indices->mData.Empty() == false)
  {
    uint indexDataSize = rendererJob->mIndexSize * rendererJob->mIndexCount;
    rendererJob->mIndexData = new byte[indexDataSize];
    memcpy(rendererJob->mIndexData, &indices->mData[0], indexDataSize);
  }

  rendererJob->mBones.Assign(mesh->mBones.All());

  AddRendererJob(rendererJob);
}

void GraphicsEngine::AddTexture(Texture* texture, bool subImage, uint xOffset, uint yOffset)
{
  // Do y inverting on main thread (if needed)
  // otherwise the render thread takes too long to upload textures
  // and ram usage builds up too high during project loading.
  // NOTE: Gpu mip generation on block compressed textures
  // also takes a decent amount of time.
  CheckTextureYInvert(texture);

  Z::gRenderer->CreateRenderData(texture);
  AddTextureJob* rendererJob = new AddTextureJob();

  rendererJob->mRenderData = texture->mRenderData;
  rendererJob->mWidth = texture->mWidth;
  rendererJob->mHeight = texture->mHeight;
  rendererJob->mMipCount = texture->mMipCount;
  rendererJob->mTotalDataSize = texture->mTotalDataSize;

  rendererJob->mMipHeaders = texture->mMipHeaders;
  rendererJob->mImageData = texture->mImageData;
  texture->mMipHeaders = nullptr;
  texture->mImageData = nullptr;

  rendererJob->mType = texture->mType;
  rendererJob->mFormat = texture->mFormat;

  rendererJob->mCompression = texture->mCompression;
  rendererJob->mAddressingX = texture->mAddressingX;
  rendererJob->mAddressingY = texture->mAddressingY;
  rendererJob->mFiltering = texture->mFiltering;
  rendererJob->mCompareMode = texture->mCompareMode;
  rendererJob->mCompareFunc = texture->mCompareFunc;
  rendererJob->mAnisotropy = texture->mAnisotropy;
  rendererJob->mMipMapping = texture->mMipMapping;

  rendererJob->mSubImage = subImage;
  rendererJob->mXOffset = xOffset;
  rendererJob->mYOffset = yOffset;

  AddRendererJob(rendererJob);
}

void GraphicsEngine::RemoveMaterial(Material* material)
{
  // Handle double remove events
  if (material->mRenderData == nullptr)
    return;
  RemoveMaterialJob* rendererJob = new RemoveMaterialJob();
  rendererJob->mRenderData = material->mRenderData;
  material->mRenderData = nullptr;
  AddRendererJob(rendererJob);
}

void GraphicsEngine::RemoveMesh(Mesh* mesh)
{
  // Handle double remove events
  if (mesh->mRenderData == nullptr)
    return;
  RemoveMeshJob* rendererJob = new RemoveMeshJob();
  rendererJob->mRenderData = mesh->mRenderData;
  mesh->mRenderData = nullptr;
  AddRendererJob(rendererJob);
}

void GraphicsEngine::RemoveTexture(Texture* texture)
{
  // Handle double remove events
  if (texture->mRenderData == nullptr)
    return;
  RemoveTextureJob* rendererJob = new RemoveTextureJob();
  rendererJob->mRenderData = texture->mRenderData;
  texture->mRenderData = nullptr;
  AddRendererJob(rendererJob);
}

void GraphicsEngine::OnRenderGroupAdded(ResourceEvent* event)
{
  mAddedRenderGroups.PushBack((RenderGroup*)event->EventResource);

  mUpdateRenderGroupCount = true;
}

void GraphicsEngine::OnRenderGroupRemoved(ResourceEvent* event)
{
  RenderGroup* renderGroup = (RenderGroup*)event->EventResource;

  ResourceListRemove(renderGroup);

  mUpdateRenderGroupCount = true;
}

void GraphicsEngine::OnMaterialAdded(ResourceEvent* event)
{
  mAddedMaterials.PushBack((Material*)event->EventResource);
}

void GraphicsEngine::OnMaterialModified(ResourceEvent* event)
{
  Material* material = (Material*)event->EventResource;

  if (material->mCompositionChanged)
  {
    UpdateUniqueComposites(material, UniqueCompositeOp::Modify);
    material->mCompositionChanged = false;

    AddMaterial(material);

    CompileShaders();
  }
}

void GraphicsEngine::OnMaterialRemoved(ResourceEvent* event)
{
  Material* material = (Material*)event->EventResource;
  
  ResourceListRemove(material);

  UpdateUniqueComposites(material, UniqueCompositeOp::Remove);

  if (mRemovedComposites.Empty() == false)
    CompileShaders();

  RemoveMaterial(material);
}

void GraphicsEngine::OnZilchFragmentAdded(ResourceEvent* event)
{
  // OnResourcesAdded will invoke a compile after this
  mModifiedFragmentFiles.PushBack(event->EventResource->Name);
}

void GraphicsEngine::OnZilchFragmentModified(ResourceEvent* event)
{
  // Happens on save, wait for successful compilation to process
  mModifiedFragmentFiles.PushBack(event->EventResource->Name);
}

void GraphicsEngine::OnZilchFragmentRemoved(ResourceEvent* event)
{
  // Only need removed fragments if going to send compile in removed resources
  mRemovedFragmentFiles.PushBack(event->EventResource->Name);
  // Cannot process modified files that were removed
  // Added/modified methods do not erase from removed list because removed has to operate on the previous fragments library
  mModifiedFragmentFiles.EraseValue(event->EventResource->Name);
}

void GraphicsEngine::OnMeshAdded(ResourceEvent* event)
{
  AddMesh((Mesh*)event->EventResource);
}

void GraphicsEngine::OnMeshModified(ResourceEvent* event)
{
  AddMesh((Mesh*)event->EventResource);
}

void GraphicsEngine::OnMeshRemoved(ResourceEvent* event)
{
  RemoveMesh((Mesh*)event->EventResource);
}

void GraphicsEngine::OnTextureAdded(ResourceEvent* event)
{
  AddTexture((Texture*)event->EventResource);
}

void GraphicsEngine::OnTextureModified(ResourceEvent* event)
{
  AddTexture((Texture*)event->EventResource);
}

void GraphicsEngine::OnTextureRemoved(ResourceEvent* event)
{
  RemoveTexture((Texture*)event->EventResource);
}

void GraphicsEngine::OnResourcesAdded(ResourceEvent* event)
{
  forRange (Material* material, mAddedMaterials.All())
    ResourceListAdd(material);

  forRange (RenderGroup* renderGroup, mAddedRenderGroups.All())
    ResourceListAdd(renderGroup);

  if (mAddedMaterials.Empty() == false)
  {
    forRange (Resource* resource, RenderGroupManager::GetInstance()->AllResources())
      ResourceListResolveReferences((RenderGroup*)resource);
  }

  if (mAddedRenderGroups.Empty() == false)
  {
    forRange (Resource* resource, MaterialManager::GetInstance()->AllResources())
      ResourceListResolveReferences((Material*)resource);
  }

  // Materials could be added in a non compiling state
  // so copy to a separate list for handling shader composites
  mAddedMaterialsForComposites.Append(mAddedMaterials.All());

  mAddedMaterials.Clear();
  mAddedRenderGroups.Clear();

  // If added ZilchFragments
  if (mModifiedFragmentFiles.Empty() == false)
  {
    ZilchManager::GetInstance()->Compile();
  }
  else
  {
    forRange (Material* material, mAddedMaterialsForComposites.All())
    {
      UpdateUniqueComposites(material, UniqueCompositeOp::Add);
      AddMaterial(material);
    }
    mAddedMaterialsForComposites.Clear();

    CompileShaders();
  }
}

// should this invoke a compile if there are removed fragment files?
void GraphicsEngine::OnResourcesRemoved(ResourceEvent* event)
{
  // Can't rebuild meta on shutdown because content system is destroyed by this point
  //if (mEngineShutdown || mRemovedFragmentFiles.Empty())
  //  return;

  //BuildFragmentsLibrary();
}

void GraphicsEngine::AddComposite(Material* material)
{
  String compositeName = material->mCompositeName;
  if (mUniqueComposites.ContainsKey(compositeName))
  {
    mUniqueComposites[compositeName].mReferences += 1;
  }
  else
  {
    UniqueComposite newComposite;
    newComposite.mName = compositeName;
    newComposite.mFragmentNames = material->mFragmentNames;
    newComposite.mFragmentNameMap.Append(newComposite.mFragmentNames.All());
    newComposite.mReferences = 1;
    mUniqueComposites.Insert(compositeName, newComposite);
    mModifiedComposites.Insert(compositeName);
  }
}

void GraphicsEngine::RemoveComposite(StringParam compositeName)
{
  ErrorIf(mUniqueComposites.ContainsKey(compositeName) == false, "Reference count error.");

  mUniqueComposites[compositeName].mReferences -= 1;

  if (mUniqueComposites[compositeName].mReferences == 0)
  {
    mRemovedComposites.Insert(compositeName);
    mModifiedComposites.Erase(compositeName);
    mUniqueComposites.Erase(compositeName);
  }
}

Shader* GraphicsEngine::GetOrCreateShader(StringParam coreVertex, StringParam composite, StringParam renderPass, ShaderMap& shaderMap)
{
  String name = BuildString(coreVertex, composite, renderPass);
  if (shaderMap.ContainsKey(name))
    return shaderMap.FindValue(name, nullptr);

  Shader* shader = Shader::sPool->AllocateType<Shader>();
  shader->mCoreVertex = coreVertex;
  shader->mComposite = composite;
  shader->mRenderPass = renderPass;
  shader->mName = name;

  shaderMap.Insert(shader->mName, shader);

  return shader;
}

void GraphicsEngine::FindShadersToCompile(Array<String>& coreVertexRange, Array<String>& compositeRange, Array<String>& renderPassRange, ShaderSetMap& testMap, uint index, ShaderSet& shaders)
{
  Array<String>* ranges[] = {&coreVertexRange, &compositeRange, &renderPassRange};

  // Index order for iteration
  uint i0 = index;
  uint i1 = (i0 + 1) % 3;
  uint i2 = (i0 + 2) % 3;

  // Fragment order from iteration order for indexing fragment names, {CoreVertex, Composite, RenderPass}
  uint f0 = (index + index) % 3;
  uint f1 = (f0 + 1) % 3;
  uint f2 = (f0 + 2) % 3;

  forRange (String frag0, ranges[i0]->All())
  {
    if (testMap.ContainsKey(frag0))
    {
      ShaderSet* shaderSet = testMap.FindPointer(frag0);
      forRange (Shader* shader, shaderSet->All())
        shaders.Insert(shader);
    }
    else
    {
      forRange (String frag1, ranges[i1]->All())
      {
        forRange (String frag2, ranges[i2]->All())
        {
          String fragmentNames[] = {frag0, frag1, frag2};

          Shader* shader = GetOrCreateShader(fragmentNames[f0], fragmentNames[f1], fragmentNames[f2], mCompositeShaders);
          shaders.Insert(shader);
        }
      }

      // Special case for composites as a post process
      if (index == 1)
      {
        Shader* shader = GetOrCreateShader("PostVertex", frag0, String(), mCompositeShaders);
        shaders.Insert(shader);
      }
    }
  }
}

void GraphicsEngine::FindShadersToRemove(Array<String>& elementRange, ShaderSetMap& testMap, ShaderSet& shaders)
{
  forRange (String name, elementRange.All())
  {
    // Composites can not be in the map if the composite exists because one of its fragments didn't compile
    if (testMap.ContainsKey(name) == false)
      continue;

    ShaderSet* shaderSet = testMap.FindPointer(name);
    forRange (Shader* shader, shaderSet->All())
    {
      shaders.Insert(shader);
      mCompositeShaders.Erase(shader->mName);
    }
  }
}

void GraphicsEngine::AddToShaderMaps(ShaderSet& shaders)
{
  forRange (Shader* shader, shaders.All())
  {
    mShaderCoreVertexMap[shader->mCoreVertex].Insert(shader);
    mShaderCompositeMap[shader->mComposite].Insert(shader);
    mShaderRenderPassMap[shader->mRenderPass].Insert(shader);
  }
}

void GraphicsEngine::RemoveFromShaderMaps(ShaderSet& shaders)
{
  forRange (Shader* shader, shaders.All())
  {
    RemoveFromShaderMap(mShaderCoreVertexMap, shader->mCoreVertex, shader);
    RemoveFromShaderMap(mShaderCompositeMap, shader->mComposite, shader);
    RemoveFromShaderMap(mShaderRenderPassMap, shader->mRenderPass, shader);
  }
}

void GraphicsEngine::RemoveFromShaderMap(ShaderSetMap& shaderMap, StringParam elementName, Shader* shader)
{
  if (shaderMap.ContainsKey(elementName))
  {
    shaderMap[elementName].Erase(shader);
    if (shaderMap[elementName].Empty())
      shaderMap.Erase(elementName);
  }
}

void GraphicsEngine::ProcessModifiedScripts(LibraryRef library)
{
  forRange (BoundType* type, library->BoundTypes.Values())
  {
    String typeName = type->Name;

    // If already an entry for this type then inputs might have changed
    bool oldEntries = mComponentShaderProperties.ContainsKey(typeName);
    mComponentShaderProperties.Erase(typeName);

    // Only look for inputs on component types
    if (type->IsA(ZilchTypeId(Component)))
    {

      forRange (Property* metaProperty, type->GetProperties())
      {
        Attribute* inputAttribute = metaProperty->HasAttribute(PropertyAttributes::cShaderInput);
        if (inputAttribute == nullptr)
          continue;

        ShaderMetaProperty shaderProperty;
        shaderProperty.mMetaPropertyName = metaProperty->Name;

        uint parameterCount = inputAttribute->Parameters.Size();
        if (parameterCount >= 2)
          shaderProperty.mInputName = inputAttribute->Parameters[1].StringValue;
        if (parameterCount >= 1)
          shaderProperty.mFragmentName = inputAttribute->Parameters[0].StringValue;

        mComponentShaderProperties[typeName].PushBack(shaderProperty);
      }
    }

    bool newEntries = mComponentShaderProperties.ContainsKey(typeName);

    // Forward to Graphicals if any relevant changes
    if (oldEntries || newEntries)
    {
      ShaderInputsEvent event;
      event.mType = type;
      DispatchEvent(Events::ShaderInputsModified, &event);
    }
  }
}

ZilchFragmentType::Enum GraphicsEngine::GetFragmentType(MaterialBlock* materialBlock)
{
  return mShaderGenerator->mFragmentTypes.FindValue(ZilchVirtualTypeId(materialBlock)->Name, ZilchFragmentType::Fragment);
}

HandleOf<RenderTarget> GraphicsEngine::GetRenderTarget(uint width, uint height, TextureFormat::Enum format, SamplerSettings samplerSettings)
{
  return mRenderTargetManager.GetRenderTarget(width, height, format, samplerSettings);
}

HandleOf<RenderTarget> GraphicsEngine::GetRenderTarget(HandleOf<Texture> texture)
{
  return mRenderTargetManager.GetRenderTarget(texture);
}

void GraphicsEngine::ClearRenderTargets()
{
  mRenderTargetManager.ClearRenderTargets();
}

void GraphicsEngine::ForceCompileAllShaders()
{
  // METAREFACTOR Check how this was being used
  //ShaderSet allShaders;
  //allShaders.Append(mCompositeShaders.Values());
  //allShaders.Append(mPostProcessShaders.Values());
  //
  //if(allShaders.Empty())
  //  return;
  //
  //AddShadersJob* addShadersJob = new AddShadersJob();
  //bool compiled = mShaderGenerator->BuildShadersLibrary(allShaders, mUniqueComposites, addShadersJob->mShaders);
  //ErrorIf(!compiled, "Shaders did not compile after composition.");
  //addShadersJob->mForceCompile = true;
  //AddRendererJob(addShadersJob);
}

void GraphicsEngine::ModifiedFragment(ZilchFragmentType::Enum type, StringParam name)
{
  switch (type)
  {
    case Zero::ZilchFragmentType::CoreVertex: mModifiedCoreVertex.PushBack(name); break;
    case Zero::ZilchFragmentType::RenderPass: mModifiedRenderPass.PushBack(name); break;
    case Zero::ZilchFragmentType::PostProcess: mModifiedPostProcess.PushBack(name); break;
  }
}

void GraphicsEngine::RemovedFragment(ZilchFragmentType::Enum type, StringParam name)
{
  switch (type)
  {
    case Zero::ZilchFragmentType::CoreVertex: mRemovedCoreVertex.PushBack(name); break;
    case Zero::ZilchFragmentType::RenderPass: mRemovedRenderPass.PushBack(name); break;
    case Zero::ZilchFragmentType::PostProcess: mRemovedPostProcess.PushBack(name); break;
  }
}

void GraphicsEngine::OnCompileZilchFragments(ZilchCompileFragmentEvent* event)
{
  String libraryName = BuildString(event->mOwningLibrary->Name, "Fragments");
  event->mReturnedLibrary = mShaderGenerator->BuildFragmentsLibrary(event->mDependencies, event->mFragments, libraryName);
}

void GraphicsEngine::OnScriptsCompiledPrePatch(ZilchCompileEvent* event)
{
  forRange (ResourceLibrary* modifiedLibrary, event->mModifiedLibraries.All())
  {
    if (!modifiedLibrary->HasPendingFragmentLibrary())
      continue;

    ZilchShaderLibraryRef currentLibrary = mShaderGenerator->GetCurrentInternalLibrary(modifiedLibrary->mCurrentFragmentLibrary);
    ZilchFragmentTypeMap& currentFragmentTypes = mShaderGenerator->mFragmentTypes;

    ZilchShaderLibraryRef pendingLibrary = mShaderGenerator->GetPendingInternalLibrary(modifiedLibrary->mPendingFragmentLibrary);
    ZilchFragmentTypeMap& pendingFragmentTypes = mShaderGenerator->mPendingFragmentTypes[modifiedLibrary->mPendingFragmentLibrary];

    // Find removed types
    if (currentLibrary != nullptr)
    {
      forRange (ShaderType* shaderType, currentLibrary->mTypes.Values())
      {
        if (shaderType->mFlags.IsSet(ShaderTypeFlags::Native))
          continue;

        // Skip if type still exists
        if (pendingLibrary->mTypes.ContainsKey(shaderType->mZilchName))
          continue;

        ZilchFragmentType::Enum fragmentType = currentFragmentTypes.FindValue(shaderType->mZilchName, ZilchFragmentType::Fragment);
        RemovedFragment(fragmentType, shaderType->mZilchName);
      }
    }

    // Find added/modified types
    if (mModifiedFragmentFiles.Empty() == false)
    {
      forRange (ShaderType* shaderType, pendingLibrary->mTypes.Values())
      {
        if (shaderType->mFlags.IsSet(ShaderTypeFlags::Native))
          continue;

        // Identify new/modified types.
        // We currently only have one class written to the complex user data
        // so we can hard-code passing 0 in (for the index).
        FragmentUserData& fragmentUserData = shaderType->mComplexUserData.ReadObject<FragmentUserData>(0);
        String resourceName = fragmentUserData.mResourceName;
        if (mModifiedFragmentFiles.Contains(resourceName))
        {
          // Check for fragments that used to have a special attribute and add them to the appropriate removed list
          ZilchFragmentType::Enum currentFragmentType = currentFragmentTypes.FindValue(shaderType->mZilchName, ZilchFragmentType::Fragment);
          ZilchFragmentType::Enum pendingFragmentType = pendingFragmentTypes.FindValue(shaderType->mZilchName, ZilchFragmentType::Fragment);

          if (pendingFragmentType != currentFragmentType)
            RemovedFragment(currentFragmentType, shaderType->mZilchName);

          ModifiedFragment(pendingFragmentType, shaderType->mZilchName);

          // If current type is fragment and pending type isn't then any affected composites
          // are just going to get removed and don't need to be checked
          if (pendingFragmentType == ZilchFragmentType::Fragment)
          {
            forRange (UniqueComposite& composite, mUniqueComposites.Values())
            {
              if (composite.mFragmentNameMap.Contains(shaderType->mZilchName))
                mModifiedComposites.Insert(composite.mName);
            }
          }

          // Find all types dependent on this one and also list them as modified
          HashSet<ShaderType*> dependents;
          pendingLibrary->GetAllDependents(shaderType, dependents);
          forRange (ShaderType* dependent, dependents.All())
          {
            ZilchFragmentType::Enum dependentType = pendingFragmentTypes.FindValue(dependent->mZilchName, ZilchFragmentType::Fragment);

            // Do not need to check composites unless it's a regular fragment type
            // Composites will otherwise be handled by the other fragment types being modified
            if (dependentType == ZilchFragmentType::Fragment)
            {
              // Check all composites
              // Post patch still needs to run composite update on materials,
              // but if a composite results in being removed it will correctly be removed from this list
              forRange (UniqueComposite& composite, mUniqueComposites.Values())
              {
                if (composite.mFragmentNameMap.Contains(dependent->mZilchName))
                  mModifiedComposites.Insert(composite.mName);
              }
            }
            else
            {
              ModifiedFragment(dependentType, dependent->mZilchName);
            }
          }
        }
      }
    }
  }

  mRemovedFragmentFiles.Clear();
  mModifiedFragmentFiles.Clear();
}

void GraphicsEngine::OnScriptsCompiledCommit(ZilchCompileEvent* event)
{
  // Update the old libraries with the new ones
  mNewLibrariesCommitted = mShaderGenerator->Commit(event);

  // After fragment libraries are committed component shader inputs can be processed
  forRange (ResourceLibrary* modifiedLibrary, event->mModifiedLibraries.All())
  {
    if (modifiedLibrary->HasPendingScriptLibrary())
      ProcessModifiedScripts(modifiedLibrary->mPendingScriptLibrary);
  }
}

void GraphicsEngine::OnScriptsCompiledPostPatch(ZilchCompileEvent* event)
{
  // Don't do anything if no new fragment libraries were made
  if (mNewLibrariesCommitted == false)
    return;

  mNewLibrariesCommitted = false;

  MaterialFactory::GetInstance()->UpdateRestrictedComponents(mShaderGenerator->mCurrentToInternal, mShaderGenerator->mFragmentTypes);

  // Re-Initialize after new types have been committed
  forRange (Resource* resource, MaterialManager::GetInstance()->AllResources())
  {
    Material* material = (Material*)resource;
    material->ReInitialize();

    if (mAddedMaterialsForComposites.Contains(material))
      UpdateUniqueComposites(material, UniqueCompositeOp::Add);
    else
      UpdateUniqueComposites(material, UniqueCompositeOp::Modify);

    AddMaterial(material);

    material->SendModified();
  }

  mAddedMaterialsForComposites.Clear();

  CompileShaders();
}

void GraphicsEngine::OnScriptCompilationFailed(Event* event)
{
  forRange (Material* material, mAddedMaterialsForComposites.All())
  {
    UpdateUniqueComposites(material, UniqueCompositeOp::Add);
    AddMaterial(material);
  }
  mAddedMaterialsForComposites.Clear();

  // If scripts failed, we want to update any material modifications to use the old fragment library
  if (!mModifiedComposites.Empty() || !mRemovedComposites.Empty())
    CompileShaders();
}

void GraphicsEngine::UpdateUniqueComposites(Material* material, UniqueCompositeOp::Enum uniqueCompositeOp)
{
  if (uniqueCompositeOp == UniqueCompositeOp::Add)
  {
    ErrorIf(material->mRenderData != nullptr, "Material has already been added.");
    material->UpdateCompositeName();
    AddComposite(material);
  }
  else if (uniqueCompositeOp == UniqueCompositeOp::Remove)
  {
    ErrorIf(material->mRenderData == nullptr, "Material has already been removed.");
    RemoveComposite(material->mCompositeName);
  }
  else if (uniqueCompositeOp == UniqueCompositeOp::Modify)
  {
    String oldCompositeName = material->mCompositeName;
    material->UpdateCompositeName();
    String compositeName = material->mCompositeName;

    if (compositeName == oldCompositeName)
      return;

    RemoveComposite(oldCompositeName);
    AddComposite(material);
  }
}

void GraphicsEngine::CompileShaders()
{
  if (mShaderGenerator->mCurrentToInternal.Empty())
    return;

  // Find shaders to remove
  ShaderSet shadersToRemove;

  Array<String> removedComposites;
  removedComposites.Append(mRemovedComposites.All());

  FindShadersToRemove(mRemovedCoreVertex, mShaderCoreVertexMap, shadersToRemove);
  FindShadersToRemove(removedComposites, mShaderCompositeMap, shadersToRemove);
  FindShadersToRemove(mRemovedRenderPass, mShaderRenderPassMap, shadersToRemove);

  RemoveFromShaderMaps(shadersToRemove);

  forRange (String fragment, mRemovedPostProcess.All())
  {
    String shaderName = BuildString("PostVertex", fragment);
    shadersToRemove.Insert(mPostProcessShaders[shaderName]);
    mPostProcessShaders.Erase(shaderName);
  }

  if (shadersToRemove.Empty() == false)
  {
    RemoveShadersJob* removeShadersJob = new RemoveShadersJob();

    forRange (Shader* shader, shadersToRemove.All())
    {
      ShaderEntry entry(shader);
      removeShadersJob->mShaders.PushBack(entry);

      Shader::sPool->DeallocateType(shader);
    }

    AddRendererJob(removeShadersJob);
  }

  mRemovedCoreVertex.Clear();
  mRemovedComposites.Clear();
  mRemovedRenderPass.Clear();
  mRemovedPostProcess.Clear();

  // Find shaders to compile
  ShaderSet shadersToCompile;

  Array<String> compositeNames;
  compositeNames.Append(mUniqueComposites.Keys());
  Array<String> modifiedComposites;
  modifiedComposites.Append(mModifiedComposites.All());

  Array<String>& coreVertexFragments = mShaderGenerator->mCoreVertexFragments;
  Array<String>& renderPassFragments = mShaderGenerator->mRenderPassFragments;

  // Process based on modified lists
  FindShadersToCompile(mModifiedCoreVertex, compositeNames, renderPassFragments, mShaderCoreVertexMap, 0, shadersToCompile);
  FindShadersToCompile(coreVertexFragments, modifiedComposites, renderPassFragments, mShaderCompositeMap, 1, shadersToCompile);
  FindShadersToCompile(coreVertexFragments, compositeNames, mModifiedRenderPass, mShaderRenderPassMap, 2, shadersToCompile);

  AddToShaderMaps(shadersToCompile);

  forRange (String fragment, mModifiedPostProcess.All())
  {
    Shader* shader = GetOrCreateShader("PostVertex", fragment, String(), mPostProcessShaders);
    shadersToCompile.Insert(shader);
  }

  mModifiedCoreVertex.Clear();
  mModifiedComposites.Clear();
  mModifiedRenderPass.Clear();
  mModifiedPostProcess.Clear();

  if (shadersToCompile.Empty() == false)
  {
    AddShadersJob* addShadersJob = new AddShadersJob();
    bool compiled = mShaderGenerator->BuildShaders(shadersToCompile, mUniqueComposites, addShadersJob->mShaders);
    ErrorIf(!compiled, "Shaders did not compile after composition.");
    addShadersJob->mForceCompile = false;
    AddRendererJob(addShadersJob);
  }
}

void GraphicsEngine::WriteTextureToFile(HandleOf<Texture> texture, StringParam filename)
{
  mDelayedTextureToFile.PushBack(TextureToFile(texture, filename));
}

int SaveToPngJob::Execute()
{
  Status status;
  SaveToPng(status, mImage, mWidth, mHeight, mBitDepth, mFilename);
  delete[] mImage;
  return 0;
}

int SaveToHdrJob::Execute()
{
  Status status;
  SaveToHdr(status, mImage, mWidth, mHeight, mFilename);
  delete[] mImage;
  return 0;
}

} // namespace Zero
