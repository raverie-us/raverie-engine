///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
DefineEvent(PathFinderFinishedGeneric);
DefineEvent(PathFinderFinished);
DefineEvent(PathFinderGridFinished);
}

//**************************************************************************************************
static const float cSqrt1 = (float)sqrt(1);
static const float cSqrt2 = (float)sqrt(2);
static const float cSqrt3 = (float)sqrt(3);

//**************************************************************************************************
PathFinderGridNodeRange::PathFinderGridNodeRange(PathFinderAlgorithmGrid* grid, IntVec3Param center) :
  mGrid(grid),
  mCenter(center),
  mIndex(0),
  mCurrentCost(0),
  mCurrentIntVec3(IntVec3::cZero)
{
  PopUntilValid();
}

//**************************************************************************************************
bool PathFinderGridNodeRange::Empty() const
{
  // Fundamentally there are 26 spaces around a single cell (27 if you count the cell itself)
  // We skip the center cell automatically in PopUntilValid
  return mIndex >= 3 * 3 * 3;
}

//**************************************************************************************************
void PathFinderGridNodeRange::PopFront()
{
  ++mIndex;
  PopUntilValid();
}

//**************************************************************************************************
Pair<IntVec3, float> PathFinderGridNodeRange::Front() const
{
  return Pair<IntVec3, float>(mCurrentIntVec3, mCurrentCost);
}

//**************************************************************************************************
PathFinderGridNodeRange& PathFinderGridNodeRange::All()
{
  return *this;
}

//**************************************************************************************************
void PathFinderGridNodeRange::PopUntilValid()
{
  while (!Empty())
  {
    // Top             Middle          Bottom
    //  0 |  1 |  2  |  9 | 10 | 11  | 18 | 19 | 20
    // ------------- | ------------- | -------------
    //  3 |  4 |  5  | 12 | 13 | 14  | 21 | 22 | 23
    // ------------- | ------------- | -------------
    //  6 |  7 |  8  | 15 | 16 | 17  | 24 | 25 | 26

    // The index 13 is the center cell
    if (mIndex == 13)
    {
      ++mIndex;
      continue;
    }

    int dx = (mIndex % 3) - 1;
    int dy = ((mIndex / 3) % 3) - 1;
    int dz = (mIndex / (3 * 3)) - 1;

    int movement = Math::Abs(dx) + Math::Abs(dy) + Math::Abs(dz);

    if (!mGrid->mDiagonalMovement)
    {
      if (movement > 1)
      {
        ++mIndex;
        continue;
      }
    }

    mCurrentIntVec3 = mCenter + IntVec3(dx, dy, dz);

    switch (movement)
    {
    case 1: mCurrentCost = cSqrt1; break;
    case 2: mCurrentCost = cSqrt2; break;
    case 3: mCurrentCost = cSqrt3; break;
    default: Error("Invalid move cost");
    }

    PathFinderCell* cell = mGrid->mCells.FindPointer(mCurrentIntVec3);
    if (cell)
    {
      if (cell->mCollision)
      {
        ++mIndex;
        continue;
      }

      mCurrentCost += cell->mCost;
    }
    break;
  }
}

//**************************************************************************************************
PathFinderCell::PathFinderCell() :
  mCost(0.0f),
  mCollision(false)
{
}

//**************************************************************************************************
PathFinderAlgorithmGrid::PathFinderAlgorithmGrid() :
  mDiagonalMovement(true)
{
}

//**************************************************************************************************
PathFinderGridNodeRange PathFinderAlgorithmGrid::QueryNeighbors(IntVec3Param node)
{
  return PathFinderGridNodeRange(this, node);
}

//**************************************************************************************************
bool PathFinderAlgorithmGrid::QueryIsValid(IntVec3Param node)
{
  return !GetCollision(node);
}

//**************************************************************************************************
float PathFinderAlgorithmGrid::QueryHeuristic(IntVec3Param node, IntVec3Param goal)
{
  int dx = Math::Abs(node.x - goal.x);
  int dy = Math::Abs(node.y - goal.y);
  int dz = Math::Abs(node.z - goal.z);

  if (mDiagonalMovement)
  {
    int sorted[] = { dx, dy, dz };
    Zero::InsertionSort(sorted, sorted + 3, Zero::less<int>(), sorted);

    int max = sorted[2];
    int mid = sorted[1];
    int min = sorted[0];

    return cSqrt1 * (max - mid) + cSqrt2 * (mid - min) + cSqrt3 * min;
  }
  else
  {
    // Manhattan distance
    return float(dx + dy + dz);
  }
}

//**************************************************************************************************
void PathFinderAlgorithmGrid::SetCollision(IntVec3Param index, bool collision)
{
  if (collision)
  {
    mCells[index].mCollision = true;
  }
  else if (PathFinderCell* cell = mCells.FindPointer(index))
  {
    // As an optimization if the cell has no cost and no collision we can remove it
    if (cell->mCost == 0.0f)
      mCells.Erase(index);
    else
      cell->mCollision = false;
  }
}

//**************************************************************************************************
bool PathFinderAlgorithmGrid::GetCollision(IntVec3Param index)
{
  PathFinderCell* cell = mCells.FindPointer(index);
  if (!cell)
    return false;

  return cell->mCollision;
}

//**************************************************************************************************
void PathFinderAlgorithmGrid::SetCost(IntVec3Param index, float cost)
{
  if (cost != 0.0f)
  {
    mCells[index].mCost = cost;
  }
  else if (PathFinderCell* cell = mCells.FindPointer(index))
  {
    // As an optimization if the cell has no cost and no collision we can remove it
    if (cell->mCollision == false)
      mCells.Erase(index);
    else
      cell->mCost = 0.0f;
  }
}

//**************************************************************************************************
float PathFinderAlgorithmGrid::GetCost(IntVec3Param index)
{
  PathFinderCell* cell = mCells.FindPointer(index);
  if (!cell)
    return 0.0f;

  return cell->mCost;
}

//**************************************************************************************************
void PathFinderAlgorithmGrid::Clear()
{
  mCells.Clear();
}

//**************************************************************************************************
ZilchDefineType(PathFinder, builder, type)
{
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindEvent(Events::PathFinderFinished, PathFinderEvent<Vec3>);

  ZilchBindMethod(FindPath);
  ZilchBindMethod(FindPathThreaded);

  ZilchBindField(mMaxIterations);
}

//**************************************************************************************************
const int cDefaultMaxIterations = 100000;
PathFinder::PathFinder() :
  mMaxIterations(cDefaultMaxIterations)
{
}

//**************************************************************************************************
void PathFinder::Serialize(Serializer& stream)
{
  SerializeNameDefault(mMaxIterations, cDefaultMaxIterations);
}

//**************************************************************************************************
void PathFinder::Initialize(CogInitializer& initializer)
{
}

//**************************************************************************************************
HandleOf<ArrayClass<Vec3>> PathFinder::FindPath(Vec3Param worldStart, Vec3Param worldGoal)
{
  Array<Variant> path;
  Variant nodeKeyStart = WorldPositionToNodeKey(worldStart);
  Variant nodeKeyGoal = WorldPositionToNodeKey(worldGoal);
  FindPathGeneric(nodeKeyStart, nodeKeyGoal, path);

  HandleOf<ArrayClass<Vec3>> array = ZilchAllocate(ArrayClass<Vec3>);
  ArrayClass<Vec3>& arrayRef = array;

  for (size_t i = 0; i < path.Size(); ++i)
    arrayRef.NativeArray.PushBack(NodeKeyToWorldPosition(path[i]));

  return array;
}

//**************************************************************************************************
HandleOf<PathFinderRequest> PathFinder::FindPathThreaded(Vec3Param worldStart, Vec3Param worldGoal)
{
  Array<Variant> path;
  Variant nodeKeyStart = WorldPositionToNodeKey(worldStart);
  Variant nodeKeyGoal = WorldPositionToNodeKey(worldGoal);
  return FindPathGenericThreaded(nodeKeyStart, nodeKeyGoal);
}

//**************************************************************************************************
ZilchDefineType(PathFinderRequest, builder, type)
{
  ZeroBindDocumented();
  ZilchBindMethod(Cancel);
  ZilchBindFieldGetter(mPathFinderComponent);
  ZilchBindFieldGetter(mStatus);
}

//**************************************************************************************************
PathFinderRequest::PathFinderRequest(PathFinder* owner, Job* job) :
  mPathFinderComponent(owner),
  mJob(job),
  mStatus(PathFinderStatus::Pending)
{
  // When the job is finished it sends an event to the request which uses thread safe handle id.
  // Any script may listen upon the request, but the request will also forward the event to the
  // Cog/Component and then delete the job.
  ConnectThisTo(this, Events::PathFinderFinishedGeneric, OnJobFinished);
}

//**************************************************************************************************
void PathFinderRequest::Cancel()
{
  mStatus = PathFinderStatus::Cancelled;
  if (mJob)
    mJob->Cancel();
}

//**************************************************************************************************
void PathFinderRequest::OnJobFinished(PathFinderBaseEvent* event)
{
  // If the user cancelled the request then early out
  if (mStatus == PathFinderStatus::Cancelled)
    return;

  ErrorIf(mStatus != PathFinderStatus::Pending,
    "The request should not have been completed yet");

  // We consider the path finding as a success if any nodes were returned
  if (event->GenericGetPathNodeCount() != 0)
    mStatus = PathFinderStatus::Succeeded;
  else
    mStatus = PathFinderStatus::Failed;

  // Note: The PathFinder component may have been deleted by this point
  PathFinder* pathFinder = mPathFinderComponent;
  if (pathFinder != nullptr)
  {
    // This occurs when the engine pumps threaded events (came from a finished job)
    // Therefore there is no debug draw space available, so we make ours the default
    Debug::ActiveDrawSpace activeDrawSpace(pathFinder->GetSpace()->GetRuntimeId());

    // Let the derived class send out its own events to the PathFinderRequest and Cog
    String derivedEventName = pathFinder->GetCustomEventName();
    DispatchEvent(derivedEventName, event);
    pathFinder->DispatchEvent(derivedEventName, event);

    // Check if the event is a world position event, then we translate it and handle it ourselves
    PathFinderEvent<Vec3> generatedEvent;
    generatedEvent.mRequest = event->mRequest;
    generatedEvent.mDuration = event->mDuration;

    generatedEvent.mStart = pathFinder->NodeKeyToWorldPosition(event->GenericGetStart());
    generatedEvent.mGoal = pathFinder->NodeKeyToWorldPosition(event->GenericGetGoal());

    size_t count = event->GenericGetPathNodeCount();
    generatedEvent.mPath.Reserve(count);
    for (size_t i = 0; i < count; ++i)
    {
      generatedEvent.mPath.PushBack(pathFinder->NodeKeyToWorldPosition(event->GenericGetPathNode(i)));
    }

    DispatchEvent(Events::PathFinderFinished, &generatedEvent);
    pathFinder->DispatchEvent(Events::PathFinderFinished, &generatedEvent);
  }

  // The job is completed and it's safe to delete it
  // Note: The job keeps our request handle alive and may be the only reference
  // We must ONLY delete this at the very end.
  SafeDelete(mJob);
}

//**************************************************************************************************
ZilchDefineType(PathFinderBaseEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldGetter(mRequest);
  ZilchBindFieldGetter(mDuration);
}

//**************************************************************************************************
ZilchDefineType(PathFinderGrid, builder, type)
{
  ZeroBindDocumented();
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  ZeroBindInterface(PathFinder);
  ZeroBindDependency(Transform);
  ZeroBindEvent(Events::PathFinderGridFinished, PathFinderEvent<IntVec3>);

  ZilchBindOverloadedMethod(FindPath, ZilchInstanceOverload(HandleOf<ArrayClass<IntVec3>>, IntVec3Param, IntVec3Param));
  ZilchBindOverloadedMethod(FindPath, ZilchInstanceOverload(HandleOf<ArrayClass<Real3>>, Real3Param, Real3Param));
  ZilchBindOverloadedMethod(FindPathThreaded, ZilchInstanceOverload(HandleOf<PathFinderRequest>, IntVec3Param, IntVec3Param));
  ZilchBindOverloadedMethod(FindPathThreaded, ZilchInstanceOverload(HandleOf<PathFinderRequest>, Real3Param, Real3Param));

  ZilchBindMethod(SetCollision);
  ZilchBindMethod(GetCollision);
  ZilchBindMethod(SetCost);
  ZilchBindMethod(GetCost);
  ZilchBindMethod(Clear);
  ZilchBindGetterSetterProperty(DiagonalMovement);
  ZilchBindGetterSetterProperty(CellSize);

  ZilchBindMethod(WorldPositionToCellIndex);
  ZilchBindMethod(LocalPositionToCellIndex);
  ZilchBindMethod(CellIndexToWorldPosition);
  ZilchBindMethod(CellIndexToLocalPosition);
}

//**************************************************************************************************
PathFinderGrid::PathFinderGrid() :
  mTransform(nullptr),
  mLocalCellSize(Vec3(1)),
  mGrid(new CopyOnWriteData<PathFinderAlgorithmGrid>())
{
}

//**************************************************************************************************
void PathFinderGrid::Serialize(Serializer& stream)
{
  PathFinder::Serialize(stream);
  SerializeNameDefault(mLocalCellSize, Vec3(1));
  bool& mDiagonalMovement = mGrid->mDiagonalMovement;
  SerializeNameDefault(mDiagonalMovement, true);
}

//**************************************************************************************************
void PathFinderGrid::Initialize(CogInitializer& initializer)
{
  ZilchBase::Initialize(initializer);
  mTransform = GetOwner()->has(Transform);
}

//**************************************************************************************************
void PathFinderGrid::DebugDraw()
{
  forRange(const auto& pair, mGrid->mCells.All())
  {
    Vec4 color;
    if (pair.second.mCollision)
      color = ToFloatColor(Color::Red);
    else
      color = ToFloatColor(Color::Green);

    Vec3 worldCenter = CellIndexToWorldPosition(pair.first);

    float xScale = Math::Length(mTransform->TransformNormal(Vec3::cXAxis));
    float yScale = Math::Length(mTransform->TransformNormal(Vec3::cYAxis));
    float zScale = Math::Length(mTransform->TransformNormal(Vec3::cZAxis));

    Vec3 halfExtents(xScale / 2.0f, yScale / 2.0f, zScale / 2.0f);

    Debug::Obb debugObb(worldCenter, halfExtents);
    debugObb.mColor = color;
    gDebugDraw->Add(debugObb);
    debugObb.SetFilled(true);
    debugObb.mColor.w = 0.1f;
    gDebugDraw->Add(debugObb);
  }
}

//**************************************************************************************************
HandleOf<ArrayClass<IntVec3>> PathFinderGrid::FindPath(IntVec3Param start, IntVec3Param goal)
{
  return FindPathHelper<IntVec3, PathFinderAlgorithmGrid>(mGrid, start, goal, mMaxIterations);
}

//**************************************************************************************************
HandleOf<PathFinderRequest> PathFinderGrid::FindPathThreaded(IntVec3Param start, IntVec3Param goal)
{
  return FindPathThreadedHelper<IntVec3, PathFinderAlgorithmGrid>(mGrid, start, goal, mMaxIterations);
}

//**************************************************************************************************
void PathFinderGrid::SetCellSize(Vec3Param size)
{
  mLocalCellSize = Math::Max(Vec3(0.001f), size);
}

//**************************************************************************************************
Vec3Param PathFinderGrid::GetCellSize()
{
  return mLocalCellSize;
}

//**************************************************************************************************
void PathFinderGrid::SetDiagonalMovement(bool value)
{
  mGrid.CopyIfNeeded();
  mGrid->mDiagonalMovement = value;
}

//**************************************************************************************************
bool PathFinderGrid::GetDiagonalMovement()
{
  return mGrid->mDiagonalMovement;
}

//**************************************************************************************************
IntVec3 PathFinderGrid::WorldPositionToCellIndex(Vec3Param worldPosition)
{
  Vec3 localPosition = mTransform->TransformPointInverse(worldPosition);
  return LocalPositionToCellIndex(localPosition);
}

//**************************************************************************************************
IntVec3 PathFinderGrid::LocalPositionToCellIndex(Vec3Param localPosition)
{
  Vec3 indexFloats = localPosition / mLocalCellSize;

  IntVec3 cellIndex(
    int(indexFloats.x) + ((indexFloats.x < 0) ? -1 : 0),
    int(indexFloats.y) + ((indexFloats.y < 0) ? -1 : 0),
    int(indexFloats.z) + ((indexFloats.z < 0) ? -1 : 0));
  return cellIndex;
}

//**************************************************************************************************
Variant PathFinderGrid::WorldPositionToNodeKey(Vec3Param worldPosition)
{
  return Variant(WorldPositionToCellIndex(worldPosition));
}

//**************************************************************************************************
Vec3 PathFinderGrid::NodeKeyToWorldPosition(VariantParam nodeKey)
{
  return CellIndexToWorldPosition((IntVec3)nodeKey);
}

//**************************************************************************************************
void PathFinderGrid::FindPathGeneric(VariantParam start, VariantParam goal, Array<Variant>& pathOut)
{
  GenericFindPathHelper<IntVec3, PathFinderAlgorithmGrid>(mGrid, start, goal, pathOut, mMaxIterations);
}

//**************************************************************************************************
HandleOf<PathFinderRequest> PathFinderGrid::FindPathGenericThreaded(VariantParam start, VariantParam goal)
{
  return GenericFindPathThreadedHelper<IntVec3, PathFinderAlgorithmGrid>(mGrid, start, goal, mMaxIterations);
}

//**************************************************************************************************
StringParam PathFinderGrid::GetCustomEventName()
{
  return Events::PathFinderGridFinished;
}

//**************************************************************************************************
void PathFinderGrid::SetCollision(IntVec3Param index, bool collision)
{
  mGrid.CopyIfNeeded();
  mGrid->SetCollision(index, collision);
}

//**************************************************************************************************
bool PathFinderGrid::GetCollision(IntVec3Param index)
{
  return mGrid->GetCollision(index);
}

//**************************************************************************************************
void PathFinderGrid::SetCost(IntVec3Param index, float cost)
{
  mGrid.CopyIfNeeded();
  mGrid->SetCost(index, cost);
}

//**************************************************************************************************
float PathFinderGrid::GetCost(IntVec3Param index)
{
  return mGrid->GetCost(index);
}

//**************************************************************************************************
void PathFinderGrid::Clear()
{
  mGrid.CopyIfNeeded();
  mGrid->Clear();
}

//**************************************************************************************************
Vec3 PathFinderGrid::CellIndexToWorldPosition(IntVec3Param index)
{
  Vec3 localPosition = CellIndexToLocalPosition(index);
  Vec3 worldPosition = mTransform->TransformPoint(localPosition);
  return worldPosition;
}

//**************************************************************************************************
Vec3 PathFinderGrid::CellIndexToLocalPosition(IntVec3Param index)
{
  Vec3 localPosition = Math::ToVector3(index) * mLocalCellSize;
  return localPosition;
}

} // namespace Zero