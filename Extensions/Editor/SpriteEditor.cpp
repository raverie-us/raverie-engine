///////////////////////////////////////////////////////////////////////////////
///
/// \file SpriteEditor.cpp
///
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// All of this should be switched over to the data driven tweakable system
namespace SpriteTileViewUi
{
  // Grey
  static Vec4 BorderColor = ToFloatColor(ByteColorRGBA(142, 142, 142, 0));
  // Faded light blue
  static Vec4 SelectedBorderColor = ToFloatColor(ByteColorRGBA(63, 169, 245, 255));
  static Vec4 MouseOverBorderColor = ToFloatColor(ByteColorRGBA(63, 169, 245, 255));
  // Tile sizes
  static float MinTileSize = 60.0f;
  static float MaxTileSize = 500.0f;
}

UvRect ComputeTextureRect(PixelRect pixelRect, Vec2 textureSize)
{
  Vec2 inverseTextureSize = Vec2(1.0f, 1.0f) / textureSize;
  UvRect texRect;
  texRect.TopLeft = Vec2(float(pixelRect.X), float(pixelRect.Y)) * inverseTextureSize;
  texRect.BotRight = texRect.TopLeft + Vec2(float(pixelRect.SizeX), float(pixelRect.SizeY)) * inverseTextureSize;
  return texRect;
}

u32 GetNextPowerOfTwo(u32 value)
{
  value--;
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  value++;
  return value;
}

//------------------------------------------------------------ SpriteFrameLayout

IntVec2 FindBestSquareFit(uint frameSizeX, uint frameSizeY, uint frameCount)
{
  uint rows = 1;
  uint bestFit = uint(-1);
  IntVec2 bestFitDim = IntVec2(frameCount, 1);
  while(rows < frameCount)
  {
    // Need the number of columns
    // integer ceiling
    uint colsNeeded = frameCount / rows;
    if(frameCount % rows != 0)
      ++colsNeeded;

    uint finalHeight = rows * frameSizeY;
    uint finalWidth = colsNeeded * frameSizeX;
    uint newFit = Math::Abs(int(finalWidth - finalHeight));

    if(newFit < bestFit)
    {
      // Found a better fit
      bestFit = newFit;
      bestFitDim =  IntVec2(rows, colsNeeded);
    }

    ++rows;
  }

  return bestFitDim;
}

SpriteFrameLayout::SpriteFrameLayout(uint frameCount, uint frameSizeX, uint frameSizeY)
{
  TotalSize.X = 0;
  TotalSize.Y = 0;
  FrameSizeX = frameSizeX;
  FrameSizeY = frameSizeY;

  IntVec2 rowAndCols = FindBestSquareFit(frameSizeX, frameSizeY, frameCount);
  FramesPerRow = rowAndCols.y;
  FramesPerCol = rowAndCols.x;

  TotalSize.SizeX = FramesPerRow * FrameSizeX;
  TotalSize.SizeY = FramesPerCol * FrameSizeY;
}

SpriteFrameLayout::SpriteFrameLayout(uint frameCount, uint frameSizeX, uint frameSizeY, uint sizeX, uint sizeY)
{
  TotalSize.X = 0;
  TotalSize.Y = 0;
  FrameSizeX = frameSizeX;
  FrameSizeY = frameSizeY;
  TotalSize.SizeX = sizeX;
  TotalSize.SizeY = sizeY;
  FramesPerRow = sizeX / frameSizeX;
}

PixelRect SpriteFrameLayout::GetFrame(uint frameIndex)
{
  PixelRect rect;
  rect.X = (frameIndex % FramesPerRow) * FrameSizeX;
  rect.Y = (frameIndex / FramesPerRow) * FrameSizeY;
  rect.SizeX = FrameSizeX;
  rect.SizeY = FrameSizeY;
  return rect;
}

//------------------------------------------------------------ Origin

void ComputeOrigins(Array<Vec2>& origins, int frameSizeX, int frameSizeY)
{
  origins.PushBack(Vec2(0, 0));
  origins.PushBack(Vec2(frameSizeX/2.0f, frameSizeY/2.0f));
  origins.PushBack(Vec2(0, 0));
  origins.PushBack(Vec2(0, (real)frameSizeY));
  origins.PushBack(Vec2(frameSizeX/2.0f, (real)frameSizeY));
}

Vec2 ComputeOrigin(SpriteOrigin::Enum origin, int frameSizeX, int frameSizeY)
{
  Array<Vec2> origins;
  ComputeOrigins(origins, frameSizeX, frameSizeY);
  return origins[origin];
}

SpriteOrigin::Enum ComputeOrigin(Vec2 currentOrigin, int frameSizeX, int frameSizeY)
{
  Array<Vec2> origins;
  ComputeOrigins(origins, frameSizeX, frameSizeY);

  SpriteOrigin::Enum origin = SpriteOrigin::Custom;
  for(uint i=1;i<SpriteOrigin::Size;++i)
  {
    if(currentOrigin == origins[i])
      origin = (SpriteOrigin::Enum)i;
  }
  return origin;
}

//------------------------------------------------------------ SpriteFrame
ZilchDefineType(SpriteFrame, builder, type)
{
  ZilchBindConstructor(SpriteFrame&);
  ZilchBindDestructor();
}

SpriteFrame::SpriteFrame()
{
  mFrameTexture = NULL;
}

SpriteFrame::SpriteFrame(SpriteFrame& spriteFrame)
{
  AllocateFrame(spriteFrame.mFrameIndex, spriteFrame.mFrameImage, spriteFrame.mFrameRect);
}

SpriteFrame::~SpriteFrame()
{
}

// Allocate a texture to display a sprite frame
void SpriteFrame::AllocateFrame(uint frameIndex, Image& sourceImage, PixelRect sourceRect)
{
  mFrameTexture = NULL;

  mFrameIndex = frameIndex;

  uint textureSize = GetNextPowerOfTwo( Math::Max(sourceRect.SizeX, sourceRect.SizeY) );;

  PixelRect localFrameRect = {0,0, sourceRect.SizeX, sourceRect.SizeY};
  mFrameRect = localFrameRect;
  mTexRect = ComputeTextureRect(localFrameRect, Vec2(float(textureSize), float(textureSize)));
  mFrameImage.Allocate(textureSize, textureSize);

  CopyImage(&mFrameImage, &sourceImage, 0, 0, sourceRect.X, sourceRect.Y, sourceRect.SizeX, sourceRect.SizeY);

  mFrameTexture = Texture::CreateRuntime();
  mFrameTexture->Upload(mFrameImage);
}

SpriteFrame* GetSpriteFrame(Widget* sidget)
{
  Widget* widget = sidget->GetParent();
  while(widget && !ZilchVirtualTypeId(widget)->IsA(ZilchTypeId(SpriteFrame)))
    widget = widget->GetParent();
  return (SpriteFrame*)widget;  
}

//------------------------------------------------------------ SpritePreview
ZilchDefineType(SpritePreview, builder, type)
{

}

SpritePreview::SpritePreview(Composite* parent)
  :Composite(parent)
{
  mPreviewFrame = new TextureView(this);
  mBackground = new TextureView(this);
  mBackground->SetTexture(TextureManager::Find("AlphaBackground"));
  mBackground->SetActive(true);
  mBackground->MoveToBack();
  mAnimating = true;
  mFramesPerSecond = 12;
  mCurrentFrame = 0;
  mT = 0.0f;

  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);
}

SpritePreview::~SpritePreview()
{

}

void SpritePreview::SetCurrentFrame(int frame)
{
  if(mFrames.Empty())
    return;
  mCurrentFrame = frame % mFrames.Size();
  UpdateFrame();
}

void SpritePreview::UpdateFrame()
{
  mCurrentFrame = mCurrentFrame % mFrames.Size();
  mPreviewFrame->SetTexture(mFrames[mCurrentFrame].mTexture);
  mPreviewFrame->SetUv(mFrames[mCurrentFrame].mUvRect.TopLeft, mFrames[mCurrentFrame].mUvRect.BotRight);
  mPreviewFrame->SetSize(mSize);
  mBackground->SetSize(mSize);
  mBackground->SetUv(Vec2(0, 0), (mSize / 2.0f) / 8.0f);
}

void SpritePreview::OnUpdate(UpdateEvent* updateEvent)
{
  // Update animation
  if(mFrames.Empty())
    return;
  mT += updateEvent->RealDt;
  float frameTime = 0.0f;
  if(mFramesPerSecond > 0.0f)
    frameTime = (1.0f / mFramesPerSecond);
  if(mAnimating && mT > frameTime)
  {
    mCurrentFrame += 1;
    mT = 0.0f;
    UpdateFrame();
  }
}

void SpritePreview::UpdateTransform()
{
  mPreviewFrame->SetSize(mSize);
  Composite::UpdateTransform();
}

//------------------------------------------------------------------------ Sprite Preview Widget
SpritePreviewWidget::SpritePreviewWidget(SpriteFrame* spriteFrame, Composite* parent)
  : PreviewWidget(parent)
{
  mTextureView = new TextureView(this);
  mTextureView->SetTexture(spriteFrame->mFrameTexture);
  mTextureView->SetUv(spriteFrame->mTexRect.TopLeft, spriteFrame->mTexRect.BotRight);

  mBackground = new TextureView(this);
  mBackground->SetTexture(TextureManager::Find("AlphaBackground"));
  mBackground->SetActive(true);
  mBackground->MoveToBack();
}

void SpritePreviewWidget::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mTextureView->SetSize(mSize);
  mBackground->SetUv(Vec2(0, 0), (mSize / 2.0f) / 8.0f);
  Composite::UpdateTransform();
}

//------------------------------------------------------------------------ Sprite Tile View Widget
SpriteTileViewWidget::SpriteTileViewWidget(Composite* parent, TileView* tileView, PreviewWidget* tileWidget, DataIndex dataIndex)
 : TileViewWidget(parent, tileView, tileWidget, dataIndex)
{
  mContentMargins = Thickness(1.5f, 1.5f, 2.5f, 2.5f);
}

void SpriteTileViewWidget::OnMouseHover(MouseEvent* event)
{
  // do nothing, we don't want the popup
}

// We do not currently support multi-select actions in the sprite editor and this 
// event handler intercepts and overrides the default tile view widget behavior until
// the actions support multi-select
void SpriteTileViewWidget::OnMouseClick(MouseEvent* event)
{
  if (event->Handled)
    return;

  Select(true);
}

//------------------------------------------------------------------------ Sprite Frame Tile View
TileViewWidget* SpriteFrameTileView::CreateTileViewWidget(Composite* parent, StringParam name, HandleParam instance, DataIndex index, PreviewImportance::Enum minImportance /*= PreviewImportance::None*/)
{
  if (SpriteFrame* frame = instance.Get<SpriteFrame*>())
  {
    SpritePreviewWidget* preview = new SpritePreviewWidget(frame, parent);
    return new SpriteTileViewWidget(parent, this, preview, index);
  }
  return nullptr;
}


void SpriteFrameTileView::OnMouseScroll(MouseEvent* event)
{
  float itemSize = GetItemSize();
  if (event->CtrlPressed)
  {
    float newSize = itemSize + event->Scroll.y * 2;

    if (newSize <= SpriteTileViewUi::MinTileSize)
    {
      itemSize = SpriteTileViewUi::MinTileSize;
      Event event;
      GetDispatcher()->Dispatch(Events::ScrolledAllTheWayOut, &event);
    }
    else if (newSize >= SpriteTileViewUi::MaxTileSize)
    {
      itemSize = SpriteTileViewUi::MaxTileSize;
      Event event;
      GetDispatcher()->Dispatch(Events::ScrolledAllTheWayIn, &event);
    }
    else
    {
      itemSize = newSize;
    }

    SetItemSize(itemSize);
    this->MarkAsNeedsUpdate();
  }
}

void SpriteFrameTileView::OnLeftMouseDrag(MouseDragEvent* e)
{
  // do nothing, in the sprite editor we do not currently support frame operations
  // for multiple selected frames
}

//------------------------------------------------------------------------ Sprite Data Source
SpriteDataSource::SpriteDataSource()
  : mRoot()
{

}

SpriteDataSource::~SpriteDataSource()
{
  ClearFrames();
}

void SpriteDataSource::AddSpriteFrame(SpriteFrame* sprite)
{
  mSpriteFrames.PushBack(sprite);
}

void SpriteDataSource::RemoveSpriteFrame(SpriteFrame* sprite)
{
  // Do not remove last frame
  if (mSpriteFrames.Size() <= 1)
    return;

  // Erase frame
  mSpriteFrames.EraseAt(sprite->mFrameIndex);
  delete sprite;

  // Update frame indexes for each frame so they are correct after a removal
  for (size_t index = 0; index < mSpriteFrames.Size(); ++index)
    mSpriteFrames[index]->mFrameIndex = index;
}

void SpriteDataSource::RemoveSpriteFrame(DataIndex frameDataIndex)
{
  // Do not remove last frame
  if (mSpriteFrames.Size() <= 1)
    return;

  // Erase frame
  SpriteFrame* sprite = (SpriteFrame*)ToEntry(frameDataIndex);
  mSpriteFrames.EraseAt(sprite->mFrameIndex);
  delete sprite;

  // Update frame indexes for each frame so they are correct after a removal
  for (size_t index = 0; index < mSpriteFrames.Size(); ++index)
    mSpriteFrames[index]->mFrameIndex = index;
}

void SpriteDataSource::ClearFrames()
{
  DeleteObjectsInContainer(mSpriteFrames);
  mSpriteFrames.Clear();
}

SpriteFrame* SpriteDataSource::GetSpriteFrame(size_t index)
{
  if (index >= mSpriteFrames.Size())
    return mSpriteFrames.Back();

  return mSpriteFrames[index];
}

size_t SpriteDataSource::Size()
{
  return mSpriteFrames.Size();
}

Array<SpriteFrame*>::range SpriteDataSource::All()
{
  return mSpriteFrames.All();
}

SpriteFrame* SpriteDataSource::Back()
{
  return mSpriteFrames.Back();
}

SpriteFrame* SpriteDataSource::operator[](size_t index)
{
  return mSpriteFrames[index];
}

DataEntry* SpriteDataSource::GetRoot()
{
  return &mRoot;
}

DataEntry* SpriteDataSource::ToEntry(DataIndex index)
{
  if (index == cRootIndex || (uint)index.Id >= mSpriteFrames.Size())
    return &mRoot;
  return mSpriteFrames[(uint)index.Id];
}

DataIndex SpriteDataSource::ToIndex(DataEntry* dataEntry)
{
  if (dataEntry == &mRoot)
    return cRootIndex;
  SpriteFrame* entry = (SpriteFrame*)dataEntry;
  uint index = mSpriteFrames.FindIndex(entry);
  return DataIndex(index);
}

Handle SpriteDataSource::ToHandle(DataEntry* dataEntry)
{
  return (SpriteFrame*)dataEntry;
}

DataEntry* SpriteDataSource::Parent(DataEntry* dataEntry)
{
  if (dataEntry == &mRoot)
    return NULL;
  return &mRoot;
}

uint SpriteDataSource::ChildCount(DataEntry* dataEntry)
{
  if (dataEntry == &mRoot)
    return mSpriteFrames.Size();
  return 0;
}

DataEntry* SpriteDataSource::GetChild(DataEntry* dataEntry, uint index, DataEntry* prev)
{
  if (dataEntry == &mRoot)
    return mSpriteFrames[index];
  return NULL;
}

bool SpriteDataSource::IsExpandable(DataEntry* dataEntry)
{
  return false;
}

void SpriteDataSource::GetData(DataEntry* dataEntry, Any& variant, StringParam column)
{
  SpriteFrame* entry = (SpriteFrame*)dataEntry;
  if (column == CommonColumns::Name)
    variant = String();
  else
    variant = entry;
}

bool SpriteDataSource::SetData(DataEntry* dataEntry, const Any& variant, StringParam column)
{
  Error("Sprite Editor: Did this get hit?");
  return false;
}

//------------------------------------------------------------------------ Sprite Source Editor
ZilchDefineType(SpriteSourceEditor, builder, type)
{
  ZilchBindGetterSetterProperty(SpriteName);

  ZilchBindGetterSetterProperty(Origin);
  ZilchBindFieldProperty(mOriginX);
  ZilchBindFieldProperty(mOriginY);

  ZilchBindFieldProperty(mLooping);

  ZilchBindGetterSetterProperty(Sampling); 
  ZilchBindGetterSetterProperty(FrameRate);
  ZilchBindFieldProperty(mPixelsPerUnit);

  ZilchBindFieldProperty(mSpriteFill);

  ZilchBindGetterSetterProperty(Left);
  ZilchBindGetterSetterProperty(Right);
  ZilchBindGetterSetterProperty(Top);
  ZilchBindGetterSetterProperty(Bottom);

  ZilchBindGetterSetterProperty(CurrentFrame);
  ZilchBindGetterSetterProperty(PreviewAnimation);
}

SpriteSourceEditor::SpriteSourceEditor(Composite* parent)
  :Composite(parent)
{
  this->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness::cZero));

  Composite* top = new Composite(this);
  top->SetLayout(CreateRowLayout());
  top->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  Composite* bottom = new Composite(this);
  bottom->SetLayout(CreateStackLayout(LayoutDirection::RightToLeft, Pixels(4, 4), Thickness(4, 4, 4, 4)));

  Composite* left = new Composite(top);
  left->SetSizing(SizeAxis::X, SizePolicy::Flex, 35);
  left->SetLayout(CreateStackLayout());

  mSpriteProperties = new PropertyView(left);
  mSpriteProperties->SetSizing(SizeAxis::Y, SizePolicy::Flex, 30);
  mSpriteProperties->ActivateAutoUpdate();

  Splitter* splitter = new Splitter(top);
  splitter->SetSize(Pixels(2, 2));

  Composite* right = new Composite(top);
  right->SetSizing(SizeAxis::X, SizePolicy::Flex, 65);
  right->SetLayout(CreateStackLayout());

  Composite* buttonRow = new Composite(right);
  buttonRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));

  mTileView = new SpriteFrameTileView(right);
  mTileView->SetActive(true);
  mTileView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mTileView->SetDataSource(&mSpriteData);
  mTileView->SetItemSizePercent(0.5f);
  
  // Events for interacting with the sprite frames
  ConnectThisTo(&mSpriteData, Events::DataSelected, OnSelection);
  ConnectThisTo(&mSpriteData, Events::DataActivated, OnSelectionActivated);

  mEditDirectory = FilePath::Combine(GetTemporaryDirectory(), "TempEdit");

  CreateDirectoryAndParents(mEditDirectory);

  mDirectoryWatcher = new EventDirectoryWatcher(mEditDirectory);

  ConnectThisTo(mDirectoryWatcher, Events::FileModified, OnFileModified);

  TextButton* textButton = new TextButton(left);
  textButton->SetText("Edit Frames Externally");
  ConnectThisTo(textButton, Events::ButtonPressed, OnEditSpriteSheet);

  textButton = new TextButton(left);
  textButton->SetText("Convert To Animation");
  ConnectThisTo(textButton, Events::ButtonPressed, OnConvertToAnimation);

  mPreview = new SpritePreview(left);
  mPreview->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  //Frames
  textButton = new TextButton(buttonRow);
  textButton->SetText("Add Frame");
  ConnectThisTo(textButton, Events::ButtonPressed, OnAddFrame);

  textButton = new TextButton(buttonRow);
  textButton->SetText("Remove Frame");
  ConnectThisTo(textButton, Events::ButtonPressed, OnRemoveFrame);

  textButton = new TextButton(buttonRow);
  textButton->SetText("Edit Frame");
  ConnectThisTo(textButton, Events::ButtonPressed, OnEditFrame);

  textButton = new TextButton(buttonRow);
  textButton->SetText("Move Frame Up");
  ConnectThisTo(textButton, Events::ButtonPressed, OnMoveFrameUp);

  textButton = new TextButton(buttonRow);
  textButton->SetText("Move Frame Down");
  ConnectThisTo(textButton, Events::ButtonPressed, OnMoveFrameDown);
  
  textButton = new TextButton(buttonRow);
  textButton->SetText("Export All Frames");
  ConnectThisTo(textButton, Events::ButtonPressed, OnExportAllFrames);
  
  textButton = new TextButton(bottom);
  textButton->SetText("Close");
  ConnectThisTo(textButton, Events::ButtonPressed, OnClosePressed);

  textButton = new TextButton(bottom);
  textButton->SetText("Save to Sprite Source");
  ConnectThisTo(textButton, Events::ButtonPressed, OnSaveToSprite);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, Events::KeyRepeated, OnKeyDown);

  //listen for when a sprite source is removed in case it's the one we're editing
  SpriteSourceManager* spriteManager = SpriteSourceManager::GetInstance();
  ConnectThisTo(spriteManager, Events::ResourceRemoved, OnSpriteSourceRemoved);

  mLeft = 0;
  mRight = 0;
  mTop = 0;
  mBottom = 0;
}

int SpriteSourceEditor::GetLeft()
{
  return mLeft;
}

void SpriteSourceEditor::SetLeft(int v)
{
  mLeft = Math::Clamp(v, 0, mFrameSizeX);
}

int SpriteSourceEditor::GetRight()
{
  return mRight;
}

void SpriteSourceEditor::SetRight(int v)
{
  mRight = Math::Clamp(v, 0, mFrameSizeX);
}

int SpriteSourceEditor::GetTop()
{
  return mTop;
}

void SpriteSourceEditor::SetTop(int v)
{
  mTop = Math::Clamp(v, 0, mFrameSizeY);
}

int SpriteSourceEditor::GetBottom()
{
  return mBottom;
}

void SpriteSourceEditor::SetBottom(int v)
{
  mBottom = Math::Clamp(v, 0, mFrameSizeY);
}

SpriteSourceEditor::~SpriteSourceEditor()
{
  SafeDelete(mDirectoryWatcher);
}

void SpriteSourceEditor::SetSpriteName(StringParam name)
{
  Status status;
  if(!IsValidFilename(name, status))
  {
    DoNotifyWarning("Invalid Sprite Name", status.Message);
    return;
  }
  mSpriteName = name;
}

bool SpriteSourceEditor::GetPreviewAnimation()
{
  return mPreview->mAnimating;
}

void SpriteSourceEditor::SetPreviewAnimation(bool animate)
{
  mPreview->mAnimating = animate;
}

int SpriteSourceEditor::GetCurrentFrame()
{
  return mPreview->GetCurrentFrame();
}

void SpriteSourceEditor::SetCurrentFrame(int frameIndex)
{
  mPreview->SetCurrentFrame(frameIndex);
}

void SpriteSourceEditor::SetFrameRate(float frameRate)
{
  if(frameRate > 0.001f)
  {
    mFrameRate = frameRate;
    mPreview->mFramesPerSecond = frameRate;
  }
}

void SpriteSourceEditor::SetSampling(SpriteSampling::Enum sampling)
{
  mSampling = sampling;
  UpdatePreview();
}

void SpriteSourceEditor::SetOrigin(SpriteOrigin::Enum newOrigin)
{
  mOrigin = newOrigin;
  Vec2 origin = ComputeOrigin(newOrigin, mFrameSizeX, mFrameSizeY);
  mOriginX = origin.x;
  mOriginY = origin.y;
}

void SpriteSourceEditor::SetTileSelection(DataIndex index)
{
  mTileView->GetSelection()->SelectNone(false);
  mTileView->GetSelection()->Select(index);
}

DataIndex SpriteSourceEditor::GetSelectedDataIndex()
{
  Array<DataIndex> selectionArray;
  mTileView->GetSelection()->GetSelected(selectionArray);
  if (selectionArray.Empty())
    return DataIndex(0);

  return selectionArray.Front();
}

SpriteFrame* SpriteSourceEditor::GetSelectedFrame()
{
  return (SpriteFrame*)mSpriteData.ToEntry(GetSelectedDataIndex());
}

void SpriteSourceEditor::RemoveSelectedFrame()
{
  SpriteFrame* selectFrame = GetSelectedFrame();
  if(selectFrame)
  {
    size_t lastSelectedIndex = selectFrame->mFrameIndex;
    // remove the sprite from the data source and refresh the tile view
    mSpriteData.RemoveSpriteFrame(selectFrame);
    RefreshTileView();
    // select the last index we were on or the end if we removed the last frame
    DataEntry* entry = mSpriteData.GetSpriteFrame(lastSelectedIndex);
    SetTileSelection(mSpriteData.ToIndex(entry));
    // update the animation preview so we are no longer referencing a freed pointer (the sprite frame)
    UpdatePreview();
  }
}

void SpriteSourceEditor::ChangedSelectedFrame(int direction)
{
  if(direction < 0)
    direction = mSpriteData.Size() - 1;

  SpriteFrame* selectFrame = GetSelectedFrame();
  if(selectFrame)
    SetTileSelection(mSpriteData.ToIndex(selectFrame));
}

void SpriteSourceEditor::RefreshTileView()
{
  mTileView->SetDataSource(&mSpriteData);
  MarkAsNeedsUpdate();
}

void SpriteSourceEditor::MoveFrame(int direction)
{
  SpriteFrame* selectFrame = GetSelectedFrame();
  if(selectFrame)
  {
    int nextLocation = selectFrame->mFrameIndex + direction;
    if(nextLocation < 0)
      return;
    if(nextLocation >= int(mSpriteData.Size()))
      return;

    SpriteFrame* nextFrame = mSpriteData.GetSpriteFrame(nextLocation);
    Math::Swap(selectFrame->mFrameIndex, nextFrame->mFrameIndex);
    Sort(mSpriteData.All(), SpriteFrameSort);

    RefreshTileView();
    // move the selection with the tile
    SetTileSelection(selectFrame->mFrameIndex);
    UpdatePreview();
  }
}

void SpriteSourceEditor::SetPreviewFrame(DataIndex frameDataIndex)
{
  SpriteFrame* spriteFrame = (SpriteFrame*)mSpriteData.ToEntry(frameDataIndex);

  // When a frame is select stop
  // animation and move the preview to that frame
  mPreview->mAnimating = false;
  mPreview->mCurrentFrame = spriteFrame->mFrameIndex;
  mPreview->UpdateFrame();
}

void SpriteSourceEditor::ConvertToSpriteSheet(Image& output)
{
  SpriteFrameLayout frameLayout(mSpriteData.Size(), mFrameSizeX, mFrameSizeY);
  output.Allocate(frameLayout.TotalSize.SizeX, frameLayout.TotalSize.SizeY);
  output.ClearColorTo(0);

  forRange(SpriteFrame* frame, mSpriteData.All())
  {
    PixelRect frameRect = frameLayout.GetFrame(frame->mFrameIndex);
    CopyImage(&output, &frame->mFrameImage, frameRect.X, frameRect.Y, 0,0, mFrameSizeX, mFrameSizeY);
  }
}

void SpriteSourceEditor::OnDoubleClickFrame(MouseEvent* event)
{
  //Edit the frame on double click
  SpriteFrame* spriteFrame = GetSpriteFrame(event->Source);
  EditFrameImage(spriteFrame->mFrameIndex);
}

void SpriteSourceEditor::OnSaveToSprite(Event* event)
{
  SaveToSpriteSource();
  CloseTabContaining(this);
}

void SpriteSourceEditor::OnAddFrame(Event* event)
{
  if(!mSpriteData.Size())
    return;
  SpriteFrame* frameToCopy = GetSelectedFrame();

  if(frameToCopy == NULL)
    frameToCopy = mSpriteData.Back();

  // Create the frame
  CreateSpriteFrame(mSpriteData.Size(), frameToCopy->mFrameImage, frameToCopy->mFrameRect);

  UpdatePreview();
}

void SpriteSourceEditor::OnMoveFrameUp(Event* event)
{
  MoveFrame(-1);
}

void SpriteSourceEditor::OnMoveFrameDown(Event* event)
{
  MoveFrame(1);
}

void SpriteSourceEditor::OnExportAllFrames(Event* event)
{
  SpriteSource* spriteSource = mSpriteSource;
  
  // save out each sprite frame as its own image
  forRange(SpriteFrame* spriteFrame, mSpriteData.All())
  {
    // Copy frame into image
    PixelRect frameRect = spriteFrame->mFrameRect;
    Image buffer;
    buffer.Allocate(frameRect.SizeX, frameRect.SizeY);
    CopyImage(&buffer, &spriteFrame->mFrameImage, 0, 0, frameRect.X, frameRect.Y, frameRect.SizeX, frameRect.SizeY);

    // Save it to a file in temp
    String name = BuildString(spriteSource->Name, "_", ToString(spriteFrame->mFrameIndex), ".png");
    String fullPath = FilePath::Combine(mEditDirectory, name);
    Status status;
    SaveToPng(status, &buffer, fullPath);
  }

  // Open the directory we saved the sprite frames into
  Os::SystemOpenFile(mEditDirectory.c_str(), Os::Verb::Open);
}

void SpriteSourceEditor::OnSpriteSourceRemoved(ResourceEvent* event)
{
  //if the sprite source that was removed is the one we're editing then close this window
  //(set active to false so we don't get 1 extra frame of logic running)
  if(event->EventResource == mSpriteSource)
  {
    SetActive(false);
    CloseTabContaining(this);
  }
}

void SpriteSourceEditor::OnEditFrame(Event* event)
{
  EditFrameImage(GetSelectedDataIndex());
}

void SpriteSourceEditor::OnKeyDown(KeyboardEvent* event)
{
  // Keyboard shortcuts
  if(event->CtrlPressed)
  {
    if(event->Key == Keys::Right)
      MoveFrame(1);
    if(event->Key == Keys::Left)
      MoveFrame(-1);
  }
  else
  {
    if(event->Key == Keys::Right)
      ChangedSelectedFrame(1);
    if(event->Key == Keys::Left)
      ChangedSelectedFrame(-1);
  }

  if(event->Key == Keys::Delete)
  {
    RemoveSelectedFrame();
  }
}

void SpriteSourceEditor::OnRemoveFrame(Event* event)
{
  RemoveSelectedFrame();
}

void SpriteSourceEditor::OnAddFrameFiles(Event* event)
{

}

void SpriteSourceEditor::OnClosePressed(Event* event)
{
  CloseTabContaining(this);
}

void SpriteSourceEditor::OnEditSpriteSheet(Event* event)
{
  // Save all the frame out as a png
  SpriteSource* spriteSource = mSpriteSource;

  // Load up all sprites
  Image output;
  ConvertToSpriteSheet(output);

  // Save it to a file in temp
  String name = BuildString(spriteSource->Name, ".png");
  String fullPath = FilePath::Combine(mEditDirectory, name);
  Status status;
  SaveToPng(status, &output, fullPath);
  Os::SystemOpenFile(fullPath.c_str(), Os::Verb::Edit);

  // Track edits
  mSheetEdit = name;
}

void SpriteSourceEditor::OnConvertToAnimation(Event* event)
{
  SpriteSheetImport(mSpriteSource);
  CloseTabContaining(this);
}

void SpriteSourceEditor::OnImportFrames(Event* event)
{

}

void SpriteSourceEditor::OnSelection(DataEvent* event)
{
  SetPreviewFrame(event->Index);
}

void SpriteSourceEditor::OnSelectionActivated(DataEvent* event)
{
  EditFrameImage(event->Index);
}

bool ValidateImage(PixelRect expectedSize, Image& newImage)
{
  if(expectedSize.SizeY != newImage.Height ||
    expectedSize.SizeX != newImage.Width)
  {
    DoNotifyWarning("Reload Error", "Sprites can not be resized externally");
    return false;
  }
  return true;
}

void SpriteSourceEditor::OnFileModified(FileEditEvent* event)
{
  SpriteFrame* frameEdited = mEditFrames.FindValue(event->FileName, NULL);
  if(frameEdited != NULL)
  {
    //Load the image
    String fullPath = FilePath::Combine(mEditDirectory, event->FileName);
    Image newImage;
    Status status;
    LoadFromPng(status, &newImage, fullPath);

    // Get the frame size
    PixelRect oldFrameRect = frameEdited->mFrameRect;

    // Prevent external edit
    if(!ValidateImage(oldFrameRect, newImage))
      return;

    // Reload frame
    frameEdited->AllocateFrame(frameEdited->mFrameIndex, newImage, frameEdited->mFrameRect);

    UpdatePreview();
  }


  // Check if this is a file for the whole sprite animation/sheet
  if(mSheetEdit == event->FileName)
  {

    // Load the image
    String fullPath = FilePath::Combine(mEditDirectory, event->FileName);
    Status status;
    Image newImage;
    LoadFromPng(status, &newImage, fullPath);

    // Recalculate how big it should be
    SpriteFrameLayout frameLayout(mSpriteData.Size(), mFrameSizeX, mFrameSizeY);
    PixelRect sheetRect = frameLayout.TotalSize;

    // Prevent external resize
    if(!ValidateImage(sheetRect, newImage))
      return;

    //Reload all frames
    LoadFramesFromSheet(newImage, mSpriteData.Size());

    UpdatePreview();
  }

  // Refresh out tile view with the new data
  RefreshTileView();
}

void SpriteSourceEditor::EditFrameImage(DataIndex frameIndex)
{
  // Save the frame input the temp directory
  // When the file is changed this frame will be reloaded
  InvalidateEdits();

  SpriteSource* spriteSource = mSpriteSource;

  if(spriteSource == NULL)
    return;

  SpriteFrame* spriteFrame = (SpriteFrame*)mSpriteData.ToEntry(frameIndex);

  // Copy frame into image
  PixelRect frameRect = spriteFrame->mFrameRect;
  Image buffer;
  buffer.Allocate(frameRect.SizeX, frameRect.SizeY);
  CopyImage(&buffer, &spriteFrame->mFrameImage, 0, 0, frameRect.X, frameRect.Y, frameRect.SizeX, frameRect.SizeY);

  // Generate a unique name for this sprite frame in the
  // form of ResourceId_Frame_#
  String idString = ToString(spriteSource->mResourceId);
  String tempFile = String::Format("%s_Frame%d.png", idString.c_str(), spriteFrame->mFrameIndex);
  String fullPath = FilePath::Combine(mEditDirectory,  tempFile);

  // Save this file to a png file
  Status status;
  SaveToPng(status, &buffer, fullPath.c_str());

  // Tell the Os to edit this type of file
  Os::SystemOpenFile(fullPath.c_str(), Os::Verb::Edit);

  // Track that this file/frame is out for editing
  mEditFrames.Insert(tempFile, spriteFrame);
}

void SpriteSourceEditor::InvalidateEdits()
{
  //Clear all other frame edits
  mEditFrames.Clear();
  mSheetEdit.Clear();
}

void SpriteSourceEditor::OnMouseDown(MouseEvent* event)
{
  Array<DataIndex> selectionArray;
  mTileView->GetSelection()->GetSelected(selectionArray);
  if (selectionArray.Empty())
    return;

  SetPreviewFrame(selectionArray.Front());
}

void SpriteSourceEditor::ClearFrames()
{
  forRange(SpriteFrame* frame, mSpriteData.All())
    delete frame;
  mSpriteData.ClearFrames();
  mEditFrames.Clear();
}

void SpriteSourceEditor::UpdatePreview()
{
  // Update all the frames on the preview sprite
  mPreview->mFrames.Clear();
  TextureFiltering::Enum filtering = mSampling == SpriteSampling::Nearest ? TextureFiltering::Nearest : TextureFiltering::Bilinear;
  for(uint i=0;i<mSpriteData.Size();++i)
  {
    mSpriteData[i]->mFrameTexture->mFiltering = filtering;
    Z::gEngine->has(GraphicsEngine)->AddTexture(mSpriteData[i]->mFrameTexture);
    TextureArea area = { mSpriteData[i]->mTexRect, mSpriteData[i]->mFrameTexture};
    mPreview->mFrames.PushBack(area);
  }
}

void SpriteSourceEditor::CreateSpriteFrame(uint frameIndex, Image& image, PixelRect rect)
{
  // Create a sprite frame
  SpriteFrame* frame = new SpriteFrame();
  frame->AllocateFrame(frameIndex, image, rect);
  mSpriteData.AddSpriteFrame(frame);
  mTileView->MarkAsNeedsUpdate();
}

void SpriteSourceEditor::LoadFramesFromSheet(Image& sourceImage, uint frameCount)
{
  mSpriteData.ClearFrames();

  SpriteFrameLayout frameLayout(frameCount, mFrameSizeX, mFrameSizeY, sourceImage.Width, sourceImage.Height);

  for(uint i=0;i<frameCount;++i)
  {
    PixelRect rect = frameLayout.GetFrame(i);
    CreateSpriteFrame(i, sourceImage, rect);
  }

  UpdatePreview();
}

void SpriteSourceEditor::SaveToSpriteSource()
{
  InvalidateEdits();

  SpriteSource* spriteSource = mSpriteSource;
  if(spriteSource == NULL)
    return;
  
  // Check for resource not Writable
  if (!spriteSource->IsWritable())
  {
    DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig);
    if (devConfig == NULL || !devConfig->mCanModifyReadOnlyResources)
    {
      DoNotifyWarning("Resource is not writable", "Resource is a protected or built in resource.");
      return;
    }
  }

  // Convert all frame to a single sprite sheet image
  Image output;
  ConvertToSpriteSheet(output);

  // Get the output file
  String sourceFile = spriteSource->mContentItem->GetFullPath();

  // Overwrite the file
  Status status;
  SaveToPng(status, &output, sourceFile);

  // Update data
  spriteSource->FrameCount = mSpriteData.Size();
  spriteSource->FrameSizeX = mFrameSizeX;
  spriteSource->FrameSizeY = mFrameSizeY;
  spriteSource->OriginX = mOriginX;
  spriteSource->OriginY = mOriginY;
  spriteSource->Looping = mLooping;
  spriteSource->FrameDelay = 1.0f / mFrameRate;
  spriteSource->PixelsPerUnit = mPixelsPerUnit;
  spriteSource->Sampling = mSampling;

  spriteSource->Slices[NineSlices::Left] = float(mLeft);
  spriteSource->Slices[NineSlices::Top] = float(mTop);
  spriteSource->Slices[NineSlices::Right] = float(mRight);
  spriteSource->Slices[NineSlices::Bottom] = float(mBottom);

  spriteSource->Fill = mSpriteFill;

  if(spriteSource->mAtlas != mAtlas)
    spriteSource->SetAtlas(mAtlas);

  // Check to see if the resource was renamed
  if(spriteSource->Name != mSpriteName)
    RenameResource(spriteSource, mSpriteName);

  // Save to context so updated data is saved
  spriteSource->mContentItem->SaveContent();

  // Swap in the new image
  spriteSource->SourceImage.Swap(&output);

  // Rebuild the sprite sheet since it may have changed size
  spriteSource->mAtlas->NeedsBuilding = true;
  SpriteSourceManager::GetInstance()->RebuildSpriteSheets();

  CloseTabContaining(this);
}

void SpriteSourceEditor::EditSpriteSource(SpriteSource* spriteSource)
{
  mSpriteSource = spriteSource;

  // Load data
  mSpriteName = spriteSource->Name;
  mLooping = spriteSource->Looping;
  mFrameSizeX = spriteSource->FrameSizeX;
  mFrameSizeY = spriteSource->FrameSizeY;
  mOriginX = spriteSource->OriginX;
  mOriginY = spriteSource->OriginY;
  mFrameRate = 1.0f / spriteSource->FrameDelay;
  mPixelsPerUnit = spriteSource->PixelsPerUnit;
  mAtlas = spriteSource->mAtlas;
  mSampling = spriteSource->Sampling;

  mLeft = int(spriteSource->Slices[NineSlices::Left]);
  mTop = int(spriteSource->Slices[NineSlices::Top]);
  mRight = int(spriteSource->Slices[NineSlices::Right]);
  mBottom = int(spriteSource->Slices[NineSlices::Bottom]);
  mSpriteFill = (SpriteFill::Enum)spriteSource->Fill;

  uint frameCount = spriteSource->FrameCount;

  mOrigin = ComputeOrigin(Vec2(mOriginX, mOriginY), mFrameSizeX, mFrameSizeY);

  LoadFramesFromSheet(spriteSource->SourceImage, frameCount);

  mSpriteProperties->SetObject(this);

  SetSampling(mSampling);

  mPreview->mFramesPerSecond = 1.0f / spriteSource->FrameDelay;
}

void EditSprite(SpriteSource* spriteSource)
{
  Window* window = new Window(Z::gEditor);
  SpriteSourceEditor* editor = new SpriteSourceEditor(window);
  window->SetTitle("Sprite Source Editor");
  window->SetSize(Pixels(800, 600));
  Vec3 offsetCenter = GetCenterPosition(Z::gEditor, window);
  window->SetTranslationAndSize(offsetCenter + Pixels(0, -1000, 0), Pixels(800, 600));
  editor->EditSpriteSource(spriteSource);
  CenterToWindow(Z::gEditor, window, true);
}

}
