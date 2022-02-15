// MIT Licensed (see LICENSE.md).
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

class HeightMapSubTool : public Object
{
public:
  /// Meta
  ZilchDeclareType(HeightMapSubTool, TypeCopyMode::ReferenceType);

  /// Virtual destructor
  virtual ~HeightMapSubTool()
  {
  }

  virtual bool LeftMouseDown(HeightMap* map, ViewportMouseEvent* e);
  virtual void LeftMouseMove(HeightMap* map, ViewportMouseEvent* e)
  {
  }
  virtual bool LeftMouseUp(HeightMap* map, ViewportMouseEvent* e);
  virtual bool MouseScroll(HeightMap* map, ViewportMouseEvent* e);
  virtual void Refresh(HeightMap* map)
  {
  }

  /// Draws the tool (has its own implementation to draw the cells that will be
  /// affected)
  virtual void Draw(HeightMap* map);

public:
  /// A pointer back to our owner
  HeightMapTool* mOwner;

  /// The local position (x,z on the terrain) that the tool cursor is at
  Vec2 mLocalToolPosition;
};

/// An abstract tool for any sort of height map manipulation
class HeightManipulationTool : public HeightMapSubTool
{
public:
  /// Meta
  ZilchDeclareType(HeightManipulationTool, TypeCopyMode::ReferenceType);

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
  virtual void OnRadiusChanged()
  {
  }

public:
  /// The radius of the tool
  float mRadius;

  /// The feather radius (controls how soft the edges are in addition to the
  /// main radius)
  float mFeatherRadius;

  /// Stores required information to perform an undo or redo of any manipulation
  /// tool operation.
  HeightMapUndoRedo* mOperation;

  /// Stores required information to record brush strokes for undo/redo
  HashMap<HeightMap*, HeightMapStateManager> mAlteredMaps;
};

/// A tool for raising and lowering the height map
class RaiseLowerTool : public HeightManipulationTool
{
public:
  /// Meta
  ZilchDeclareType(RaiseLowerTool, TypeCopyMode::ReferenceType);

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

/// A tool for smoothing or sharpening the height map
class SmoothSharpenTool : public HeightManipulationTool
{
public:
  /// Meta
  ZilchDeclareType(SmoothSharpenTool, TypeCopyMode::ReferenceType);

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

  /// How many uniform samples we make when smoothing (high values will go
  /// slow!)
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

/// A tool for flattening out areas of the height map
class FlattenTool : public HeightManipulationTool
{
public:
  /// Meta
  ZilchDeclareType(FlattenTool, TypeCopyMode::ReferenceType);

  /// Constructor
  FlattenTool();

  /// HeightManipulationTool interface
  void ApplyToCells(HeightMapCellRange& range, ViewportMouseEvent* e) override;

  /// HeightMapSubTool interface
  bool LeftMouseDown(HeightMap* map, ViewportMouseEvent* e) override;

public:
  /// The amount of strength used to flatten [0, 1] where 0 is no strength, 1 is
  /// full strength
  float mStrength;

  /// The height that we want to flatten to (usually sampled and not set
  /// directly)
  float mHeight;

  /// The plane normal that we flatten the height map to (straight up means
  /// flat)
  Vec3 mSlopeNormal;

  /// Flattens the height map to the point where clicked
  bool mSampleOnMouseDown;

  /// Whether we sample the normal when sampling the height map
  bool mSampleNormal;
};

/// A tool for creating or destroying patches of the terrain
class CreateDestroyTool : public HeightMapSubTool
{
public:
  /// Meta
  ZilchDeclareType(CreateDestroyTool, TypeCopyMode::ReferenceType);

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

  /// Stores required information to perform an undo or redo of a create or
  /// destroy operation.
  HeightPatchUndoRedo* mOperation;
};

/// Declaration of WeightPainter texture channels
DeclareEnum4(HeightTextureSelect, Texture0, Texture1, Texture2, Texture3);

class WeightPainterTool : public HeightMapSubTool
{
public:
  /// Meta
  ZilchDeclareType(WeightPainterTool, TypeCopyMode::ReferenceType);

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

class HeightMapMouseCapture : public MouseManipulation
{
public:
  typedef HeightMapMouseCapture ZilchSelf;

  HandleOf<ReactiveViewport> mViewport;
  HandleOf<HeightMapTool> mHeightMapTool;

  HeightMapMouseCapture(Mouse* mouse, Viewport* Viewport, HeightMapTool* tool);
  virtual ~HeightMapMouseCapture();

  // Named MouseEvent Handlers
  void OnMouseUp(MouseEvent* event) override;
  void OnMouseScroll(MouseEvent* event) override;

  // Generic MouseEvent Handlers
  void OnMouseMove(MouseEvent* event) override;
};

/// Declaration debug index type
DeclareEnum2(CellIndexType, Local, Absoulte)

    /// <Commands>
    ///   <command name = "CreateDestroy">
    ///     <shortcut> Shift + 1 </shortcut>
    ///     <description>
    ///       Set the HeightMapTool's sub-tool to the CreateDestroy tool.
    ///     </description>
    ///   </command>
    ///   <command name = "RaiseLower">
    ///     <shortcut> Shift + 2 </shortcut>
    ///     <description>
    ///       Set the HeightMapTool's sub-tool to the RaiseLower tool.
    ///     </description>
    ///   </command>
    ///   <command name = "SmoothSharpen">
    ///     <shortcut> Shift + 3 </shortcut>
    ///     <description>
    ///       Set the HeightMapTool's sub-tool to the SmoothSharpen tool.
    ///     </description>
    ///   </command>
    ///   <command name = "Flatten">
    ///     <shortcut> Shift + 4 </shortcut>
    ///     <description>
    ///       Set the HeightMapTool's sub-tool to the Flatten tool.
    ///     </description>
    ///   </command>
    ///   <command name = "WeightPainter">
    ///     <shortcut> Shift + 5 </shortcut>
    ///     <description>
    ///       Set the HeightMapTool's sub-tool to the WeightPainter tool.
    ///     </description>
    ///   </command>
    ///   <command name = "ResizeBrush">
    ///     <shortcut> Shift + MouseScroll </shortcut>
    ///     <description>
    ///       Resize the brush size up or down depending on the scroll
    ///       direction.
    ///     </description>
    ///   </command>
    ///   <command name = "SubToolCommands">
    ///     <shortcut> Shift + LeftClick/LeftDrag </shortcut>
    ///     <description>
    ///       CreateDestroy:\Destroy patches instead of creating them.\ \
///       RaiseLower:\Lower height map with the brush instead of raising
    ///       it.\ \ SmoothSharpen:\Sharpen height map with the brush instead
    ///       of smoothing it.
    ///     </description>
    ///   </command>
    /// </Commands>
    class HeightMapTool : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(HeightMapTool, TypeCopyMode::ReferenceType);

  /// Constructor and destructor
  HeightMapTool();
  ~HeightMapTool();

  /// Component Interface.
  void Initialize(CogInitializer& initializer);

  /// Event response.
  void OnToolActivate(Event* e);
  void OnToolDeactivate(Event* e);
  void OnKeyDown(KeyboardEvent* e);
  void OnLeftMouseDown(ViewportMouseEvent* e);
  void OnLeftMouseUp(ViewportMouseEvent* e);
  void OnMouseMove(ViewportMouseEvent* e);
  void OnMouseScroll(ViewportMouseEvent* e);
  void OnToolDraw(Event* e);
  void OnSelectionFinal(SelectionChangedEvent* e);
  void OnLeftClickAddHeightMapWidget(MouseEvent* e);
  void OnLeftMouseUpAddHeightMapWidget(MouseEvent* e);

  void DebugDraw() override;
  void DrawDebugIndexes();

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

  /// Debug Options
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

} // namespace Zero
