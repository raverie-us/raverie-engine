///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
namespace Events
{
DeclareEvent(PathFinderFinishedGeneric);
DeclareEvent(PathFinderFinished);
DeclareEvent(PathFinderGridFinished);
}

template <typename Priority = float>
class PriorityNode
{
public:
  PriorityNode() :
    mPriority(0),
    mQueueIndex((size_t)-1)
  {
  }

  Priority mPriority;
  size_t mQueueIndex;
};

/// Node must have the following:
///   Priority mPriority;
///   size_t mQueueIndex;
template <typename Node, typename Priority = float>
class PriorityQueue
{
public:
  PriorityQueue(size_t maxNodes = 1) :
    mNodeCount(0)
  {
    ErrorIf(maxNodes <= 0, "The queue must have at least 1 item");
    Resize(maxNodes);
  }

  size_t GetMaxSize()
  {
    return mNodes.Size() - 1;
  }

  size_t Count()
  {
    return mNodeCount;
  }

  bool Empty()
  {
    return mNodeCount == 0;
  }

  void Clear()
  {
    Error("May need to clean up nodes");
    memset(mNodes.Data(), 0, mNodes.Size() * sizeof(Node*));
    mNodeCount = 0;
  }

  bool Contains(Node* node)
  {
    ErrorIf(node == nullptr, "Node was null");
    if (node->mQueueIndex >= mNodes.Size())
      return false;
    return mNodes[node->mQueueIndex] == node;
  }

  void Enqueue(Node* node, Priority priority)
  {
    ErrorIf(node == nullptr, "Node was null");
    ErrorIf(Contains(node), "Node is already in the queue");
    if (mNodeCount >= mNodes.Size() - 1)
      Resize(mNodes.Size() * 2 + 1);

    node->mPriority = priority;
    ++mNodeCount;
    mNodes[mNodeCount] = node;
    node->mQueueIndex = mNodeCount;
    CascadeUp(node);
  }

  /// Removes the head of the queue and returns it.
  /// If queue is empty, result is undefined
  /// O(log n)
  Node* Dequeue()
  {
    ErrorIf(mNodeCount <= 0, "Cannot call Dequeue() on an empty queue");
#if ZeroDebug
    if (!IsValidQueue())
    {
      Error("Queue has been corrupted (Did you update a node priority manually instead of calling UpdatePriority()? "
        "Or add the same node to two different queues?)");
    }
#endif

    Node* returnMe = mNodes[1];

    // If the node is already the last node, we can remove it immediately
    if (mNodeCount == 1)
    {
      mNodes[1] = nullptr;
      mNodeCount = 0;
      return returnMe;
    }

    // Swap the node with the last node
    Node* formerLastNode = mNodes[mNodeCount];
    mNodes[1] = formerLastNode;
    formerLastNode->mQueueIndex = 1;
    mNodes[mNodeCount] = nullptr;
    --mNodeCount;

    // Now bubble formerLastNode (which is no longer the last node) down
    CascadeDown(formerLastNode);
    return returnMe;
  }

  /// Resize the queue so it can accept more nodes.  All currently enqueued nodes are remain.
  /// Attempting to decrease the queue size to a size too small to hold the existing nodes results in undefined behavior
  /// O(n)
  void Resize(size_t maxNodes)
  {
    ErrorIf(maxNodes < mNodeCount, "Cannot resize to a smaller size");
    size_t originalSize = mNodes.Size();
    mNodes.Resize(maxNodes + 1);

    memset(mNodes.Data() + originalSize, 0, (maxNodes - originalSize + 1) * sizeof(Node*));
  }

  Node* Front()
  {
    ErrorIf(mNodeCount == 0, "Cannot call Front() on an empty queue");
    return mNodes[1];
  }

  void UpdatePriority(Node* node, float priority)
  {
    ErrorIf(node == nullptr, "Node is null");
    ErrorIf(!Contains(node), "Cannot call UpdatePriority() on a node which is not enqueued");

    node->mPriority = priority;
    OnNodeUpdated(node);
  }

  /// Removes a node from the queue.  The node does not need to be the head of the queue.
  /// If the node is not in the queue, the result is undefined.  If unsure, check Contains() first
  /// O(log n)
  void Remove(Node* node)
  {
    ErrorIf(node == nullptr, "Node is null");
    ErrorIf(!Contains(node), "Cannot call Remove() on a node which is not enqueued");

    // If the node is already the last node, we can remove it immediately
    if (node->mQueueIndex == mNodeCount)
    {
      mNodes[mNodeCount] = nullptr;
      --mNodeCount;
      return;
    }

    // Swap the node with the last node
    Node* formerLastNode = mNodes[mNodeCount];
    mNodes[node->mQueueIndex] = formerLastNode;
    formerLastNode->mQueueIndex = node->mQueueIndex;
    mNodes[mNodeCount] = nullptr;
    --mNodeCount;

    // Now bubble formerLastNode (which is no longer the last node) up or down as appropriate
    OnNodeUpdated(formerLastNode);
  }

private:

  void CascadeUp(Node* node)
  {
    size_t parent;
    if (node->mQueueIndex > 1)
    {
      parent = node->mQueueIndex >> 1;
      Node* parentNode = mNodes[parent];
      if (HasHigherOrEqualPriority(parentNode, node))
        return;

      //Node has lower priority value, so move parent down the heap to make room
      mNodes[node->mQueueIndex] = parentNode;
      parentNode->mQueueIndex = node->mQueueIndex;

      node->mQueueIndex = parent;
    }
    else
    {
      return;
    }
    while (parent > 1)
    {
      parent >>= 1;
      Node* parentNode = mNodes[parent];
      if (HasHigherOrEqualPriority(parentNode, node))
        break;

      //Node has lower priority value, so move parent down the heap to make room
      mNodes[node->mQueueIndex] = parentNode;
      parentNode->mQueueIndex = node->mQueueIndex;

      node->mQueueIndex = parent;
    }
    mNodes[node->mQueueIndex] = node;
  }

  void CascadeDown(Node* node)
  {
    //aka Heapify-down
    size_t finalQueueIndex = node->mQueueIndex;
    size_t childLeftIndex = 2 * finalQueueIndex;

    // If leaf node, we're done
    if (childLeftIndex > mNodeCount)
    {
      return;
    }

    // Check if the left-child is higher-priority than the current node
    size_t childRightIndex = childLeftIndex + 1;
    Node* childLeft = mNodes[childLeftIndex];
    if (HasHigherPriority(childLeft, node))
    {
      // Check if there is a right child. If not, swap and finish.
      if (childRightIndex > mNodeCount)
      {
        node->mQueueIndex = childLeftIndex;
        childLeft->mQueueIndex = finalQueueIndex;
        mNodes[finalQueueIndex] = childLeft;
        mNodes[childLeftIndex] = node;
        return;
      }
      // Check if the left-child is higher-priority than the right-child
      Node* childRight = mNodes[childRightIndex];
      if (HasHigherPriority(childLeft, childRight))
      {
        // left is highest, move it up and continue
        childLeft->mQueueIndex = finalQueueIndex;
        mNodes[finalQueueIndex] = childLeft;
        finalQueueIndex = childLeftIndex;
      }
      else
      {
        // right is even higher, move it up and continue
        childRight->mQueueIndex = finalQueueIndex;
        mNodes[finalQueueIndex] = childRight;
        finalQueueIndex = childRightIndex;
      }
    }
    // Not swapping with left-child, does right-child exist?
    else if (childRightIndex > mNodeCount)
    {
      return;
    }
    else
    {
      // Check if the right-child is higher-priority than the current node
      Node* childRight = mNodes[childRightIndex];
      if (HasHigherPriority(childRight, node))
      {
        childRight->mQueueIndex = finalQueueIndex;
        mNodes[finalQueueIndex] = childRight;
        finalQueueIndex = childRightIndex;
      }
      // Neither child is higher-priority than current, so finish and stop.
      else
      {
        return;
      }
    }

    while (true)
    {
      childLeftIndex = 2 * finalQueueIndex;

      // If leaf node, we're done
      if (childLeftIndex > mNodeCount)
      {
        node->mQueueIndex = finalQueueIndex;
        mNodes[finalQueueIndex] = node;
        break;
      }

      // Check if the left-child is higher-priority than the current node
      childRightIndex = childLeftIndex + 1;
      childLeft = mNodes[childLeftIndex];
      if (HasHigherPriority(childLeft, node))
      {
        // Check if there is a right child. If not, swap and finish.
        if (childRightIndex > mNodeCount)
        {
          node->mQueueIndex = childLeftIndex;
          childLeft->mQueueIndex = finalQueueIndex;
          mNodes[finalQueueIndex] = childLeft;
          mNodes[childLeftIndex] = node;
          break;
        }
        // Check if the left-child is higher-priority than the right-child
        Node* childRight = mNodes[childRightIndex];
        if (HasHigherPriority(childLeft, childRight))
        {
          // left is highest, move it up and continue
          childLeft->mQueueIndex = finalQueueIndex;
          mNodes[finalQueueIndex] = childLeft;
          finalQueueIndex = childLeftIndex;
        }
        else
        {
          // right is even higher, move it up and continue
          childRight->mQueueIndex = finalQueueIndex;
          mNodes[finalQueueIndex] = childRight;
          finalQueueIndex = childRightIndex;
        }
      }
      // Not swapping with left-child, does right-child exist?
      else if (childRightIndex > mNodeCount)
      {
        node->mQueueIndex = finalQueueIndex;
        mNodes[finalQueueIndex] = node;
        break;
      }
      else
      {
        // Check if the right-child is higher-priority than the current node
        Node* childRight = mNodes[childRightIndex];
        if (HasHigherPriority(childRight, node))
        {
          childRight->mQueueIndex = finalQueueIndex;
          mNodes[finalQueueIndex] = childRight;
          finalQueueIndex = childRightIndex;
        }
        // Neither child is higher-priority than current, so finish and stop.
        else
        {
          node->mQueueIndex = finalQueueIndex;
          mNodes[finalQueueIndex] = node;
          break;
        }
      }
    }
  }

  inline bool HasHigherOrEqualPriority(Node* higher, Node* lower)
  {
    return higher->mPriority <= lower->mPriority;
  }

  inline bool HasHigherPriority(Node* higher, Node* lower)
  {
    return higher->mPriority < lower->mPriority;
  }

  void OnNodeUpdated(Node* node)
  {
    // Bubble the updated node up or down as appropriate
    size_t parentIndex = node->mQueueIndex >> 1;

    if (parentIndex > 0 && HasHigherPriority(node, mNodes[parentIndex]))
    {
      CascadeUp(node);
    }
    else
    {
      // Note that CascadeDown will be called if parentNode == node (that is, node is the root)
      CascadeDown(node);
    }
  }

  bool IsValidQueue()
  {
    for (size_t i = 1; i < mNodes.Size(); ++i)
    {
      if (mNodes[i] != nullptr)
      {
        size_t childLeftIndex = 2 * i;
        if (childLeftIndex < mNodes.Size() && mNodes[childLeftIndex] != nullptr && HasHigherPriority(mNodes[childLeftIndex], mNodes[i]))
          return false;

        size_t childRightIndex = childLeftIndex + 1;
        if (childRightIndex < mNodes.Size() && mNodes[childRightIndex] != nullptr && HasHigherPriority(mNodes[childRightIndex], mNodes[i]))
          return false;
      }
    }

    return true;
  }

  // Internals
  Array<Node*> mNodes;
  size_t mNodeCount;
};

//------------------------------------------------------------------------------ PathFinderAlgorithm
// To derive from PathFinderAlgorithm you must provide the following interface:
// Template Types:
//   Derived   - Your derived PathFinder type such as PathFinderGridAlgorithm
//   NodeKey   - A unique identifier for a node (must be usable as a key in a HashMap)
//   NodeRange - A range of nodes used in a neighbor query (front results in Pair<NodeKey, float> where float is cost
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
    PathFinderNode(NodeKeyParam nodeKey) :
      mKey(nodeKey),
      mCameFrom(nullptr),
      mCostSoFar(0)
    {
    }

    PathFinderNode(NodeKeyParam nodeKey, PathFinderNode* cameFrom, float costSoFar) :
      mKey(nodeKey),
      mCameFrom(cameFrom),
      mCostSoFar(costSoFar)
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
        while (iterator = iterator->mCameFrom)
        {
          pathOut.PushBack(iterator->mKey);
        }
        Reverse(pathOut.Begin(), pathOut.End());
        break;
      }

      typedef Pair<NodeKey, float> NodeCostPair;
      forRange(const NodeCostPair& next, self->QueryNeighbors(currentNode->mKey))
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

//-------------------------------------------------------------------------- PathFinderAlgorithmGrid
class PathFinderAlgorithmGrid;

class PathFinderGridNodeRange
{
public:
  PathFinderGridNodeRange(PathFinderAlgorithmGrid* grid, IntVec3Param center);

  // Range Interface
  typedef Pair<IntVec3, float> FrontResult;
  inline bool Empty() const;
  inline void PopFront();
  inline Pair<IntVec3, float> Front() const;
  inline PathFinderGridNodeRange& All();

  // Internals

  // Returns true if it pops, or false if the cell is valid
  inline void PopUntilValid();

  PathFinderAlgorithmGrid* mGrid;
  IntVec3 mCenter;
  float mCurrentCost;
  IntVec3 mCurrentIntVec3;
  int mIndex;
};

/// A cell in the grid that contains the cost and collision information.
/// A cell only exists if it has either a non-zero cost or collision.
class PathFinderCell
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PathFinderCell();

  /// The user added cost to traversing a node. This can be used to simulate
  /// areas that you would like a path to avoid (but can still go through if it must).
  float mCost;

  /// If this cell is non-traversable (paths may not move through it).
  bool mCollision;
};

class PathFinderAlgorithmGrid : public PathFinderAlgorithm<PathFinderAlgorithmGrid, IntVec3, PathFinderGridNodeRange>
{
public:
  PathFinderAlgorithmGrid();

  // PathFinderAlgorithm Interface
  PathFinderGridNodeRange QueryNeighbors(IntVec3Param node);
  bool QueryIsValid(IntVec3Param node);
  float QueryHeuristic(IntVec3Param node, IntVec3Param goal);

  /// If there is collision at a cell then the A* algorithm cannot traverse that cell.
  void SetCollision(IntVec3Param index, bool collision);
  bool GetCollision(IntVec3Param index);

  /// A higher cost of a cell makes the A* algorithm less likely to traverse that cell.
  /// The default cost of any cell is 1.0.
  void SetCost(IntVec3Param index, float cost);
  float GetCost(IntVec3Param index);

  /// Clear the grid of all collision and costs.
  void Clear();

  /// Whether the A* path can move diagonally or only on the cardinal axes.
  bool mDiagonalMovement;

  // Internals
  HashMap<IntVec3, PathFinderCell> mCells;
};

//--------------------------------------------------------------------------------------- PathFinder
class PathFinderRequest;
class PathFinderBaseEvent;

/// A base class for all path finding implementations. The base provides functionality such as
/// path finding with Real3 vectors from one position to another (in world space).
class PathFinder : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PathFinder();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  virtual Variant WorldPositionToNodeKey(Vec3Param worldPosition) = 0;
  virtual Vec3 NodeKeyToWorldPosition(VariantParam nodeKey) = 0;

  // Helper functions that the derived class implements that will path find between world positions.
  // If we fail to find a path the array will be empty.
  virtual void FindPathGeneric(VariantParam start, VariantParam goal, Array<Variant>& pathOut) = 0;
  virtual HandleOf<PathFinderRequest> FindPathGenericThreaded(VariantParam start, VariantParam goal) = 0;
  
  // A custom event name used to differentiate the derived
  // class event (e.g. PathFinderGridFinished or PathFinderNavMeshFinished).
  virtual StringParam GetCustomEventName() = 0;
  
  template <typename NodeKey, typename Algorithm>
  HandleOf<ArrayClass<NodeKey>> FindPathHelper(CopyOnWriteHandle<Algorithm>& algorithm, const NodeKey& start, const NodeKey& goal, size_t maxIterations)
  {
    HandleOf<ArrayClass<NodeKey>> array = ZilchAllocate(ArrayClass<NodeKey>);
    algorithm->FindNodePath(start, goal, array->NativeArray, maxIterations);
    return array;
  }

  template <typename NodeKey, typename Algorithm>
  void GenericFindPathHelper(CopyOnWriteHandle<Algorithm>& algorithm, VariantParam start, VariantParam goal, Array<Variant>& pathOut, size_t maxIterations)
  {
    Array<NodeKey> path;
    algorithm->FindNodePath(start.GetOrDefault<NodeKey>(), goal.GetOrDefault<NodeKey>(), path, maxIterations);

    forRange(const NodeKey& nodeKey, path)
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

  /// Finds a path between world positions (or returns an empty array if no path could be found).
  HandleOf<ArrayClass<Vec3>> FindPath(Vec3Param worldStart, Vec3Param worldGoal);

  /// Finds a path on another thread between the closest nodes to the given world positions.
  /// When the thread is completed, the events PathFinderGridCompleted or PathFinderGridFailed will be 
  /// sent on both the returned PathFinderRequest and on the Cog that owns this component (on this.Owner).
  HandleOf<PathFinderRequest> FindPathThreaded(Vec3Param worldStart, Vec3Param worldGoal);

  /// The number of iterations we allow for the path finding algorithm before we terminate it.
  /// This prevents infinite loops when we have an unbounded number of nodes/edges.
  int mMaxIterations;
};

DeclareEnum4(PathFinderStatus, Pending, Succeeded, Failed, Cancelled);

/// Represents a threaded path finding calculation. You may listen on the
/// PathFinderRequest for events such as PathFinderFinished
/// (or more specific events such as PathFinderGridFinished if you are using a PathFinderGrid, etc).
/// The status of the threaded request is also available on this class.
class PathFinderRequest : public ReferenceCountedThreadSafeId32EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PathFinderRequest(PathFinder* owner, Job* job);

  /// Requests to stop the thread running the path finding calculation.
  void Cancel();

  // Internals
  void OnJobFinished(PathFinderBaseEvent* event);
  Job* mJob;

  /// The component that initiated this request.
  /// You may need to cast this into the derived type (for example to PathFinderGrid).
  HandleOf<PathFinder> mPathFinderComponent;

  /// The status of the threaded path finding calculation.
  PathFinderStatus::Enum mStatus;
};

/// An event that contains common data between all path-finding implementations.
class PathFinderBaseEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PathFinderBaseEvent() :
    mDuration(0)
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
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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
    HandleOf<ArrayClass<NodeKey>> array = ZilchAllocate(ArrayClass<NodeKey>);
    array->NativeArray = mPath;
    return array;
  }

  NodeKey mStart;
  NodeKey mGoal;
  Array<NodeKey> mPath;
};

template <typename NodeKey>
ZilchDefineType(PathFinderEvent<NodeKey>, builder, type)
{
  ZilchBindFieldGetter(mStart);
  ZilchBindFieldGetter(mGoal);
  ZilchBindGetter(Path);
}

template <typename NodeKey, typename Algorithm>
class PathFinderJob : public Job
{
public:
  PathFinderJob() :
    mMaxIterations((size_t)-1),
    mCancel(false),
    mMainThreadPathFinderDispatcher(nullptr)
  {
    mDeletedOnCompletion = false;
  }

  // Job Interface
  int Execute() override
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
    return 0;
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

//----------------------------------------------------------------------------------- PathFinderGrid
/// A* pathfinding on a grid. The grid supports cardinal or diagonal movement.
/// Collision and costs can be set for each tile to make path-finding avoid cells.
/// Sends the PathFinderGridFinished event on itself when a threaded request completes.
class PathFinderGrid : public PathFinder
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  PathFinderGrid();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void DebugDraw() override;

  // PathFinder Interface
  Variant WorldPositionToNodeKey(Vec3Param worldPosition) override;
  Vec3 NodeKeyToWorldPosition(VariantParam nodeKey) override;
  void FindPathGeneric(VariantParam start, VariantParam goal, Array<Variant>& pathOut) override;
  HandleOf<PathFinderRequest> FindPathGenericThreaded(VariantParam start, VariantParam goal) override;
  StringParam GetCustomEventName() override;

  // PathFinderGrid Interface
  /// If there is collision at a cell then the A* algorithm cannot traverse that cell.
  void SetCollision(IntVec3Param index, bool collision);
  bool GetCollision(IntVec3Param index);

  /// A higher cost of a cell makes the A* algorithm less likely to traverse that cell.
  void SetCost(IntVec3Param index, float cost);
  float GetCost(IntVec3Param index);

  /// Clear the grid of all collision and costs.
  void Clear();

  /// Finds a path between cell indices (or returns an empty array if no path could be found).
  HandleOf<ArrayClass<IntVec3>> FindPath(IntVec3Param start, IntVec3Param goal);
  using ZilchBase::FindPath;

  /// Finds a path on another thread between cell indices.
  /// When the thread is completed, the events PathFinderGridCompleted or PathFinderGridFailed will be 
  /// sent on both the returned PathFinderRequest and on the Cog that owns this component (on this.Owner).
  HandleOf<PathFinderRequest> FindPathThreaded(IntVec3Param start, IntVec3Param goal);
  using ZilchBase::FindPathThreaded;

  /// The size of the cell in local space units.
  /// If the PathFinderGrid has no parent, or the parent's transform has
  /// no scale then this will be the same as the world cell size.
  void SetCellSize(Vec3Param size);
  Vec3Param GetCellSize();

  /// Whether the A* path can move diagonally or only on the cardinal axes.
  void SetDiagonalMovement(bool value);
  bool GetDiagonalMovement();

  /// Returns the cell that the world position occupies.
  IntVec3 WorldPositionToCellIndex(Vec3Param worldPosition);

  /// Returns the cell that the local position occupies.
  IntVec3 LocalPositionToCellIndex(Vec3Param localPosition);

  /// Returns the center position of the cell in world space.
  Vec3 CellIndexToWorldPosition(IntVec3Param index);

  /// Returns the center position of the cell in local space.
  Vec3 CellIndexToLocalPosition(IntVec3Param index);

  // Internals
  Transform* mTransform;
  CopyOnWriteHandle<PathFinderAlgorithmGrid> mGrid;
  Vec3 mLocalCellSize;
};

} // namespace Zero
