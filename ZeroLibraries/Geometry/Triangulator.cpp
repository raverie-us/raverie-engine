///////////////////////////////////////////////////////////////////////////////
///
/// \file Triangulator.cpp
/// Implementation for Seidel's triangulation algorithm.
/// 
/// Authors: Killian Koenig
/// Copyright 2013, DigiPen Institute of Technology
///
//////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

using Zero::Array;
using Zero::Link;
using Zero::InList;
using Zero::TrapezoidMap;

namespace Geometry
{

namespace
{

struct ContourVertex
{ 
  Link<ContourVertex> link;
  Link<ContourVertex> windingLink;
  TrapezoidMap::VertexId Index;
  bool IsReflex;
  bool IsBase;
};

bool FillRule_PositiveWinding(s32 winding, s32 depth)
{
  return winding > 0;
}

bool FillRule_NonZeroWinding(s32 winding, s32 depth)
{
  return winding != 0;
}

bool FillRule_EvenOddWinding(s32 winding, s32 depth)
{
  s32 value = winding < 0 ? -winding : winding;
  return value & 1;
}

bool FillRule_All(s32 winding, s32 depth)
{
  return true;
}

typedef bool (*FillRuleFunc)(s32, s32);

static const FillRuleFunc fillRuleFunctions[4] = 
{ 
  FillRule_NonZeroWinding,
  FillRule_EvenOddWinding,
  FillRule_All,
  FillRule_PositiveWinding
};

bool EqualsExact(const Vec2& a, const Vec2& b)
{
  return a.x == b.x && a.y == b.y;
}

bool IsVertexConvex(ContourVertex* curr, const Array<Vec2>& vertexBuffer, InList<ContourVertex>& vertices)
{
  ContourVertex* prev = vertices.PrevWrap(curr);
  ContourVertex* next = vertices.NextWrap(curr);

  Vec2 p1 = vertexBuffer[prev->Index];
  Vec2 p2 = vertexBuffer[curr->Index];
  Vec2 p3 = vertexBuffer[next->Index];

  return Cross(p2 - p1, p3 - p1) > 0.000001f;
};

bool TriangulateMountain(const Array<Vec2>& vertexBuffer,
                         Array<ContourVertex>& mountain,
                         Array<uint>* indices)
{
  // Build chain so that the first and last vertices create the base edge
  InList<ContourVertex> vertices;
  for(size_t i = 0; i < mountain.Size(); ++i)
  {
    ContourVertex* vertex = &mountain[i];
    if(!vertices.Empty() && 
       EqualsExact(vertexBuffer[vertex->Index], vertexBuffer[vertices.Back().Index]))
    {
      continue;
    }

    vertices.PushBack(vertex);
  }

  // Build a separate list of convex vertices
  InList<ContourVertex, &ContourVertex::windingLink> convexList;
  forRange(ContourVertex& curr, vertices.All())
  {
    curr.IsReflex = !IsVertexConvex(&curr, vertexBuffer, vertices);
    if(!curr.IsReflex && !curr.IsBase)
    {
      convexList.PushBack(&curr);
    }
  }

  // Due to properties of monotone mountains, all convex vertices are ears
  while(!convexList.Empty())
  {
    ContourVertex* v2 = &convexList.Front();
    ContourVertex* v1 = vertices.PrevWrap(v2);
    ContourVertex* v3 = vertices.NextWrap(v2); 
    
    // Fill out the index buffer for this triangle
    indices->PushBack(v1->Index);
    indices->PushBack(v2->Index);
    indices->PushBack(v3->Index);

    vertices.Erase(v2);
    convexList.Erase(v2);

    // Removing the ear will potentially create new ears
    if(v1->IsReflex && !v1->IsBase && IsVertexConvex(v1, vertexBuffer, vertices))
    {
      v1->IsReflex = false;
      convexList.PushBack(v1);
    }

    if(v3->IsReflex && !v3->IsBase && IsVertexConvex(v3, vertexBuffer, vertices))
    {
      v3->IsReflex = false;
      convexList.PushBack(v3);
    }
  }

  return vertices.Empty();
}

}

bool BuildSet(const TrapezoidMap& map, Array<uint>* indices, s32 seed, FillRule::Type rule)
{
  FillRuleFunc func = fillRuleFunctions[rule];

  Array<bool> alreadyUsedLeft(map.mRegions.GetCapacity());
  for(size_t i = 0; i < alreadyUsedLeft.Size(); ++i)
  {
    alreadyUsedLeft[i] = false;
  } 
  Array<bool> alreadyUsedRight(map.mRegions.GetCapacity());
  for(size_t i = 0; i < alreadyUsedRight.Size(); ++i)
  {
    alreadyUsedRight[i] = false;
  }

  Array<ContourVertex> mountain;
  ContourVertex vertex;
  vertex.IsBase = false;

  for(s32 i = 0; i < map.mRegions.GetCapacity(); ++i)
  {
    if(seed != -1 && seed != i) continue;
    
    const TrapezoidMap::Region* region = map.mRegions.GetElement(i);
    if(region->NodeIndex != -1)
    {
      if(region->Depth > 0 && func(region->WindingOrder, region->Depth))
      {
        if(map.mEndIndex[region->TopVertex] != region->BotVertex &&
           map.mEndIndex[region->BotVertex] != region->TopVertex)
        {
          if(!alreadyUsedLeft[i])
          {
            TrapezoidMap::VertexId baseId = map.mTopIndex[region->LeftEdge];
            mountain.Clear();
            // Add base edge
            vertex.IsBase = true;
            vertex.Index = map.mTopIndex[region->LeftEdge];
            mountain.PushBack(vertex);
            vertex.Index = map.mBotIndex[region->LeftEdge];
            mountain.PushBack(vertex);
            vertex.IsBase = false;

            // Use region graph to get to bottom of base edge
            const TrapezoidMap::Region* curr = region;
            const TrapezoidMap::Region* prev = curr;

            while(map.mTopIndex[curr->LeftEdge] == baseId)
            {
              prev = curr;
              alreadyUsedLeft[map.mRegions.GetIndex(prev)] = true; 

              TrapezoidMap::RegionId next = curr->BotNeighbor[0];
              if(next == -1)
              {
                break;    
              }
              curr = map.mRegions.GetElement(next);
            }

            // Add diagonal
            curr = prev;
            while(curr->TopVertex != baseId && curr->TopVertex != -1)
            {
              // Add right edge endpoint
              vertex.Index = curr->TopVertex;
              mountain.PushBack(vertex);

              TrapezoidMap::RegionId next = curr->TopNeighbor[0];
              if(next == -1)
              {
                break;
              }
              curr = map.mRegions.GetElement(next);
              alreadyUsedLeft[map.mRegions.GetIndex(curr)] = true; 
            }

            TriangulateMountain(map.mVertices, mountain, indices);
          }

          if(!alreadyUsedRight[i])
          {
            TrapezoidMap::VertexId baseId = map.mTopIndex[region->RightEdge];
            mountain.Clear();

            // Add base edge
            vertex.IsBase = true;
            vertex.Index = map.mTopIndex[region->RightEdge];
            mountain.PushBack(vertex);
            vertex.Index = map.mBotIndex[region->RightEdge];
            mountain.PushBack(vertex);
            vertex.IsBase = false;

            // Use region graph to get to bottom of base edge
            const TrapezoidMap::Region* curr = region;
            const TrapezoidMap::Region* prev = curr;
            while(map.mTopIndex[curr->RightEdge] == baseId)
            {
              prev = curr;
              alreadyUsedRight[map.mRegions.GetIndex(prev)] = true;                                          
              TrapezoidMap::RegionId next = curr->BotNeighbor[1] != -1 
                                          ? curr->BotNeighbor[1]
                                          : curr->BotNeighbor[0];

              if(next == -1)
              {
                break;
              }
              curr = map.mRegions.GetElement(next);
            }

            // Add diagonal
            curr = prev;
            while(curr->TopVertex != baseId && curr->TopVertex != -1)
            {
              // Add right edge endpoint
              vertex.Index = curr->TopVertex;
              mountain.PushBack(vertex);
              TrapezoidMap::RegionId next = curr->TopNeighbor[1] != -1 
                                          ? curr->TopNeighbor[1]
                                          : curr->TopNeighbor[0];
              if(next == -1)
              {
                break;
              }
              curr = map.mRegions.GetElement(next);
              alreadyUsedRight[map.mRegions.GetIndex(curr)] = true;     
            }

            if(mountain.Empty() == false)
            {
              Zero::Reverse(mountain.Begin(), mountain.End());
              TriangulateMountain(map.mVertices, mountain, indices); 
            }
          }
        }
      }
    }
  }

  for(s32 i = 0; i < map.mRegions.GetCapacity(); ++i)
  {
    if(seed != -1 && seed != i)
    {
      continue;
    }
    
    if(alreadyUsedLeft[i] || alreadyUsedRight[i])
    {
      continue;
    }

    const TrapezoidMap::Region* region = map.mRegions.GetElement(i);
    if(region->NodeIndex != -1)
    {
      if(region->Depth > 0 && func(region->WindingOrder, region->Depth))
      {
          if(map.mTopIndex[region->LeftEdge] == map.mTopIndex[region->RightEdge])
          {
              const TrapezoidMap::Region* below = map.mRegions.GetElement(region->BotNeighbor[0]);
          
                if(!map.IsAboveInternal(map.mBotIndex[region->LeftEdge], map.mBotIndex[region->RightEdge]))
                {
                  TrapezoidMap::VertexId baseId = map.mTopIndex[region->LeftEdge];
                  mountain.Clear();
                  // Add base edge
                  vertex.IsBase = true;
                  vertex.Index = map.mTopIndex[region->LeftEdge];
                  mountain.PushBack(vertex);
                  vertex.Index = map.mBotIndex[region->LeftEdge];
                  mountain.PushBack(vertex);
                  vertex.IsBase = false;

                  // Use region graph to get to bottom of base edge
                  const TrapezoidMap::Region* curr = region;
                  const TrapezoidMap::Region* prev = curr;

                  while(map.mTopIndex[curr->LeftEdge] == baseId)
                  {
                    prev = curr;
                    alreadyUsedLeft[map.mRegions.GetIndex(prev)] = true; 

                    TrapezoidMap::RegionId next = curr->BotNeighbor[0];
                    if(next == -1)
                    {
                      break;    
                    }
                    curr = map.mRegions.GetElement(next);
                  }

                  // Add diagonal
                  curr = prev;
                  while(curr->TopVertex != baseId && curr->TopVertex != -1)
                  {
                    // Add right edge endpoint
                    vertex.Index = curr->TopVertex;
                    mountain.PushBack(vertex);

                    TrapezoidMap::RegionId next = curr->TopNeighbor[0];
                    if(next == -1)
                    {
                      break;
                    }
                    curr = map.mRegions.GetElement(next);
                    alreadyUsedLeft[map.mRegions.GetIndex(curr)] = true; 
                  }

                  TriangulateMountain(map.mVertices, mountain, indices);
                }
                else
                {
                  TrapezoidMap::VertexId baseId = map.mTopIndex[region->RightEdge];
                  mountain.Clear();

                  // Add base edge
                  vertex.IsBase = true;
                  vertex.Index = map.mTopIndex[region->RightEdge];
                  mountain.PushBack(vertex);
                  vertex.Index = map.mBotIndex[region->RightEdge];
                  mountain.PushBack(vertex);
                  vertex.IsBase = false;

                  // Use region graph to get to bottom of base edge
                  const TrapezoidMap::Region* curr = region;
                  const TrapezoidMap::Region* prev = curr;
                  while(map.mTopIndex[curr->RightEdge] == baseId)
                  {
                    prev = curr;
                    alreadyUsedRight[map.mRegions.GetIndex(prev)] = true;                                          
                    TrapezoidMap::RegionId next = curr->BotNeighbor[1] != -1 
                                                ? curr->BotNeighbor[1]
                                                : curr->BotNeighbor[0];

                    if(next == -1)
                    {
                      break;
                    }
                    curr = map.mRegions.GetElement(next);
                  }

                  // Add diagonal
                  curr = prev;
                  while(curr->TopVertex != baseId && curr->TopVertex != -1)
                  {
                    // Add right edge endpoint
                    vertex.Index = curr->TopVertex;
                    mountain.PushBack(vertex);
                    TrapezoidMap::RegionId next = curr->TopNeighbor[1] != -1 
                                                ? curr->TopNeighbor[1]
                                                : curr->TopNeighbor[0];
                    if(next == -1)
                    {
                      break;
                    }
                    curr = map.mRegions.GetElement(next);
                    alreadyUsedRight[map.mRegions.GetIndex(curr)] = true;     
                  }

                  if(mountain.Empty() == false)
                  {
                    Zero::Reverse(mountain.Begin(), mountain.End());
                    TriangulateMountain(map.mVertices, mountain, indices); 
                  }
                }
          }
      }
    }
  }
  
  return true;
}

bool Triangulate(const Array<Vec2>& vertices,
                 const Array<uint>& contourSizes,
                 Array<uint>* indices,
                 FillRule::Type rule)
{
  TrapezoidMap map(vertices, contourSizes, -1, uint(0));
  return map.IsValid() && BuildSet(map, indices, -1, rule);
}

bool Triangulate(const Array<Vec2>& vertices, Array<uint>* indices)
{
  Array<uint> contourSizes(1);
  contourSizes[0] = vertices.Size();
  return Triangulate(vertices, contourSizes, indices);
}


} // Geometry

