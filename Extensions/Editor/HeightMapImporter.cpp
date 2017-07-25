////////////////////////////////////////////////////////////////////////////////
///
/// \file HeightMapImporter.cpp
/// Implementation of the height map importer interface.
///
/// Authors: Dane Curbow
/// Copyright 2015, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

HeightMapImporter* CreateImporter(Editor* editor)
{
  Window* window = new Window(editor);
  window->SetTitle("Height Map Importer");
  HeightMapImporter* importer = new HeightMapImporter(window, editor);
  window->SetSize(Pixels(1280, 720));
  CenterToWindow(editor, window, true);
  return importer;
}

void ImportHeightMap(Editor* editor)
{
  HeightMapImporter* importer = CreateImporter(editor);

  //Open the open file dialog
  FileDialogConfig config;
  config.EventName = "OnFileSelected";
  config.CallbackObject = importer;
  config.Title = "Select height map to import";
  config.AddFilter("Png File", "*.png");
  config.StartingDirectory = editor->GetProjectPath();
  Z::gEngine->has(OsShell)->OpenFile(config);
}


ZilchDefineType(HeightMapImporter, builder, type)
{
  ZilchBindFieldProperty(mName);
  ZilchBindFieldProperty(mBaseHeight);
  ZilchBindFieldProperty(mMinHeightRange);
  ZilchBindFieldProperty(mMaxHeightRange);
  ZilchBindFieldProperty(mPatchSize);
  // These variables must refresh the displayed texture when updated
  ZilchBindGetterSetterProperty(ImportMode);
  ZilchBindGetterSetterProperty(PatchColumns);
  ZilchBindGetterSetterProperty(PatchRows);
}

HeightMapImporter::HeightMapImporter(Composite* parent, Editor* editor)
  : Composite(parent), mEditor(editor)
{
  ConnectThisTo(this, "OnFileSelected", OnFileSelected);

  // Initialize member variables
  mName = "NewImageHeightMap";
  mImportMode = ImportMode::MaintainAspectRatio;
  mBaseHeight = 0.f;
  mMinHeightRange = 0.f;
  mMaxHeightRange = 1.f;
  mPatchColumns = 4;
  mPatchRows = 4;
  mPatchSize = 32;

  SetLayout(CreateStackLayout());
  
  Composite* top = new Composite(this);
  top->SetLayout(CreateRowLayout());
  top->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  //left side panel displaying user selected height map settings
  Composite* left = new Composite(top);
  left->SetLayout(CreateStackLayout());
  left->SetSizing(SizeAxis::X, SizePolicy::Flex, 15);

  mPropertyView = new PropertyView(left);
  mPropertyView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  //height map preview image

  Splitter* splitter = new Splitter(top);
  splitter->SetSize(Pixels(4, 4));

  Composite* right = new Composite(top);
  right->SetSizing(SizeAxis::X, SizePolicy::Flex, 65);
  right->SetLayout(CreateStackLayout());

  mScrollArea = new ScrollArea(right);
  mScrollArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  mBackground = new ColorBlock(mScrollArea);
  mBackground->SetSize(Pixels(10000, 10000));
  mBackground->SetColor(ToFloatColor(EditorColor::Gray3));

  mImageBackground = new TextureView(mScrollArea);
  mImageBackground->SetTexture(TextureManager::Find("AlphaBackground"));
  mSourceDisplay = new TextureView(mScrollArea);
  mSourceDisplay->SetSize(Pixels(400, 200));
  //We may not need the PatchGridArea to be a composite, we will see moving forward
  mPatchGrid = new PatchGridArea(mScrollArea, this);
  ConnectThisTo(mSourceDisplay, Events::LeftMouseDown, OnMouseDown);


  Composite* bottom = new Composite(this);
  bottom->SetLayout(CreateStackLayout(LayoutDirection::RightToLeft, Pixels(10, 0), Thickness(10, 4, 10, 4)));

  TextButton* button = new TextButton(bottom);
  button->SetText("Close");
  ConnectThisTo(button, Events::ButtonPressed, OnClosePressed);

  TextButton* createHeightMap = new TextButton(bottom);
  createHeightMap->SetText("Create Height Map");
  ConnectThisTo(createHeightMap, Events::ButtonPressed, GenerateHeightMap);

  mPropertyView->SetObject(this);
  mPropertyView->ActivateAutoUpdate();

  mPreviewArea = Vec2(800.f,600.f);
}

HeightMapImporter::~HeightMapImporter()
{

}

void HeightMapImporter::OnFileSelected(OsFileSelection* event)
{
  if (!event->Success)
    Close();
  else
  {
    LoadImage(event->Files[0]);
  }
}

void HeightMapImporter::LoadImage(StringParam filename)
{
  Status status;
  LoadFromPng(status, &mHeightMap, filename);
  if (!status)
  {
    DoNotifyStatus(status);
    Close();
    return;
  }

  mName = FilePath::GetFileNameWithoutExtension(filename);

  UpdateTexture();
  RefreshTexture();
}

void HeightMapImporter::RefreshTexture()
{
  if (mImportMode == ImportMode::MaintainAspectRatio)
  { 
    if (mHeightMap.Width > mHeightMap.Height)
      mPreviewSize = Pixels(mPreviewArea.x, (float)mHeightMap.Height * (mPreviewArea.x / (float)mHeightMap.Width));
    else
      mPreviewSize = Pixels((float)mHeightMap.Width * (mPreviewArea.y / (float)mHeightMap.Height), mPreviewArea.y);
    //calculate grid size and set it
    Vec2 gridSize = ScaleGridSizeToImage();
    mPatchGrid->SetGridSize(gridSize);
    mScrollArea->SetClientSize(gridSize);
  } 
  else if (mImportMode == ImportMode::FitToPatches)
  {
    //look at numbers of patches and their current size in relation to the "maintain aspect ratio"
    //and stretch the image to fix the grid
    mPreviewSize = ScaleGridSizeToArea();
    mPatchGrid->SetGridSize(mPreviewSize);
    mScrollArea->SetClientSize(mPreviewSize);
  }

  mSourceDisplay->SetSize(mPreviewSize);
  //mGrid->SetSize(mPreviewSize);
  mImageBackground->SetSize(mPreviewSize);
  mImageBackground->SetUv(Vec2(0, 0), (mPreviewSize / 2.0f) / 8.0f);
}

Vec2 HeightMapImporter::ScaleGridSizeToImage()
{
  //find the x and y ratios between the image and the height map patches for proper scaling
  //to maintain aspect ratio
  Vec2 gridSize;
  float xRatio = mPreviewSize.x / mPatchColumns;
  float yRatio = mPreviewSize.y / mPatchRows;
  if (xRatio >= yRatio)
  {
    gridSize.x = mPreviewSize.x;
    gridSize.y = (gridSize.x / mPatchColumns) * mPatchRows;
    patchScaledToX = true;
  }
  else
  {
    gridSize.y = mPreviewSize.y;
    gridSize.x = (gridSize.y / mPatchRows) * mPatchColumns;
    patchScaledToX = false;
  }
  return gridSize;
}

Vec2 HeightMapImporter::ScaleGridSizeToArea()
{
  Vec2 gridSize;
  if (mPreviewArea.x >= mPreviewArea.y)
  {
    gridSize.x = mPreviewArea.x;
    gridSize.y = (gridSize.x / mPatchColumns) * mPatchRows;
  }
  else
  {
    gridSize.y = mPreviewArea.y;
    gridSize.x = (gridSize.y / mPatchRows) * mPatchColumns;
  }
  return gridSize;
}

void HeightMapImporter::UpdateTexture()
{
  if (!mSourceTexture)
    mSourceTexture = Texture::CreateRuntime();

  mSourceTexture->Upload(mHeightMap);
  mSourceDisplay->SetTexture(mSourceTexture);
}

void HeightMapImporter::GenerateHeightMap(Event* e)
{
  // TODO
    //Properly implement scaling of image -> heightmapmodel

  Space* space = mEditor->GetEditSpace();

  Cog* editorCameraObject = space->FindObjectByName(SpecialCogNames::EditorCamera);
  if (editorCameraObject == NULL)
    return;

  EditorCameraController* editorCameraController = editorCameraObject->has(EditorCameraController);
  if (editorCameraController == NULL)
    return;

  //create transform
  Archetype* archetype = ArchetypeManager::Find("DefaultHeightMap");
  Vec3 creationPoint = editorCameraController->GetLookTarget();
  //set Y value based on our base height
  creationPoint.y = mBaseHeight;
  Cog* cog = CreateFromArchetype(mEditor->GetOperationQueue(), space, archetype, creationPoint);
  if (cog == NULL)
    return;

  cog->SetName(mName);
  HeightMap* map = cog->has(HeightMap);
  map->SetUnitsPerPatch((float)mPatchSize);
  
  //first we will create the patches as defined by the user
  for (uint i = 0; i < mPatchColumns; ++i)
    for (uint j = 0; j < mPatchRows; ++j)
      map->CreatePatchAtIndex(IntVec2(i, j));
  
  //set vertices's height based on our min/max
  mHeightRange = mMaxHeightRange - mMinHeightRange;
  mPixelsPerColumnPatch = mHeightMap.Width / mPatchColumns;
  mPixelsPerRowPatch = mHeightMap.Height / mPatchRows;
  //in the case where we are maintaining the aspect ratio we need the pixels
  //per patch to be uniform and not stretched to match the user defined patch width and height
  if (mImportMode == ImportMode::MaintainAspectRatio)
  {
    if (patchScaledToX)
      mPixelsPerRowPatch = mPixelsPerColumnPatch = mHeightMap.Width  / mPatchColumns;
    else
      mPixelsPerRowPatch = mPixelsPerColumnPatch = mHeightMap.Height / mPatchRows;
  }
  mPixelsPerXVert = mPixelsPerColumnPatch / HeightPatch::Size;
  mPixelsPerYVert = mPixelsPerRowPatch / HeightPatch::Size;

  //loop over the patches columns and rows
  for (uint column = 0; column < mPatchColumns; ++column)
    for (uint row = 0; row < mPatchRows; ++row)
    {
      HeightPatch* patch = map->GetPatchAtIndex(IntVec2(column, row));
      //go over the patches 
      for (uint x = 0; x < HeightPatch::Size; ++x)
        for (uint y = 0; y < HeightPatch::Size; ++y)
        {
          float& height = patch->GetHeight(IntVec2(x,y));
          height = CalculateAveragePixelHeight(column, row, x, y);
        }
    }
  map->SignalAllPatchesModified();
  // close the height map importer view
  Close();
}

float HeightMapImporter::CalculateAveragePixelHeight(uint column, uint row, uint patchCellX, uint patchCellY)
{
  uint imageStartX = column * mPixelsPerColumnPatch + patchCellX * mPixelsPerXVert;
  uint imageStartY = row * mPixelsPerRowPatch + patchCellY * mPixelsPerYVert;
  uint imageEndX = imageStartX + mPixelsPerXVert;
  uint imageEndY = imageStartY + mPixelsPerYVert;
  if (imageEndX > (uint)mHeightMap.Width)
    imageEndX = mHeightMap.Width;
  if (imageEndY > (uint)mHeightMap.Height)
    imageEndY = mHeightMap.Height;
    
  float curColor = 0.f;
  uint count = mPixelsPerXVert * mPixelsPerYVert;
  for (uint x = imageStartX; x < imageEndX; ++x)
    for (uint y = imageStartY; y < imageEndY; ++y)
      curColor += ToFloatColor(mHeightMap.GetPixel(x, y)).x;
  
  curColor /= (float)count;
  //new_value = ( (old_value - old_min) / (old_max - old_min) ) * (new_max - new_min) + new_min
  //               curColor - 0              1 - 0               heightMax - heightMin + heightMin
  return (curColor / 1.f) * mHeightRange + mMinHeightRange;
}

void HeightMapImporter::OnMouseDown(MouseEvent* e)
{
  new DragSizeManipulator(e->GetMouse(), mScrollArea, this);
}

void HeightMapImporter::OnClosePressed(Event* e)
{
  Close();
}

ImportMode::Enum HeightMapImporter::GetImportMode()
{
  return mImportMode;
}

void HeightMapImporter::SetImportMode(ImportMode::Enum mode)
{
  mImportMode = mode;
  RefreshTexture();
}

uint HeightMapImporter::GetPatchColumns()
{
  return mPatchColumns;
}

void HeightMapImporter::SetPatchColumns(uint PatchRows)
{
  mPatchColumns = PatchRows;
  RefreshTexture();
}

uint HeightMapImporter::GetPatchRows()
{
  return mPatchRows;
}

void HeightMapImporter::SetPatchRows(uint PatchColumns)
{
  mPatchRows = PatchColumns;
  RefreshTexture();
}

void HeightMapImporter::ScalePreviewArea(float scale)
{
  Vec2 newPrev = mPreviewArea * scale;
  //make sure we never scale the area so small that we lose it forever
    //if you scale it very small while viewing a scroll area outside the view of its smaller version
    //since the scroll area matches the image when you go to scroll to it "disappears" and is lost
  if (newPrev.x < 100.f || newPrev.y < 100.f)
    return;

  mPreviewArea = newPrev;
  RefreshTexture();
}

void HeightMapImporter::Close()
{
  CloseTabContaining(this);
}

PatchGridArea::PatchGridArea(Composite* parent, HeightMapImporter* importer)
  : Widget(parent), mImporter(importer)
{
  mLineColor = ToFloatColor(Color::Red);
}

PatchGridArea::~PatchGridArea()
{

}

void PatchGridArea::SetGridSize(Vec2 gridSize)
{
  mGridSize = gridSize;
}
Vec2 PatchGridArea::GetGridSize()
{
  return mGridSize;
}

void PatchGridArea::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  Texture* texture = TextureManager::FindOrNull("White");
  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, texture);
  // Setup our view node for drawing lines
  viewNode.mStreamedVertexType = PrimitiveType::Lines;
  // Setup our grid to be drawn in the current render pass
  SetupGrid(frameBlock, viewNode.mLocalToView);
  viewNode.mStreamedVertexCount = frameBlock.mRenderQueues->mStreamedVertices.Size() - viewNode.mStreamedVertexStart;
}

void PatchGridArea::SetupGrid(FrameBlock& frameBlock, Math::Mat4Param localToView)
{
  int PatchRows = mImporter->GetPatchColumns();
  int PatchColumns = mImporter->GetPatchRows();
  Vec2 spacing = Vec2(mGridSize.x / (float)PatchRows, mGridSize.y / (float)PatchColumns);

  // Lets draw our patch grid 
  // 0 - horizontal, 1 - vertical
  SetupLines(frameBlock, 0, spacing.x, mGridSize, localToView, PatchRows);
  SetupLines(frameBlock, 1, spacing.y, mGridSize, localToView, PatchColumns);

}

void PatchGridArea::SetupLines(FrameBlock& frameBlock, uint axis, float spacing, Vec2 totalSize, Mat4Param localToView, uint lineCount)
{
  for (uint line = 0; line<lineCount + 1; ++line)
  {
    Vec3 start = Vec3::cZero;
    start[axis] = spacing * line;
    Vec3 end = start;
    end[!axis] += totalSize[!axis];
    // Translate our points into the space of our current view node
    start = Math::TransformPoint(localToView, start);
    end   = Math::TransformPoint(localToView, end);
    // Setup our end and start vertices using the set line color
    StreamedVertex startVertex(SnapToPixels(start) + Pixels(0.5, 0.5, 0), Vec2::cZero, mLineColor);
    StreamedVertex endVertex(SnapToPixels(end) + Pixels(0.5, 0.5, 0), Vec2::cZero, mLineColor);
    // Add our start and end points in order
    frameBlock.mRenderQueues->mStreamedVertices.PushBack(startVertex);
    frameBlock.mRenderQueues->mStreamedVertices.PushBack(endVertex);
  }
}

DragSizeManipulator::DragSizeManipulator(Mouse* mouse, Composite* relative, HeightMapImporter* importer)
  : MouseManipulation(mouse, relative)
{
  mImporter = importer;
  mPrevY = mouse->GetClientPosition().y;
}

void DragSizeManipulator::OnMouseUpdate(MouseEvent* event)
{
  float curY = event->Position.y;
  float change = curY - mPrevY;
  if (change > 0.f)
    mImporter->ScalePreviewArea(curY/mPrevY);
  else if (change < 0.f)
    mImporter->ScalePreviewArea(curY/mPrevY);

  mPrevY = curY;
}

void DragSizeManipulator::OnMouseUp(MouseEvent* event)
{
  this->Destroy();
}

} // namespace Zero
