// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Priority Node
template <typename Priority = float>
class PriorityNode
{
public:
  PriorityNode() : mPriority(0), mQueueIndex((size_t)-1)
  {
  }

  Priority mPriority;
  size_t mQueueIndex;
};

// Priority Queue
/// Node must have the following:
///   Priority mPriority;
///   size_t mQueueIndex;

/// TODO: Add MIT license comment
template <typename Node, typename Priority = float>
class PriorityQueue
{
public:
  PriorityQueue(size_t maxNodes = 1) : mNodeCount(0)
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
#if RaverieDebug
    if (!IsValidQueue())
    {
      Error("Queue has been corrupted (Did you update a node priority manually "
            "instead of calling UpdatePriority()? "
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

  /// Resize the queue so it can accept more nodes.  All currently enqueued
  /// nodes are remain. Attempting to decrease the queue size to a size too
  /// small to hold the existing nodes results in undefined behavior O(n)
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

  /// Removes a node from the queue.  The node does not need to be the head of
  /// the queue. If the node is not in the queue, the result is undefined.  If
  /// unsure, check Contains() first O(log n)
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

    // Now bubble formerLastNode (which is no longer the last node) up or down
    // as appropriate
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

      // Node has lower priority value, so move parent down the heap to make
      // room
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

      // Node has lower priority value, so move parent down the heap to make
      // room
      mNodes[node->mQueueIndex] = parentNode;
      parentNode->mQueueIndex = node->mQueueIndex;

      node->mQueueIndex = parent;
    }
    mNodes[node->mQueueIndex] = node;
  }

  void CascadeDown(Node* node)
  {
    // aka Heapify-down
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
      // Note that CascadeDown will be called if parentNode == node (that is,
      // node is the root)
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

} // namespace Raverie
