// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
namespace Events
{
DeclareEvent(PathFinderFinishedGeneric);
DeclareEvent(PathFinderFinished);
} // namespace Events

template <typename NodeKey, typename Algorithm>
class PathFinderJob;

// PathFinderAlgorithm
// To derive from PathFinderAlgorithm you must provide the following interface:
// Template Types:
//   Derived   - Your derived PathFinder type such as PathFinderGridAlgorithm
//   NodeKey   - A unique identifier for a node (must be usable as a key in a
//   HashMap) NodeRange - A range of nodes used in a neighbor query (front
//   results in Pair<NodeKey, float> where float is cost
// Functions:
//   NodeRange QueryNeighbors(NodeKeyParam node);
//   bool QueryIsValid(NodeKeyParam node);
//   float QueryHeuristic(NodeKeyParam node, NodeKeyParam goal);
template <typename Derived, typename NodeKey, typename NodeRange>
class PathFinderAlgorithm
{
public:
  typedef const NodeKey& NodeKeyParam;

  // The node MUST be of POD type because we do not destruct it!
  class PathFinderNode : public PriorityNode<float>
  {
  public:
    PathFinderNode(NodeKeyParam nodeKey) : mKey(nodeKey), mCameFrom(nullptr), mCostSoFar(0)
    {
    }

    PathFinderNode(NodeKeyParam nodeKey, PathFinderNode* cameFrom, float costSoFar) : mKey(nodeKey), mCameFrom(cameFrom), mCostSoFar(costSoFar)
    {
    }

    NodeKey mKey;
    PathFinderNode* mCameFrom;
    float mCostSoFar;
  };

  void FindNodePath(NodeKeyParam start, NodeKeyParam goal, Array<NodeKey>& pathOut, size_t maxIterations, const bool* cancel = nullptr)
  {
    const float cTieBreaker = 1.00001f;

    pathOut.Clear();
    Derived* self = static_cast<Derived*>(this);

    if (!self->QueryIsValid(start) || !self->QueryIsValid(goal))
      return;

    Memory::Pool pool("PathFinderNodePool", nullptr, sizeof(PathFinderNode), 128, true);

    PriorityQueue<PathFinderNode> frontier(100);
    HashMap<NodeKey, PathFinderNode*> keyToNode;
    PathFinderNode* startNode = pool.AllocateType<PathFinderNode>(start);
    keyToNode[start] = startNode;
    frontier.Enqueue(startNode, 0);

    while (!frontier.Empty())
    {
      if (maxIterations == 0)
        break;

      // If an outside entity wanted us to terminate early...
      if (cancel && *cancel)
        return;

      PathFinderNode* currentNode = frontier.Dequeue();

      if (currentNode->mKey == goal)
      {
        const PathFinderNode* iterator = currentNode;
        pathOut.PushBack(goal);
        while ((iterator = iterator->mCameFrom))
        {
          pathOut.PushBack(iterator->mKey);
        }
        Reverse(pathOut.Begin(), pathOut.End());
        break;
      }

      typedef Pair<NodeKey, float> NodeCostPair;
      forRange (const NodeCostPair& next, self->QueryNeighbors(currentNode->mKey))
      {
        float newCost = currentNode->mCostSoFar + next.second;
        PathFinderNode*& nextNode = keyToNode[next.first];
        if (!nextNode)
        {
          nextNode = pool.AllocateType<PathFinderNode>(next.first, currentNode, newCost);
          float priority = newCost + self->QueryHeuristic(next.first, goal) * cTieBreaker;
          frontier.Enqueue(nextNode, priority);
        }
        else if (newCost < nextNode->mCostSoFar)
        {
          nextNode->mCameFrom = currentNode;
          nextNode->mCostSoFar = newCost;
          float priority = newCost + self->QueryHeuristic(next.first, goal) * cTieBreaker;
          if (frontier.Contains(nextNode))
            frontier.UpdatePriority(nextNode, priority);
          else
            frontier.Enqueue(nextNode, priority);
        }
      }

      --maxIterations;
    }
  }
};

// PathFinder
class PathFinderRequest;
class PathFinderBaseEvent;
class PathFinder;

DeclareEnum4(PathFinderStatus, Pending, Succeeded, Failed, Cancelled);

/// Represents a threaded path finding calculation. You may listen on the
/// PathFinderRequest for events such as PathFinderFinished
/// (or more specific events such as PathFinderGridFinished if you are using a
/// PathFinderGrid, etc). The status of the threaded request is also available
/// on this class.
class PathFinderRequest : public ReferenceCountedThreadSafeId32EventObject
{
public:
  RaverieDeclareType(PathFinderRequest, TypeCopyMode::ReferenceType);

  PathFinderRequest(PathFinder* owner, Job* job);

  /// Requests to stop the thread running the path finding calculation.
  void Cancel();

  // Internals
  void OnJobFinished(PathFinderBaseEvent* event);
  HandleOf<Job> mJob;

  /// The component that initiated this request.
  /// You may need to cast this into the derived type (for example to
  /// PathFinderGrid).
  HandleOf<PathFinder> mPathFinderComponent;

  /// The status of the threaded path finding calculation.
  PathFinderStatus::Enum mStatus;
};

/// A base class for all path finding implementations. The base provides
/// functionality such as path finding with Real3 vectors from one position to
/// another (in world space).
class PathFinder : public Component
{
public:
  RaverieDeclareType(PathFinder, TypeCopyMode::ReferenceType);

  PathFinder();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  virtual Variant WorldPositionToNodeKey(Vec3Param worldPosition) = 0;
  virtual Vec3 NodeKeyToWorldPosition(VariantParam nodeKey) = 0;

  // Helper functions that the derived class implements that will path find
  // between world positions. If we fail to find a path the array will be empty.
  virtual void FindPathGeneric(VariantParam start, VariantParam goal, Array<Variant>& pathOut) = 0;
  virtual HandleOf<PathFinderRequest> FindPathGenericThreaded(VariantParam start, VariantParam goal) = 0;

  // A custom event name used to differentiate the derived
  // class event (e.g. PathFinderGridFinished or PathFinderNavMeshFinished).
  virtual StringParam GetCustomEventName() = 0;

  template <typename NodeKey, typename Algorithm>
  HandleOf<ArrayClass<NodeKey>> FindPathHelper(CopyOnWriteHandle<Algorithm>& algorithm, const NodeKey& start, const NodeKey& goal, size_t maxIterations)
  {
    HandleOf<ArrayClass<NodeKey>> array = RaverieAllocate(ArrayClass<NodeKey>);
    algorithm->FindNodePath(start, goal, array->NativeArray, maxIterations);
    return array;
  }

  template <typename NodeKey, typename Algorithm>
  void GenericFindPathHelper(CopyOnWriteHandle<Algorithm>& algorithm, VariantParam start, VariantParam goal, Array<Variant>& pathOut, size_t maxIterations)
  {
    Array<NodeKey> path;
    algorithm->FindNodePath(start.GetOrDefault<NodeKey>(), goal.GetOrDefault<NodeKey>(), path, maxIterations);

    forRange (const NodeKey& nodeKey, path)
      pathOut.PushBack(Variant(nodeKey));
  }

  template <typename NodeKey, typename Algorithm>
  HandleOf<PathFinderRequest> FindPathThreadedHelper(CopyOnWriteHandle<Algorithm>& algorithm, const NodeKey& start, const NodeKey& goal, size_t maxIterations)
  {
    typedef PathFinderJob<NodeKey, Algorithm> PathFinderAlgorithmJob;
    PathFinderAlgorithmJob* job = new PathFinderAlgorithmJob();
    PathFinderRequest* request = new PathFinderRequest(this, job);

    job->mStart = start;
    job->mGoal = goal;
    job->mRequest = request;
    job->mMainThreadPathFinder = this;
    job->mMainThreadPathFinderDispatcher = GetDispatcher();
    job->mAlgorithm = algorithm;
    job->mMaxIterations = maxIterations;
    Z::gJobs->AddJob(job);

    return request;
  }

  template <typename NodeKey, typename Algorithm>
  HandleOf<PathFinderRequest> GenericFindPathThreadedHelper(CopyOnWriteHandle<Algorithm>& algorithm, VariantParam start, VariantParam goal, size_t maxIterations)
  {
    return FindPathThreadedHelper<NodeKey, Algorithm>(algorithm, start.GetOrDefault<NodeKey>(), goal.GetOrDefault<NodeKey>(), maxIterations);
  }

  /// Finds a path between world positions (or returns an empty array if no path
  /// could be found).
  HandleOf<ArrayClass<Vec3>> FindPath(Vec3Param worldStart, Vec3Param worldGoal);

  /// Finds a path on another thread between the closest nodes to the given
  /// world positions. When the thread is completed, the events
  /// PathFinderGridCompleted or PathFinderGridFailed will be sent on both the
  /// returned PathFinderRequest and on the Cog that owns this component (on
  /// this.Owner).
  HandleOf<PathFinderRequest> FindPathThreaded(Vec3Param worldStart, Vec3Param worldGoal);

  /// The number of iterations we allow for the path finding algorithm before we
  /// terminate it. This prevents infinite loops when we have an unbounded
  /// number of nodes/edges.
  int mMaxIterations;
};

/// An event that contains common data between all path-finding implementations.
class PathFinderBaseEvent : public Event
{
public:
  RaverieDeclareType(PathFinderBaseEvent, TypeCopyMode::ReferenceType);

  PathFinderBaseEvent() : mDuration(0)
  {
  }

  virtual size_t GenericGetPathNodeCount() = 0;
  virtual Variant GenericGetPathNode(size_t index) = 0;
  virtual Variant GenericGetStart() = 0;
  virtual Variant GenericGetGoal() = 0;

  HandleOf<PathFinderRequest> mRequest;
  float mDuration;
};

template <typename NodeKey>
class PathFinderEvent : public PathFinderBaseEvent
{
public:
  RaverieDeclareType(PathFinderEvent, TypeCopyMode::ReferenceType);

  // PathFinderBaseEvent Interface
  size_t GenericGetPathNodeCount() override
  {
    return mPath.Size();
  }

  Variant GenericGetPathNode(size_t index) override
  {
    ReturnIf(index >= mPath.Size(), Variant(NodeKey()), "Invalid index given to generic GetPathNode");
    return Variant(mPath[index]);
  }

  Variant GenericGetStart() override
  {
    return Variant(mStart);
  }

  Variant GenericGetGoal() override
  {
    return Variant(mGoal);
  }

  HandleOf<ArrayClass<NodeKey>> GetPath()
  {
    HandleOf<ArrayClass<NodeKey>> array = RaverieAllocate(ArrayClass<NodeKey>);
    array->NativeArray = mPath;
    return array;
  }

  NodeKey mStart;
  NodeKey mGoal;
  Array<NodeKey> mPath;
};

template <typename NodeKey>
RaverieDefineType(PathFinderEvent<NodeKey>, builder, type)
{
  RaverieBindFieldGetter(mStart);
  RaverieBindFieldGetter(mGoal);
  RaverieBindGetter(Path);
}

template <typename NodeKey, typename Algorithm>
class PathFinderJob : public Job
{
public:
  PathFinderJob() : mMaxIterations((size_t)-1), mCancel(false), mMainThreadPathFinderDispatcher(nullptr)
  {
  }

  // Job Interface
  void Execute() override
  {
    Timer timer;

    PathFinderEvent<NodeKey>* toSend = new PathFinderEvent<NodeKey>();
    toSend->mStart = mStart;
    toSend->mGoal = mGoal;
    toSend->mRequest = mRequest;
    mAlgorithm->FindNodePath(mStart, mGoal, toSend->mPath, mMaxIterations, &mCancel);

    // We may have cancelled right as the path was finished
    if (mCancel)
      toSend->mPath.Clear();

    toSend->mDuration = (float)timer.UpdateAndGetTime();

    Z::gDispatch->DispatchOn(mMainThreadPathFinder, mMainThreadPathFinderDispatcher, Events::PathFinderFinishedGeneric, toSend);
  }

  int Cancel() override
  {
    mCancel = true;
    return 0;
  }

  NodeKey mStart;
  NodeKey mGoal;
  size_t mMaxIterations;
  HandleOf<PathFinderRequest> mRequest;
  HandleOf<PathFinder> mMainThreadPathFinder;
  EventDispatcher* mMainThreadPathFinderDispatcher;
  CopyOnWriteHandle<Algorithm> mAlgorithm;
  bool mCancel;
};

} // namespace Raverie
