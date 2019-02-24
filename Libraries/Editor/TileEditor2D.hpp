// MIT Licensed (see LICENSE.md).
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
  TileEditor2DOperation()
  {
    mName = "TileEditor2D Operation";
  }

  void Undo() override;
  void Redo() override;

  UndoHandleOf<Cog> mTileMapHandle;
  TileMapChangeList mChanges;
};

class TileEditor2DSubTool : public Object
{
public:
  ZilchDeclareType(TileEditor2DSubTool, TypeCopyMode::ReferenceType);

  TileEditor2DSubTool(TileEditor2D* owner);
  virtual ~TileEditor2DSubTool(){};

  void StartPrimaryAction(TileMap* map);
  void StartSecondaryAction(TileMap* map);
  void ContinueAction(TileMap* map);
  void EndAction(TileMap* map);

  virtual void Draw(TileMap* map){};

  virtual void PrimaryStart(TileMap* map){};
  virtual void PrimaryContinue(TileMap* map){};
  virtual void PrimaryEnd(TileMap* map){};

  virtual void SecondaryStart(TileMap* map){};
  virtual void SecondaryContinue(TileMap* map){};
  virtual void SecondaryEnd(TileMap* map){};

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
  ZilchDeclareType(TileEditor2DDrawTool, TypeCopyMode::ReferenceType);

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
  ZilchDeclareType(TileEditor2DSelectTool, TypeCopyMode::ReferenceType);

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

/// <Commands>
///   <command name = "TileDraw">
///     <shortcut> Shift + 1 </shortcut>
///     <description>
///       Set the TileEditor's sub-tool to the tile DrawTool.
///     </description>
///   </command>
///   <command name = "TileSelection 1">
///     <shortcut> Shift + 2 </shortcut>
///     <description>
///       Set the TileEditor's sub-tool to the tile SelectionTool.
///     </description>
///   </command>
///   <command name = "TileSelection 2">
///     <shortcut> Shift + LeftClick (in Editor Viewport) </shortcut>
///     <description>
///       Set the TileEditor's sub-tool to the tile SelectionTool.
///     </description>
///   </command>
///   <command name = "PrimaryBrushAction">
///     <shortcut> LeftMouse + Drag </shortcut>
///     <description>
///       DrawTool:\Paint tiles using the current brush.\ \
///       SelectionTool:\Select a collection of tiles bound by the initial drag
///       point and current mouse cursor position.
///     </description>
///   </command>
///   <command name = "SecondaryBrushAction">
///     <shortcut> RightMouse + Drag </shortcut>
///     <description>
///       DrawTool:\Erase tiles using the current brush size.\ \
///       SelectionTool:\Commit the current tile-selection to be used as the new
///       brush.
///     </description>
///   </command>
///   <command name = "SelectTilePaletteBrush">
///     <shortcut> Esc </shortcut>
///     <description>
///       Return the brush's tile-selection to the current TilePalette
///       selection, or if there isn't one - return the brush to the default,
///       single-tile TilePalette setup.
///     </description>
///   </command>
///   <command name = "TilePaletteSelection">
///     <shortcut> LeftMouse + Drag\(in TilePalette view) </shortcut>
///     <description>
///       Select an area in the TilePalette view bound by the initial drag
///       point and current mouse cursor position.  The current brush will be
///       set to the tile configuration in this area.
///     </description>
///   </command>
///   <command name = "TilePaletteDeletion">
///     <shortcut> RightClick\(in TilePalette view) </shortcut>
///     <description>
///       Delete a single tile in the TilePalette view.
///     </description>
///   </command>
///   <command name = "TilePaletteCopy">
///     <shortcut> Shift + RightClick\(in TilePalette view) </shortcut>
///     <description>
///       Paste a copy of the current tile-selection at the click-point
///       in the TilePalette view.\ \
///       Note:\  - Current tile-selection may be in either the TilePalette view
///       or in any other view containing tiles recognized by the TileEditor2D
///       tool.
///     </description>
///   </command>
///   <command name = "TilePaletteZoom">
///     <shortcut> Ctrl + MouseScroll\(in TilePalette view) </shortcut>
///     <description>
///       Zoom in/out on the TilePalette view.
///     </description>
///   </command>
/// </Commands>
class TileEditor2D : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TileEditor2D, TypeCopyMode::ReferenceType);

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

} // namespace Zero
