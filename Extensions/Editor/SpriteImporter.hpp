///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
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
  PixelRect Rect;
};

//------------------------------------------------------------------------ Pixel Grid Area
class PixelGridArea : public Widget
{
public:
  typedef PixelGridArea self_type;
  SpriteSheetImporter* mOwner;

  PixelGridArea(Composite* parent, SpriteSheetImporter* owner);

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;
};

//------------------------------------------------------------------------ Sprite Sheet Importer
class SpriteSheetImporter : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SpriteSheetImporter(Composite* parent);

  void ComputeFrameWidthAndCount(int& frameSize, int& frameCount, int newFrameSize, int spacing, int sourceSize, int offset);
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
  
  void SaveDataToSpriteSource(SpriteSource* sprite, PixelRect frameSize, uint numberOfFrames);
  SpriteSource* AddSpriteResource(StringParam name, Image& output, PixelRect frameSize, uint numberOfFrames);
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

  void DrawLines(Array<StreamedVertex>& lines, uint axis, float zoom, float spacing, Vec2 totalSize, Vec2 startOffset, uint lineCount);
  void DrawRedirect(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect);
  void NudgePosition(IntVec2 move);
  void AddFrame(IntVec2 gridCell);
  void RemoveFrame(int frameIndex);
  PixelRect GetRectAtIndex(IntVec2 gridCell);
  bool CheckFramesAt(IntVec2 loction, int& frameSelected);
  IntVec2 GetGridIndex(IntVec2 imagePos);
  IntVec2 ToImagePosition(Vec2 screenPoint);
  bool IsValidGridIndex(IntVec2 gridIndex);
  void SetZoom(float zoom);

  // Getter/Setters
  int GetFrameWidth();
  void SetFrameWidth(int frameSizeX);

  int GetFrameHeight();
  void SetFrameHeight(int frameSizeY);

  int GetFramesPerRow();
  void SetFramesPerRow(int framesX);

  int GetNumberOfRows();
  void SetNumberOfRows(int framesY);

  int GetOffsetX();
  void SetOffsetX(int offset);

  int GetOffsetY();
  void SetOffsetY(int offset);

  int GetSpacingX();
  void SetSpacingX(int spacingX);

  int GetSpacingY();
  void SetSpacingY(int spacingY);

  int GetFrameCount();

  ImportFrames::Type GetImportFrames();
  void SetImportFrames(ImportFrames::Type newMode);

  bool GetPreviewAnimate();
  void SetPreviewAnimate(bool state);

  float GetFrameRate();
  void SetFrameRate(float state);

  int GetPreviewFrame();
  void SetPreviewFrame(int frame);

  bool GetUseAlphaColorKey();
  void SetUseAlphaColorKey(bool colorKey);

  Vec4 GetAlphaColor();
  void SetAlphaColor(Vec4 alphaColor);

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

  //Settings
  String Name;
  SpriteOrigin::Enum mOrigin;
  ImportFrames::Type mImportFrames;

  int FrameSizeX;
  int FrameSizeY;
  int FramesX;
  int FramesY;
  int OffsetX;
  int OffsetY;
  int SpacingX;
  int SpacingY;
  int SourceSizeX;
  int SourceSizeY;
  int PixelsPerUnit;
  SpriteSampling::Enum Sampling;
  bool CreatePalette;
  float mZoom;

  //Pixel color to be made transparent
  bool UseAlphaColorKey;
  Vec4 AlphaColor;

  Image mSourcePixels;
  Image mFixedPixels;
  Array<FrameArea> mFrames;
  int FrameSelected;
  IntVec2 PixelCursor;
};
}//namespace Zero

