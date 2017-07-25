///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Ryan Edgemon
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class HeightMap;
class HeightMapTool;
struct HeightPatch;
class CellRange;
class ViewportMouseEvent;
class HeightMapUndoRedo;
class HeightPatchUndoRedo;
class WeightMapUndoRedo;

/// Declaration of all tools
DeclareEnum5(HeightTool, CreateDestroy, RaiseLower, SmoothSharpen, Flatten, WeightPainter);

//---------------------------------------------------------- Height Map Sub Tool
class HeightMapSubTool : public Object
{
public:
  /// Meta
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Virtual destructor
  virtual ~HeightMapSubTool(){}

  virtual bool LeftMouseDown(HeightMap* map, ViewportMouseEvent* e);
  virtual void LeftMouseMove(HeightMap* map, ViewportMouseEvent* e){}
  virtual bool LeftMouseUp(HeightMap* map, ViewportMouseEvent* e);
  virtual bool MouseScroll(HeightMap* map, ViewportMouseEvent* e);
  virtual void Refresh(HeightMap* map){}

  /// Draws the tool (has its own implementation to draw the cells that will be affected)
  virtual void Draw(HeightMap* map);

public:

  /// A pointer back to our owner
  HeightMapTool* mOwner;

  /// The local position (x,z on the terrain) that the tool cursor is at
  Vec2 mLocalToolPosition;
};

//----------------------------------------------------- Height Manipulation Tool
/// An abstract tool for any sort of height map manipulation
class HeightManipulationTool : public HeightMapSubTool
{
public:
  /// Meta
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor
  HeightManipulationTool();

  /// HeightMapSubTool interface
  bool LeftMouseUp(HeightMap* map, ViewportMouseEvent* e) override;
  bool LeftMouseDown(HeightMap* map, ViewportMouseEvent* e) override;
  void LeftMouseMove(HeightMap* map, ViewportMouseEvent* e) override;
  bool MouseScroll(HeightMap* map, ViewportMouseEvent* e) override;
  void Draw(HeightMap* map) override;

  /// Gets or sets the radius of the brush
  float GetRadius();
  void SetRadius(float value);

  /// Gets or sets the feather radius of the brush
  float GetFeatherRadius();
  void SetFeatherRadius(float value);

protected:

  /// Apply a particular function to a cell query (such as adjusting height)
  /// The mode that is passed in tells us what input the user was giving us
  virtual void ApplyToCells(HeightMapCellRange& range, ViewportMouseEvent* e) = 0;

  /// Perform the cell query, and returns if the query grabbed anything
  void PerformQuery(HeightMap* map, ViewportMouseEvent* e);

  /// Tell anyone who wants to know when the radius changes
  virtual void OnRadiusChanged(){}

public:
  /// The radius of the tool
  float mRadius;

  /// The feather radius (controls how soft the edges are in addition to the main radius)
  float mFeatherRadius;

  /// Stores required information to perform an undo or redo of any manipulation tool operation.
  HeightMapUndoRedo* mOperation;

  /// Stores required information to record brush strokes for undo/redo
  HashMap<HeightMap*, HeightMapStateManager> mAlteredMaps;
};

//------------------------------------------------------------- Raise/Lower Tool
/// A tool for raising and lowering the height map
class RaiseLowerTool : public HeightManipulationTool
{
public:
  /// Meta
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor
  RaiseLowerTool();

  /// HeightManipulationTool interface
  void ApplyToCells(HeightMapCellRange& range, ViewportMouseEvent* e) override;

public:

  /// The amount of strength used when raising or lowering the height map
  float mStrength;

  /// Whether or not the tool uses updated data every frame, or uses old data
  bool mRelative;
};

//---------------------------------------------------------- Smooth/Sharpen Tool
/// A tool for smoothing or sharpening the height map
class SmoothSharpenTool : public HeightManipulationTool
{
public:
  /// Meta
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor
  SmoothSharpenTool();

  /// HeightManipulationTool interface
  void ApplyToCells(HeightMapCellRange& range, ViewportMouseEvent* e) override;
  void OnRadiusChanged() override;
  void Refresh(HeightMap* map) override;

  /// Determines the number of samples based on brush radius
  void DetermineSamples(HeightMap* map);

private:

  /// Smooths the query
  void Smooth(HeightMapCellRange& range);

  /// Sharpens the query
  void Sharpen(HeightMapCellRange& range);

public:

  /// The amount of strength used in smoothing or sharpening
  float mStrength;

  /// How many uniform samples we make when smoothing (high values will go slow!)
  int mUniformSamples;

  /// How many random samples we make when smoothing
  int mRandomSamples;

  /// How far out from any given cell do we sample randomly
  int mRandomSampleDistance;

  /// Tells us if we should automatically determine the sample size
  bool mAutoDetermineSamples;

  /// Random number generator
  Math::Random mRandom;
};

//----------------------------------------------------------------- Flatten Tool
/// A tool for flattening out areas of the height map
class FlattenTool : public HeightManipulationTool
{
public:
  /// Meta
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor
  FlattenTool();

  /// HeightManipulationTool interface
  void ApplyToCells(HeightMapCellRange& range, ViewportMouseEvent* e) override;

  /// HeightMapSubTool interface
  bool LeftMouseDown(HeightMap* map, ViewportMouseEvent* e) override;

public:

  /// The amount of strength used to flatten [0, 1] where 0 is no strength, 1 is full strength
  float mStrength;

  /// The height that we want to flatten to (usually sampled and not set directly)
  float mHeight;

  /// The plane normal that we flatten the height map to (straight up means flat)
  Vec3 mSlopeNormal;

  /// Flattens the height map to the point where clicked
  bool mSampleOnMouseDown;

  /// Whether we sample the normal when sampling the height map
  bool mSampleNormal;
};


//---------------------------------------------------------- Create/Destroy Tool
/// A tool for creating or destroying patches of the terrain
class CreateDestroyTool : public HeightMapSubTool
{
public:
  /// Meta
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor
  CreateDestroyTool();

  /// HeightMapSubTool interface
  bool LeftMouseDown(HeightMap* map, ViewportMouseEvent* e) override;
  void LeftMouseMove(HeightMap* map, ViewportMouseEvent* e) override;
  bool LeftMouseUp(HeightMap* map, ViewportMouseEvent* e) override;
  void Draw(HeightMap* map) override;

public:

  /// The height that we create the patch at
  float mBaseHeight;

  /// Whether or not we use perlin noise to generate the terrain
  bool mUsePerlinNoise;

  /// The frequency of perlin noise that we use to generate the terrain
  float mPerlinFrequency;

  /// The large the perlin noise is allowed to make the terrain
  float mPerlinAmplitude;

  /// Stores required information to perform an undo or redo of a create or destroy operation.
  HeightPatchUndoRedo* mOperation;
};

//---------------------------------------------------------- WeightPainter Tool

/// Declaration of WeightPainter texture channels
DeclareEnum4(HeightTextureSelect, Texture0, Texture1, Texture2, Texture3);

class WeightPainterTool : public HeightMapSubTool
{
public:
  /// Meta
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor
  WeightPainterTool();

  bool LeftMouseDown(HeightMap* map, ViewportMouseEvent* e) override;
  void LeftMouseMove(HeightMap* map, ViewportMouseEvent* e) override;
  bool LeftMouseUp(HeightMap* map, ViewportMouseEvent* e) override;
  void Draw(HeightMap* map) override;
  void Paint(HeightMap* map);
  bool MouseScroll(HeightMap* map, ViewportMouseEvent* e) override;

public:
  // Texture index that is being painted
  HeightTextureSelect::Enum mTextureChannel;

  float mStrength;
  float mRadius;
  float mFeatherRadius;
  float mTarget;

  WeightMapUndoRedo* mOperation;
};

//-------------------------------------------------------- HeightMapMouseCapture

class HeightMapMouseCapture : public MouseManipulation
{
public:
  typedef HeightMapMouseCapture ZilchSelf;

  HandleOf<ReactiveViewport> mViewport;
  HandleOf<HeightMapTool> mHeightMapTool;

  HeightMapMouseCapture(Mouse* mouse, Viewport* Viewport, HeightMapTool* tool);
  virtual ~HeightMapMouseCapture( );

  // Named MouseEvent Handlers
  void OnLeftMouseUp(MouseEvent* event) override;
  void OnMouseScroll(MouseEvent* event) override;

  // Generic MouseEvent Handlers
  void OnMouseMove(MouseEvent* event) override;

  // Named, but not inherited, MouseEvent Handlers
  void OnLeftMouseDrag(MouseEvent* event);
};

//-------------------------------------------------------------- Height Map Tool
/// Declaration debug index type
DeclareEnum2(CellIndexType, Local, Absoulte)

class HeightMapTool : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor and destructor
  HeightMapTool();
  ~HeightMapTool();

  /// Component Interface.
  void Initialize(CogInitializer& initializer);

  /// Event response.
  void OnToolActivate(Event* e);
  void OnToolDeactivate(Event* e);
  void OnLeftMouseDown(ViewportMouseEvent* e);
  void OnLeftMouseDrag(ViewportMouseEvent* e);
  void OnLeftMouseUp(ViewportMouseEvent* e);
  void OnMouseMove(ViewportMouseEvent* e);
  void OnMouseScroll(ViewportMouseEvent* e);
  void OnToolDraw(Event* e);
  void OnSelectionFinal(SelectionChangedEvent* e);
  void OnLeftClickAddHeightMapWidget(MouseEvent* e);
  void OnLeftMouseUpAddHeightMapWidget(MouseEvent* e);

  void DebugDraw() override;
  void DrawDebugIndexes( );

  void CreateHeightMap();

  /// Gets the height map we are currently modifying (or NULL)
  HeightMap* GetHeightMap();

  /// Get or set the current tool
  HeightTool::Enum GetCurrentTool();
  void SetCurrentTool(HeightTool::Enum tool);

private:

  /// Tells the sub-tool where the current mouse position is at
  /// Returns true if the mouse intersects the map, false otherwise
  bool SetupLocalPosition(HeightMap* map, Viewport* viewport, ViewportMouseEvent* e);

  /// Sets the selection to a new selection
  void SetEditingMap(CogId cog, HeightMap* map);

private:
  CogId mLastEdited;
  HandleOf<ViewportTextWidget> mAddHeightMapWidget;

  HandleOf<HeightMapMouseCapture> mMouseCapture;

  ///Debug Options
  bool mShowPatchIndex;
  bool mShowCellIndex;
  CellIndexType::Enum mCellIndexType;

  /// The current tool we have selected (same as sub tool)
  HeightTool::Enum mCurrentTool;

  /// The current sub tool we have selected
  HeightMapSubTool* mSubTool;

  /// All of the sub tools we can select from
  Array<HeightMapSubTool*> mSubTools;

  /// Store the last selected cog with a height map component
  CogId mSelection;
};

}//namespace Zero
