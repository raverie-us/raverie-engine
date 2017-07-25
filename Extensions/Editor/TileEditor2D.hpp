////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2013, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Tool.hpp"

namespace Zero
{

class TileEditor2D;
class TilePaletteView;
class TilePaletteProperty;
class TilePaletteSource;
class ViewportTextWidget;
class ViewportMouseEvent;
class ToolUiEvent;

typedef TileMap::Tile Tile;

struct TileMapChange
{
  IntVec2 GridPos;
  Tile PrevTile;
  Tile NewTile;
};
typedef Array<TileMapChange> TileMapChangeList;

struct TileMapSelection
{
  Array<Tile> Tiles;
  uint Width;
  IntVec2 Offset;
};

DeclareEnum2(TileEditor2DSubToolType, DrawTool, SelectionTool);

class TileEditor2DOperation : public Operation
{
public:
  TileEditor2DOperation( ) { mName = "TileEditor2D Operation"; }

  void Undo() override;
  void Redo() override;

  UndoHandleOf<Cog> mTileMapHandle;
  TileMapChangeList mChanges;
};

class TileEditor2DSubTool : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TileEditor2DSubTool(TileEditor2D* owner);
  virtual ~TileEditor2DSubTool() {};

  void StartPrimaryAction(TileMap* map);
  void StartSecondaryAction(TileMap* map);
  void ContinueAction(TileMap* map);
  void EndAction(TileMap* map);

  virtual void Draw(TileMap* map) {};

  virtual void PrimaryStart(TileMap* map) {};
  virtual void PrimaryContinue(TileMap* map) {};
  virtual void PrimaryEnd(TileMap* map) {};

  virtual void SecondaryStart(TileMap* map) {};
  virtual void SecondaryContinue(TileMap* map) {};
  virtual void SecondaryEnd(TileMap* map) {};

  bool HasChange(TileMapChange& change);
  void CommitOperation(TileMap* map);

protected:
  TileEditor2D* mOwner;
  bool mPrimaryActive;
  bool mSecondaryActive;
  IntVec2 mGridPosition;
  TileMapChangeList mChanges;
};

class TileEditor2DDrawTool : public TileEditor2DSubTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TileEditor2DDrawTool(TileEditor2D* owner);

  void Draw(TileMap* map) override;
  void PrimaryStart(TileMap* map) override;
  void SecondaryStart(TileMap* map) override;
  void PrimaryContinue(TileMap* map) override;
  void SecondaryContinue(TileMap* map) override;
  void PrimaryEnd(TileMap* map) override;
  void SecondaryEnd(TileMap* map) override;

private:
  void DrawTile(TileMap* map);
  void EraseTile(TileMap* map);
  void ApplyChange(TileMap* map, TileMapChange& change);
  void ApplyChanges(TileMap* map, Array<TileMapChange>& changes);
};

class TileEditor2DSelectTool : public TileEditor2DSubTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TileEditor2DSelectTool(TileEditor2D* owner);

  void Draw(TileMap* map) override;
  void PrimaryStart(TileMap* map) override;
  void PrimaryContinue(TileMap* map) override;
  void SecondaryStart(TileMap* map) override;

private:
  bool mHasSelection;
  IntVec2 mStart;
  IntVec2 mEnd;
};

class TileEditor2D : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  static const char* cDefaultName;
  static const char* cDefaultArchetype;

  TileEditor2D();
  ~TileEditor2D();

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;

  void GeneratePhysicsMeshResource(const Array<Vec3>& originalPoints, StringParam name);
  void OnSelectionFinal(SelectionChangedEvent* event);
  void SetTileMapPalette(TilePaletteSource* tilePalette);

  /// Event response.
  void OnToolActivate(Event* e);
  void OnToolDeactivate(Event* e);
  void OnKeyDown(KeyboardEvent* e);
  void OnLeftMouseDown(ViewportMouseEvent* e);
  void OnRightMouseDown(ViewportMouseEvent* e);
  void OnMouseMove(ViewportMouseEvent* e);
  void OnMouseDragMove(ViewportMouseEvent* e);
  void OnMouseDragEnd(Event* e);
  void OnToolDraw(Event* e);
  void OnGetToolInfo(ToolUiEvent* e);
  void OnLeftMouseDownAddTileMapWidget(MouseEvent* e);
  void OnLeftClickAddTileMapWidget(MouseEvent* e);

  void CreateTileMap();

  TileMap* GetTileMap();
  IntVec2 GetMouseGridPosition(TileMap* map);

  const TileMapSelection& GetSelection();
  void SetSelection(const TileMapSelection& selection);

  TileEditor2DSubToolType::Enum GetToolType();
  void SetToolType(TileEditor2DSubToolType::Enum type);

  bool GetShowCollision();
  void SetShowCollision(bool showCollision);

  bool GetShowCoordinates();
  void SetShowCoordinates(bool showCoordinates);

  bool TiledDrawing();
  UvRect GetCollisionTextureUv(StringParam resourceName);
  void SetCustomSelection();

private:
  friend class TileEditor2DSubTool;
  friend class TilePaletteView;

  IntVec2 GridPositionFromWorld(Vec3 pos);
  void DebugDrawQuad(IntVec2 gridPos);

  void CommitOperation(TileMap* map, Array<TileMapChange>& changes);

  bool mActive;

  HandleOf<ViewportTextWidget> mAddTileMapWidget;

  TileEditor2DSubToolType::Enum mToolType;

  bool mShowCollision;
  bool mShowCoordinates;
  bool mShowArchetype;
  bool mShowInvalid;
  bool mShowGrid;
  bool mTiledDrawing;

  TileEditor2DSubTool* mCurrentTool;
  Array<TileEditor2DSubTool*> mSubTools;

  TileMapSelection mTileMapSelection;

  CogId mLastEdited;
  Vec3 mMousePos;
  TilePaletteView* mTilePalatte;
  TilePaletteProperty* mTilePaletteProperty;
  HandleOf<TilePaletteSource> mPaletteSource;

  HashMap<String, UvRect> mCollisionTextureUv;
};

}//namespace Zero
