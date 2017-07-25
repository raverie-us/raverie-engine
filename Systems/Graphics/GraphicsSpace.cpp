#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(UpdateActiveCameras);
  DefineEvent(UpdateSkeletons);
}

//---------------------------------------------------------------- GraphicsSpace
ZilchDefineType(GraphicsSpace, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  type->AddAttribute(ObjectAttributes::cCore);

  ZeroBindDependency(Space);

  ZilchBindFieldProperty(mActive);
  ZilchBindFieldProperty(mRandomSeed)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mSeed)->ZeroFilterEquality(mRandomSeed, bool, false);
}

void GraphicsSpace::Serialize(Serializer& stream)
{
  SerializeNameDefault(mActive, true);
  SerializeNameDefault(mRandomSeed, false);
  SerializeNameDefault(mSeed, 0u);
}

void GraphicsSpace::Initialize(CogInitializer& initializer)
{
  mFrameTime = 0.0f;
  mLogicTime = 0.0f;

  for (uint i = 0; i < 8; ++i)
    mDebugDrawGraphicals[i] = nullptr;

  if (!mRandomSeed)
    mRandom.SetSeed(mSeed);

  mGraphicsEngine = Z::gEngine->has(GraphicsEngine);
  mGraphicsEngine->AddSpace(this);

  ConnectThisTo(this, Events::SpaceDestroyed, OnSpaceDestroyed);
  ConnectThisTo(this, Events::SystemLogicUpdate, OnLogicUpdate);
  //ConnectThisTo(GetOwner(), Events::GraphicsFrameUpdate, OnFrameUpdate);
}

void GraphicsSpace::OnSpaceDestroyed(ObjectEvent* event)
{
  mGraphicsEngine->RemoveSpace(this);
}

void GraphicsSpace::AddGraphical(Graphical* graphical)
{
  mGraphicalsToAdd.PushBack(graphical);
}

void GraphicsSpace::AddDebugGraphical(Graphical* graphical)
{
  mDebugGraphicals.PushBack(graphical);
}

void GraphicsSpace::RemoveGraphical(Graphical* graphical)
{
  QueueVisibilityEvents(*graphical);
  SendVisibilityEvents();

  GraphicalList::Unlink(graphical);
  if (graphical->mProxy.ToVoidPointer() != nullptr)
  {
    mBroadPhase.RemoveProxy(graphical->mProxy);
    graphical->mProxy = BroadPhaseProxy();
  }
}

void GraphicsSpace::AddCamera(Camera* camera)
{
  mCameras.PushBack(camera);
  RegisterVisibility(camera);
}

void GraphicsSpace::RemoveCamera(Camera* camera)
{
  CameraList::Unlink(camera);
  UnregisterVisibility(camera);
}

void GraphicsSpace::OnLogicUpdate(UpdateEvent* event)
{
  mLogicTime += event->Dt;
}

// currently considering keeping this as a part of graphics update and not frame update
void GraphicsSpace::OnFrameUpdate(float frameDt)
{
  Event event;
  // Tells CameraViewports to resolve active camera objects if anything changed
  DispatchEvent(Events::UpdateActiveCameras, &event);
  // Tells Skeletons to rebuild if their bone structure changed
  DispatchEvent(Events::UpdateSkeletons, &event);

  mFrameTime += frameDt;

  ZeroForRangeVar (Graphical& graphical, graphicalRange, mGraphicalsToAdd.All())
  {
    GraphicalList::Unlink(&graphical);

    if (graphical.mVisible == false)
      mGraphicalsAlwaysCulled.PushBack(&graphical);
    else if (graphical.mViewCulling == false)
      mGraphicalsNeverCulled.PushBack(&graphical);
    else
    {
      mGraphicals.PushBack(&graphical);
      BaseBroadPhaseData<Graphical*> data;
      data.mClientData = &graphical;
      data.mAabb = graphical.GetWorldAabb();
      mBroadPhase.CreateProxy(graphical.mProxy, data);
    }
  }

  CreateDebugGraphicals();

  mVisibleGraphicals.Clear();
  uint lastIndex = 0;

  uint renderGroupCount = mGraphicsEngine->GetRenderGroupCount();
  ErrorIf(renderGroupCount == 0, "No render groups, core resources must be missing.");

  // for each view object in use
  forRange (Camera& camera, mCameras.All())
  {
    // Ranges must be cleared from the last this camera was used
    // Must be cleared before RenderTasks event is sent out
    // because render pass tasks can add to this array
    camera.mGraphicalIndexRanges.Clear();

    // resize to number of render types
    camera.mRenderGroupCounts.Resize(renderGroupCount);
    for (uint i = 0; i < camera.mRenderGroupCounts.Size(); ++i)
      camera.mRenderGroupCounts[i] = 0;

    Vec3 cameraPos = camera.mTransform->GetWorldTranslation();
    Mat3 rotation = Math::ToMatrix3(camera.mTransform->GetWorldRotation());
    Vec3 cameraDir = -rotation.BasisZ();

    Frustum frustum = camera.GetFrustum(camera.mViewportInterface->GetAspectRatio());

    // Visibility culled graphicals
    forRangeBroadphaseTree (GraphicsBroadPhase, mBroadPhase, Frustum, frustum)
      AddToVisibleGraphicals(*range.Front(), camera, cameraPos, cameraDir, &frustum);

    // Not culled
    forRange (Graphical& graphical, mGraphicalsNeverCulled.All())
      AddToVisibleGraphicals(graphical, camera, cameraPos, cameraDir);

    // Get DebugGraphical entries, not broadphased
    // DebugGraphicals exist for one frame and are not placed in broadphase
    forRange (Graphical& graphical, mDebugGraphicals.All())
    {
      DebugGraphical* debugGraphical = (DebugGraphical*)&graphical;
      if (debugGraphical->mDebugObjects.Size() == 0)
        continue;

      AddToVisibleGraphicals(graphical, camera, cameraPos, cameraDir);
    }

    uint index = mVisibleGraphicals.Size();
    IndexRange indexRange(lastIndex, index);
    lastIndex = index;

    uint start = indexRange.start;
    uint size = indexRange.end - indexRange.start;

    camera.mGraphicalIndexRanges.PushBack(indexRange);

    // Sort entries of this camera
    // This sort will have all entries correctly organized by RenderGroup
    // If a custom sort is enabled, it can then be re-sorted within that RenderGroup
    Sort(mVisibleGraphicals.SubRange(start, size));

    // Check for any RenderGroup with a custom sort and find its range of elements
    for (uint i = 0, rangeStart = 0; i < camera.mRenderGroupCounts.Size(); ++i)
    {
      uint rangeEnd = rangeStart + camera.mRenderGroupCounts[i];

      RenderGroup* renderGroup = (RenderGroup*)mGraphicsEngine->mRenderGroups[i];
      if (renderGroup->mGraphicalSortMethod == GraphicalSortMethod::SortEvent)
      {
        GraphicalSortEvent sortEvent;
        sortEvent.mGraphicalEntries = mVisibleGraphicals.SubRange(rangeStart, rangeEnd - rangeStart);
        sortEvent.mRenderGroup = renderGroup;
        camera.mViewportInterface->SendSortEvent(&sortEvent);
        Sort(mVisibleGraphicals.SubRange(rangeStart, rangeEnd - rangeStart));
      }

      rangeStart = rangeEnd;
    }
  }

  QueueVisibilityEvents(mGraphicals);
  QueueVisibilityEvents(mGraphicalsNeverCulled);
  QueueVisibilityEvents(mGraphicalsAlwaysCulled);

  SendVisibilityEvents();

  forRange (Camera& camera, mRemovedCameras.All())
    UnregisterVisibility(&camera);
  mRemovedCameras.Clear();
}

void GraphicsSpace::RenderTasksUpdate(RenderTasks& renderTasks)
{
  RenderTasksEvent event;
  event.mRenderTasks = &renderTasks;
  event.mGraphicsSpace = this;
  DispatchEvent(Events::RenderTasksUpdateInternal, &event);
}

void GraphicsSpace::RenderQueuesUpdate(RenderTasks& renderTasks, RenderQueues& renderQueues)
{
  FrameBlock& frameBlock = renderQueues.mFrameBlocks.PushBack();
  frameBlock.mRenderQueues = &renderQueues;
  Array<FrameNode>& frameNodes = frameBlock.mFrameNodes;

  // link RenderTasks to FrameBlock
  uint frameBlockIndex = renderQueues.mFrameBlocks.Size() - 1;
  for (uint i = 0; i < mRenderTaskRangeIndices.Size(); ++i)
    renderTasks.mRenderTaskRanges[mRenderTaskRangeIndices[i]].mFrameBlockIndex = frameBlockIndex;
  mRenderTaskRangeIndices.Clear();

  // get frame block data
  frameBlock.mFrameTime = mFrameTime;
  frameBlock.mLogicTime = mLogicTime;

  uint viewBlockStartIndex = renderQueues.mViewBlocks.Size();

  // for each view object
  forRange (Camera& camera, mCameras.All())
  {
    // if no render tasks associated with this camera, do nothing
    if (!camera.mRenderQueuesDataNeeded)
      continue;
    camera.mRenderQueuesDataNeeded = false;

    // add view block
    ViewBlock& viewBlock = renderQueues.mViewBlocks.PushBack();

    // link RenderTasks to ViewBlock
    uint viewBlockIndex = renderQueues.mViewBlocks.Size() - 1;
    for (uint i = 0; i < camera.mRenderTaskRangeIndices.Size(); ++i)
      renderTasks.mRenderTaskRanges[camera.mRenderTaskRangeIndices[i]].mViewBlockIndex = viewBlockIndex;
    camera.mRenderTaskRangeIndices.Clear();

    // get view block data
    camera.GetViewData(viewBlock);

    uint totalCounts = 0;
    // setup ranges for accessing render groups
    for (uint i = 0, rangeStart = 0; i < camera.mRenderGroupCounts.Size(); ++i)
    {
      totalCounts += camera.mRenderGroupCounts[i];

      uint rangeEnd = rangeStart + camera.mRenderGroupCounts[i];
      viewBlock.mRenderGroupRanges.PushBack(IndexRange(rangeStart, rangeEnd));
      rangeStart = rangeEnd;
    }

    // allocate view nodes
    viewBlock.mViewNodes.Reserve(totalCounts);

    // make nodes for every graphical entry
    forRange (IndexRange& indexRange, camera.mGraphicalIndexRanges.All())
    {
      uint start = indexRange.start;
      uint size = indexRange.end - indexRange.start;
      Array<GraphicalEntry>::range graphicals = mVisibleGraphicals.SubRange(start, size);

      // assign references to graphicals in view nodes
      forRange (GraphicalEntry& entry, graphicals)
      {
        Graphical* graphical = entry.mData->mGraphical;
        ViewNode& viewNode = viewBlock.mViewNodes.PushBack();
        viewNode.mGraphicalEntry = &entry;

        // no frame node made for this entry yet
        if (entry.mData->mFrameNodeIndex == -1)
        {
          FrameNode& frameNode = frameNodes.PushBack();
          frameNode.mGraphicalEntry = &entry;
          entry.mData->mFrameNodeIndex = frameNodes.Size() - 1;

          // per object shader input overrides
          frameNode.mShaderInputRange.start = renderTasks.mShaderInputs.Size();

          // from meta properties
          forRange (PropertyShaderInput& input, graphical->mPropertyShaderInputs.All())
          {
            input.mShaderInput.SetValue(input.mMetaProperty->GetValue(input.mComponent));
            renderTasks.mShaderInputs.PushBack(input.mShaderInput);
          }

          // from the graphical interface
          if (ShaderInputs* shaderInputs = graphical->GetShaderInputs())
            renderTasks.mShaderInputs.Append(shaderInputs->mShaderInputs.Values());

          frameNode.mShaderInputRange.end = renderTasks.mShaderInputs.Size();
        }

        // assign references to frame nodes in view nodes
        viewNode.mFrameNodeIndex = entry.mData->mFrameNodeIndex;
      }
    }
  }

  // extract frame node data
  forRange (FrameNode& node, frameNodes.All())
  {
    node.Extract(frameBlock);
  }

  // only process view blocks from this graphics space
  for (uint i = viewBlockStartIndex; i < renderQueues.mViewBlocks.Size(); ++i)
  {
    // extract view node data
    ViewBlock& viewBlock = renderQueues.mViewBlocks[i];
    forRange (ViewNode& node, viewBlock.mViewNodes.All())
    {
      node.Extract(viewBlock, frameBlock);
    }
  }
}

void GraphicsSpace::AddToVisibleGraphicals(Graphical& graphical, Camera& camera, Vec3 cameraPos, Vec3 cameraDir, Frustum* frustum)
{
  if (GetOwner()->IsEditorMode() && graphical.GetOwner()->GetEditorViewportHidden())
    return;

  if (graphical.GetOwner()->GetMarkedForDestruction())
    return;

  graphical.mVisibleFlags.SetFlag(camera.mVisibilityId);

  Array<GraphicalEntry> entries;
  graphical.MidPhaseQuery(entries, camera, frustum);
  forRange (GraphicalEntry& entry, entries.All())
  {
    Vec3 pos = entry.mData->mPosition;
    // Make entry for each RenderGroup associated with this graphical's material
    // Can maybe skip some RenderGroups if we know that no render passes use them
    forRange (RenderGroup* renderGroup, graphical.mMaterial->mActiveResources.All())
    {
      entry.SetRenderGroupSortValue(renderGroup->mSortId);
      s32 graphicalSortValue = GetGraphicalSortValue(graphical, renderGroup->mGraphicalSortMethod, pos, cameraPos, cameraDir);
      entry.SetGraphicalSortValue(graphicalSortValue);

      // Materials will not refer to RenderGroups that have not been given an id
      mVisibleGraphicals.PushBack(entry);
      // Add to RenderGroup counters so they can be accessed by index later
      ++camera.mRenderGroupCounts[renderGroup->mSortId];
    }
  }
}

void GraphicsSpace::CreateDebugGraphicals()
{
  if (mDebugDrawGraphicals[0] == nullptr)
  {
    // temporary until transient object creation support
    bool spaceModified = GetSpace()->GetModified();

    // indexes 0-1 are for wireframe shapes, 2-3 are for filled shapes,
    // 4-5 are for thick-lines(borders), and 6-7 are for text
    //   - note: indexes 1, 3, 5, 7 are for on-top objects
    for (uint i = 0; i < 8; ++i)
    {
      Cog* cog = GetSpace()->Create(ArchetypeManager::Find(CoreArchetypes::Transform));
      cog->mFlags.SetFlag(CogFlags::Transient | CogFlags::Persistent | CogFlags::ObjectViewHidden);
      if (i < 4)
      {
        mDebugDrawGraphicals[i] = HasOrAdd<DebugGraphicalPrimitive>(cog);

        if (i < 2)
          mDebugDrawGraphicals[i]->mPrimitiveType = PrimitiveType::Lines;
        else
          mDebugDrawGraphicals[i]->mPrimitiveType = PrimitiveType::Triangles;
      }
      else if (i < 6)
      {
        mDebugDrawGraphicals[i] = HasOrAdd<DebugGraphicalThickLine>(cog);
        mDebugDrawGraphicals[i]->mPrimitiveType = PrimitiveType::Lines;
      }
      else
      {
        mDebugDrawGraphicals[i] = HasOrAdd<DebugGraphicalText>(cog);
        mDebugDrawGraphicals[i]->mPrimitiveType = PrimitiveType::Triangles;
      }

    }

    mDebugDrawGraphicals[0]->mMaterial = MaterialManager::Find("DebugDraw");
    mDebugDrawGraphicals[1]->mMaterial = MaterialManager::Find("DebugDrawOnTop");
    mDebugDrawGraphicals[2]->mMaterial = MaterialManager::Find("DebugDraw");
    mDebugDrawGraphicals[3]->mMaterial = MaterialManager::Find("DebugDrawOnTop");
    mDebugDrawGraphicals[4]->mMaterial = MaterialManager::Find("DebugDraw");
    mDebugDrawGraphicals[5]->mMaterial = MaterialManager::Find("DebugDrawOnTop");
    mDebugDrawGraphicals[6]->mMaterial = MaterialManager::Find("DebugDraw");
    mDebugDrawGraphicals[7]->mMaterial = MaterialManager::Find("DebugDrawOnTop");

    if (spaceModified == false)
      GetSpace()->MarkNotModified();
  }

  for (uint i = 0; i < 8; ++i)
    mDebugDrawGraphicals[i]->mDebugObjects.Clear();

  Debug::DebugDrawObjectArray::range debugObjects = gDebugDraw->GetDebugObjects(GetOwner()->GetId().Id);

  forRange (Debug::DebugDrawObjectAny& debugObject, debugObjects)
  {
    uint index = 0;

    if (debugObject->GetDebugType() == Debug::DebugType::Text)
      index = 6;
    else if(debugObject->IsSet(Debug::DebugDrawFlags::Border))
      index = 4;
    else if(debugObject->IsSet(Debug::DebugDrawFlags::Filled))
      index = 2;

    // OnTop is on odd indexes
    if (debugObject->IsSet(Debug::DebugDrawFlags::OnTop))
      index += 1;

    mDebugDrawGraphicals[index]->mDebugObjects.PushBack(debugObject);
  }
}

// void GraphicsSpace::CastAabb(Aabb& aabb, Array<Cog*>& cogs)
// {
//   forRangeBroadphaseTree(GraphicsBroadPhase,BroadPhase,Aabb,aabb)
//   {
//     cogs.PushBack(range.Front()->GetOwner());
//   }
// }

void GraphicsSpace::RegisterVisibility(Camera* camera)
{
  ErrorIf(camera->mVisibilityId <= VisibilityFlag::sMaxVisibilityId, "Visibility is already registered");

  uint visibilityId = 0;
  while (visibilityId <= VisibilityFlag::sMaxVisibilityId)
  {
    if (mRegisteredVisibility.CheckFlag(visibilityId) == false)
    {
      mRegisteredVisibility.SetFlag(visibilityId);
      mRegisteredVisibilityMap.Insert(visibilityId, camera);

      camera->mVisibilityId = visibilityId;
      return;
    }

    ++visibilityId;
  }

  ErrorIf(true, "Exceeded maximum visibility sets");
}

void GraphicsSpace::UnregisterVisibility(Camera* camera)
{
  uint visibilityId = camera->mVisibilityId;

  ErrorIf(visibilityId > VisibilityFlag::sMaxVisibilityId, "Camera was not registered for visibility");

  // Manually queue events for removed cameras
  QueueVisibilityEvents(mGraphicals, camera);
  QueueVisibilityEvents(mGraphicalsNeverCulled, camera);
  QueueVisibilityEvents(mGraphicalsAlwaysCulled, camera);
  SendVisibilityEvents();

  mRegisteredVisibility.ClearFlag(visibilityId);
  mRegisteredVisibilityMap.Erase(visibilityId);

  camera->mVisibilityId = (uint)-1;
}

void GraphicsSpace::QueueVisibilityEvents(Graphical& graphical)
{
  if (graphical.mVisibilityEvents == false)
    return;

  VisibilityEvent event;
  event.mVisibleObject = graphical.GetOwner();
  event.mViewingObject = nullptr;

  bool lastAny = graphical.mLastVisibleFlags.Any();
  bool currentAny = graphical.mVisibleFlags.Any();

  if (!lastAny && currentAny)
  {
    event.mName = Events::EnterViewAny;
    mVisibilityEvents.PushBack(event);
  }
  else if (lastAny && !currentAny)
  {
    event.mName = Events::ExitViewAll;
    mVisibilityEvents.PushBack(event);
  }

  forRange(uint visibilityId, graphical.mVisibleFlags.GetChangeRange(graphical.mLastVisibleFlags))
  {
    Camera* camera = mRegisteredVisibilityMap.FindValue(visibilityId, nullptr);
    if (!camera)
      continue;

    event.mViewingObject = camera->mViewportInterface->GetOwner();

    if (graphical.mVisibleFlags.CheckFlag(visibilityId))
      event.mName = Events::EnterView;
    else
      event.mName = Events::ExitView;

    mVisibilityEvents.PushBack(event);
  }

  graphical.mLastVisibleFlags = graphical.mVisibleFlags;
  graphical.mVisibleFlags.ClearAll();
}

void GraphicsSpace::QueueVisibilityEvents(GraphicalList& graphicals)
{
  forRange(Graphical& graphical, graphicals.All())
    QueueVisibilityEvents(graphical);
}

void GraphicsSpace::QueueVisibilityEvents(GraphicalList& graphicals, Camera* camera)
{
  uint visibilityId = camera->mVisibilityId;

  forRange (Graphical& graphical, graphicals.All())
  {
    if (graphical.mVisibilityEvents == false)
      continue;

    if (graphical.mLastVisibleFlags.CheckFlag(visibilityId))
    {
      VisibilityEvent event;
      event.mVisibleObject = graphical.GetOwner();
      event.mViewingObject = camera->mViewportInterface->GetOwner();
      event.mName = Events::ExitView;

      mVisibilityEvents.PushBack(event);
    }
  }
}

void GraphicsSpace::SendVisibilityEvents()
{
  GraphicalEvent graphicalEvent;

  forRange (VisibilityEvent& event, mVisibilityEvents.All())
  {
    graphicalEvent.mViewingObject = event.mViewingObject;
    event.mVisibleObject->DispatchEvent(event.mName, &graphicalEvent);
  }

  mVisibilityEvents.Clear();
}

} // namespace Zero
