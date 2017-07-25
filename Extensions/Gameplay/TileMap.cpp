////////////////////////////////////////////////////////////////////////////////
///
/// \file TileMap.cpp
///
/// Authors: Nathan Carlson
/// Copyright 2013, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace
{

IntVec2 TileOffset[] =
{
  IntVec2(-1, 0),
  IntVec2(0, 1),
  IntVec2(1, 0),
  IntVec2(0, -1),
};

struct TileEdge
{
  TileEdge() {}
  TileEdge(Vec2 a, Vec2 b) : p0(a), p1(b) {}
  Vec2 p0, p1;
};

TileEdge TileEdges[] =
{
  TileEdge(Vec2(-0.5, -0.5), Vec2(-0.5, 0.5)),
  TileEdge(Vec2(-0.5, 0.5), Vec2(0.5, 0.5)),
  TileEdge(Vec2(0.5, 0.5), Vec2(0.5, -0.5)),
  TileEdge(Vec2(0.5, -0.5), Vec2(-0.5, -0.5)),
};

bool FindEdge(const EdgeGraph& contours, Vec2 e0, Vec2 e1, TileEdge& edge)
{
  Vec2 e[2] = {e0, e1};

  for (uint i = 0; i < 2; ++i)
  {
    if (contours.Find(e[i]).Empty())
      continue;

    EdgeGraph::range range = contours.Find(e[i]);
    Vec2 p0 = range.Front().first;
    EndPoints& endPoints = range.Front().second;
    for (uint n = 0; n < endPoints.size; ++n)
    {
      Vec2 p1 = endPoints.points[n];
      if (p0.x == p1.x || p0.y == p1.y)
      {
        if ((p1 - p0).Dot(e1 - e0) > 0.0f)
        {
          // On TileEdge
          edge = TileEdge(p0, p1);
          return true;
        }
      }
    }

    //break;
  }

  return false;
}

bool OverlapEdges(EdgeGraph& contours, EdgeGraph& adjacent, Vec2 pos)
{
  bool overlap = false;

  Array<TileEdge> newEdges;

  for (uint i = 0; i < 4; ++i)
  {
    Vec2 edge[3];
    edge[0] = pos + TileEdges[i].p0;
    edge[2] = pos + TileEdges[i].p1;
    edge[1] = (edge[0] + edge[2]) * 0.5;

    TileEdge edgeA;
    TileEdge edgeB;

    if (!FindEdge(contours, edge[0], edge[1], edgeA))
      continue;
    if (!FindEdge(adjacent, edge[2], edge[1], edgeB))
      continue;

    uint equalAxis;
    uint overlapAxis;
    if (edgeA.p0.x == edgeA.p1.x)
    {
      equalAxis = 0;
      overlapAxis = 1;
    }
    else
    {
      equalAxis = 1;
      overlapAxis = 0;
    }

    Vec2 minA = edgeA.p0[overlapAxis] < edgeA.p1[overlapAxis] ? edgeA.p0 : edgeA.p1;
    Vec2 maxA = edgeA.p0[overlapAxis] > edgeA.p1[overlapAxis] ? edgeA.p0 : edgeA.p1;

    Vec2 minB = edgeB.p0[overlapAxis] < edgeB.p1[overlapAxis] ? edgeB.p0 : edgeB.p1;
    Vec2 maxB = edgeB.p0[overlapAxis] > edgeB.p1[overlapAxis] ? edgeB.p0 : edgeB.p1;

    if (minB[overlapAxis] >= maxA[overlapAxis])
      continue;
    if (maxB[overlapAxis] <= minA[overlapAxis])
      continue;

    if (minA[overlapAxis] != minB[overlapAxis])
    {
      if (minA == edgeA.p0)
        newEdges.PushBack(TileEdge(minA, minB));
      else
        newEdges.PushBack(TileEdge(minB, minA));
    }

    if (maxA[overlapAxis] != maxB[overlapAxis])
    {
      if (maxA == edgeA.p1)
        newEdges.PushBack(TileEdge(maxB, maxA));
      else
        newEdges.PushBack(TileEdge(maxA, maxB));
    }

    overlap = true;

    EndPoints& endPointsA = contours.Find(edgeA.p0).Front().second;
    if (endPointsA.size == 1)
    {
      contours.Erase(edgeA.p0);
    }
    else
    {
      for (uint n = 0; n < endPointsA.size; ++n)
      {
        if (endPointsA.points[n].x == edgeA.p1.x && endPointsA.points[n].y == edgeA.p1.y)
        {
          endPointsA.RemovePoint(n);
          break;
        }
      }
    }

    EndPoints& endPointsB = adjacent.Find(edgeB.p0).Front().second;
    if (endPointsB.size == 1)
    {
      adjacent.Erase(edgeB.p0);
    }
    else
    {
      for (uint n = 0; n < endPointsB.size; ++n)
      {
        if (endPointsB.points[n].x == edgeB.p1.x && endPointsB.points[n].y == edgeB.p1.y)
        {
          endPointsB.RemovePoint(n);
          break;
        }
      }
    }
  }

  if (!overlap)
    return false;

  for (uint i = 0; i < newEdges.Size(); ++i)
    contours[newEdges[i].p0].AddPoint(newEdges[i].p1);

  for (EdgeGraph::range range = adjacent.All(); range.Empty() == false; range.PopFront())
  {
    Vec2 key = range.Front().first;
    EndPoints& endPoints = range.Front().second;

    EndPoints& contour = contours[key];

    for (uint i = 0; i < endPoints.size; ++i)
      contour.AddPoint(endPoints.points[i]);
  }

  return true;
}

// Removes redundant vertices
void CombineCollinearEdges(Array<Vec2>& points)
{
  for (uint i = 0; i < points.Size(); ++i)
  {
    uint j = (i + 1) % points.Size();
    uint k = (i + 2) % points.Size();

    Vec2 p0 = points[i];
    Vec2 p1 = points[j];
    Vec2 p2 = points[k];

    real denom1 = p1.x - p0.x;
    real denom2 = p2.x - p1.x;

    if (denom1 == 0.0f || denom2 == 0.0f)
    {
      if (denom1 == denom2)
      {
        points.EraseAt(j);
        --i;
      }
    }
    else
    {
      real slope1 = (p1.y - p0.y) / denom1;
      real slope2 = (p2.y - p1.y) / denom2;
      if (slope1 == slope2)
      {
        points.EraseAt(j);
        --i;
      }
    }
  }
}

void FindFirstEdge(EdgeGraph& edges, Array<Vec2>& contour)
{
  for (EdgeGraph::range range = edges.All(); range.Empty() == false; range.PopFront())
  {
    if (range.Front().second.size == 1)
    {
      contour.PushBack(range.Front().first);
      contour.PushBack(range.Front().second.points[0]);
      edges.Erase(range.Front().first);
      return;
    }
  }
}

void ComputeContours(EdgeGraph& edges, Array<Vec2>& points, Array<uint>& contours)
{
  Array<Vec2> contour;
  FindFirstEdge(edges, contour);

  while (!edges.Empty())
  {
    Vec2 key = contour.Back();

    EdgeGraph::range range = edges.Find(key);

    EndPoints& endPoints = range.Front().second;

    Vec2 nextPoint;
    if (endPoints.size == 1)
    {
      nextPoint = endPoints.points[0];
      edges.Erase(key);
    }
    else
    {
      Vec2 edge = key - contour[contour.Size() - 2];

      uint index;
      float best = -Math::cInfinite;
      for (uint i = 0; i < endPoints.size; ++i)
      {
        Vec2 adjEdge = endPoints.points[i] - key;
        float angle = std::atan2(edge.x * adjEdge.y - edge.y * adjEdge.x, edge.x * adjEdge.x + edge.y * adjEdge.y);
        if (angle > best)
        {
          best = angle;
          index = i;
        }
      }

      nextPoint = endPoints.points[index];
      endPoints.RemovePoint(index);
    }

    if (edges.Find(nextPoint).Empty())
    {
      CombineCollinearEdges(contour);
      points.Append(contour.All());
      contours.PushBack(contour.Size());
      contour.Clear();
      FindFirstEdge(edges, contour);
    }
    else
    {
      contour.PushBack(nextPoint);
    }
  }
}

// Extrudes face back and connects all triangles
void CompletePhysicsMesh(Array<Vec3>& vertices, Array<uint>& indices, const Array<uint>& contours, real thickness)
{
  uint size = vertices.Size();
  for (uint i = 0; i < size; ++i)
    vertices.PushBack(vertices[i] + Vec3(0.0, 0.0, -thickness));

  uint indexCount = indices.Size();
  for (uint i = 0; i < indexCount; i += 3)
  {
    indices.PushBack(size + indices[i + 2]);
    indices.PushBack(size + indices[i + 1]);
    indices.PushBack(size + indices[i]);
  }

  uint start = 0;
  for (uint i = 0; i < contours.Size(); ++i)
  {
    uint end = start + contours[i];

    for (uint j = start; j < end; ++j)
    {
      uint i0 = j;
      uint i1 = j + size;
      uint i2 = (j + 1 - start) % (end - start) + start;
      uint i3 = (j + 1 - start) % (end - start) + start + size;

      indices.PushBack(i0);
      indices.PushBack(i1);
      indices.PushBack(i2);
      indices.PushBack(i2);
      indices.PushBack(i1);
      indices.PushBack(i3);
    }

    start += contours[i];
  }
}

} // namespace (anonymous)

ZilchDefineType(TileMap, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(Transform);
  ZeroBindDependency(MultiSprite);

  ZilchBindCustomGetterPropertyAs(GetSource, "Source");
  ZilchBindFieldProperty(mMeshThickness);
}

TileMap::TileMap()
  : mModified(false)
  , mDirtySprites(false)
  , mDirtyContours(false)
  , mBadTile(false)
  , mMergeCounter(0)
  , mTestSpace(nullptr)
{
}

//--------------------------------------------------------------------- Tile ---

TileMap::Tile::Tile( )
{
  ArchetypeResource = 0;
  SpriteResource = 0;
  CollisionResource = 0;
}

TileMap::Tile::Tile(ResourceId archetype, ResourceId sprite, ResourceId collision, bool mergable)
{
  ArchetypeResource = archetype;
  SpriteResource = sprite;
  CollisionResource = collision;
  Merge = mergable;
}

bool TileMap::Tile::operator==(const Tile& tile) const
{
  return (ArchetypeResource == tile.ArchetypeResource
    && SpriteResource == tile.SpriteResource
    && CollisionResource == tile.CollisionResource);
}

size_t TileMap::Tile::Hash( ) const
{
  size_t hash = 0;
  hash ^= (size_t)ArchetypeResource;
  hash ^= (size_t)SpriteResource;
  hash ^= (size_t)CollisionResource;
  return hash;
}

Archetype* TileMap::Tile::GetArchetypeResource( ) const
{
  return (Archetype*)Z::gResources->GetResourceManager(ZilchTypeId(Archetype))->
    GetResource(ArchetypeResource, ResourceNotFound::ReturnNull);
}

SpriteSource* TileMap::Tile::GetSpriteResource( ) const
{
  return (SpriteSource*)Z::gResources->GetResourceManager(ZilchTypeId(SpriteSource))->
    GetResource(SpriteResource, ResourceNotFound::ReturnNull);
}

PhysicsMesh* TileMap::Tile::GetCollisionResource( ) const
{
  return (PhysicsMesh*)Z::gResources->GetResourceManager(ZilchTypeId(PhysicsMesh))->
    GetResource(CollisionResource, ResourceNotFound::ReturnNull);
}


//------------------------------------------------------------------ TileMap ---

TileMap::~TileMap()
{
  if (mTestSpace)
    mTestSpace->Destroy();
  TileMapSourceManager* sourceManager = TileMapSourceManager::GetInstance();
}

void TileMap::Initialize(CogInitializer& initializer)
{
  Archetype* spaceArchetype = ArchetypeManager::Find(CoreArchetypes::DefaultSpace);

  mTestSpace = GetGameSession()->CreateEditorSpace(spaceArchetype);
  mTestSpace->has(TimeSpace)->SetPaused(true);
  mTestSpace->has(GraphicsSpace)->mActive = false;

  if(GetSpace()->IsEditorMode() == true)
  {
    BuildContours();

    for (TileMap::TileHashMap::range range = mTileMap.All(); range.Empty() == false; range.PopFront())
    {
      TileMap::TileHashMap::value_type pair = range.Front();
      IntVec2 pos = pair.first;
      Tile tile = pair.second;

      mSpriteMap[pos] = tile.GetSpriteResource();
    }

    RefreshMultiSprite();

    ConnectThisTo(initializer.mSpace, Events::FrameUpdate, OnUpdate);
  }
  else
  {
    ConnectThisTo(&initializer, Events::AllObjectsInitialized, OnAllObjectsInitialized);
  }

}

void TileMap::Serialize(Serializer& stream)
{
  if (stream.GetMode() == SerializerMode::Saving)
    SaveToTileMapSource(stream);

  SerializeResourceName(mSource, TileMapSourceManager);
  SerializeNameDefault(mPaletteName, String());
  SerializeNameDefault(mMeshThickness, real(1.2));

  if (stream.GetMode() == SerializerMode::Loading)
    LoadFromTileMapSource(stream);
}

void TileMap::DebugDraw()
{
}

void TileMap::TransformUpdate(TransformUpdateInfo& info)
{
  // infinite recursion
  //if (info.TransformFlags & TransformUpdateFlags::Scale)
  //  GetOwner()->has(Transform)->SetWorldScale(Vec3(1, 1, 1));
}

void TileMap::OnUpdate(UpdateEvent* event)
{
  if (mDirtySprites)
  {
    RefreshMultiSprite();
    mDirtySprites = false;
  }
}

void TileMap::OnAllObjectsInitialized(CogInitializerEvent* event)
{
  BuildContours();

  // After building the contours in a running game mTestSpace is no longer needed so we
  // should destroy it as it appears as a space when pressing F9 to edit running game
  mTestSpace->Destroy();
  mTestSpace = nullptr;

  Transform* mapTransform = GetOwner()->has(Transform);

  for (MergeObjectMap::range range = mMergeObjects.All(); range.Empty() == false; range.PopFront())
  {
    MergeObject& mergeObj = range.Front().second;

    bool processCollision = mergeObj.contours.Size() != 0;
    bool processSprites = mergeObj.sprites.Size() != 0;

    Archetype* archetype = mergeObj.archetype;
    Cog* cog = GetOwner()->GetSpace()->Create(archetype);

    if (!cog->has(Transform))
      cog->AddComponentByName("Transform");

    Transform* transform = cog->has(Transform);
    // Set local translation for child object
    transform->SetLocalTranslation(Vec3::cZero);

    if (processCollision)
    {
      if (!cog->has(MeshCollider))
        cog->AddComponentByName("MeshCollider");

      Array<Vec2> vertices2D;
      Array<uint> contours;
      Array<uint> indices;

      ComputeContours(mergeObj.contours, vertices2D, contours);

      bool triangulated = Geometry::Triangulate(vertices2D, contours, &indices);
      if (triangulated)
      {
        Array<Vec3> vertices;
        for (uint i = 0; i < vertices2D.Size(); ++i)
          vertices.PushBack(Math::ToVector3(vertices2D[i], mMeshThickness * 0.5f));

        CompletePhysicsMesh(vertices, indices, contours, mMeshThickness);

        HandleOf<PhysicsMesh> physmesh = PhysicsMesh::CreateRuntime();
        physmesh->Upload(vertices, indices);
        cog->has(MeshCollider)->SetPhysicsMesh(physmesh);
      }
    }

    if (processSprites)
    {
      MultiSprite* thisSprite = GetOwner()->has(MultiSprite);
      MultiSprite* multiSprite = cog->has(MultiSprite);

      if (!cog->has(MultiSprite))
      {
        cog->AddComponentByName("MultiSprite");
        multiSprite = cog->has(MultiSprite);

        // Copy all component data
        DataBlock block = SerializeObjectToDataBlock(thisSprite);
        SerializeObjectFromDataBlock(block, multiSprite);
        zDeallocate(block.Data);
      }

      for (uint i = 0; i < mergeObj.sprites.Size(); ++i)
      {
        Vec3 pos = mergeObj.position;
        IntVec2 location = mergeObj.sprites[i].offset;
        multiSprite->Set(location, mergeObj.sprites[i].sprite);
      }
    }

    transform->UpdateAll();

    cog->AttachTo(GetOwner());
  }

  Cog* owner = GetOwner();
  owner->has(MultiSprite)->mVisible = false;
}

void TileMap::SaveToTileMapSource(Serializer& stream)
{
  CogSavingContext* context = (CogSavingContext*)stream.GetSerializationContext();
  Archetype* archetype = context ? (Archetype*)context->SavingArchetype : 0;

  // Will return a new resource if it needs to be copied for any reason
  if(Z::gRuntimeEditor)
    mSource = (TileMapSource*)Z::gRuntimeEditor->NewResourceOnWrite(TileMapSourceManager::GetInstance(), ZilchTypeId(TileMap), "Source", GetSpace(), mSource, archetype, mModified);

  if (mSource)
  {
    mSource->mData = mTileMap;

    if (mSource->mContentItem)
      mSource->mContentItem->SaveContent();
  }

  mModified = false;
}

void TileMap::LoadFromTileMapSource(Serializer& stream)
{
  if (!mSource)
    return;

  mTileMap = mSource->mData;

  // Get space object is being created in
  CogCreationContext* context = (CogCreationContext*)stream.GetSerializationContext();
  if (!context)
    return;

  Space* space = context->mSpace;
  if (!space)
    return;

  if (!mSource->mBuilder)
    return;

  // Get the level that owns this resource
  String resourceIdName = mSource->mBuilder->GetResourceOwner();
  LevelManager* levelManager = LevelManager::GetInstance();
  Resource* levelOwner = levelManager->GetResource(resourceIdName, ResourceNotFound::ReturnNull);

  // If being loaded into a different level, mark space as modified after load
  if (levelOwner && levelOwner != space->GetCurrentLevel())
    ConnectThisTo(space, Events::SpaceLevelLoaded, OnLevelLoaded);
}

void TileMap::OnLevelLoaded(ObjectEvent* event)
{
  // Mark space as modified so resource can be copied
  GetOwner()->GetSpace()->MarkModified();
}

TileMapSource* TileMap::GetSource()
{
  return mSource;
}

void TileMap::SetTile(IntVec2 gridPos, Tile tile)
{
  if (tile.GetArchetypeResource() == nullptr)
  {
    if (mSpriteMap.Find(gridPos).Empty() == false)
    {
      mDirtySprites = true;
      mSpriteMap.Erase(gridPos);
    }

    TileHashMap::range range = mTileMap.Find(gridPos);
    if (range.Empty() == false)
    {
      if (range.Front().second.GetCollisionResource())
        mDirtyContours = true;
      mTileMap.Erase(gridPos);

      Modified();
    }
  }
  else
  {
    TileStatus::Enum status = ValidTile(tile);
    if (status != TileStatus::Valid)
    {
      String error = FormatTileError(status, gridPos, tile);
      DoNotifyWarning("TileMap Error", error);
      return;
    }

    SpriteSource* sprite = tile.GetSpriteResource( );

    if (mSpriteMap.Find(gridPos).Empty() == false)
    {
      if (*mSpriteMap[gridPos] != sprite)
      {
        mSpriteMap[gridPos] = sprite;
        mDirtySprites = true;
      }
    }
    else
    {
      mSpriteMap[gridPos] = sprite;
      mDirtySprites = true;
    }

    TileHashMap::range range = mTileMap.Find(gridPos);
    if (range.Empty() == false)
    {
      Tile& curTile = range.Front().second;
      if (curTile.GetCollisionResource() != tile.GetCollisionResource() || curTile.Merge != tile.Merge)
        mDirtyContours = true;
      else if (curTile.GetCollisionResource() && curTile.GetArchetypeResource() != tile.GetArchetypeResource())
        mDirtyContours = true;
    }
    else if (tile.GetCollisionResource())
    {
      mDirtyContours = true;
    }

    mTileMap[gridPos] = tile;

    Modified();
  }
}

TileMap::Tile TileMap::GetTile(IntVec2 gridPos)
{
  TileHashMap::range range = mTileMap.Find(gridPos);
  if (range.Empty() == false)
    return range.Front().second;

  return Tile();
}

TileMap::TileRange TileMap::GetTiles()
{
  return mTileMap.All();
}

String TileMap::GetPaletteName()
{
  return mPaletteName;
}

void TileMap::SetPaletteName(StringParam name)
{
  mPaletteName = name;
}

void TileMap::DrawContours(ByteColor color)
{
  if (mDirtyContours)
  {
    BuildContours();
    mDirtyContours = false;
  }

  Transform* mapTransform = GetOwner()->has(Transform);

  forRange (MergeObjectMap::value_type mergePair, mMergeObjects.All())
  {
    MergeObject& mergeObj = mergePair.second;
    forRange (EdgeGraph::value_type pair, mergeObj.contours.All())
    {
      Vec2 start = pair.first;
      Vec3 p0 = mapTransform->TransformPoint(Math::ToVector3(start, 0));

      EndPoints& endPoints = pair.second;
      for (uint i = 0; i < endPoints.size; ++i)
      {
        Vec2 end = endPoints.points[i];
        Vec3 p1 = mapTransform->TransformPoint(Math::ToVector3(end, 0));

        gDebugDraw->Add(Debug::Line(p0, p1).Color(color));
      }
    }
  }
}

TileStatus::Enum TileMap::ValidTile(Tile tile)
{
  /* METAREFACTOR - IsNotNullAndCantResolve
  if (tile.CollisionResource.IsNotNullAndCantResolve())
    return TileStatus::MissingPhysicsMesh;
  if (tile.SpriteResource.IsNotNullAndCantResolve())
    return TileStatus::MissingSpriteSource;
  */
  return ValidArchetype(tile.GetArchetypeResource(), tile.GetCollisionResource() != nullptr, tile.GetSpriteResource() != nullptr);
}

TileStatus::Enum TileMap::ValidArchetype(Archetype* archetype, bool tilemapCollision, bool tilemapSprites)
{
  CogCreationContext context;
  Cog* cog = mTestSpace->Create(archetype);
  if (!cog)
    return TileStatus::MissingArchetype;

  TileStatus::Enum status = ValidConfiguration(cog, tilemapCollision, tilemapSprites);

  cog->Destroy();

  return status;
}

TileStatus::Enum TileMap::ValidConfiguration(Cog* cog, bool tilemapCollision, bool tilemapSprites)
{
  forRange (Component* component, cog->GetComponents())
  {
    BoundType* componentType = ZilchVirtualTypeId(component);
    if (tilemapCollision && componentType->IsA(ZilchTypeId(Collider)))
    {
      Collider* collider = (Collider*)component;
      if (collider->mType != Collider::cMesh)
        return TileStatus::ConflictMeshCollider;
    }
    else if (tilemapSprites && componentType->IsA(ZilchTypeId(Graphical)))
    {
      if (!componentType->IsA(ZilchTypeId(MultiSprite)))
        return TileStatus::ConflictMultiSprite;
    }
  }

  return TileStatus::Valid;
}

void TileMap::Modified()
{
  mModified = true;
  GetSpace()->MarkModified();
}

void TileMap::RefreshMultiSprite()
{
  MultiSprite* sprites = GetOwner()->has(MultiSprite);

  sprites->Clear();

  for (SpriteHashMap::range range = mSpriteMap.All(); !range.Empty(); range.PopFront())
  {
    SpriteHashMap::value_type pair = range.Front();
    if (pair.second)
      sprites->Set(pair.first, pair.second);
  }

  //sprites->UpdateAabb();
  sprites->UpdateBroadPhaseAabb();
}

void TileMap::BuildContours()
{
  mMergeCounter = 0;
  mProcessed.Clear();
  mMergeId.Clear();
  mMergeObjects.Clear();
  forRange (TileHashMap::value_type pair, mTileMap.All())
    MergeTile(pair.first, pair.second);

  if (mBadTile)
  {
    String error = String::Format("One or more tiles in '%s' are invalid.", GetOwner()->GetName().c_str());
    DoNotifyWarning("TileMap Error", error);
    mBadTile = false;
  }
}

void TileMap::MergeTile(IntVec2 pos, Tile tile)
{
  TileStatus::Enum status;

  TileStatus::Enum* test = mValidTileTests.FindPointer(tile);
  if (test != nullptr)
    status = *test;
  else
  {
    status = ValidTile(tile);
    mValidTileTests[tile] = status;
  }

  if (status != TileStatus::Valid)
  {
    ZPrint("Error : %s\n", FormatTileError(status, pos, tile).c_str());
    mBadTile = true;
    return;
  }

  Vec2 worldPos = Vec2(pos.x + 0.5f, pos.y + 0.5f);

  PhysicsMesh* mesh = tile.GetCollisionResource();
  EdgeGraph contour;
  if (mesh)
  {
    const Array<Vec3>& vertices = mesh->GetVertexArray();

    Array<Vec2> points;
    uint size = vertices.Size() / 2;
    for (uint i = 0; i < size; ++i)
      points.PushBack(Vec2(vertices[i].x, vertices[i].y) + worldPos);

    for (uint i = 0; i < size; ++i)
      contour[points[i]].AddPoint(points[(i + 1) % size]);
  }

  SpriteSource* sprite = tile.GetSpriteResource( );

  // Check all tile adjacencies
  for (uint i = 0; i < 4; ++i)
  {
    // Skip all checks if this tile should not merge
    if (tile.Merge == false)
      break;

    IntVec2 adjPos = pos + TileOffset[i];

    // Skip if adjacent tile is unprocessed
    ProcessedHashMap::range found = mProcessed.Find(adjPos);
    if (found.Empty())
      continue;
    uint foundId = found.Front().second;

    // Skip if adjacent tile should not be merged
    TileHashMap::range adjPair = mTileMap.Find(adjPos);
    if (adjPair.Empty() == false)
    {
      if (adjPair.Front().second.Merge == false)
        continue;
    }

    // Skip if already merged with adjacent tile
    ProcessedHashMap::range foundThis = mProcessed.Find(pos);
    if (foundThis.Empty() == false)
    {
      if (mMergeId[foundId] == mMergeId[foundThis.Front().second])
        continue;
    }

    // Skip if archetypes don't match
    uint id = mMergeId[foundId];
    MergeObject& adjObj = mMergeObjects[id];
    if (*adjObj.archetype != tile.GetArchetypeResource())
      continue;

    // If this tile has not been processed
    if (foundThis.Empty())
    {
      if (adjObj.contours.Size() && contour.Size())
      {
        if (OverlapEdges(adjObj.contours, contour, worldPos))
        {
            if (sprite)
              adjObj.sprites.PushBack(SpriteOffset(sprite, pos));

            mProcessed[pos] = foundId;
        }
      }
      else if (!adjObj.contours.Size() && !contour.Size())
      {
        if (sprite)
          adjObj.sprites.PushBack(SpriteOffset(sprite, pos));

        mProcessed[pos] = foundId;
      }
    }
    else
    {
      // Attempt to merge adjacent outside contour
      uint thisId = mMergeId[foundThis.Front().second];
      MergeObject& thisObj = mMergeObjects[thisId];

      if (thisObj.contours.Size() && adjObj.contours.Size())
      {
        if (OverlapEdges(adjObj.contours, thisObj.contours, worldPos))
        {
          // Add sprites
          adjObj.sprites.Append(thisObj.sprites.All());

          // Remap mesh id and remove other mesh
          for (uint n = 0; n < mMergeId.Size(); ++n)
            if (mMergeId[n] == thisId)
              mMergeId[n] = id;
          mMergeObjects.Erase(thisId);
        }
      }
      else if (!thisObj.contours.Size() && !adjObj.contours.Size())
      {
        // Add sprites
        thisObj.sprites.Append(adjObj.sprites.All());

        // Remap mesh id and remove other mesh
        for (uint n = 0; n < mMergeId.Size(); ++n)
          if (mMergeId[n] == id)
            mMergeId[n] = thisId;
        mMergeObjects.Erase(id);
      }
    }
  }

  // Not merged with anything, create new
  ProcessedHashMap::range foundThis = mProcessed.Find(pos);
  if (foundThis.Empty())
  {
    MergeObject& newMerge = mMergeObjects[mMergeCounter];
    // newMerge.contours.Rehash(1024);

    newMerge.archetype = tile.GetArchetypeResource();
    newMerge.position = Vec3(real(pos.x), real(pos.y), 0.0);

    if (contour.Size())
      newMerge.contours = contour;

    if (sprite)
    {
      IntVec2 offset = IntVec2((int)Math::Floor(newMerge.position.x), (int)Math::Floor(newMerge.position.y));
      newMerge.sprites.PushBack(SpriteOffset(sprite, offset));
    }

    mMergeId.PushBack(mMergeCounter);
    mProcessed[pos] = mMergeCounter;
    ++mMergeCounter;
  }
}

String TileMap::FormatTileError(TileStatus::Enum status, IntVec2 pos, Tile tile)
{
  String error;
  switch (status)
  {
    case TileStatus::Valid: return String();

    case TileStatus::MissingArchetype:
      error = String::Format("Could not resolve Archetype %s", tile.GetArchetypeResource()->Name);
    break;
    case TileStatus::MissingPhysicsMesh:
      error = String::Format("Could not resolve PhysicsMesh %s", tile.GetCollisionResource()->Name);
    break;
    case TileStatus::MissingSpriteSource:
      error = String::Format("Could not resolve SpriteSource %s", tile.GetSpriteResource()->Name);
    break;
    case TileStatus::ConflictMeshCollider:
      error = "Archetype has a collider component that is not of type 'MeshCollider'";
    break;
    case TileStatus::ConflictMultiSprite:
      error = "Archetype has a graphical component that is not of type 'MultiSprite'";
    break;
  }

  return String::Format("Invalid tile at position (%d, %d) - %s", pos.x, pos.y, error.c_str());
}

} // namespace Zero
