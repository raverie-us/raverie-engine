#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(RenderTasksUpdate);
  DefineEvent(RenderTasksUpdateInternal);
}

ZilchDefineType(GraphicalRangeInterface, builder, type)
{
  ZilchBindMethod(Add);
  ZilchBindMethod(Clear);
  ZilchBindGetterProperty(Count);

  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;
}

void GraphicalRangeInterface::Add(Graphical* graphical)
{
  if (graphical != nullptr)
    mGraphicals.PushBack(graphical);
}

void GraphicalRangeInterface::Clear()
{
  mGraphicals.Clear();
}

uint GraphicalRangeInterface::GetCount()
{
  return mGraphicals.Size();
}

RenderTaskBuffer::RenderTaskBuffer()
  : mTaskCount(0)
  , mCurrentIndex(0)
{
  mRenderTaskData.Resize(128);
}

void RenderTaskBuffer::Clear()
{
  // Have to manually destruct render tasks because they're stored in a generic buffer
  uint taskIndex = 0;
  while (taskIndex < mCurrentIndex)
  {
    byte* task = &mRenderTaskData[taskIndex];
    switch (*task)
    {
      case RenderTaskType::ClearTarget:
      ((RenderTaskClearTarget*)task)->~RenderTaskClearTarget();
      taskIndex += sizeof(RenderTaskClearTarget);
      break;

      case RenderTaskType::RenderPass:
      ((RenderTaskRenderPass*)task)->~RenderTaskRenderPass();
      taskIndex += sizeof(RenderTaskRenderPass);
      break;

      case RenderTaskType::PostProcess:
      ((RenderTaskPostProcess*)task)->~RenderTaskPostProcess();
      taskIndex += sizeof(RenderTaskPostProcess);
      break;

      case RenderTaskType::BackBufferBlit:
      ((RenderTaskBackBufferBlit*)task)->~RenderTaskBackBufferBlit();
      taskIndex += sizeof(RenderTaskBackBufferBlit);
      break;

      case RenderTaskType::TextureUpdate:
      ((RenderTaskTextureUpdate*)task)->~RenderTaskTextureUpdate();
      taskIndex += sizeof(RenderTaskTextureUpdate);
      break;

      default:
      Error("Render task destruction not implemented.");
      break;
    }
  }

  mTaskCount = 0;
  mCurrentIndex = 0;
}

void RenderTaskBuffer::AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color, float depth, uint stencil, uint stencilWriteMask)
{
  if (!ValidateRenderTargets(renderSettings))
    return;

  RenderTaskClearTarget* renderTask = NewRenderTask<RenderTaskClearTarget>();
  renderTask->mId = RenderTaskType::ClearTarget;
  renderTask->mRenderSettings = renderSettings;
  renderTask->mColor = color;
  renderTask->mDepth = depth;
  renderTask->mStencil = stencil;
  renderTask->mStencilWriteMask = stencilWriteMask;

  ++mTaskCount;
}

void RenderTaskBuffer::AddRenderTaskRenderPass(RenderSettings& renderSettings, uint renderGroupIndex, StringParam renderPassName, uint shaderInputsId)
{
  if (!ValidateRenderTargets(renderSettings))
    return;

  RenderTaskRenderPass* renderTask = NewRenderTask<RenderTaskRenderPass>();
  renderTask->mId = RenderTaskType::RenderPass;
  renderTask->mRenderSettings = renderSettings;
  renderTask->mRenderGroupIndex = renderGroupIndex;
  renderTask->mRenderPassName = renderPassName;
  renderTask->mShaderInputsId = shaderInputsId;

  ++mTaskCount;
}

void RenderTaskBuffer::AddRenderTaskPostProcess(RenderSettings& renderSettings, StringParam postProcessName, uint shaderInputsId)
{
  if (!ValidateRenderTargets(renderSettings))
    return;

  RenderTaskPostProcess* renderTask = NewRenderTask<RenderTaskPostProcess>();
  renderTask->mId = RenderTaskType::PostProcess;
  renderTask->mRenderSettings = renderSettings;
  renderTask->mPostProcessName = postProcessName;
  renderTask->mMaterialRenderData = nullptr;
  renderTask->mShaderInputsId = shaderInputsId;

  ++mTaskCount;
}

void RenderTaskBuffer::AddRenderTaskPostProcess(RenderSettings& renderSettings, MaterialRenderData* materialRenderData, uint shaderInputsId)
{
  if (!ValidateRenderTargets(renderSettings))
    return;

  RenderTaskPostProcess* renderTask = NewRenderTask<RenderTaskPostProcess>();
  renderTask->mId = RenderTaskType::PostProcess;
  renderTask->mRenderSettings = renderSettings;
  renderTask->mPostProcessName = String();
  renderTask->mMaterialRenderData = materialRenderData;
  renderTask->mShaderInputsId = shaderInputsId;

  ++mTaskCount;
}

void RenderTaskBuffer::AddRenderTaskBackBufferBlit(RenderTarget* colorTarget, ScreenViewport viewport)
{
  // Don't need to validate render target, only used internally
  RenderTaskBackBufferBlit* renderTask = NewRenderTask<RenderTaskBackBufferBlit>();
  renderTask->mId = RenderTaskType::BackBufferBlit;
  Texture* texture = colorTarget->mTexture;
  renderTask->mColorTarget = texture->mRenderData;
  renderTask->mTextureWidth = texture->mWidth;
  renderTask->mTextureHeight = texture->mHeight;
  renderTask->mViewport = viewport;

  ++mTaskCount;
}

void RenderTaskBuffer::AddRenderTaskTextureUpdate(Texture* texture)
{
  RenderTaskTextureUpdate* renderTask = NewRenderTask<RenderTaskTextureUpdate>();
  renderTask->mId = RenderTaskType::TextureUpdate;
  renderTask->mRenderData = texture->mRenderData;
  renderTask->mWidth = texture->mWidth;
  renderTask->mHeight = texture->mHeight;
  renderTask->mType = texture->mType;
  renderTask->mFormat = texture->mFormat;
  renderTask->mAddressingX = texture->mAddressingX;
  renderTask->mAddressingY = texture->mAddressingY;
  renderTask->mFiltering = texture->mFiltering;
  renderTask->mCompareMode = texture->mCompareMode;
  renderTask->mCompareFunc = texture->mCompareFunc;
  renderTask->mAnisotropy = texture->mAnisotropy;
  renderTask->mMipMapping = texture->mMipMapping;

  texture->mDirty = false;

  ++mTaskCount;
}

bool RenderTaskBuffer::ValidateRenderTargets(RenderSettings& renderSettings)
{
  static Array<IntVec2> sizes;
  sizes.Clear();

  uint targetCount = renderSettings.mSingleColorTarget ? 1 : 8;

  // Format validations
  for (uint i = 0; i < targetCount; ++i)
  {
    Texture* texture = renderSettings.mColorTextures[i];
    if (texture != nullptr)
    {
      if (IsColorFormat(texture->mFormat))
        sizes.PushBack(texture->GetSize());
      else
        return DoNotifyException("Error", "Invalid color attachment."), false;
    }
  }

  Texture* depthTexture = renderSettings.mDepthTexture;
  if (depthTexture != nullptr)
  {
    if (IsDepthFormat(depthTexture->mFormat))
      sizes.PushBack(depthTexture->GetSize());
    else
      return DoNotifyException("Error", "Invalid depth attachment."), false;
  }

  // Size validations
  if (sizes.Empty())
    return DoNotifyException("Error", "No RenderTargets."), false;

  for (uint i = 1; i < sizes.Size(); ++i)
    if (sizes[i] != sizes[i - 1])
      return DoNotifyException("Error", "Mismatching RenderTarget sizes."), false;

  renderSettings.mTargetsWidth = (uint)sizes[0].x;
  renderSettings.mTargetsHeight = (uint)sizes[0].y;

  // Add render tasks for dirty textures
  for (uint i = 0; i < targetCount; ++i)
  {
    Texture* texture = renderSettings.mColorTextures[i];
    if (texture != nullptr && texture->mDirty)
      AddRenderTaskTextureUpdate(texture);
  }

  if (renderSettings.mDepthTexture && renderSettings.mDepthTexture->mDirty)
    AddRenderTaskTextureUpdate(renderSettings.mDepthTexture);

  return true;
}

bool RenderTaskRange::operator<(const RenderTaskRange& other) const
{
  return mRenderOrder < other.mRenderOrder;
}


//------------------------------------------------------------- RenderTasksEvent
ZilchDefineType(RenderTasksEvent, builder, type)
{
  ZeroBindEvent(Events::RenderTasksUpdate, RenderTasksEvent);

  ZilchBindGetter(ViewportSize);
  ZilchBindGetter(CameraViewportCog);

  ZilchBindOverloadedMethod(GetRenderTarget, ZilchInstanceOverload(HandleOf<RenderTarget>, IntVec2, TextureFormat::Enum));
  ZilchBindOverloadedMethod(GetRenderTarget, ZilchInstanceOverload(HandleOf<RenderTarget>, IntVec2, TextureFormat::Enum, SamplerSettings&));
  ZilchBindOverloadedMethod(GetRenderTarget, ZilchInstanceOverload(HandleOf<RenderTarget>, HandleOf<Texture>));

  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderTarget*, Vec4));
  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderTarget*, float));
  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderTarget*, float, uint));
  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderTarget*, float, uint, uint));
  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderTarget*, RenderTarget*, Vec4, float));
  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderTarget*, RenderTarget*, Vec4, float, uint));
  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderTarget*, RenderTarget*, Vec4, float, uint, uint));
  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderSettings&, Vec4));
  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderSettings&, Vec4, float));
  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderSettings&, Vec4, float, uint));
  ZilchBindOverloadedMethod(AddRenderTaskClearTarget, ZilchInstanceOverload(void, RenderSettings&, Vec4, float, uint, uint));

  ZilchBindOverloadedMethod(AddRenderTaskRenderPass, ZilchInstanceOverload(void, RenderSettings&, RenderGroup&, MaterialBlock&));
  ZilchBindOverloadedMethod(AddRenderTaskRenderPass, ZilchInstanceOverload(void, RenderSettings&, GraphicalRangeInterface&, MaterialBlock&));

  ZilchBindOverloadedMethod(AddRenderTaskPostProcess, ZilchInstanceOverload(void, RenderTarget*, Material&));
  ZilchBindOverloadedMethod(AddRenderTaskPostProcess, ZilchInstanceOverload(void, RenderTarget*, MaterialBlock&));
  ZilchBindOverloadedMethod(AddRenderTaskPostProcess, ZilchInstanceOverload(void, RenderSettings&, Material&));
  ZilchBindOverloadedMethod(AddRenderTaskPostProcess, ZilchInstanceOverload(void, RenderSettings&, MaterialBlock&));

  ZilchBindOverloadedMethod(GetFinalTarget, ZilchInstanceOverload(HandleOf<RenderTarget>, IntVec2, TextureFormat::Enum));
  ZilchBindOverloadedMethod(GetFinalTarget, ZilchInstanceOverload(HandleOf<RenderTarget>, IntVec2, TextureFormat::Enum, SamplerSettings&));
}

RenderTasksEvent::RenderTasksEvent()
  : mViewportSize(IntVec2::cZero)
  , mRenderTasks(nullptr)
  , mFinalTexture(nullptr)
  , mGraphicsSpace(nullptr)
  , mCamera(nullptr)
{
}

Cog* RenderTasksEvent::GetCameraViewportCog()
{
  return mCameraViewportCog;
}

IntVec2 RenderTasksEvent::GetViewportSize()
{
  return mViewportSize;
}

HandleOf<RenderTarget> RenderTasksEvent::GetRenderTarget(IntVec2 size, TextureFormat::Enum format)
{
  SamplerSettings samplerSettings;
  return GetRenderTarget(size, format, samplerSettings);
}

HandleOf<RenderTarget> RenderTasksEvent::GetRenderTarget(IntVec2 size, TextureFormat::Enum format, SamplerSettings& samplerSettings)
{
  // need to determine a good size limit
  size = Math::Clamp(size, IntVec2(1, 1), IntVec2(4096, 4096));
  return Z::gEngine->has(GraphicsEngine)->GetRenderTarget((uint)size.x, (uint)size.y, format, samplerSettings);
}

HandleOf<RenderTarget> RenderTasksEvent::GetRenderTarget(HandleOf<Texture> texture)
{
  return Z::gEngine->has(GraphicsEngine)->GetRenderTarget(texture);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* colorTarget, Vec4 color)
{
  RenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  AddRenderTaskClearTarget(renderSettings, color, 0.0f, 0, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* depthTarget, float depth)
{
  RenderSettings renderSettings;
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, Vec4::cZero, depth, 0, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* depthTarget, float depth, uint stencil)
{
  RenderSettings renderSettings;
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, Vec4::cZero, depth, stencil, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* depthTarget, float depth, uint stencil, uint stencilWriteMask)
{
  RenderSettings renderSettings;
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, Vec4::cZero, depth, stencil, stencilWriteMask);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* colorTarget, RenderTarget* depthTarget, Vec4 color, float depth)
{
  RenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, color, depth, 0, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* colorTarget, RenderTarget* depthTarget, Vec4 color, float depth, uint stencil)
{
  RenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, color, depth, stencil, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderTarget* colorTarget, RenderTarget* depthTarget, Vec4 color, float depth, uint stencil, uint stencilWriteMask)
{
  RenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  renderSettings.SetDepthTarget(depthTarget);
  AddRenderTaskClearTarget(renderSettings, color, depth, stencil, stencilWriteMask);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color)
{
  AddRenderTaskClearTarget(renderSettings, color, 0, 0, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color, float depth)
{
  AddRenderTaskClearTarget(renderSettings, color, depth, 0, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color, float depth, uint stencil)
{
  AddRenderTaskClearTarget(renderSettings, color, depth, stencil, 0xFF);
}

void RenderTasksEvent::AddRenderTaskClearTarget(RenderSettings& renderSettings, Vec4 color, float depth, uint stencil, uint stencilWriteMask)
{
  mRenderTasks->mRenderTaskBuffer.AddRenderTaskClearTarget(renderSettings, color, depth, stencil, stencilWriteMask);
}

void RenderTasksEvent::AddRenderTaskRenderPass(RenderSettings& renderSettings, RenderGroup& renderGroup, MaterialBlock& renderPass)
{
  ZilchFragmentType::Enum fragmentType = Z::gEngine->has(GraphicsEngine)->GetFragmentType(&renderPass);
  if (fragmentType != ZilchFragmentType::RenderPass)
    return DoNotifyException("Error", "Fragment is not a [RenderPass]");

  uint shaderInputsId = GetUniqueShaderInputsId();

  AddShaderInputs(&renderPass, shaderInputsId);
  forRange (Material* material, renderGroup.mActiveResources.All())
    AddShaderInputs(material, shaderInputsId);

  AddShaderInputs(renderSettings.mGlobalShaderInputs, shaderInputsId);

  String renderPassName = ZilchVirtualTypeId(&renderPass)->Name;
  mRenderTasks->mRenderTaskBuffer.AddRenderTaskRenderPass(renderSettings, renderGroup.mSortId, renderPassName, shaderInputsId);
}

void RenderTasksEvent::AddRenderTaskRenderPass(RenderSettings& renderSettings, GraphicalRangeInterface& graphicalRange, MaterialBlock& renderPass)
{
  ZilchFragmentType::Enum fragmentType = Z::gEngine->has(GraphicsEngine)->GetFragmentType(&renderPass);
  if (fragmentType != ZilchFragmentType::RenderPass)
    return DoNotifyException("Error", "Fragment is not a [RenderPass]");

  uint count = graphicalRange.mGraphicals.Size();
  if (count == 0)
    return;

  HashSet<Material*> materials;

  // Adding to render group counts will make the camera setup a range for nodes that will end up in view block
  // The indexes of this array correspond to render group id's, adding to it acts as a custom render group
  mCamera->mRenderGroupCounts.PushBack(count);
  uint groupId = mCamera->mRenderGroupCounts.Size() - 1;

  // Need to add an index range for this set of entries
  IndexRange indexRange;
  indexRange.start = mGraphicsSpace->mVisibleGraphicals.Size();

  forRange (Graphical* graphical, graphicalRange.mGraphicals.All())
  {
    // Check if graphical is from a different space
    if (graphical->GetSpace() != mGraphicsSpace->GetOwner())
      continue;

    // No sort values are needed, these entries are to be rendered in the order given
    graphical->MidPhaseQuery(mGraphicsSpace->mVisibleGraphicals, *mCamera, nullptr);
    materials.Insert(graphical->mMaterial);
  }

  indexRange.end = mGraphicsSpace->mVisibleGraphicals.Size();
  mCamera->mGraphicalIndexRanges.PushBack(indexRange);

  uint shaderInputsId = GetUniqueShaderInputsId();

  AddShaderInputs(&renderPass, shaderInputsId);
  forRange (Material* material, materials.All())
    AddShaderInputs(material, shaderInputsId);

  AddShaderInputs(renderSettings.mGlobalShaderInputs, shaderInputsId);

  String renderPassName = ZilchVirtualTypeId(&renderPass)->Name;
  mRenderTasks->mRenderTaskBuffer.AddRenderTaskRenderPass(renderSettings, groupId, renderPassName, shaderInputsId);
}

void RenderTasksEvent::AddRenderTaskPostProcess(RenderTarget* colorTarget, Material& material)
{
  RenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  AddRenderTaskPostProcess(renderSettings, material);
}

void RenderTasksEvent::AddRenderTaskPostProcess(RenderTarget* colorTarget, MaterialBlock& postProcess)
{
  RenderSettings renderSettings;
  renderSettings.SetColorTarget(colorTarget);
  AddRenderTaskPostProcess(renderSettings, postProcess);
}

void RenderTasksEvent::AddRenderTaskPostProcess(RenderSettings& renderSettings, Material& material)
{
  uint shaderInputsId = GetUniqueShaderInputsId();
  AddShaderInputs(&material, shaderInputsId);

  AddShaderInputs(renderSettings.mGlobalShaderInputs.Dereference(), shaderInputsId);

  mRenderTasks->mRenderTaskBuffer.AddRenderTaskPostProcess(renderSettings, material.mRenderData, shaderInputsId);
}

void RenderTasksEvent::AddRenderTaskPostProcess(RenderSettings& renderSettings, MaterialBlock& postProcess)
{
  ZilchFragmentType::Enum fragmentType = Z::gEngine->has(GraphicsEngine)->GetFragmentType(&postProcess);
  if (fragmentType != ZilchFragmentType::PostProcess)
    return DoNotifyException("Error", "Fragment is not a [PostProcess]");

  uint shaderInputsId = GetUniqueShaderInputsId();
  AddShaderInputs(&postProcess, shaderInputsId);

  AddShaderInputs(renderSettings.mGlobalShaderInputs, shaderInputsId);

  String postProcessName = ZilchVirtualTypeId(&postProcess)->Name;
  mRenderTasks->mRenderTaskBuffer.AddRenderTaskPostProcess(renderSettings, postProcessName, shaderInputsId);
}

void RenderTasksEvent::AddRenderTaskBackBufferBlit(RenderTarget* colorTarget, ScreenViewport viewport)
{
  mRenderTasks->mRenderTaskBuffer.AddRenderTaskBackBufferBlit(colorTarget, viewport);
}

HandleOf<RenderTarget> RenderTasksEvent::GetFinalTarget(IntVec2 size, TextureFormat::Enum format)
{
  SamplerSettings samplerSettings;
  return GetFinalTarget(size, format, samplerSettings);
}

HandleOf<RenderTarget> RenderTasksEvent::GetFinalTarget(IntVec2 size, TextureFormat::Enum format, SamplerSettings& samplerSettings)
{
  size = Math::Clamp(size, IntVec2(1, 1), IntVec2(4096, 4096));

  // Allow changes to CameraViewport's target through this method
  Texture* texture = mFinalTexture;
  texture->mProtected = false;
  texture->SetSize(size);
  texture->SetFormat(format);
  texture->SetAddressingX(samplerSettings.mAddressingX);
  texture->SetAddressingY(samplerSettings.mAddressingY);
  texture->SetFiltering(samplerSettings.mFiltering);
  texture->SetCompareMode(samplerSettings.mCompareMode);
  texture->SetCompareFunc(samplerSettings.mCompareFunc);

  HandleOf<RenderTarget> renderTarget = GetRenderTarget(mFinalTexture);
  texture->mProtected = true;

  return renderTarget;
}

uint RenderTasksEvent::GetUniqueShaderInputsId()
{
  static uint sIncrementalShaderInputsId = 0;
  return ++sIncrementalShaderInputsId;
}

void RenderTasksEvent::AddShaderInputs(Material* material, uint shaderInputsId)
{
  Pair<u64, uint> key((u64)material->mResourceId, shaderInputsId);
  IndexRange range = material->AddShaderInputs(mRenderTasks->mShaderInputs, mRenderTasks->mShaderInputsVersion);
  mRenderTasks->mShaderInputRanges.Insert(key, range);
}

void RenderTasksEvent::AddShaderInputs(MaterialBlock* materialBlock, uint shaderInputsId)
{
  Pair<u64, uint> key(cFragmentShaderInputsId, shaderInputsId);
  IndexRange range = materialBlock->AddShaderInputs(mRenderTasks->mShaderInputs);
  mRenderTasks->mShaderInputRanges.Insert(key, range);
}

void RenderTasksEvent::AddShaderInputs(ShaderInputs* globalInputs, uint shaderInputsId)
{
  if (globalInputs == nullptr)
    return;

  uint start = mRenderTasks->mShaderInputs.Size();
  mRenderTasks->mShaderInputs.Append(globalInputs->mShaderInputs.Values());
  uint end = mRenderTasks->mShaderInputs.Size();

  Pair<u64, uint> key(cGlobalShaderInputsId, shaderInputsId);
  IndexRange range(start, end);
  mRenderTasks->mShaderInputRanges.Insert(key, range);
}

void RenderTasks::Clear()
{
  mRenderTaskRanges.Clear();
  mRenderTaskBuffer.Clear();

  mShaderInputRanges.Clear();
  mShaderInputs.Clear();
}

void RenderTasksUpdateHelper(RenderTasksUpdateData& update)
{
  Array<RenderTaskRange>& renderTaskRanges = update.mEvent->mRenderTasks->mRenderTaskRanges;
  RenderTaskBuffer& renderTaskBuffer = update.mEvent->mRenderTasks->mRenderTaskBuffer;

  RenderTaskRange range;
  range.mTaskIndex = renderTaskBuffer.mCurrentIndex;

  uint startingTaskCount = renderTaskBuffer.mTaskCount;

  // get tasks from renderer script
  update.mDispatcher->Dispatch(Events::RenderTasksUpdate, update.mEvent);

  range.mTaskCount = renderTaskBuffer.mTaskCount - startingTaskCount;

  // check for render tasks and texture output
  if (range.mTaskCount == 0)
  {
    // undo the added render tasks that aren't going to be used
    renderTaskBuffer.mCurrentIndex = range.mTaskIndex;
    renderTaskBuffer.mTaskCount -= range.mTaskCount;
    return;
  }

  update.mTaskCount = range.mTaskCount;

  range.mRenderOrder = update.mRenderOrder;
  renderTaskRanges.PushBack(range);
  uint rangeIndex = renderTaskRanges.Size() - 1;

  // signal to the camera that its data is needed for the RenderQueues
  update.mCamera->mRenderQueuesDataNeeded = true;

  // tell GraphicsSpace and the Camera the index of the RenderTasks object so that
  // they can later tell the RenderTasks what the FrameBlock and ViewBlock index is
  update.mGraphicsSpace->mRenderTaskRangeIndices.PushBack(rangeIndex);
  update.mCamera->mRenderTaskRangeIndices.PushBack(rangeIndex);
}

} // namespace Zero
