////////////////////////////////////////////////////////////////////////////////
///
/// \file HeightMapImporter.hpp
/// Declaration of the height map importer interface.
///
/// Authors: Dane Curbow
/// Copyright 2015, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//forward declaration
class Editor;
class PatchGridArea;

void ImportHeightMap(Editor* editor);

DeclareEnum2(ImportMode, MaintainAspectRatio, FitToPatches);

class HeightMapImporter : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor
  HeightMapImporter(Composite* parent, Editor* editor);
  ~HeightMapImporter();

  // Getters/Setters
  ImportMode::Enum GetImportMode();
  void             SetImportMode(ImportMode::Enum mode);
  uint  GetPatchColumns();
  void  SetPatchColumns(uint PatchRows);
  uint  GetPatchRows();
  void  SetPatchRows(uint PatchColumns);

  //
  void ScalePreviewArea(float scale);
  void OnFileSelected(OsFileSelection* event);
  void LoadImage(StringParam filename);
  void RefreshTexture();
  void UpdateTexture();
  void Close();

private:
  Vec2 ScaleGridSizeToImage();
  Vec2 ScaleGridSizeToArea();
  void GenerateHeightMap(Event* e);
  float CalculateAveragePixelHeight(uint column, uint row, uint patchCellX, uint patchCellY);

  void OnMouseDown(MouseEvent* e);
  void OnClosePressed(Event* e);

  Editor* mEditor;
  Image mHeightMap;

  PropertyView* mPropertyView;
  TextureView* mSourceDisplay;
  PatchGridArea* mPatchGrid;
  ScrollArea* mScrollArea;
  TextureView* mImageBackground;
  ColorBlock* mBackground;

  HandleOf<Texture> mSourceTexture;
  Vec2 mPreviewArea;
  Vec2 mPreviewSize;

  //Vars used in generating the height map
  uint mPixelsPerColumnPatch;
  uint mPixelsPerRowPatch;
  uint mPixelsPerXVert;
  uint mPixelsPerYVert;
  float mHeightRange;

  /// Settings for the image to height map conversion
  String mName;
  float mBaseHeight;
  float mMinHeightRange;
  float mMaxHeightRange;
  ImportMode::Enum mImportMode;
  uint mPatchColumns;
  uint mPatchRows;
  uint mPatchSize;

  //is the patch side being scaled along the x or y size of the image
  bool patchScaledToX;
};

class PatchGridArea : public Widget
{
public:
  PatchGridArea(Composite* parent, HeightMapImporter* importer);
  ~PatchGridArea();

  void SetGridSize(Vec2 gridSize);
  Vec2 GetGridSize();

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;
  void SetupGrid(FrameBlock& frameBlock, Mat4Param localToView);
  void SetupLines(FrameBlock& frameBlock, uint axis, float spacing, Vec2 totalSize, Mat4Param localToView, uint lineCount);

private:
  Vec4 mLineColor;
  HeightMapImporter* mImporter;
  Vec2 mGridSize;
};

class DragSizeManipulator : public MouseManipulation
{
public:
  DragSizeManipulator(Mouse* mouse, Composite* relative, HeightMapImporter* importer);
  ~DragSizeManipulator() {};
  
  void OnMouseUpdate(MouseEvent* event);
  void OnMouseUp(MouseEvent* event);

private:
  HeightMapImporter* mImporter;
  float mPrevY;
};

} // namespace Zero
