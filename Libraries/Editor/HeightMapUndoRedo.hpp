///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Ryan Edgemon
/// Copyright 2016-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////


#pragma once


namespace Zero
{

struct HeightMapCell;


//---------------------------------------------------- ModifiedHeightMapCell ---
struct ModifiedHeightMapCell
{
  PatchIndex PatchIndex;

  CellIndex Index;
  float OriginalHeight;
  float AppliedHeight;

  ModifiedHeightMapCell( ) : OriginalHeight(0.0f), AppliedHeight(0.0f) { Index.Set(-1,-1); }
  ModifiedHeightMapCell(const HeightMapCell& cell, float originalHeight, float height);
  void Set(const HeightMapCell& cell, float originalHeight, float height);
};

//-------------------------------------------------------- HeightMapUndoRedo ---
class HeightMapUndoRedo : public Operation
{
  public:
    typedef HashMap<int, ModifiedHeightMapCell> YAxisCells;

    HeightMapUndoRedo(HeightMap* heightMap, StringParam name = "HeightMapOperation");

    void AddCell(const HeightMapCell& cell, float preDeltaHeight, float height);

    void SetAABB(const HeightMapCellRange& cells);
    void UpdateAABB(const HeightMapCellRange& additionalCells);

    void ApplyHeightHelper(int useAppliedHeight);

    void Undo( ) override;
    void Redo( ) override;

  public:
    Vec2 mMin;
    Vec2 mMax;

    HandleOf<HeightMap> mHeightMap;
    UndoHandleOf<Cog> mHeightMapObject;
    HashMap<int, YAxisCells> mModifiedCells;
};

//------------------------------------------------------ HeightPatchUndoRedo ---
class HeightPatchUndoRedo : public Operation
{
  public:
    struct ModifiedPatch
    {
      ModifiedPatch( ) {}
      ModifiedPatch(bool create, PatchIndexParam index) : Create(create), Index(index) {}

      bool Create;
      PatchIndex Index;
    };

    HeightPatchUndoRedo(HeightMap* heightMap, StringParam name = "HeightPatchOperation");

    void AddPatch(bool create, PatchIndexParam index);
    void SetNoise(bool usePerlin, float baseHeight, float frequency, float amplitude);

    void Create(PatchIndex& index);
    void Destroy(PatchIndex& index);

    void Undo( ) override;
    void Redo( ) override;

  public:
    bool mUsePerlinNoise;
    float mBaseHeight;
    float mPerlinFrequency;
    float mPerlinAmplitude;

    UndoHandleOf<Cog> mHeightMapObject;
    Array<ModifiedPatch> mPatches;
};

//--------------------------------------------------- ModifiedWeightMapPixel ---
struct ModifiedWeightMapPixel
{
  PatchIndex PatchIndex;

  IntVec2 Coord;
  ByteColor OriginalWeight;
  ByteColor AppliedWeight;

  ModifiedWeightMapPixel( ) : OriginalWeight(0), AppliedWeight(0) { Coord.Set(-1, -1); }
  ModifiedWeightMapPixel(PatchIndexParam index, uint x, uint y, ByteColor originalWeight, ByteColor weight);
  void Set(PatchIndexParam index, uint x, uint y, ByteColor originalWeight, ByteColor weight);
};

//------------------------------------------------------ WeightPatchUndoRedo ---
class WeightMapUndoRedo : public Operation
{
  public:
    typedef HashMap<int, ModifiedWeightMapPixel> YAxisPixels;

    WeightMapUndoRedo(HeightMap* heightMap, StringParam name = "WeightPainterOperation");

    void AddPixel(PatchIndexParam index, uint x, uint y, ByteColor preDeltaWeight, ByteColor weight);

    void ApplyWeightHelper(ByteColor useAppliedWeight);

    void Undo( ) override;
    void Redo( ) override;

  public:
    UndoHandleOf<Cog> mHeightMapObject;
    HashMap<int, YAxisPixels> mModifiedPixels;
};

}//namespace Zero