// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
DefineEvent(PathFinderFinishedGeneric);
DefineEvent(PathFinderFinished);
} // namespace Events

RaverieDefineType(PathFinder, builder, type)
{
  RaverieBindDocumented();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindEvent(Events::PathFinderFinished, PathFinderEvent<Vec3>);

  RaverieBindMethod(FindPath);
  RaverieBindMethod(FindPathThreaded);

  RaverieBindField(mMaxIterations);
}

const int cDefaultMaxIterations = 100000;
PathFinder::PathFinder() : mMaxIterations(cDefaultMaxIterations)
{
}

void PathFinder::Serialize(Serializer& stream)
{
  SerializeNameDefault(mMaxIterations, cDefaultMaxIterations);
}

void PathFinder::Initialize(CogInitializer& initializer)
{
}

HandleOf<ArrayClass<Vec3>> PathFinder::FindPath(Vec3Param worldStart, Vec3Param worldGoal)
{
  Array<Variant> path;
  Variant nodeKeyStart = WorldPositionToNodeKey(worldStart);
  Variant nodeKeyGoal = WorldPositionToNodeKey(worldGoal);
  FindPathGeneric(nodeKeyStart, nodeKeyGoal, path);

  HandleOf<ArrayClass<Vec3>> array = RaverieAllocate(ArrayClass<Vec3>);
  ArrayClass<Vec3>& arrayRef = array;

  for (size_t i = 0; i < path.Size(); ++i)
    arrayRef.NativeArray.PushBack(NodeKeyToWorldPosition(path[i]));

  return array;
}

HandleOf<PathFinderRequest> PathFinder::FindPathThreaded(Vec3Param worldStart, Vec3Param worldGoal)
{
  Array<Variant> path;
  Variant nodeKeyStart = WorldPositionToNodeKey(worldStart);
  Variant nodeKeyGoal = WorldPositionToNodeKey(worldGoal);
  return FindPathGenericThreaded(nodeKeyStart, nodeKeyGoal);
}

RaverieDefineType(PathFinderRequest, builder, type)
{
  RaverieBindDocumented();
  RaverieBindMethod(Cancel);
  RaverieBindFieldGetter(mPathFinderComponent);
  RaverieBindFieldGetter(mStatus);
}

PathFinderRequest::PathFinderRequest(PathFinder* owner, Job* job) :
    mPathFinderComponent(owner),
    mJob(job),
    mStatus(PathFinderStatus::Pending)
{
  // When the job is finished it sends an event to the request which uses thread
  // safe handle id. Any script may listen upon the request, but the request
  // will also forward the event to the Cog/Component and then delete the job.
  ConnectThisTo(this, Events::PathFinderFinishedGeneric, OnJobFinished);
}

void PathFinderRequest::Cancel()
{
  mStatus = PathFinderStatus::Cancelled;
  if (mJob)
    mJob->Cancel();
}

void PathFinderRequest::OnJobFinished(PathFinderBaseEvent* event)
{
  // If the user cancelled the request then early out
  if (mStatus == PathFinderStatus::Cancelled)
    return;

  ErrorIf(mStatus != PathFinderStatus::Pending, "The request should not have been completed yet");

  // We consider the path finding as a success if any nodes were returned
  if (event->GenericGetPathNodeCount() != 0)
    mStatus = PathFinderStatus::Succeeded;
  else
    mStatus = PathFinderStatus::Failed;

  // Note: The PathFinder component may have been deleted by this point
  PathFinder* pathFinder = mPathFinderComponent;
  if (pathFinder != nullptr)
  {
    // This occurs when the engine pumps threaded events (came from a finished
    // job) Therefore there is no debug draw space available, so we make ours
    // the default
    Debug::ActiveDrawSpace activeDrawSpace(pathFinder->GetSpace()->GetRuntimeId());

    // Let the derived class send out its own events to the PathFinderRequest
    // and Cog
    String derivedEventName = pathFinder->GetCustomEventName();
    DispatchEvent(derivedEventName, event);
    pathFinder->DispatchEvent(derivedEventName, event);

    // Check if the event is a world position event, then we translate it and
    // handle it ourselves
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

  // The job is completed and it's safe to delete it / release it
  // Note: The job keeps our request handle alive and may be the only reference
  // We must ONLY delete this at the very end.
  mJob = nullptr;
}

RaverieDefineType(PathFinderBaseEvent, builder, type)
{
  RaverieBindDocumented();
  RaverieBindFieldGetter(mRequest);
  RaverieBindFieldGetter(mDuration);
}

} // namespace Raverie
