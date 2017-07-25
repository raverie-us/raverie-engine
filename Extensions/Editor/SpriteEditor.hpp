///////////////////////////////////////////////////////////////////////////////
///
/// \file SpriteEditor.hpp
///
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

UvRect ComputeTextureRect(PixelRect pixelRect, Vec2 textureSize);

const float cMinZoom = 0.1f;
const float cMaxZoom = 20.0f;

class SpriteFrameLayout
{
public:
  SpriteFrameLayout(uint frameCount, uint frameSizeX, uint frameSizeY);
  SpriteFrameLayout(uint frameCount, uint frameSizeX, uint frameSizeY, uint sizeX, uint sizeY);

  PixelRect TotalSize;
  int FrameSizeX;
  int FrameSizeY;
  int FramesPerRow;
  int FramesPerCol;
  PixelRect GetFrame(uint frameIndex);
};

class SpriteSource;
class UpdateEvent;
class EventDirectoryWatcher;
class ScrollArea;
class PropertyView;
class KeyboardEvent;
class FileEditEvent;

//------------------------------------------------------------------------  Sprite Frame
// Display a frame of sprite
class SpriteFrame
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SpriteFrame();
  ~SpriteFrame();

  SpriteFrame(SpriteFrame& spriteFrame);

  void AllocateFrame(uint frameNumber, Image& sourceImage, PixelRect sourceRect);

  int mFrameIndex;
  UvRect mTexRect;
  PixelRect mFrameRect;
  Image mFrameImage;
  HandleOf<Texture> mFrameTexture;
};

// Helpers for Sprite Frame
inline bool SpriteFrameSort(SpriteFrame* left, SpriteFrame* right)
{
  return left->mFrameIndex < right->mFrameIndex;
}

SpriteFrame* GetSpriteFrame(Widget* object);

struct TextureArea
{
  UvRect mUvRect;
  Texture* mTexture;
};

//------------------------------------------------------------------------ Sprite Preview
// Animated Preview of a set of Frames as areas of a texture
class SpritePreview : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SpritePreview(Composite* parent);
  ~SpritePreview();

  int GetCurrentFrame(){return mCurrentFrame;}
  void SetCurrentFrame(int frame);
  void UpdateTransform() override;
  void UpdateFrame();
  void OnUpdate(UpdateEvent* updateEvent);

  // Animation Data
  bool mAnimating;
  float mFramesPerSecond;
  int mCurrentFrame;
  float mT;

  Array<TextureArea> mFrames;
  TextureView* mPreviewFrame;
  TextureView* mBackground;
};

DeclareEnum5(SpriteOrigin, Custom, Center, UpperLeft, BottomLeft, BottomCenter);
Vec2 ComputeOrigin(SpriteOrigin::Enum origin, int width, int height);
SpriteOrigin::Enum ComputeOrigin(Vec2 point, int width, int height);

//------------------------------------------------------------------------ Sprite Preview Widget
class SpritePreviewWidget : public PreviewWidget
{
public:
  typedef SpritePreviewWidget self_type;
  
  SpritePreviewWidget(SpriteFrame* spriteFrame, Composite* parent);
  void UpdateTransform();
  
  TextureView* mTextureView;
};

//------------------------------------------------------------------------ Sprite Tile View Widget
class SpriteTileViewWidget : public TileViewWidget
{
public:
  SpriteTileViewWidget(Composite* parent, TileView* tileView,
                       PreviewWidget* tileWidget, DataIndex dataIndex);
  
  //TileViewWidget Event Handlers
  void OnMouseHover(MouseEvent* event) override;
  void OnMouseClick(MouseEvent* event) override;
};

//------------------------------------------------------------------------  Sprite Frame Tile View
class SpriteFrameTileView : public TileView
{
public:
  SpriteFrameTileView(Composite* parent) : TileView(parent) {}
  TileViewWidget* CreateTileViewWidget(Composite* parent,
                                       StringParam name, HandleParam instance, DataIndex index,
                                       PreviewImportance::Enum minImportance = PreviewImportance::None) override;

  void OnMouseScroll(MouseEvent* event) override;
  void OnLeftMouseDrag(MouseDragEvent* e) override;

};

//------------------------------------------------------------------------  Sprite Data Source
class SpriteDataSource : public DataSource
{
public:
  SpriteDataSource();
  ~SpriteDataSource();

  void AddSpriteFrame(SpriteFrame* sprite);
  void RemoveSpriteFrame(SpriteFrame* sprite);
  void RemoveSpriteFrame(DataIndex frameDataIndex);
  SpriteFrame* GetSpriteFrame(size_t index);
  void ClearFrames();

  // Data access to edit underlying SpriteFrame array
  size_t Size();
  Array<SpriteFrame*>::range All();
  SpriteFrame* Back();
  SpriteFrame* operator[](size_t index);

  // Data Source Interface
  DataEntry* GetRoot() override;
  DataEntry* ToEntry(DataIndex index) override;
  DataIndex ToIndex(DataEntry* dataEntry) override;
  Handle ToHandle(DataEntry* dataEntry) override;
  DataEntry* Parent(DataEntry* dataEntry) override;
  uint ChildCount(DataEntry* dataEntry) override;
  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override;
  bool IsExpandable(DataEntry* dataEntry) override;
  void GetData(DataEntry* dataEntry,       Any& variant, StringParam column) override;
  bool SetData(DataEntry* dataEntry, const Any& variant, StringParam column) override;

  SpriteFrame mRoot; 
  Array<SpriteFrame*> mSpriteFrames;
};

//---------------------------------------------------- Sprite Source Editor
class SpriteSourceEditor : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SpriteSourceEditor(Composite* parent);
  ~SpriteSourceEditor();

  HandleOf<SpriteSource> mSpriteSource;
  TileView* mTileView;

  PropertyView* mSpriteProperties;
  SpriteDataSource mSpriteData;

  int GetLeft();
  void SetLeft(int value);

  int GetRight();
  void SetRight(int value);

  int GetTop();
  void SetTop(int value);

  int GetBottom();
  void SetBottom(int value);

  SpriteFill::Enum mSpriteFill;
  int mLeft;
  int mRight;
  int mTop;
  int mBottom;


  // Local copy of Sprite Source Settings
  String mSpriteName;
  Atlas* mAtlas;
  int mFrameSizeX;
  int mFrameSizeY;
  float mOriginX;
  float mOriginY;
  float mPixelsPerUnit;
  float mFrameRate;
  SpriteSampling::Enum mSampling;
  SpritePreview* mPreview;
  bool mLooping;
  SpriteOrigin::Enum mOrigin;

  // External Edit
  EventDirectoryWatcher* mDirectoryWatcher;
  HashMap<String, SpriteFrame*> mEditFrames;
  String mSheetEdit;
  String mEditDirectory;

  // Getters and Setters for Various Options
  String GetSpriteName(){return mSpriteName;}
  void SetSpriteName(StringParam name);

  bool GetPreviewAnimation();
  void SetPreviewAnimation(bool animate);

  int GetCurrentFrame();
  void SetCurrentFrame(int frameNumber);

  float GetFrameRate(){return mFrameRate;}
  void SetFrameRate(float frameRate);

  SpriteSampling::Enum GetSampling(){return mSampling;}
  void SetSampling(SpriteSampling::Enum sampling);

  SpriteOrigin::Enum GetOrigin(){return mOrigin;}
  void SetOrigin(SpriteOrigin::Enum newOrigin);

  // Functions
  void SetTileSelection(DataIndex frameDataIndex);
  DataIndex GetSelectedDataIndex();
  SpriteFrame* GetSelectedFrame();

  void SetPreviewFrame(DataIndex frameDataIndex);
  void ClearFrames();
  void RemoveSelectedFrame();
  void MoveFrame(int direction);
  void ChangedSelectedFrame(int direction);
  void RefreshTileView();

  void EditSpriteSource(SpriteSource* spriteSource);
  void SaveToSpriteSource();

  void ConvertToSpriteSheet(Image& output);
  void CreateSpriteFrame(uint frameNumber, Image& image, PixelRect rect);
  void UpdatePreview();
  void InvalidateEdits();

  //Frame Edit
  void EditFrameImage(DataIndex frameIndex);
  void LoadFramesFromSheet(Image& sourceImage, uint frameCount);

  //Events
  void OnDoubleClickFrame(MouseEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  void OnFileModified(FileEditEvent* event);
  void OnMouseDown(MouseEvent* event);
  void OnAddFrameFiles(Event* event);
  void OnEditSpriteSheet(Event* event);

  void OnSaveToSprite(Event* event);
  void OnClosePressed(Event* event);
  void OnConvertToAnimation(Event* event);
  void OnImportFrames(Event* event);

  void OnSelection(DataEvent* event);
  void OnSelectionActivated(DataEvent* event);

  void OnAddFrame(Event* event);
  void OnEditFrame(Event* event);
  void OnRemoveFrame(Event* event);
  void OnMoveFrameUp(Event* event);
  void OnMoveFrameDown(Event* event);
  // Exports each frame as an individual image into a temp folder
  void OnExportAllFrames(Event* event);

  void OnSpriteSourceRemoved(ResourceEvent* event);
};

void EditSprite(SpriteSource* spriteSource);

}
