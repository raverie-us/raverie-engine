///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------ Pixel Grid Area
PixelGridArea::PixelGridArea(Composite* parent, SpriteSheetImporter* owner)
  : Widget(parent)
{
  this->SetInteractive(true);
  this->SetTakeFocusMode(FocusMode::Hard);
  mOwner = owner;
}

void PixelGridArea::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);
  mOwner->DrawRedirect(viewBlock, frameBlock, mWorldTx, colorTx, clipRect);
}

//------------------------------------------------------------------------ Sprite Sheet Importer
ZilchDefineType(SpriteSheetImporter, builder, type)
{
  ZilchBindFieldProperty(Name);
  ZilchBindGetterSetterProperty(FrameWidth);
  ZilchBindGetterSetterProperty(FrameHeight);
  ZilchBindGetterSetterProperty(FramesPerRow);
  ZilchBindGetterSetterProperty(NumberOfRows);
  
  ZilchBindGetterSetterProperty(OffsetX);
  ZilchBindGetterSetterProperty(OffsetY);
  
  ZilchBindGetterSetterProperty(SpacingX);
  ZilchBindGetterSetterProperty(SpacingY);
  
  ZilchBindGetterSetterProperty(FrameRate);
  ZilchBindFieldProperty(PixelsPerUnit);
  ZilchBindGetterSetterProperty(Smoothing);
  ZilchBindFieldProperty(CreatePalette);
  ZilchBindFieldProperty(mOrigin);
  
  ZilchBindGetterSetterProperty(PreviewAnimate);
  ZilchBindGetterSetterProperty(PreviewFrame);
  ZilchBindGetterSetterProperty(ImportFrames);
  ZilchBindGetterSetterProperty(UseAlphaColorKey);
  ZilchBindGetterSetterProperty(AlphaColor);
  ZilchBindGetterProperty(FrameCount);
  ZilchBindField(SourceSizeX);
  ZilchBindField(SourceSizeY);
}

SpriteSheetImporter::SpriteSheetImporter(Composite* parent)
  : Composite(parent)
{
  ConnectThisTo(this, "OnFileSelected", OnFileSelected);

  SetLayout(CreateStackLayout());

  Composite* top = new Composite(this);
  top->SetLayout(CreateRowLayout());
  top->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  Composite* left = new Composite(top);
  left->SetSizing(SizeAxis::X, SizePolicy::Flex, 35);
  left->SetLayout(CreateStackLayout());

  mPropertyView = new PropertyView(left);
  mPropertyView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  TextButton* clearButton = new TextButton(left);
  clearButton->SetText("Clear Frames");
  ConnectThisTo(clearButton, Events::ButtonPressed, OnClearPressed);

  mPreviewSprite = new SpritePreview(left);
  mPreviewSprite->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mPreviewSprite->SetSize(Pixels(100, 100));

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
  mGrid = new PixelGridArea(mScrollArea, this);
  mSourceDisplay->SetSize(Pixels(200, 200));

  Composite* bottom = new Composite(this);
  bottom->SetLayout(CreateStackLayout(LayoutDirection::RightToLeft, Pixels(10, 0), Thickness(10, 4, 10, 4)));

  TextButton* button;
  button = new TextButton(bottom);
  button->SetText("Close");
  ConnectThisTo(button, Events::ButtonPressed, OnClosePressed);

  button = new TextButton(bottom);
  button->SetText("Add and Continue");
  ConnectThisTo(button, Events::ButtonPressed, OnAddAndContinue);

  button = new TextButton(bottom);
  button->SetText("Add and Close");
  ConnectThisTo(button, Events::ButtonPressed, OnAddPressed);

  button = new TextButton(bottom);
  button->SetText("Add Frames as Sprites");
  ConnectThisTo(button, Events::ButtonPressed, OnAddTiles);


  mOrigin = SpriteOrigin::Center;
  mImportFrames = ImportFrames::AllFrames;
  FrameSizeX = 0;
  FrameSizeY = 0;
  SourceSizeX = 0;
  SourceSizeY = 0;
  FramesX = 0;
  FramesY = 0;
  OffsetX = 0;
  OffsetY = 0;
  SpacingX = 0;
  SpacingY = 0;
  UseAlphaColorKey = false;
  PixelsPerUnit = 64;
  Sampling = SpriteSampling::Nearest;
  SetZoom(0.5f);
  CreatePalette = false;

  mSourceDisplay->SetUv(Vec2(0, 0), Vec2(1, 1));

  ConnectThisTo(mGrid, Events::LeftMouseDown, OnLeftMouseDownGrid);
  ConnectThisTo(mGrid, Events::RightMouseDown, OnRightMouseDownGrid);
  ConnectThisTo(mGrid, Events::MouseMove, OnMouseMoveGrid);
  ConnectThisTo(mScrollArea->GetClientWidget(), Events::MouseScroll, OnMouseScrollGrid);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, Events::KeyRepeated, OnKeyDown);

  mPropertyView->SetObject(this);
  mPropertyView->ActivateAutoUpdate();
}

void SpriteSheetImporter::ComputeFrameWidthAndCount(int& frameSize, int& frameCount, int newFrameSize, int spacing, int sourceSize, int offset)
{
  frameSize = Math::Clamp(newFrameSize, cMinFrameSize, sourceSize);
  int fullFrameSize = frameSize + spacing;
  int area = sourceSize - offset;
  frameCount = area / fullFrameSize;
  int excess = area % fullFrameSize;

  if (excess >= frameSize)
    ++frameCount;
}

void SpriteSheetImporter::UpdateTexture()
{
  if (!mSourceTextrue)
    mSourceTextrue = Texture::CreateRuntime();

  byte* data = (byte*)mSourcePixels.Data;
  if (UseAlphaColorKey)
  {
    mFixedPixels.Allocate(SourceSizeX, SourceSizeY);
    CopyImage(&mFixedPixels, &mSourcePixels);
    SetColorToAlpha(&mFixedPixels, ToByteColor(AlphaColor));
    data = (byte*)mFixedPixels.Data;
  }

  mSourceTextrue->Upload(mSourcePixels);

  //if(Smoothing)
  //  mSourceTextrue->SetFiltering(Filtering::Bilinear);
  //else
  //  mSourceTextrue->SetFiltering(Filtering::Point);

  mSourceDisplay->SetTexture(mSourceTextrue);
}

void SpriteSheetImporter::LoadImages(Array<String>& files)
{
  if (files.Empty())
    return;

  Array<Image> images;
  images.Resize(files.Size());

  for (uint i = 0; i < files.Size(); ++i)
  {
    Image& frameImage = images[i];

    Status status;
    LoadFromPng(status, &frameImage, files[i]);
    if (!status)
    {
      DoNotifyStatus(status);
      Close();
      return;
    }

    // Check to make sure all frames images are the same size
    bool notFirstFrame = i != 0;
    if (notFirstFrame)
    {
      if (frameImage.Width != images[0].Width ||
          frameImage.Height != images[0].Height)
      {
        DoNotifyError("Importing", "All images must be the same width and height.");
        Close();
        return;
      }
    }
  }

  // Copy all frames out to an image for editing
  SpriteFrameLayout frameLayout(files.Size(), images[0].Width, images[0].Height);
  mSourcePixels.Allocate(frameLayout.TotalSize.SizeX, frameLayout.TotalSize.SizeY);
  mSourcePixels.ClearColorTo(0);

  for (uint i = 0; i < files.Size(); ++i)
  {
    Image& frameImage = images[i];
    PixelRect place = frameLayout.GetFrame(i);
    CopyImage(&mSourcePixels, &frameImage, place.X, place.Y, 0, 0, place.SizeX, place.SizeY);
  }

  // Use file name as name for sprite
  Name = FilePath::GetFileNameWithoutExtension(files[0]);

  FinishLoad();

  // Set the frame size to first frame size
  SetFrameHeight(images[0].Width);
  SetFrameWidth(images[0].Height);

  mFrames.Resize(files.Size());
  UpdatePreview();
}

void SpriteSheetImporter::LoadSprite(SpriteSource* spriteSource)
{
  if (spriteSource == NULL)
  {
    Close();
    return;
  }

  Status status;
  LoadFromPng(status, &mSourcePixels, spriteSource->mContentItem->GetFullPath());
  if (!status)
  {
    DoNotifyStatus(status);
    Close();
    return;
  }

  FinishLoad();
  UpdatePreview();

  mDestination = spriteSource;
}

void SpriteSheetImporter::LoadImage(StringParam filename)
{
  Status status;
  LoadFromPng(status, &mSourcePixels, filename);
  if (!status)
  {
    DoNotifyStatus(status);
    Close();
    return;
  }

  if (mSourcePixels.Width > MaxSpriteSize || mSourcePixels.Height > MaxSpriteSize)
  {
    DoNotifyError("Import", "Image is too large");
    Close();
    return;
  }

  Name = FilePath::GetFileNameWithoutExtension(filename);

  FinishLoad();
  UpdatePreview();
}

void SpriteSheetImporter::FinishLoad()
{
  SourceSizeX = mSourcePixels.Width;
  SourceSizeY = mSourcePixels.Height;

  SetFrameHeight(SourceSizeY);
  SetFrameWidth(SourceSizeX);

  AlphaColor = ToFloatColor(mSourcePixels.GetPixel(0, 0));

  UpdateTexture();

  Vec2 size = Pixels((float)mSourcePixels.Width, (float)mSourcePixels.Height);
  mSourceDisplay->SetSize(size);
  mGrid->SetSize(size);
  mScrollArea->SetClientSize(size);
  mImageBackground->SetSize(size);
  mImageBackground->SetUv(Vec2(0, 0), (size / 2.0f) / 8.0f);

  // find the zoom level that fits the image size to the display area
  Vec2 displaySize = mScrollArea->GetClientVisibleSize();
  float xZoom = displaySize.x / SourceSizeX;
  float yZoom = displaySize.y / SourceSizeY;

  // use the axis that requires the most zoomed out to fit within the area without scrollbars
  if (xZoom <= yZoom)
    SetZoom(xZoom);
  else
    SetZoom(yZoom);
}

void SpriteSheetImporter::Close()
{
  CloseTabContaining(this);
}

void SpriteSheetImporter::CheckFrames()
{
  ClearSelectedFrames();
  if (mImportFrames == ImportFrames::AllFrames)
    AddAllFrames();
}

void SpriteSheetImporter::AddAllFrames()
{
  mFrames.Clear();

  int strideX = FrameSizeX + SpacingX;
  int strideY = FrameSizeY + SpacingY;

  for (int y = 0; y < FramesY; ++y)
  {
    for (int x = 0; x < FramesX; ++x)
    {
      PixelRect rect = { OffsetX + x * strideX, OffsetY + y * strideY, FrameSizeX, FrameSizeY };
      FrameArea frameArea = { false, rect };
      mFrames.PushBack(frameArea);
    }
  }

  UpdatePreview();
}

void SpriteSheetImporter::UpdatePreview()
{
  mPreviewSprite->mFrames.Clear();
  forRange(FrameArea& selection, mFrames.All())
  {
    TextureArea area;
    area.mUvRect = ComputeTextureRect(selection.Rect, Vec2(float(SourceSizeX), float(SourceSizeY)));
    area.mTexture = mSourceTextrue;
    mPreviewSprite->mFrames.PushBack(area);
  }

  //Reset frame back
  mPreviewSprite->SetCurrentFrame(mPreviewSprite->mCurrentFrame);
}

void SpriteSheetImporter::ClearSelectedFrames()
{
  mFrames.Clear();
  UpdatePreview();
}

void SpriteSheetImporter::OnClearPressed(ObjectEvent* event)
{
  ClearSelectedFrames();
  SetImportFrames(ImportFrames::SelectedFrames);
}

void SpriteSheetImporter::SaveDataToSpriteSource(SpriteSource* sprite, PixelRect frameSize, uint numberOfFrames)
{
  Vec2 origin = ComputeOrigin(mOrigin, frameSize.SizeX, frameSize.SizeY);

  sprite->FrameCount = numberOfFrames;
  sprite->FrameSizeX = frameSize.SizeX;
  sprite->FrameSizeY = frameSize.SizeY;
  sprite->OriginX = origin.x;
  sprite->OriginY = origin.y;
  sprite->FrameDelay = 1.0f / mPreviewSprite->mFramesPerSecond;
  sprite->Sampling = Sampling;
  sprite->PixelsPerUnit = (float)PixelsPerUnit;
  sprite->Looping = true;

  sprite->mContentItem->SaveContent();
}

SpriteSource* SpriteSheetImporter::AddSpriteResource(StringParam name, Image& output, PixelRect frameSize, uint numberOfFrames)
{
  String fileName = FilePath::Combine(GetTemporaryDirectory(), "SpriteTemp.png");

  Status status;
  SaveToPng(status, &output, fileName);

  if (!status)
  {
    DoNotifyStatus(status);
    return NULL;
  }

  ResourceAdd resourceAdd;
  resourceAdd.Library = Z::gEditor->mProjectLibrary;
  resourceAdd.Name = name;
  resourceAdd.SourceFile = fileName;

  AddNewResource(SpriteSourceManager::GetInstance(), resourceAdd);

  if (resourceAdd.WasSuccessful())
  {
    SpriteSource* spriteSource = (SpriteSource*)resourceAdd.SourceResource;
    SaveDataToSpriteSource(spriteSource, frameSize, numberOfFrames);
    return spriteSource;
  }
  else
  {
    return NULL;
  }
}

bool SpriteSheetImporter::AddFramesAsSprites()
{
  uint frameCount = mFrames.Size();

  if (frameCount == 0)
  {
    DoNotifyError("Error", "No frames selected to add.");
    return false;
  }

  PixelsPerUnit = FrameSizeX;

  Image output;
  output.Allocate(FrameSizeX, FrameSizeY);

  PixelRect frameRect;
  frameRect.X = 0;
  frameRect.Y = 0;
  frameRect.SizeX = FrameSizeX;
  frameRect.SizeY = FrameSizeY;

  Array<SpriteSource*> addedFrames;

  // number of digits for frames
  // Changed decimal places to always be 3 for the resource names
  // so that imported groups can continue under the same name or fill
  // deleted spots and still match naming scheme up to 1000 sprites
  uint tilePlaces = 3;

  uint imageNum = 0;
  for (uint i = 0; i < mFrames.Size(); ++i)
  {
    String subName;
    do
    {
      subName = String::Format("%s%0*d", Name.c_str(), tilePlaces, imageNum++);
    } while (SpriteSourceManager::FindOrNull(subName) != NULL);

    FrameArea& frame = mFrames[i];
    CopyImage(&output, &mSourcePixels, 0, 0, frame.Rect.X, frame.Rect.Y, frame.Rect.SizeX, frame.Rect.SizeY);

    if (UseAlphaColorKey)
      SetColorToAlpha(&output, ToByteColor(AlphaColor));

    SpriteSource* sprite = AddSpriteResource(subName, output, frameRect, 1);
    addedFrames.PushBack(sprite);
  }

  if (CreatePalette)
  {
    // Setup defaults
    Archetype* archetype = ArchetypeManager::FindOrNull("DefaultTile");
    PhysicsMesh* mesh = PhysicsMeshManager::FindOrNull("Box");
    TilePaletteSource* tilePalette = new TilePaletteSource();

    // Add each added sprite to the tile Palette
    for (uint i = 0; i < addedFrames.Size(); ++i)
    {
      SpriteSource* sprite = addedFrames[i];
      // Recompute the index from the top left pixel
      IntVec2 gridIndex = GetGridIndex(mFrames[i].Rect.TopLeft());

      // Map the tile
      TileMap::Tile tile(archetype->mResourceId, sprite->mResourceId, mesh->mResourceId, true);
      tilePalette->mData[gridIndex] = tile;
    }

    // Add the tile palette resource
    ResourceAdd resourceAdd;
    resourceAdd.Library = Z::gEditor->mProjectLibrary;
    resourceAdd.Name = Name;
    resourceAdd.SourceResource = tilePalette;
    AddNewResource(TilePaletteSourceManager::GetInstance(), resourceAdd);
  }

  SpriteSourceManager::GetInstance()->RebuildSpriteSheets();

  return true;
}

bool SpriteSheetImporter::AddMultiFrameSprite()
{
  uint frameCount = mFrames.Size();

  if (frameCount == 0)
  {
    DoNotifyError("Error", "No frames selected to add.");
    return false;
  }

  SpriteFrameLayout frameLayout(frameCount, FrameSizeX, FrameSizeY);

  if (frameLayout.TotalSize.SizeX > MaxSpriteSize ||
      frameLayout.TotalSize.SizeY > MaxSpriteSize)
  {
    DoNotifyError("Error", "Sprite is too large");
    return false;
  }

  Image output;
  output.Allocate(frameLayout.TotalSize.SizeX, frameLayout.TotalSize.SizeY);
  output.ClearColorTo(0);

  for (uint i = 0; i < mFrames.Size(); ++i)
  {
    PixelRect destRect = frameLayout.GetFrame(i);
    FrameArea& sourceFrame = mFrames[i];
    CopyImage(&output, &mSourcePixels, destRect.X, destRect.Y, sourceFrame.Rect.X, sourceFrame.Rect.Y, sourceFrame.Rect.SizeX, sourceFrame.Rect.SizeY);
  }

  if (UseAlphaColorKey)
    SetColorToAlpha(&output, ToByteColor(AlphaColor));

  SpriteSource* source = AddSpriteResource(Name, output, frameLayout.GetFrame(0), mFrames.Size());
  if (source == nullptr)
    return false;

  source->mAtlas->NeedsBuilding = true;
  mFrames.Clear();

  SpriteSourceManager::GetInstance()->RebuildSpriteSheets();

  return true;
}

void SpriteSheetImporter::OnAddAndContinue(ObjectEvent* event)
{
  AddMultiFrameSprite();
}

void SpriteSheetImporter::OnAddTiles(ObjectEvent* event)
{
  AddFramesAsSprites();
}

void SpriteSheetImporter::OnAddPressed(ObjectEvent* event)
{
  bool added = AddMultiFrameSprite();
  if (added)
    Close();
}

void SpriteSheetImporter::OnClosePressed(ObjectEvent* event)
{
  Close();
}

void SpriteSheetImporter::OnFileSelected(OsFileSelection* event)
{
  if (!event->Success)
    Close();
  else
  {
    if (event->Files.Size() > 1)
      LoadImages(event->Files);
    else
      LoadImage(event->Files[0]);
  }
}

void SpriteSheetImporter::OnKeyDown(KeyboardEvent* keyEvent)
{
  IntVec2 nudgeDirection = IntVec2(0, 0);

  if (keyEvent->Key == Keys::Down)
    nudgeDirection = IntVec2(0, 1);
  if (keyEvent->Key == Keys::Up)
    nudgeDirection = IntVec2(0, -1);

  if (keyEvent->Key == Keys::Left)
    nudgeDirection = IntVec2(-1, 0);
  if (keyEvent->Key == Keys::Right)
    nudgeDirection = IntVec2(1, 0);

  NudgePosition(nudgeDirection);

  if (keyEvent->Key == Keys::Delete)
  {
    Array<FrameArea> framesToKeep;
    forRange(FrameArea& frameArea, mFrames.All())
    {
      if (!frameArea.Active)
        framesToKeep.PushBack(frameArea);
    }
    mFrames.Swap(framesToKeep);
  }
}

void SpriteSheetImporter::OnMouseScrollGrid(MouseEvent* mouseEvent)
{
  if (mouseEvent->CtrlPressed)
  {
    Vec2 percent = mGrid->ToLocal(mouseEvent->Position) / mGrid->GetSize();
    float zoom;
    if (mouseEvent->Scroll.y > 0)
      zoom = mZoom + cSpriteImporterZoomIncrement;
    else
      zoom = mZoom - cSpriteImporterZoomIncrement;
    SetZoom(zoom);

    UpdateZoomedSize();

    Vec2 newZoomPosition = mGrid->GetSize() * percent;

    Vec2 mouseLocalToScroll = mScrollArea->ToLocal(mouseEvent->Position);

    mScrollArea->SetScrolledOffset(-mouseLocalToScroll + newZoomPosition);
  }
}

void SpriteSheetImporter::OnMouseMoveGrid(MouseEvent* mouseEvent)
{
  if (HandleGridMouseControls(mouseEvent))
    return;

  IntVec2 imagePos = ToImagePosition(mouseEvent->Position);
  IntVec2 gridIndex = GetGridIndex(imagePos);

  if (!IsValidGridIndex(gridIndex))
    return;

  if (mouseEvent->IsButtonDown(MouseButtons::Left))
  {
    int frameIndex = 0;
    if (!CheckFramesAt(imagePos, frameIndex))
    {
      AddFrame(gridIndex);
    }
  }

  if (mouseEvent->IsButtonDown(MouseButtons::Right))
  {
    int frameIndex = 0;
    if (CheckFramesAt(imagePos, frameIndex))
    {
      RemoveFrame(frameIndex);
    }
  }
}

// Returns true if it handles the input!
bool SpriteSheetImporter::HandleGridMouseControls(MouseEvent* mouseEvent)
{
  if (!mouseEvent->IsButtonDown(MouseButtons::Left))
    return false;

  IntVec2 imagePos = ToImagePosition(mouseEvent->Position);

  if (mouseEvent->CtrlPressed && mouseEvent->AltPressed)
  {
    int dx = imagePos.x - OffsetX;
    int dy = imagePos.y - OffsetY;

    SetSpacingX(dx - FrameSizeX);
    SetSpacingY(dy - FrameSizeY);
    return true;
  }

  // Ctrl to move origin
  if (mouseEvent->CtrlPressed)
  {
    SetOffsetX(imagePos.x);
    SetOffsetY(imagePos.y);
    return true;
  }

  if (mouseEvent->AltPressed)
  {
    SetFrameWidth(imagePos.x - OffsetX);
    SetFrameHeight(imagePos.y - OffsetY);
    return true;
  }

  return false;
}

void SpriteSheetImporter::OnLeftMouseDownGrid(MouseEvent* mouseEvent)
{
  if (HandleGridMouseControls(mouseEvent))
    return;

  IntVec2 imagePos = ToImagePosition(mouseEvent->Position);
  IntVec2 gridIndex = GetGridIndex(imagePos);

  if (!IsValidGridIndex(gridIndex))
    return;

  mSourceDisplay->HardTakeFocus();

  // Auto change to selected frames
  SetImportFrames(ImportFrames::SelectedFrames);

  // Multi select only if shift
  if (!mouseEvent->ShiftPressed)
  {
    forRange(FrameArea& selection, mFrames.All())
      selection.Active = false;
  }

  int frameIndex = 0;
  if (CheckFramesAt(imagePos, frameIndex))
  {
    FrameArea& frameArea = mFrames[frameIndex];
    frameArea.Active = !frameArea.Active;
    mPreviewSprite->SetCurrentFrame(frameIndex);
  }
  else
  {
    AddFrame(gridIndex);
  }
}

void SpriteSheetImporter::OnRightMouseDownGrid(MouseEvent* mouseEvent)
{
  if (HandleGridMouseControls(mouseEvent))
    return;

  IntVec2 imagePos = ToImagePosition(mouseEvent->Position);
  IntVec2 gridIndex = GetGridIndex(imagePos);

  if (!IsValidGridIndex(gridIndex))
    return;

  mSourceDisplay->HardTakeFocus();

  int frameIndex = 0;
  if (CheckFramesAt(imagePos, frameIndex))
    RemoveFrame(frameIndex);
}

void SpriteSheetImporter::UpdateZoomedSize()
{
  Vec2 imageDisplaySize = Pixels(float(SourceSizeX), float(SourceSizeY)) * mZoom;
  mSourceDisplay->SetSize(imageDisplaySize);
  mGrid->SetSize(imageDisplaySize);
  mImageBackground->SetSize(imageDisplaySize);
  mScrollArea->SetClientSize(imageDisplaySize);
}

void SpriteSheetImporter::UpdateTransform()
{
  UpdateZoomedSize();
  UpdatePreview();
  Composite::UpdateTransform();
}


void SpriteSheetImporter::DrawLines(Array<StreamedVertex>& lines, uint axis, float zoom, float spacing, Vec2 totalSize, Vec2 startOffset, uint lineCount)
{
  Vec4 color = ToFloatColor(Color::Red);
  for (uint line = 0; line < lineCount + 1; ++line)
  {
    Vec3 start = Vec3(0, 0, 0);
    start[axis] = startOffset[axis] + spacing * line;
    start[!axis] = startOffset[!axis];

    Vec3 end = start;
    end[!axis] += totalSize[!axis];

    lines.PushBack(StreamedVertex(SnapToPixels(start*zoom) + Pixels(0.5, 0.5, 0), Vec2(0, 0), color));
    lines.PushBack(StreamedVertex(SnapToPixels(end*zoom) + Pixels(0.5, 0.5, 0), Vec2(0, 0), color));
  }
}

void SpriteSheetImporter::DrawRedirect(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Mat4 oldWorldTx = mWorldTx;
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  Vec2 frameSize = Pixels(float(FrameSizeX), float(FrameSizeY));
  Vec2 spacing = Pixels(float(FrameSizeX + SpacingX), float(FrameSizeY + SpacingY));
  Vec2 offset = Pixels(float(OffsetX), float(OffsetY));

  uint cellsX = FramesX;
  uint cellsY = FramesY;

  Vec2 usedSize = Vec2(spacing.x * cellsX, spacing.y * cellsY);

  Array<StreamedVertex> lines;

  DrawLines(lines, 0, mZoom, spacing.x, usedSize, offset, cellsX);
  DrawLines(lines, 1, mZoom, spacing.y, usedSize, offset, cellsY);

  if (SpacingX > 0.0f)
    DrawLines(lines, 0, mZoom, spacing.x, usedSize, offset + Vec2(frameSize.x, 0), cellsX - 1);

  if (SpacingY > 0.0f)
    DrawLines(lines, 1, mZoom, spacing.y, usedSize, offset + Vec2(0, frameSize.y), cellsY - 1);

  CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);

  ByteColor color = Color::Red;
  SetAlphaByte(color, 80);

  ByteColor colorSelect = Color::Blue;
  SetAlphaByte(colorSelect, 80);

  RenderQueues* renderQueues = frameBlock.mRenderQueues;
  static Texture* white = TextureManager::FindOrNull("White");
  static RenderFont* font = FontManager::GetInstance()->GetRenderFont("NotoSans-Regular", 11, 0);

  ViewNode& viewNodeQuads = AddRenderNodes(viewBlock, frameBlock, clipRect, white);

  // Draw the selected frames
  // Only draw the frame selected boxes in selected frame to prevent clutter
  if (mImportFrames == ImportFrames::SelectedFrames)
  {
    forRange(FrameArea& frameArea, mFrames.All())
    {
      Vec3 t = Pixels(float(frameArea.Rect.X), float(frameArea.Rect.Y), 0) * mZoom;
      Vec3 s = Pixels(float(frameArea.Rect.SizeX), float(frameArea.Rect.SizeY), 0) * mZoom;

      if (frameArea.Active)
        renderQueues->AddStreamedQuad(viewNodeQuads, t, t + s, Vec2(0, 0), Vec2(1, 1), ToFloatColor(colorSelect));
      else
        renderQueues->AddStreamedQuad(viewNodeQuads, t, t + s, Vec2(0, 0), Vec2(1, 1), ToFloatColor(color));
    }
  }

  ViewNode& viewNodeText = AddRenderNodes(viewBlock, frameBlock, clipRect, font->mTexture);
  FontProcessor fontProcessor(renderQueues, &viewNodeText, ToFloatColor(Color::Black));

  // Draw a number over each frame
  uint i = 0;
  forRange(FrameArea& frameArea, mFrames.All())
  {
    Vec2 t = Pixels(float(frameArea.Rect.X), float(frameArea.Rect.Y));
    Vec2 s = Pixels(float(frameArea.Rect.SizeX), float(frameArea.Rect.SizeY));

    String name = String::Format("%d", i);
    ProcessTextRange(fontProcessor, font, name, t * mZoom, TextAlign::Left, Vec2(1, 1), s);

    ++i;
  }

  mWorldTx = oldWorldTx;
}

void SpriteSheetImporter::NudgePosition(IntVec2 move)
{
  forRange(FrameArea& frameArea, mFrames.All())
  {
    if (frameArea.Active)
    {
      frameArea.Rect.X += move.x;
      frameArea.Rect.Y += move.y;
    }
  }
  UpdatePreview();
}

void SpriteSheetImporter::AddFrame(IntVec2 gridCell)
{
  FrameArea frame;
  frame.Active = false;
  frame.Rect = GetRectAtIndex(gridCell);
  mFrames.PushBack(frame);
  UpdatePreview();
}

void SpriteSheetImporter::RemoveFrame(int frameIndex)
{
  mFrames.EraseAt(frameIndex);
  UpdatePreview();
}

PixelRect SpriteSheetImporter::GetRectAtIndex(IntVec2 gridCell)
{
  PixelRect r;
  r.X = gridCell.x * (FrameSizeX + SpacingX) + OffsetX;
  r.Y = gridCell.y * (FrameSizeY + SpacingY) + OffsetY;
  r.SizeX = FrameSizeX;
  r.SizeY = FrameSizeY;
  return r;
}

bool SpriteSheetImporter::CheckFramesAt(IntVec2 loction, int& frameSelected)
{
  for (int i = 0; i<int(mFrames.Size()); ++i)
  {
    FrameArea& area = mFrames[i];
    if (area.Rect.Contains(loction))
    {
      frameSelected = i;
      return true;
    }
  }
  return false;
}

// Get the grid / frame index from an image position
// Will return -1,-1 if the position is between frames in the spacing
IntVec2 SpriteSheetImporter::GetGridIndex(IntVec2 imagePos)
{
  IntVec2 offsetPos = imagePos - IntVec2(OffsetX, OffsetY);
  IntVec2 frameSpacing = IntVec2(FrameSizeX + SpacingX, FrameSizeY + SpacingY);
  int x = offsetPos.x / frameSpacing.x;
  int y = offsetPos.y / frameSpacing.y;

  IntVec2 gridIndex = IntVec2(x, y);
  PixelRect rect = GetRectAtIndex(gridIndex);

  if (rect.Contains(imagePos))
    return gridIndex;
  else
    return IntVec2(-1, -1);
}

IntVec2 SpriteSheetImporter::ToImagePosition(Vec2 screenPoint)
{
  Vec2 localOffset = mGrid->ToLocal(screenPoint);
  return ToIntVec2(localOffset * (1.0f / mZoom));
}

bool SpriteSheetImporter::IsValidGridIndex(IntVec2 gridIndex)
{
  if (gridIndex.x >= FramesX || gridIndex.y >= FramesY)
    return false;

  if (gridIndex.x < 0 || gridIndex.y < 0)
    return false;

  return true;
}

void SpriteSheetImporter::SetZoom(float zoom)
{
  mZoom = zoom;
  mZoom = Math::Clamp(mZoom, cMinSpriteImporterZoom, cMaxSpriteImporterZoom);
}

//------------------------------------------------------------------------ Getters/Setters
int SpriteSheetImporter::GetFrameWidth(){return FrameSizeX;}
void SpriteSheetImporter::SetFrameWidth(int frameSizeX)
{
  ComputeFrameWidthAndCount(FrameSizeX, FramesX, frameSizeX, SpacingX, SourceSizeX, OffsetX);
  CheckFrames();
}

int SpriteSheetImporter::GetFrameHeight(){return FrameSizeY;}
void SpriteSheetImporter::SetFrameHeight(int frameSizeY)
{
  ComputeFrameWidthAndCount(FrameSizeY, FramesY, frameSizeY, SpacingY, SourceSizeY, OffsetY);
  CheckFrames();
}

int SpriteSheetImporter::GetFramesPerRow(){return FramesX;}
void SpriteSheetImporter::SetFramesPerRow(int framesX)
{
  FramesX = Math::Clamp(framesX, 1, SourceSizeX);
  FrameSizeX = ( (SourceSizeX - OffsetX) / FramesX) - SpacingX;
  CheckFrames();
}

int SpriteSheetImporter::GetNumberOfRows(){return FramesY;}
void SpriteSheetImporter::SetNumberOfRows(int framesY)
{
  FramesY = Math::Clamp(framesY, 1, SourceSizeY);
  FrameSizeY = ( (SourceSizeY - OffsetY) / FramesY) - SpacingY;
  CheckFrames();
}

int SpriteSheetImporter::GetOffsetX(){return OffsetX;}
void SpriteSheetImporter::SetOffsetX(int offset)
{
  OffsetX = Math::Clamp(offset, 0, SourceSizeX);
  SetFrameWidth(FrameSizeX);
}

int SpriteSheetImporter::GetOffsetY(){return OffsetY;}
void SpriteSheetImporter::SetOffsetY(int offset)
{
  OffsetY = Math::Clamp(offset, 0, SourceSizeY);
  SetFrameHeight(FrameSizeY);
}

int SpriteSheetImporter::GetSpacingX(){return SpacingX;}
void SpriteSheetImporter::SetSpacingX(int spacingX)
{
    SpacingX = Math::Clamp(spacingX, 0, SourceSizeX);
    SetFrameWidth(FrameSizeX);
}

int SpriteSheetImporter::GetSpacingY(){return SpacingY;}
void SpriteSheetImporter::SetSpacingY(int spacingY)
{
  SpacingY = Math::Clamp(spacingY, 0, SourceSizeY);
  SetFrameHeight(FrameSizeY);
}

int SpriteSheetImporter::GetFrameCount()
{
  return mFrames.Size();
}

ImportFrames::Type SpriteSheetImporter::GetImportFrames(){return mImportFrames;}
void SpriteSheetImporter::SetImportFrames(ImportFrames::Type newMode)
{
  //redundant set
  if(mImportFrames == newMode)
    return;

  mImportFrames = newMode;

  if(newMode == ImportFrames::AllFrames)
    AddAllFrames();
  else
    ClearSelectedFrames();
}

bool SpriteSheetImporter::GetPreviewAnimate(){return mPreviewSprite->mAnimating;}
void SpriteSheetImporter::SetPreviewAnimate(bool state)
{
  mPreviewSprite->mAnimating = state;
}

float SpriteSheetImporter::GetFrameRate(){return mPreviewSprite->mFramesPerSecond;}
void SpriteSheetImporter::SetFrameRate(float state)
{
  if(state > 0.001f)
    mPreviewSprite->mFramesPerSecond = state;
}

int SpriteSheetImporter::GetPreviewFrame(){return mPreviewSprite->mCurrentFrame;}
void SpriteSheetImporter::SetPreviewFrame(int frame)
{
  mPreviewSprite->SetCurrentFrame(frame);
  mPreviewSprite->mAnimating = false;
}

bool SpriteSheetImporter::GetUseAlphaColorKey(){return UseAlphaColorKey;}
void SpriteSheetImporter::SetUseAlphaColorKey(bool colorKey)
{
  UseAlphaColorKey = colorKey;
  UpdateTexture();
}

Vec4 SpriteSheetImporter::GetAlphaColor(){return AlphaColor;}
void SpriteSheetImporter::SetAlphaColor(Vec4 alphaColor)
{
  AlphaColor = alphaColor;
  if (UseAlphaColorKey)
    UpdateTexture();
}

SpriteSampling::Enum SpriteSheetImporter::GetSmoothing(){return Sampling;}
void SpriteSheetImporter::SetSmoothing(SpriteSampling::Enum sampling)
{
  Sampling = sampling;
  //if(Smoothing)
  //  mSourceTextrue->SetFiltering(Filtering::Bilinear);
  //else
  //  mSourceTextrue->SetFiltering(Filtering::Point);
}

//------------------------------------------------------------------------
SpriteSheetImporter* CreateImporter()
{
  Editor* editor = Z::gEditor;
  Window* window = new Window(Z::gEditor);
  window->SetTitle("Sprite Importer");
  SpriteSheetImporter* importer = new SpriteSheetImporter(window);
  window->SetSize(Pixels(800, 600));
  window->UpdateTransformExternal();
  CenterToWindow(Z::gEditor, window, true);
  return importer;
}

void SpriteSheetImport(StringParam filename)
{
  SpriteSheetImporter* importer = CreateImporter();
  importer->LoadImage(filename);
}

void SpriteSheetImport(SpriteSource* spriteSource)
{
  SpriteSheetImporter* importer = CreateImporter();
  importer->LoadImage(FilePath::Combine(spriteSource->mContentItem->mLibrary->SourcePath, spriteSource->mContentItem->Filename));

  // This usage doesn't work properly
  //importer->LoadSprite(spriteSource);
}

void SpriteSheetImport(Editor* editor)
{
  SpriteSheetImporter* importer = CreateImporter();

  //Open the open file dialog
  FileDialogConfig config;
  config.EventName = "OnFileSelected";
  config.CallbackObject = importer;
  config.Title = "Select sprite sheet to import";
  config.AddFilter("Png File", "*.png");
  config.StartingDirectory = editor->GetProjectPath();
  config.Flags |= FileDialogFlags::MultiSelect;
  Z::gEngine->has(OsShell)->OpenFile(config);
}

}
