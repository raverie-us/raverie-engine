////////////////////////////////////////////////////////////////////////////////
///
/// \file TileMap.hpp
///
/// Authors: Nathan Carlson
/// Copyright 2013, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class SpriteSource;
class PhysicsMesh;
class Archetype;
class TileMapSource;

DeclareEnum6(TileStatus, Valid, MissingArchetype, MissingPhysicsMesh, MissingSpriteSource, ConflictMeshCollider, ConflictMultiSprite);

template<>
struct HashPolicy<Vec2>
{
  inline size_t operator()(Vec2Param value) const
  {
    return HashUint(*(uint*)&value.x) +
           HashUint(*(uint*)&value.y);
  }
  inline bool Equal(Vec2Param left, Vec2Param right) const
  {
    return left.x == right.x &&
           left.y == right.y;
  }
};

struct SpriteOffset
{
  SpriteOffset() {}
  SpriteOffset(HandleOf<SpriteSource> source, IntVec2 spriteOffset) : sprite(source), offset(spriteOffset) {}

  HandleOf<SpriteSource> sprite;
  IntVec2 offset;
};

struct EndPoints
{
  EndPoints() : size(0) {}

  void AddPoint(Vec2Param point)
  {
    points[size] = point;
    ++size;
  }

  void RemovePoint(uint index)
  {
    points[index] = points[size - 1];
    --size;
  }

  Vec2 points[4];
  uint size;
};

typedef HashMap<Vec2, EndPoints> EdgeGraph;

struct MergeObject
{
  HandleOf<Archetype> archetype;
  Vec3 position;
  Array<SpriteOffset> sprites;
  EdgeGraph contours;
};

class TileMap : public Component
{
public:
  struct Tile
  {
    ResourceId ArchetypeResource;
    ResourceId SpriteResource;
    ResourceId CollisionResource;
    bool Merge;

    Tile();
    Tile(ResourceId archetype, ResourceId sprite, ResourceId collision, bool mergable);

    bool operator==(const Tile& tile) const;

    size_t Hash() const;

    Archetype* GetArchetypeResource( ) const;
    SpriteSource* GetSpriteResource( ) const;
    PhysicsMesh* GetCollisionResource( ) const;
  };

  HashMap<Tile, TileStatus::Enum> mValidTileTests;

  ZilchDeclareType(TypeCopyMode::ReferenceType);

  typedef HashMap<IntVec2, Tile> TileHashMap;
  typedef TileHashMap::range TileRange;
  typedef HashMap<IntVec2, HandleOf<SpriteSource> > SpriteHashMap;

  TileMap();
  ~TileMap();

  // Component interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void DebugDraw() override;
  void TransformUpdate(TransformUpdateInfo& info) override;

  void OnUpdate(UpdateEvent* event);
  void OnAllObjectsInitialized(CogInitializerEvent* event);

  void SaveToTileMapSource(Serializer& stream);
  void LoadFromTileMapSource(Serializer& stream);
  void OnLevelLoaded(ObjectEvent* event);

  TileMapSource* GetSource();

  void SetTile(IntVec2 gridPos, Tile tile);
  Tile GetTile(IntVec2 gridPos);
  TileRange GetTiles();

  String GetPaletteName();
  void SetPaletteName(StringParam name);

  void DrawContours(ByteColor color);

  TileStatus::Enum ValidTile(Tile tile);
  TileStatus::Enum ValidArchetype(Archetype* archetype, bool tilemapCollision, bool tilemapSprites);
  TileStatus::Enum ValidConfiguration(Cog* cog, bool tilemapCollision, bool tilemapSprites);

private:
  void Modified();
  void RefreshMultiSprite();
  void BuildContours();
  void MergeTile(IntVec2 gridPos, Tile tile);

  String FormatTileError(TileStatus::Enum status, IntVec2 pos, Tile tile);

  TileHashMap mTileMap;
  SpriteHashMap mSpriteMap;
  bool mModified;
  bool mDirtySprites;
  bool mDirtyContours;
  bool mBadTile;

  /// Source for tile map data
  HandleOf<TileMapSource> mSource;

  String mPaletteName;

  real mMeshThickness;

  uint mMergeCounter;
  typedef HashMap<IntVec2, uint> ProcessedHashMap;
  ProcessedHashMap mProcessed;
  Array<uint> mMergeId;
  typedef HashMap<uint, MergeObject> MergeObjectMap;
  MergeObjectMap mMergeObjects;

  Space* mTestSpace;
};

} // namespace Zero
