// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

const float cMinSpriteImporterZoom = 0.1f;
const float cMaxSpriteImporterZoom = 10.0f;
const float cSpriteImporterZoomIncrement = 0.1f;

class SpriteSource;
class SpriteSheetImporter;
void SpriteSheetImport(Editor* editor);
void SpriteSheetImport(StringParam filename);
void SpriteSheetImport(SpriteSource* spriteSource);

DeclareEnum2(SelectionMode, CellSelect, FreeSelect);
DeclareEnum2(ImportFrames, AllFrames, SelectedFrames);

struct FrameArea
{
  bool Active;
  IntRect Rect;
};

// Grid Area
class PixelGridArea : public Widget
{
public:
  typedef PixelGridArea self_type;
  SpriteSheetImporter* mOwner;

  PixelGridArea(Composite* parent, SpriteSheetImporter* owner);

  void RenderUpdate(ViewBlock& viewBlock,
                    FrameBlock& frameBlock,
                    Mat4Param parentTx,
                    ColorTransform colorTx,
                    WidgetRect clipRect) override;
};

// Sprite Sheet Importer
class SpriteSheetImporter : public Composite
{
public:
  ZilchDeclareType(SpriteSheetImporter, TypeCopyMode::ReferenceType);

  SpriteSheetImporter(Composite* parent);

  void
  ComputeFrameWidthAndCount(int& frameSize, int& frameCount, int newFrameSize, int spacing, int sourceSize, int offset);
  void UpdateTexture();
  void LoadImages(Array<String>& files);
  void LoadSprite(SpriteSource* spriteSource);
  void LoadImage(StringParam filename);
  void FinishLoad();
  void Close();

  void CheckFrames();
  void AddAllFrames();
  void UpdatePreview();
  void ClearSelectedFrames();
  void OnClearPressed(ObjectEvent* event);

  void SaveDataToSpriteSource(SpriteSource* sprite, IntRect frameSize, uint numberOfFrames);
  SpriteSource* AddSpriteResource(StringParam name, Image& output, IntRect frameSize, uint numberOfFrames);
  bool AddFramesAsSprites();
  bool AddMultiFrameSprite();

  // Event Handlers
  void OnAddAndContinue(ObjectEvent* event);
  void OnAddTiles(ObjectEvent* event);
  void OnAddPressed(ObjectEvent* event);
  void OnClosePressed(ObjectEvent* event);
  void OnFileSelected(OsFileSelection* event);
  void OnKeyDown(KeyboardEvent* keyEvent);
  void OnMouseScrollGrid(MouseEvent* mouseEvent);
  void OnMouseMoveGrid(MouseEvent* mouseEvent);
  bool HandleGridMouseControls(MouseEvent* mouseEvent);
  void OnLeftMouseDownGrid(MouseEvent* mouseEvent);
  void OnRightMouseDownGrid(MouseEvent* mouseEvent);

  void UpdateZoomedSize();
  void UpdateTransform() override;

  void DrawLines(Array<StreamedVertex>& lines,
                 uint axis,
                 float zoom,
                 float spacing,
                 Vec2 totalSize,
                 Vec2 startOffset,
                 uint lineCount);
  void DrawRedirect(
      ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, WidgetRect clipRect);
  void NudgePosition(IntVec2 move);
  void AddFrame(IntVec2 gridCell);
  void RemoveFrame(int frameIndex);
  IntRect GetRectAtIndex(IntVec2 gridCell);
  bool CheckFramesAt(IntVec2 loction, int& frameSelected);
  IntVec2 GetGridIndex(IntVec2 imagePos);
  IntVec2 ToImagePosition(Vec2 screenPoint);
  bool IsValidGridIndex(IntVec2 gridIndex);
  void SetZoom(float zoom);

  // Getter/Setters

  /// The width of each sprite frame in pixels located on the sprite sheet
  /// (Updates FramesPerRow)
  int GetFrameWidth();
  void SetFrameWidth(int frameSizeX);

  /// The height of each sprite frame in pixels located on the sprite sheet
  /// (Updates NumberOfRows)
  int GetFrameHeight();
  void SetFrameHeight(int frameSizeY);

  /// The total number of frames on the horizontal axis of the sprite sheet
  /// (Updates FrameWidth)
  int GetFramesPerRow();
  void SetFramesPerRow(int framesX);

  /// The total number of frames on the vertical axis of the sprite sheet
  /// (Updates FrameHeight)
  int GetNumberOfRows();
  void SetNumberOfRows(int framesY);

  /// The offset along the x-axis in pixels for the origin of the sprite sheet
  int GetOffsetX();
  void SetOffsetX(int offset);

  /// The offset along the y-axis in pixels for the origin of the sprite sheet
  int GetOffsetY();
  void SetOffsetY(int offset);

  /// The padding on the x-axis in pixels between each sprite frame
  int GetSpacingX();
  void SetSpacingX(int spacingX);

  /// The padding on the y-axis in pixels between each sprite frame
  int GetSpacingY();
  void SetSpacingY(int spacingY);

  /// The total number of frames in the resulting sprite animation
  int GetFrameCount();

  /// Mode for determining which sprite frames to import
  ImportFrames::Enum GetImportFrames();
  void SetImportFrames(ImportFrames::Enum newMode);

  /// Animate the preview of the generated sprite animation
  bool GetPreviewAnimate();
  void SetPreviewAnimate(bool state);

  /// Frames per second for the resulting sprite animation
  float GetFrameRate();
  void SetFrameRate(float state);

  /// The current frame of the sprite animation preview
  int GetPreviewFrame();
  void SetPreviewFrame(int frame);

  /// Enable converting a pixel color to be transparent
  bool GetUseAlphaColorKey();
  void SetUseAlphaColorKey(bool colorKey);

  /// Pixel color to be made transparent
  Vec4 GetAlphaColor();
  void SetAlphaColor(Vec4 alphaColor);

  /// Texture smoothing algorithm
  SpriteSampling::Enum GetSmoothing();
  void SetSmoothing(SpriteSampling::Enum sampling);

  PropertyView* mPropertyView;
  TextureView* mSourceDisplay;
  SpritePreview* mPreviewSprite;
  ScrollArea* mScrollArea;
  PixelGridArea* mGrid;
  HandleOf<Texture> mSourceTextrue;
  TextureView* mImageBackground;
  ColorBlock* mBackground;
  HandleOf<SpriteSource> mDestination;

  // Settings
  /// The name for the new SpriteSource resource
  String Name;
  /// The local translation on the sprite considered to be 0,0
  SpriteOrigin::Enum mOrigin;
  ImportFrames::Enum mImportFrames;

  int FrameSizeX;
  int FrameSizeY;
  int FramesX;
  int FramesY;
  int OffsetX;
  int OffsetY;
  int SpacingX;
  int SpacingY;
  /// The number of pixels that occupy one unit of distance in the engine's
  /// world space
  int PixelsPerUnit;
  SpriteSampling::Enum Sampling;
  /// Create a TilePaletteSource from all the selected sprite frames
  bool CreatePalette;
  float mZoom;

  // The original sprite sheets dimensions in pixels
  int SourceSizeX;
  int SourceSizeY;

  // Pixel color to be made transparent
  bool UseAlphaColorKey;
  Vec4 AlphaColor;

  Image mSourcePixels;
  Image mFixedPixels;
  Array<FrameArea> mFrames;
  int FrameSelected;
  IntVec2 PixelCursor;
};
} // namespace Zero
