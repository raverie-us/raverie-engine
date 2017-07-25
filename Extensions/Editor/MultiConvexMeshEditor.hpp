///////////////////////////////////////////////////////////////////////////////
///
/// \file MultiConvexMeshEditor.hpp
/// 
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Cog;
class EditorCameraController;
class GridDraw;
class MetaDropEvent;
class MouseEvent;
class MultiConvexMeshCollider;
class MultiConvexMeshEditor;
class PropertyView;
class Renderer;
class Space;
class SpriteSource;
class ToggleIconButton;
class Viewport;

//-------------------------------------------------------------------MultiConvexMeshDrawer
class MultiConvexMeshDrawer : public Widget
{
public:
  MultiConvexMeshDrawer(Composite* parent, MultiConvexMeshEditor* editor);

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  void DrawOuterContour(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect);
  void DrawPoints(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect);
  void DrawClosestPointOnEdge(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect);
  void DrawAutoComputedContours(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect);

  MultiConvexMeshEditor* mEditor;
};

//-------------------------------------------------------------------MultiConvexMeshPoint
/// A point in the MultiConvexMesh ui. This allows selection/movement/deletion of this point.
class MultiConvexMeshPoint : public Widget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MultiConvexMeshPoint(Composite* parent, MultiConvexMeshEditor* editor);
  MultiConvexMeshPoint(Composite* parent, MultiConvexMeshEditor* editor, Vec3Param worldPoint);

  void Setup(MultiConvexMeshEditor* editor, Vec3Param worldPoint);

  void OnHandleMouseEvent(MouseEvent* e);
  void OnLeftClick(MouseEvent* e);
  void OnLeftMouseDrag(MouseEvent* e);
  void OnRightMouseDown(MouseEvent* e);
  void OnMouseEnter(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);

  /// Response from right click context menu
  void OnDeletePoint(Event* e);

  /// Update the viewport position of this point from its world position
  void UpdateViewportPosition();

  void StartDrag(Mouse* mouse);

  /// Is the mouse currently over this point? Used for highlights.
  bool mMouseOver;
  Vec3 mWorldPoint;
  Vec3 mViewportPoint;
  MultiConvexMeshEditor* mEditor;
};

//-------------------------------------------------------------------PointMovementOp
/// Undo operation for a point changing position
class PointMovementOp : public Operation
{
public:
  PointMovementOp(MultiConvexMeshPoint* point, Vec3Param startPosition);

  void Undo() override;
  void Redo() override;

  void SetPosition(Vec3Param newPosition);

  uint mIndex;
  Vec3 mStartPosition, mEndPosition;
  MultiConvexMeshEditor* mEditor;
};

//-------------------------------------------------------------------PointAddRemoveOp
/// Undo operation for adding/removing a single point
class PointAddRemoveOp : public Operation
{
public:
  PointAddRemoveOp(MultiConvexMeshPoint* point, bool add);

  void Undo() override;
  void Redo() override;

  void PerformOp(bool add);

  uint mIndex;
  Vec3 mWorldPosition;
  bool mAdd;
  MultiConvexMeshEditor* mEditor;
};

/// How should a mesh be debug drawn.
DeclareEnum3(MultiConvexMeshDrawMode, None, Edges, Filled);
/// How should mesh points be snapped.
/// <param name="None">Don't auto-snap mesh points</param>
/// <param name="IfClose">Only snap a point if it's within some threshold of the grid</param>
/// <param name="Always">Always snap a point to the grid</param>
DeclareEnum3(MultiConvexMeshSnappingMode, None, IfClose, Always);
/// How should auto-computation of a mesh build boundaries? Should pixel boundaries be
/// respected (corners are preserved) or should marching squares be used?
DeclareEnum2(MultiConvexMeshAutoComputeMethod, Pixels, MarchingSquares);
/// How an image should be sampled for auto-computation. Should the boundary lines between
/// empty/solid cells be computed from a pixel's alpha value or from it's intensity value?
DeclareEnum2(MultiConvexMeshAutoComputeMode, Alpha, Intensity);

//-------------------------------------------------------------------MultiConvexMeshPropertyViewInfo
/// Structure bound to the property view for the main editor.
/// Contains the different settings that the user can modify.
struct MultiConvexMeshPropertyViewInfo : public EventObject
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MultiConvexMeshPropertyViewInfo();

  // Properties

  /// Since the mesh is on a 2d plane, they need some thickness for
  /// their z-depth. This controls how thick the meshes are.
  float GetMeshThickness();
  void SetMeshThickness(float thickness);
  /// The clear color of the viewport used to render.
  Vec4 GetClearColor();
  void SetClearColor(Vec4Param color);
  /// The color to draw edges with
  Vec4 GetOuterContourColor();
  void SetOuterContourColor(Vec4Param color);
  /// The sprite source used as a reference for drawing the mesh. Note: this is not always
  /// what's visible as the user can drag in archetypes to view as well.
  SpriteSource* GetSpriteSource();
  void SetSpriteSource(SpriteSource* source); 
  /// Resets the points of the mesh to an approximation for the current sprite.
  void AutoCompute();

  /// How thick of a mesh should be created?
  float mMeshThickness;
  /// What color should the background clear color use?
  Vec4 mClearColor;
  /// What color should the outer contour use?
  Vec4 mOuterContourColor;
  HandleOf<SpriteSource> mSpriteSource;
  /// How should the collection of meshes be drawn?
  MultiConvexMeshDrawMode::Enum mDrawMode;
  /// Should the auto-computed mesh be calculated from
  /// the alpha or the intensity of the sprite?
  MultiConvexMeshAutoComputeMode::Enum mAutoComputeMode;
  /// When the sprite is sampled using the AutoComputeMode, what value
  /// should be used to determine where a surface is.
  float mSurfaceLevelThreshold;
  /// A threshold to control when vertices should be removed (simplified).
  /// This value is related to the area of a triangle.
  float mSimplificationThreshold;
  /// What method of auto-computing should be used?
  /// Most likely 'pixel' is the mode that should be used.
  MultiConvexMeshAutoComputeMethod::Enum mAutoComputeMethod;

  MultiConvexMeshEditor* mEditor;
};

//-------------------------------------------------------------------MultiConvexMeshEditor
/// The main editor for drawing a contour to be decomposed into several convex meshes.
class MultiConvexMeshEditor : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MultiConvexMeshEditor(Composite* parent, MultiConvexMesh* mesh);

  void OnDestroy() override;

  void SetupPreviewSpace();
  void CreateToolbar(Composite* toolbarParent);

  MultiConvexMeshPoint* FindPointAtScreenPosition(Vec2Param screenPosition);
  /// Adds a point from the given screen position. Figures out what edge (if any)
  /// is the closest to this point and replaces it with two edges connecting to the new point. 
  void AddPointAtScreenPosition(Vec2Param screenPosition);
  /// Adds a point at a given index. By default queues an undo operation.
  MultiConvexMeshPoint* AddPointAt(uint index, Vec3Param worldPosition, bool queueUndo = true, bool testMesh = true);
  /// Removes a point and queues an undo operation.
  void RemovePoint(MultiConvexMeshPoint* point, bool queueUndo = true, bool testMesh = true);
  /// Clear all points so the mesh is empty
  void ClearPoints(bool queueUndo = true);
  /// Auto compute a best guess for the sprite
  void AutoCompute();

  /// Computes the world space point from the screen point
  /// and if need be snaps that point to the grid.
  Vec3 ScreenPointToSnappedWorldPoint(Vec2Param screenPosition);

  // Events
  void OnLeftMouseDown(MouseEvent* e);
  void OnRightMouseDown(MouseEvent* e);
  void OnAddPoint(ObjectEvent* e);
  void OnRemovePoint(ObjectEvent* e);
  void OnMiddleMouseDown(MouseEvent* e);
  void OnDoubleClick(MouseEvent* e);
  void OnLeftMouseDrag(MouseDragEvent* e);
  /// Use Mouse update instead of mouse move because this is used for
  /// edge highlight and which edge should be selected can change even
  /// when the mouse doesn't move (focus, camera movement, etc...)
  void OnMouseUpdate(MouseEvent* e);
  void OnMouseScroll(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);
  void OnKeyDown(KeyboardEvent* e);
  void OnWidgetUpdate(Event* e);
  void OnResourceRemoved(ResourceEvent* e);
  void OnMetaDrop(MetaDropEvent* e);
  void OnSelectedPointXChanged(Event* e);
  void OnSelectedPointYChanged(Event* e);
  void OnSelectedPointChangedGeneric(TextBox* changedTextBox, uint axisChanged);
  void OnToggleShowGrid(Event* e);
  void OnGridCellSizeChanged(Event* e);
  void OnSetGridSizeToPixels(Event* e);
  void OnChangeSnappingMode(Event* e);


  struct ClosestEdgeInfo
  {
    uint mClosestEdge;
    float mClosestDistance;
    Vec3 mClosestViewportPoint;
  };
  /// Find information about the closest edge to a point in screen space.
  void FindClosestEdge(Vec2Param testScreenPoint, ClosestEdgeInfo& info);
  void ClearSelectedEdge();

  //void SetSelectedPoint(MultiConvexMeshPoint* selectedPoint);
  void AddToSelection(MultiConvexMeshPoint* selectedPoint);
  void RemoveFromSelection(MultiConvexMeshPoint* selectedPoint);
  void DeleteSelection();
  void ClearSelection();
  /// Update the property view's text for the currently selected point.
  void UpdateSelectedPointText();

  static real SamplePixelAlpha(Vec2Param pixelCoord, void* userData);
  static real SamplePixelIntensity(Vec2Param pixelCoord, void* userData);
  static Vec2 SamplePixelWorldPosition(Vec2Param pixelCoord, void* userData);
  static Vec2 SamplePixelWorldPositionAtCenter(Vec2Param pixelCoord, void* userData);
  
  
  /// Set the MultiConvexMesh currently being edited.
  void SetMultiConvexMesh(MultiConvexMesh* multiConvexMesh);
  /// Updates the preview cog to either be a sprite source or an archetype.
  void UpdatePreview(SpriteSource* spriteSource, Archetype* archetype = NULL);
  /// Focuses on the combined aabb of the preview cog and the current mesh.
  void FocusOnPreviewCog();

  
  bool GetDrawGrid();
  /// What is the spacing (in world space) between the grid cells.
  real GetGridCellSize();
  void SetGridCellSize(real cellSize);
  /// Sets the grid cell size to based upon the pixels per unit of the current sprite source.
  void SetGridSizeToPixels();
  
  void UpdateGridDrawing();
  /// Mark that we've modified our resource.
  void MarkMeshModified();
  /// Tries to build convex meshes and if it fails it'll undo the last op (that likely caused the bad mesh)
  void TestConvexMeshes();
  /// Rebuild the convex mesh decomposition of the current contour.
  bool BuildConvexMeshes();
  void DrawMesh();
  

  MultiConvexMeshPropertyViewInfo mPropertyViewInfo;

  HandleOf<MultiConvexMesh> mMesh;
  Array<MultiConvexMeshPoint*> mPoints;
  /// Is the current mesh valid (did we fail at decomposition?)
  bool mIsValid;

  PropertyView* mPropertyView;
  GameWidget* mGameWidget;
  Viewport* mViewport;
  Component* mRenderer;
  Space* mPreviewSpace;
  Cog* mPreviewCog;
  GridDraw* mGridDraw;
  MultiConvexMeshCollider* mPreviewMesh;
  EditorCameraController* mCameraController;

  TextBox* mSelectedPointX;
  TextBox* mSelectedPointY;
  TextBox* mGridCellSizeTextBox;
  ToggleIconButton* mShowGridButton;
  StringSource mSnappingModeSource;
  ComboBox* mSnappingModeComboBox;
  
  /// Used to store where a right click menu was created so we
  /// can create a point at the location.
  Vec2 mCachedMousePosition;
  /// What points are currently selected. Used for displaying the current x and y values.
  typedef HashSet<MultiConvexMeshPoint*> SelectionSet;
  SelectionSet mSelection;

  /// The grid cell size (for drawing and snapping)
  float mGridCellSize;
  /// What snapping mode to use as the user drags around control points
  MultiConvexMeshSnappingMode::Type mSnappingMode;
  
  /// What edge was currently selected
  uint mSelectedEdge;
  /// This is only saved for debug drawing some information
  ClosestEdgeInfo mClosestEdgeInfo;
  /// Used to auto-compute the collision mesh (stored for debug drawing purposes)
  MarchingSquares mMarchingSquares;

  /// We have our own operation queue for undo/redo
  OperationQueue mQueue;
};

}//namespace Zero
