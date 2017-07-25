///////////////////////////////////////////////////////////////////////////////
///
/// \file TrapezoidMap.cpp
/// Implementation for the trapezoid map.
/// 
/// Authors: Killian Koenig
/// Copyright 2013, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

bool EqualsExact(const Vec2& a, const Vec2& b)
{
  return a.x == b.x && a.y == b.y;
}

void Initialize(TrapezoidMap::Node* node, 
                TrapezoidMap::Node::NodeType type,
                s32 value,
                TrapezoidMap::NodeId left = -1,
                TrapezoidMap::NodeId right = -1)
{
  node->Type = type;
  node->Value = value;
  node->Left = left;
  node->Right = right;
}

bool IsAbove(const Vec2& a, const Vec2& b)
{
  if(a.y == b.y)
  {
    return a.x > b.x;
  }

  return a.y > b.y;
}

s32 ComputeLogStarN(s32 N)
{
  s32 i = 0;
  f32 val = f32(N);
  while(val >= 1.0f)
  {
    val = Math::Log(val);
    ++i;
  }
  return i - 1;
}

struct Frame
{
  TrapezoidMap::RegionId regionId;
  s32 depth;
  s32 winding;

  Frame() {}
  Frame(TrapezoidMap::RegionId regionId_, s32 depth_, s32 winding_) 
  : regionId(regionId_) 
  , depth(depth_)
  , winding(winding_) {}
};

TrapezoidMap::TrapezoidMap(const Array<Vec2>& vertices, 
                           const Array<uint>& contours, 
                           s32 edgeCount, 
                           s32 seed)
  : mRandom(seed)
  , mRegions(vertices.Size() * 2 + 1 + 1)
  , mNodes(vertices.Size() * 4 + 1 + vertices.Size() * 2)
  , mIsValid(true)
{
  // Total number of graph regions is n + k + 1
  // where n is the number of vertices and k is the number of edges
  // 1 extra since there is overlap when we add regions that should be merged

  // Total number of tree nodes is AT least n + k + r
  // where n is the number of vertices and r is the number of regions
  // k is the number of edges
  // When regions are merged edge nodes are duplicated so there are more 

  mVertices = vertices;

  for(s32 i = 0; i < mRegions.GetCapacity(); ++i)
  {
    Region* region = mRegions.GetElement(i);
    region->NodeIndex = -1;
    region->WindingOrder = 0;
  }

  mRoot = mNodes.Allocate();
  Node* root = mNodes.GetElement(mRoot);
  Initialize(root, Node::Region, mRegions.Allocate());

  Region* region = mRegions.GetElement(root->Value);
  region->Depth = -1;
  region->TopVertex = region->BotVertex = -1;
  region->LeftEdge = region->RightEdge = -1;
  region->SetTop(-1);
  region->SetBottom(-1);
  region->NodeIndex = mRoot;

  // Compute end index table
  size_t vertexCount = mVertices.Size();
  mContourId.Resize(vertexCount);
  mEndIndex.Resize(vertexCount);
  mWindingOrder.Resize(vertexCount);
  size_t offset = 0;
  for(size_t i = 0; i < contours.Size(); ++i)
  {
    size_t size = contours[i];
    if(size > 0)
    {
      s8 winding = Geometry::DetermineWindingOrder(&mVertices[offset], size) >= 0.f ? 1 : -1;
      for(size_t j = 0; j < size; ++j)
      {
        mContourId[j + offset] = i;
        mEndIndex[j + offset] = (j + 1) % size + offset;  
        mWindingOrder[j + offset] = winding;  
      }
    }
    offset += size;
  }
  mPrevIndex.Resize(vertexCount);
  for(size_t i = 0; i < vertexCount; ++i)
  {
    mPrevIndex[mEndIndex[i]] = i;
  }

  mFarthestEnd.Resize(vertexCount);
  mFarthestPrev.Resize(vertexCount);
  for (size_t i = 0; i < vertexCount; ++i)
  {
    mFarthestEnd[i] = i;
    mFarthestPrev[i] = i;
  }

  // Compute top/bot index tables
  mTopIndex.Resize(vertexCount);
  mBotIndex.Resize(vertexCount);
  for(size_t i = 0; i < vertexCount; ++i)
  {
    // Edge goes from a to b
    VertexId indexA = i;
    VertexId indexB = mEndIndex[indexA];

    // Compute vertical ordering for adjacent duplicate vertices
    size_t prev = indexA;
    for (size_t j = indexB; j != indexA; j = mEndIndex[j])
    {
      Vec2 delta = mVertices[j] - mVertices[indexA];
      f32 length = delta.Length();

      if (length != 0.f)
      {
        for (size_t k = indexA; k != j; k = mEndIndex[k])
        {
          mFarthestEnd[k] = prev;
        }

        break;
      }

      prev = j;
    }
  }

  for(size_t i = 0; i < vertexCount; ++i)
  {
    // Edge goes from a to b
    VertexId indexA = mEndIndex[mFarthestEnd[i]];
    VertexId indexB = mEndIndex[indexA];

    // Compute vertical ordering for adjacent duplicate vertices
    for (size_t j = indexB; j != indexA; j = mEndIndex[j])
    {
      Vec2 delta = mVertices[j] - mVertices[indexA];
      f32 length = delta.Length();

      if (length != 0.f)
      {
        for (size_t k = indexA; k != mFarthestEnd[j]; k = mEndIndex[k])
        {
          mFarthestPrev[k] = indexA;
        }

        break;
      }
    }
  }


  for (size_t i = 0; i < vertexCount; ++i)
  {
    // Edge goes from a to b
    VertexId indexA = i;
    VertexId indexB = mEndIndex[indexA];

    if(IsAboveInternal(indexA, indexB))
    {
      mTopIndex[i] = indexA;
      mBotIndex[i] = indexB;
    }
    else
    {
      mTopIndex[i] = indexB;
      mBotIndex[i] = indexA;
    }

  }

  // Initialize regions below lookup
  mRegionsBelow.Resize(vertexCount);
  for(size_t i = 0; i < vertexCount; ++i)
  {
    BelowCache* cache = &mRegionsBelow[i];
    cache->Index[0] = -1;
    cache->Index[1] = -1;
    cache->Index[2] = -1;
  }

  Array<VertexId> order(vertices.Size());
  for(size_t i = 0; i < order.Size(); ++i)
  {
    order[i] = i;
  }

  s32 edgesInserted = 0;

  // Insert edges in a random order
  Array<VertexId> random;
  random.Reserve(vertices.Size());

  while(!order.Empty())
  {
    s32 index = mRandom.Uint32() % order.Size();
    Zero::Swap(order[index], order.Back());

    random.PushBack(order.Back());
    order.PopBack();
  }  

  Array<NodeId> rootId(vertices.Size());
  Array<char> alreadyAdded(vertices.Size());
  for(size_t i = 0; i < vertices.Size(); ++i)
  {
    alreadyAdded[i] = 0;
    rootId[i] = 0;
  }

  s32 phaseCount = ComputeLogStarN(vertices.Size());
  size_t verticesPerPhase = phaseCount == 0 ? 0 : mVertices.Size() / phaseCount;
  for(s32 i = 0; i < phaseCount; ++i)
  {
    size_t start = verticesPerPhase * i;
    size_t end = start + verticesPerPhase;
    if(i == phaseCount - 1)
    {
      end = vertices.Size();
    }

    for(size_t j = start; j < end; ++j)
    {
      VertexId i1 = random[j];
      VertexId i2 = mEndIndex[i1];

      if(edgeCount >= 0 && edgesInserted >= edgeCount)
      {
        break;
      }

      if(!alreadyAdded[i1])
      {
        InsertVertex(rootId[i1], i1);
        alreadyAdded[i1] = 1;

        ++edgesInserted;   
        if(edgeCount >= 0 && edgesInserted >= edgeCount)
        {
          break;
        }
      }

      if(!alreadyAdded[i2])
      {
        InsertVertex(rootId[i2], i2);
        alreadyAdded[i2] = 1;

        ++edgesInserted;   
        if(edgeCount >= 0 && edgesInserted >= edgeCount)
        {
          break;
        }        
      }

      if(InsertEdge(i1) == false)
      {
        mIsValid = false;
        return;
      }
      ++edgesInserted;      
    }

    // Update root positions for remaining vertices
    for(size_t j = end; j < vertices.Size(); ++j)
    {
      VertexId id = random[j];
      rootId[id] = QueryInternal(rootId[id], id)->NodeIndex;
    }
  }

  // Maintain a bool for each region that we have already traversed
  Array<bool> cache(mRegions.GetCapacity());
  for(size_t i = 0; i < cache.Size(); ++i)
  {
    cache[i] = false;    
  }

  Array<Frame> current;
  Array<Frame> next;
  current.Reserve(mRegions.GetSize());
  next.Reserve(mRegions.GetSize());
  next.PushBack(Frame(0, 0, 0));

  while(!next.Empty())
  {
    current.Swap(next);
    next.Clear();
    
    while(!current.Empty())
    {
      Frame frame = current.Back();
      current.PopBack();

      s32 regionId = frame.regionId;
      s32 depth = frame.depth;
      s32 winding = frame.winding;

      // Terminate when if we have hit an invalid region
      if(regionId != -1 && cache[regionId] == false)
      {
        Region* region = mRegions.GetElement(regionId);
        if(region->NodeIndex < 0)
        {
          mIsValid = false;
          return;
        }

        cache[regionId] = true;

        region->Depth = depth;
        region->WindingOrder = winding;

        current.PushBack(Frame(region->TopNeighbor[0], depth, winding));
        current.PushBack(Frame(region->TopNeighbor[1], depth, winding));
        current.PushBack(Frame(region->BotNeighbor[0], depth, winding));
        current.PushBack(Frame(region->BotNeighbor[1], depth, winding));

        VertexId topId = region->TopVertex;
        if(topId == -1)
        {
          continue;
        }
 
        const BelowCache* belowCache = &mRegionsBelow[topId];
        s32 index = 0;
        for(size_t i = 0; i < 3; ++i)
        {
          if (belowCache->Index[i] == regionId)
          {
            index = i;
          }
        }

        // Cross right edge to find right region neighbor
        if(index < 2 && belowCache->Index[index + 1] != -1)
        {
          RegionId neighborId = belowCache->Index[index + 1];
          const Region* neighbor = mRegions.GetElement(neighborId);
          
          // Only consider regions with uninitialized depths
          if(neighbor->Depth == -1)
          {
            next.PushBack(Frame(neighborId, 
                           depth + 1, 
                           winding + mWindingOrder[region->RightEdge]));
          }
        }

        // Cross left edge to find left region neighbor
        if(index > 0)
        {
          RegionId neighborId = belowCache->Index[index - 1];
          ErrorIf(neighborId == -1);
          const Region* neighbor = mRegions.GetElement(neighborId);
          
          // Only consider regions with uninitialized depths
          if(neighbor->Depth == -1)
          {
            next.PushBack(Frame(neighborId, 
                                 depth + 1,
                                 winding + mWindingOrder[region->LeftEdge]));
          }
        }
      }
    }
  }
}

bool TrapezoidMap::IsValid() const
{
  return mIsValid;
}

bool TrapezoidMap::IsAboveInternal(TrapezoidMap::VertexId idA, 
                                   TrapezoidMap::VertexId idB) const
{
  const Vec2& a = mVertices[idA];
  const Vec2& b = mVertices[idB];

  if(a.y == b.y)
  {
    if(a.x == b.x)
    { 
      Vec2 delta1 = mVertices[mEndIndex[mFarthestEnd[idA]]] - a;

      s32 end = mEndIndex[mFarthestEnd[idA]];
      s32 size = mVertices.Size();
      if (idA < end)
      {
        idA += size;
      }

      if (idB < end)
      {
        idB += size;
      }

      if (delta1.y == 0.f)
      {
        if (delta1.x > 0.f)
        {
          return idA > idB;
        }
      }

      if (delta1.y > 0.f)
      {
        return idA > idB;
      }

      return idA < idB;
    }

    return a.x > b.x;
  }

  return a.y > b.y;
}

bool TrapezoidMap::IsLeft(EdgeId start, 
                          EdgeId end, 
                          const Vec2& point) const 
{
  Vec2 p1 = mVertices[start];
  Vec2 p2 = mVertices[end];

  if(!IsAboveInternal(end, start))
  {
    Zero::Swap(p1, p2);
  }

  return Cross(p2 - p1, point - p1) > 0.f;
}

bool TrapezoidMap::IsLeftInternal(EdgeId start, 
                          EdgeId end, 
                          VertexId pointId) const 
{ 
  VertexId bot = start;
  VertexId top = mEndIndex[mFarthestEnd[bot]];
  Vec2 p1 = mVertices[bot];
  Vec2 p2 = mVertices[top];
  Vec2 delta1 = p2 - p1;
  Vec2 point = mVertices[pointId];
  if(!IsAboveInternal(top, bot))
  {
    Zero::Swap(p1, p2);
    Zero::Swap(bot, top);
  }

  // If the point is on and/or collinear to the edge
  // determinant will be zero in which case we need to test the top point
  // of the query point with respect to the original edge
  f32 det = Cross(p2 - p1, point - p1);

  if (det == 0.f)
  {
    if (mContourId[start] == mContourId[pointId])
    {
      // Get the next vertex along P
      VertexId next = mEndIndex[mFarthestEnd[pointId]];
      point = mVertices[next];

      // Assuming edge going from S to E and P are on the same contour
      if (EqualsExact(mVertices[pointId], mVertices[start]))
      {
        VertexId prev = mPrevIndex[mFarthestPrev[pointId]];
        if (next == end)
        {
          point = mVertices[prev];
        }
      }
    }
    else
    {
      // Get the next vertex along P
      VertexId next = mEndIndex[mFarthestEnd[pointId]];
      point = mVertices[next];

      // Assuming edge going from S to E and P are on the same contour
      if (EqualsExact(mVertices[pointId], mVertices[end]))
      {
        VertexId prev = mPrevIndex[mFarthestPrev[pointId]];
        point = mVertices[prev];
      }
    }


    det = Cross(p2 - p1, point - p1);
  }

  return det > 0.f;
}

const TrapezoidMap::Region* TrapezoidMap::Query(const Vec2& point) const
{
  return QueryFrom(mRoot, point);
}

const TrapezoidMap::Region* TrapezoidMap::QueryFrom(NodeId rootId, const Vec2& point) const
{
  const Node* node = mNodes.GetElement(rootId);
  while(node->Type != Node::Region)
  {
    NodeId child;
    if(node->Type == (u8)Node::Vertex)
    {
      Vec2 nodePoint = mVertices[node->Value];
      child = IsAbove(point, nodePoint) ? node->Left : node->Right;
    }
    else
    {
      bool left = IsLeft(node->Value, mEndIndex[node->Value], point);
      child = left ? node->Left : node->Right;
    }

    node = mNodes.GetElement(child);
  }

  return mRegions.GetElement(node->Value);
}

const TrapezoidMap::Region* TrapezoidMap::QueryInternal(NodeId rootId, VertexId vertexId) const
{
  const Node* node = mNodes.GetElement(rootId);
  while(node->Type != Node::Region)
  {
    NodeId child;
    if(node->Type == Node::Vertex)
    {
      child = IsAboveInternal(vertexId, node->Value) ? node->Left : node->Right;
    }
    else
    {
      bool left = IsLeftInternal(node->Value, mEndIndex[node->Value], vertexId);
      child = left ? node->Left : node->Right;
    }

    node = mNodes.GetElement(child);
  }

  return mRegions.GetElement(node->Value);
}

// Find the common top point shared by the edge dividing the two given
// regions
TrapezoidMap::VertexId TrapezoidMap::FindCommonTopPoint(RegionId indexL, RegionId indexR)
{
  Region* L = mRegions.GetElement(indexL);
  Region* R = mRegions.GetElement(indexR);

  if(L->RightEdge == -1 || R->LeftEdge == -1)
  {
    ErrorIf(true);
    return -1;
  }

  VertexId topLR = mTopIndex[L->RightEdge];
  VertexId topRL = mTopIndex[R->LeftEdge];


  if (topLR != topRL)
  {
    ErrorIf(true);
    return -1;
  }

  return topLR;
}

// Finds the region below the current one along a given edge
TrapezoidMap::RegionId TrapezoidMap::FindRegionBelow(RegionId regionIndex, EdgeId edgeStart)
{
  Region* O = mRegions.GetElement(regionIndex);
  
  // Check if there are any regions below the current
  if(O->BotNeighbor[0] == -1)
  {
    return -1;
  }

  // If there is only 1 region below just return that one
  if(O->BotNeighbor[1] == -1)
  {
    return O->BotNeighbor[0];
  }

  // There are 2 below so choose the one that intersects the given edge
  VertexId topIndex = FindCommonTopPoint(O->BotNeighbor[0], O->BotNeighbor[1]);
  if (topIndex == -1)
  {
    return -1;
  }

  EdgeId edgeEnd = mEndIndex[edgeStart];
  if (edgeStart == topIndex || edgeEnd == topIndex)
  {
    return -1;
  }

  return IsLeftInternal(edgeStart, edgeEnd, topIndex) ? O->BotNeighbor[1] : O->BotNeighbor[0];
}

  // Key:
  // A = Top left region
  // O = Old region (the one that we are splitting)
  // (x) = Top edge point
  // L = new left region
  // R = new right region
  // +... = new edge
  // |... = existing edge
bool TrapezoidMap::UpdateAbove(Region* O, 
                               RegionId indexL, 
                               RegionId indexR,
                               VertexId topEdgeIndex,
                               RegionId leftParent,
                               RegionId rightParent)
{
  // If a region has only 1 top region we will refer to it as A
  RegionId indexA = O->TopNeighbor[0];
  RegionId indexB = O->TopNeighbor[1];

  Region* L = mRegions.GetElement(indexL);
  Region* R = mRegions.GetElement(indexR);

  // Also check for double edge connectivity here
  // TODO: For case 2+ we need to verify that (x) is shared by maximum one edge
  // already or an error occurred  
  if (indexA == -1)
  {
    L->SetTop(-1);
    R->SetTop(-1);
    //Error("Trapezoid graph ended up in invalid configuration.");
    return false;
  }

  Region* A = mRegions.GetElement(indexA);

  if (indexB != -1)
  {
    Region* B = mRegions.GetElement(indexB);

    if (rightParent != -1)
    {
      A->SetBottom(indexL);
      B->SetBottom(indexL);
      L->SetTop(indexA, indexB);
     
      Region* parent = mRegions.GetElement(rightParent);
      parent->SetBottom(indexR);
      R->SetTop(rightParent);
      return true;
    }
    else if (leftParent != -1)
    {
      A->SetBottom(indexR);
      B->SetBottom(indexR);
      R->SetTop(indexA, indexB);

      Region* parent = mRegions.GetElement(leftParent);
      parent->SetBottom(indexL);
      L->SetTop(leftParent);
      return true;
    }

    // 1 existing graph edge shares (x) and (x) is the bottom point of that edge
    // 
    //                |                                        |
    //         A      |      B                         A       |       B            
    //                |                                        |              
    // --------------(x)--------------   --->   --------------(x)--------------
    //                                                         +                 
    //                O                                L       +       R
    //                                                         +                     
    // 

    if (A->BotNeighbor[0] != indexL && A->BotNeighbor[0] != -1)
    {
      A->SetBottom(A->BotNeighbor[0], indexL);
    }
    else
    {
      A->SetBottom(indexL);
    }

    B->BotNeighbor[0] = indexR;

    L->SetTop(indexA);
    R->SetTop(indexB);     
    return true;
  }

  ErrorIf(leftParent != -1 || rightParent != -1);

  if (A->BotNeighbor[1] == -1)
  {
    // No existing graph edges share (x)
    //
    //                A                                        A                    
    //                                                                        
    // --------------(x)--------------   --->   --------------(x)--------------
    //                                                         +                 
    //                O                                L       +       R
    //                                                         +                     
    //                                                                            
    L->SetTop(indexA);
    R->SetTop(indexA);
    A->SetBottom(indexL, indexR);
    return true;
  }

  // Find the index of the existing edge that shares a point with the top
  // point of the new edge
  EdgeId indexTLE = O->LeftEdge != -1 ? mTopIndex[O->LeftEdge] : -1;
  EdgeId indexTRE = O->RightEdge != -1 ? mTopIndex[O->RightEdge] : -1;
  if (indexTLE == topEdgeIndex && indexTRE == topEdgeIndex)
  {
    Error("Error building trapezoid graph.");
    return false;
  }

  if (indexTLE == topEdgeIndex)
  {
    // 1 existing graph edge shares (x) and (x) is the top point of that edge
    // The bottom point of (x) is to the left of the bottom point of the
    // new edge
    //                A                                        A              
    //                                                                        
    // --------------(x)--------------   --->   --------------(x)--------------
    //               /                                        / +             
    //      Q       /       O                         Q      /   +
    //             /                                        /  L  +     R     
    //            /                                        /       +
    // 
    RegionId indexQ = A->BotNeighbor[0];
    A->SetBottom(indexQ, indexR);
    L->SetTop(-1);
    R->SetTop(indexA);  
    return true;   
  }

  if (indexTRE == topEdgeIndex)
  {
    // 1 existing graph edge shares (x) and (x) is the top point of that edge
    // The bottom point of (x) is to the right of the bottom point of the
    // new edge
    //                A                                        A                    
    //                                                                        
    // --------------(x)--------------   --->   --------------(x)--------------
    //                 \                                      + \                  
    //        O         \       Q                            +   \   
    //                   \                            L     +  R  \     Q                     
    //                    \                                +       \
    //
    RegionId indexQ = A->BotNeighbor[1];
    A->SetBottom(indexL, indexQ);
    L->SetTop(indexA);
    R->SetTop(-1);  
    return true;  
  }

  //Error("Error building trapezoid graph.");
  return false;
}


bool TrapezoidMap::UpdateBelow(Region* O, 
                               RegionId indexL, 
                               RegionId indexR,
                               EdgeId edgeId,
                               RegionId* leftParent,
                               RegionId* rightParent,
                               bool isLast)
{
  EdgeId bottomEdgeIndex = mBotIndex[edgeId];
  *leftParent = *rightParent = -1;

  // If a region has only 1 bottom region we will refer to it as A
  RegionId indexA = O->BotNeighbor[0];
  RegionId indexB = O->BotNeighbor[1];

  Region* L = mRegions.GetElement(indexL);
  Region* R = mRegions.GetElement(indexR);

  // Also check for double edge connectivity here
  // TODO: For case 2+ we need to verify that (x) is shared by maximum one edge
  // already or an error occurred  
  if (indexA == -1)
  {
    //Error("Trapezoid graph ended up in invalid configuration.");
    L->SetBottom(-1);
    R->SetBottom(-1);
    return false;
  }

  Region* A = mRegions.GetElement(indexA);

  if (indexB != -1)
  {
    Region* B = mRegions.GetElement(indexB);
    if (O->BotVertex != bottomEdgeIndex)
    {
      if(!IsLeftInternal(edgeId, mEndIndex[edgeId], A->TopVertex))
      {
        A->SetTop(indexL, indexR);
        L->SetBottom(indexA);

        B->SetTop(indexR);
        R->SetBottom(indexA, indexB);
      }
      else
      {
        A->SetTop(indexL);
        L->SetBottom(indexA, indexB);

        B->SetTop(indexL, indexR);
        R->SetBottom(indexB);
      }
    
      return true;

    }

    // 1 existing graph edge shares (x) and (x) is the top point of that edge
    // 
    //                                                         +
    //                O                                 L      +      R            
    //                                                         +              
    // --------------(x)--------------   --->   --------------(x)--------------
    //                |                                        |                 
    //         A      |      B                          A      |      B
    //                |                                        |                     
    // 
    

    A->SetTop(indexL);
    L->SetBottom(indexA);

    B->SetTop(indexR);
    R->SetBottom(indexB);    
    return true;
  }

  if (A->TopNeighbor[1] == -1)
  {
    // No existing graph edges share (x)
    //                                                         +
    //                O                                L       +       R            
    //                                                         +              
    // --------------(x)--------------   --->   --------------(x)--------------
    //                                                                           
    //                A                                        A        
    //                                                                               
    //                                                                            
    L->SetBottom(indexA);
    R->SetBottom(indexA);
    A->SetTop(indexL, indexR);
    return true;
  }

  // Find the index of the existing edge that shares a point with the bottom
  // point of the new edge
  EdgeId indexBLE = O->LeftEdge != -1 ? mBotIndex[O->LeftEdge] : -1;
  EdgeId indexBRE = O->RightEdge != -1 ? mBotIndex[O->RightEdge] : -1;

  if (indexBLE == bottomEdgeIndex && indexBRE == bottomEdgeIndex)
  {
    Error("Error building trapezoid graph.");
    return false;
  }

  // If the old top right region was split
  if (!isLast && A->TopNeighbor[1] == indexL)
  {
    // Because we reused the left region in the split we don't need
    // to update the left
    
    R->SetBottom(indexA);
    *rightParent = indexR;
    return true;
  }
  else if (!isLast && A->TopNeighbor[0] == indexL)
  {
    A->SetTop(indexR, A->TopNeighbor[1]);
    L->SetBottom(indexA);
    *leftParent = indexL;
    return true;
  }

  if (indexBLE == bottomEdgeIndex)
  {
    // 1 existing graph edge shares (x) and (x) is the bottom point of that edge
    // The top point of (x) is to the left of the top point of the new edge
    //
    //            \                                        \       +
    //      Q      \        O                        Q      \  L  +     R
    //              \                                        \   +              
    //               \                                        \ +               
    // --------------(x)--------------   --->   --------------(x)--------------
    //                                                                     
    //                                                 
    //                A                                        A     
    //                                                            
    // 
    RegionId indexQ = A->TopNeighbor[0];
    A->SetTop(indexQ, indexR);
    L->SetBottom(-1);
    R->SetBottom(indexA);  
    return true;   
  }

  if (indexBRE == bottomEdgeIndex)
  {
    // 1 existing graph edge shares (x) and (x) is the bottom point of that edge
    // The top point of (x) is to the right of the top point of the new edge
    //
    //                    /                                +       /
    //        O          /     Q                      L     +  R  /     Q
    //                  /                                    +   /              
    //                 /                                      + /               
    // --------------(x)--------------   --->   --------------(x)--------------
    //                                                                     
    //                                                 
    //                A                                        A     
    //                                                            
    // 
    RegionId indexQ = A->TopNeighbor[1];
    A->SetTop(indexL, indexQ);
    L->SetBottom(indexA);
    R->SetBottom(-1);
    return true;
  }

  return false;
}

void Add(TrapezoidMap::BelowCache* cache, TrapezoidMap::RegionId id)
{
  for (size_t i = 0; i < 3; ++i)
  {
    if (cache->Index[i] == id)
    {
      return;
    }
  }

  if (cache->Index[0] == -1)
  {
    cache->Index[0] = id;
    return;
  }

  if (cache->Index[1] == -1)
  {
    cache->Index[1] = id;
    return;
  }

  cache->Index[2] = id;
}

bool TrapezoidMap::InsertEdge(EdgeId edgeId)
{
  EdgeId topEdgeIndex = mTopIndex[edgeId];
  EdgeId botEdgeIndex = mBotIndex[edgeId];

  RegionId regionAId = mRegionsBelow[topEdgeIndex].Index[0];
  RegionId regionBId = mRegionsBelow[topEdgeIndex].Index[1];
  RegionId currRegion = regionAId;

  // If two regions below we need to select the that Contains this edge
  if(regionBId != -1)
  {
    Region* regionA = mRegions.GetElement(regionAId);
    Region* regionB = mRegions.GetElement(regionBId);

    // The edge currently dividing region A and region B
    EdgeId oldEdgeId = regionA->RightEdge;

    //ErrorIf(EqualsExact(mVertices[oldEdgeId], mVertices[botEdgeIndex]));
    //ErrorIf(EqualsExact(mVertices[mEndIndex[oldEdgeId]], mVertices[botEdgeIndex]));
    
    // Check if the edge is contained in the right region
    if(!IsLeftInternal(oldEdgeId, mEndIndex[oldEdgeId], botEdgeIndex))
    {
      currRegion = regionBId;
    }
  }

  // Before all regions affected by the new edge are split, its possible that
  // there exists a dangling left or right region that will need to be
  // updated later
  RegionId floatL = -1;
  RegionId floatR = -1;

  // Track how many regions were split by this edge insertion
  s32 splitCount = 0;
  
  for(;;)
  {
    RegionId next = FindRegionBelow(currRegion, edgeId);
    ErrorIf(next == currRegion);

    // Reuse current region as the left region after the split
    RegionId leftRegionId = currRegion;
    RegionId rightRegionId = mRegions.Allocate();

    Region originalCopy = *mRegions.GetElement(currRegion);
    currRegion = next;

    Region* L = mRegions.GetElement(leftRegionId);
    Region* R = mRegions.GetElement(rightRegionId);

    *L = originalCopy;
    *R = originalCopy;

    // Update the region graph
    // Update left/right edges for the new split regions
    L->LeftEdge = originalCopy.LeftEdge;
    R->RightEdge = originalCopy.RightEdge;
    L->RightEdge = edgeId;
    R->LeftEdge = edgeId;

    // Update regions below top vertex
    VertexId topId = originalCopy.TopVertex;
    if(topId == -1)
    {
      return false;
    }

    BelowCache* cache = &mRegionsBelow[topId];
    if (splitCount++ == 0 && cache->Index[2] == -1)
    {
      if (cache->Index[1] != -1)
      {
        if (cache->Index[0] == leftRegionId)
        {
          cache->Index[2] = cache->Index[1];
          cache->Index[1] = rightRegionId;
        }
        else
        {
          Add(cache, leftRegionId);
          Add(cache, rightRegionId); 
        }
      }
      else
      {
        Add(cache, rightRegionId);        
      }
    }
    else
    { 
      //ErrorIf(EqualsExact(mVertices[edgeId], mVertices[topId]));
      //ErrorIf(EqualsExact(mVertices[mEndIndex[edgeId]], mVertices[topId]));
      if (!IsLeftInternal(edgeId, mEndIndex[edgeId], topId))
      {
        cache = &mRegionsBelow[topId];
        for (size_t i = 0; i < 3; ++i)
        {
          if (cache->Index[i] == leftRegionId)
          {
            cache->Index[i] = rightRegionId;
            break;
          }
        }
      }
    }

    bool result;
    result = UpdateAbove(&originalCopy, leftRegionId, rightRegionId, topEdgeIndex, floatL, floatR);
    if(result == false)
    {
      //ErrorIf(true);
      return false;
    }

    bool isLast = next == -1 || mRegions.GetElement(next)->TopVertex == botEdgeIndex;
    result = UpdateBelow(&originalCopy, leftRegionId, rightRegionId, edgeId, &floatL, &floatR, isLast);
    if(result == false)
    {
      //ErrorIf(true);
      return false;
    }

    // Create the new left and right children
    NodeId leftIndex = MergeAbove(leftRegionId, &floatL);
    NodeId rightIndex = MergeAbove(rightRegionId, &floatR);

    // Convert the old leaf into an edge node
    Node* node = mNodes.GetElement(originalCopy.NodeIndex);
    Initialize(node, Node::Edge, edgeId, leftIndex, rightIndex);
    if (next == -1)
    {
      break;
    }

    Region* last = mRegions.GetElement(next);
    if (last->TopVertex == botEdgeIndex)
    {
      break;
    }
  } 

  return true;
}

// If they are the same, merge this region with the one above it and update
// the tree and graph accordingly
TrapezoidMap::NodeId TrapezoidMap::MergeAbove(RegionId regionId, RegionId* parent)
{
  Region* curr = mRegions.GetElement(regionId);

  if(curr->TopNeighbor[0] == -1)
  {
    NodeId nodeId = mNodes.Allocate();

    Node* node = mNodes.GetElement(nodeId);
    Initialize(node, Node::Region, regionId);
    curr->NodeIndex = nodeId;
    return nodeId;
  }

  VertexId aboveId = curr->TopNeighbor[0];
  Region* above = mRegions.GetElement(curr->TopNeighbor[0]);

  if(curr->LeftEdge != above->LeftEdge || 
     curr->RightEdge != above->RightEdge)
  {
    NodeId nodeId = mNodes.Allocate();

    Node* node = mNodes.GetElement(nodeId);
    Initialize(node, Node::Region, regionId);
    curr->NodeIndex = nodeId;
    return nodeId;
  }

  ErrorIf(above->BotNeighbor[1] != -1);
  ErrorIf(above->BotNeighbor[0] != regionId);

  above->BotNeighbor[0] = curr->BotNeighbor[0];
  above->BotNeighbor[1] = curr->BotNeighbor[1];
  above->BotVertex = curr->BotVertex;

  // Update the graph
  if(curr->BotNeighbor[0] != -1)
  {
    Region* region = mRegions.GetElement(curr->BotNeighbor[0]);
    if(region->TopNeighbor[0] == regionId)
    {
      region->TopNeighbor[0] = aboveId;
    }
    if(region->TopNeighbor[1] == regionId)
    {
      region->TopNeighbor[1] = aboveId;
    }
  }

  if(curr->BotNeighbor[1] != -1)
  {
    Region* region = mRegions.GetElement(curr->BotNeighbor[1]);
    if(region->TopNeighbor[0] == regionId)
    {
      region->TopNeighbor[0] = aboveId;
    }
    if(region->TopNeighbor[1] == regionId)
    {
      region->TopNeighbor[1] = aboveId;
    }
  }

  // Clear for debugging
  curr->SetTop(-1);
  curr->SetBottom(-1);
  curr->LeftEdge = -1;
  curr->RightEdge = -1;
  curr->TopVertex = -1;
  curr->BotVertex = -1;
  curr->NodeIndex = -1;

  // Free the old region
  mRegions.Free(regionId);

  if (*parent == regionId)
  {
    *parent = aboveId;
  }

  // Reuse the region node for the above region
  return above->NodeIndex;
}

void TrapezoidMap::InsertVertex(NodeId rootId, VertexId vertexId)
{
  Vec2 vertex = mVertices[vertexId];

  // Traverse the tree until we hit the leaf region that Contains this vertex
  NodeId nodeId = QueryInternal(rootId, vertexId)->NodeIndex;
  Node* node = mNodes.GetElement(nodeId);

  // Create the new top and bottom (left and right children) regions
  // that bound the insertion vertex
  RegionId topRegionIndex = node->Value;
  RegionId botRegionIndex = mRegions.Allocate();

  mRegionsBelow[vertexId].Index[0] = botRegionIndex;

  // We want to split the leaf into two new regions
  // The old leaf becomes a vertex node
  // We need to copy the old region on the stack since we are going to reuse
  // it as the left region after the split
  Region originalCopy = *mRegions.GetElement(node->Value);

  Region* regionL = mRegions.GetElement(topRegionIndex);
  Region* regionR = mRegions.GetElement(botRegionIndex);
  *regionL = originalCopy;
  *regionR = originalCopy;

  regionL->BotVertex = vertexId;
  regionR->TopVertex = vertexId;

  regionL->SetBottom(botRegionIndex);
  regionR->SetTop(topRegionIndex);

  // Since we reused the top region as the new top child after the split
  // We only need to update the graph for the region below the new bottom

  // Update region below to point to new bot region
  if(originalCopy.BotNeighbor[0] != -1)
  {
    Region* below = mRegions.GetElement(originalCopy.BotNeighbor[0]);
    if(below->TopNeighbor[1] == topRegionIndex)
    {
      below->TopNeighbor[1] = botRegionIndex;
    }
    else
    {
      below->TopNeighbor[0] = botRegionIndex;
    }
  }

  if(originalCopy.BotNeighbor[1] != -1)
  {
    Region* below = mRegions.GetElement(originalCopy.BotNeighbor[1]);
    if(below->TopNeighbor[1] == topRegionIndex)
    {
      below->TopNeighbor[1] = botRegionIndex;
    }
    else
    {
      below->TopNeighbor[0] = botRegionIndex;
    }    
  }

  // Create the new left and right children in the tree
  NodeId leftChildId = mNodes.Allocate();
  NodeId rightChildId = mNodes.Allocate();

  Initialize(mNodes.GetElement(leftChildId), Node::Region, topRegionIndex);
  Initialize(mNodes.GetElement(rightChildId), Node::Region, botRegionIndex);

  regionL->NodeIndex = leftChildId;
  regionR->NodeIndex = rightChildId;

  // Update the old region leaf to be a vertex node
  node = mNodes.GetElement(nodeId);
  Initialize(node, Node::Vertex, vertexId, leftChildId, rightChildId);
}

} // Zero
