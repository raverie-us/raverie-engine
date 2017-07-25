///////////////////////////////////////////////////////////////////////////////
///
/// \file TilePaletteView.hpp
///
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Spacer;
class TileEditor2D;
class PaletteText;
class TilePaletteSprite;

//------------------------------------------------------------------------ Tile Palette Entry
struct TilePaletteEntry
{
  TilePaletteSprite* frame;
  TileMap::Tile tile;
};

//------------------------------------------------------------------------ Tile Palette Change
struct TilePaletteChange
{
  TilePaletteChange() {}
  TilePaletteChange(IntVec2 location) : mLocation(location) {}

  enum ChangeType {Edited, Created, Removed};

  IntVec2 mLocation;
  TileMap::Tile mOldTile;
  TileMap::Tile mNewTile;
  ChangeType mChangeType;
};

typedef Array<TilePaletteChange> TilePaletteChangeList;

//------------------------------------------------------------------------ Tile Sprite
class TilePaletteSprite : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  TilePaletteSprite(Composite* parent);
  ~TilePaletteSprite();

  void SetBackground(bool background);
  void SetFrame(uint frameIndex, SpriteSource* sprite);

  void SetFrameOverlay(StringParam textureName, UvRect uv);
  void RemoveFrameOverlay();

  // Get the size needed for this composite
  // if the frame to be given size
  Vec2 GetSizeNeeded(Vec2 frameSize);
  void UpdateTransform() override;

  bool mBorderEnabled;
  TextureView* mBackground;
  Element* mBorder;
  TextureView* mFrameDisplay;
  TextureView* mFrameOverlay;
};

//------------------------------------------------------------------------ Tile Palette Operation
class TilePaletteOperation : public Operation
{
public:
  TilePaletteOperation( ) { mName = "TilePalette Operation"; }
  void Undo() override;
  void Redo() override;

  UndoHandleOf<TilePaletteView> mTilePaletteView;
  UndoHandleOf<TilePaletteSource> mTilePaletteSource;
  TilePaletteChangeList mChangeList;
};

//------------------------------------------------------------------------ Tile Palette View
class TilePaletteView : public Composite
{
public:
  typedef HashMap<IntVec2, TilePaletteEntry> PaletteEntryMap;
  typedef PaletteEntryMap::pair PalettePair;

  static const String mCollisionTextureName;

  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TilePaletteView(Composite* parent, TileEditor2D* editor);
  ~TilePaletteView();

  void ActivatePaletteUi();
  void DeactivatePaletteUi();
  void ClearPaletteTiles();

  TilePaletteSource* GetTilePalette();
  void SetTilePalette(TilePaletteSource* paletteSource);

  PropertyState GetTilePaletteState();
  PropertyState GetArchetypeState();
  PropertyState GetSpriteState();
  PropertyState GetCollisionState();
  PropertyState GetMergeableState();

  Archetype* GetArchetype();
  SpriteSource* GetSprite();
  PhysicsMesh* GetCollision();
  bool GetMergeable();
  void SetArchetype(Archetype* newResource);
  void SetSprite(SpriteSource* newResource);
  void SetCollision(PhysicsMesh* newResource);
  void SetMergeable(bool mergeable);

  void SetDefaultResources();
  void SetResourcesFromEntry(TilePaletteEntry* entry);
  void SetSelectionBorderActive(bool active);
  void RefreshSelection();
  void CreateNewEntry(IntVec2 location, bool defaults = true);
  void DeleteEntry(IntVec2 location);

  void OnResourceRemoved(ResourceEvent* event);

  void OnMouseDownArea(MouseEvent* event);
  void OnMouseUpArea(MouseEvent* event);
  void OnMouseMoveArea(MouseEvent* event);
  void OnMouseScroll(MouseEvent* event);
  void OnRightMouseDownArea(MouseEvent* event);
  void OnKeyDown(KeyboardEvent* event);

  void UpdateTransform() override;
  Vec2 Measure(LayoutArea& data) override;

  IntVec2 GetTileLocation(Vec2 localOffset);

  void LoadPalette();

  void SetShowCollision(bool showCollision);
  void AddCollisionOverlays();
  void RemoveCollisionOverlays();
  TilePaletteSprite* CreateTilePaletteSprite(SpriteSource* sprite, PhysicsMesh* collision);

  void AddToPaletteSource(IntVec2 location, TileMap::Tile tile);
  void RemoveFromPaletteSource(IntVec2 location);

  void CreateChangeOperation();

  HandleOf<TilePaletteSource> mPaletteSource;
  HandleOf<Archetype> mArchetype;
  HandleOf<SpriteSource> mSprite;
  HandleOf<PhysicsMesh> mCollision;
  bool mMergeable;

  PaletteEntryMap mPaletteTiles;

  int mTileSize;
  ScrollArea* mScrollArea;
  Composite* mPaletteArea;

  Spacer* mSpacer;
  Element* mSelectionBorder;
  BaseDefinition* mActiveBorder;
  BaseDefinition* mInactiveBorder;

  IntVec2 mSelectionStart;
  IntVec2 mSelectionEnd;
  bool mMouseDown;
  TileEditor2D* mEditor;
  String mNewPaletteName;

  TilePaletteChangeList mPaletteChangeList;

  bool mShowCollision;

  PaletteText* mPaletteText;

  class SelectionRange
  {
  public:

    struct Selection
    {
      Selection(TilePaletteEntry* entry, IntVec2 location) : mEntry(entry), mLocation(location) {}
      TilePaletteEntry* mEntry;
      IntVec2 mLocation;
    };

    SelectionRange(PaletteEntryMap* paletteTiles, IntVec2 selectionStart, IntVec2 selectionEnd);

    bool Empty();
    Selection Front();
    void PopFront();

    PaletteEntryMap* mPaletteTiles;
    TilePaletteEntry* mCurrentEntry;
    IntVec2 mSelectionMin;
    IntVec2 mSelectionMax;
    IntVec2 mSelection;
  };
};

}