///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentStore.hpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class ContentPackage;
class TagChainTextBox;
class TileView;
class Composite;
class ContentPackageExporter;

//--------------------------------------------------------- Content Package Tile
class ContentPackageTile : public TileViewWidget
{
public:

  ContentPackage* mContentPackage;
};

//---------------------------------------------------- Content Package Tile View
class ContentPackageTileView : public TileView
{
public:
  ContentPackageTileView(Composite* parent) : TileView(parent) {}
  TileViewWidget* CreateTileViewWidget(Composite* parent,
      StringParam name, HandleParam instance, DataIndex index,
      PreviewImportance::Enum minImportance = PreviewImportance::None) override;
};

//------------------------------------------------------ Content Package Preview
class ContentPackagePreview : public Composite
{
public:
  typedef ContentPackagePreview ZilchSelf;
  ContentPackagePreview(Composite* parent);
  ~ContentPackagePreview();

  /// Widget Interface.
  void UpdateTransform() override;

  void ViewContentPackage(ContentPackage* contentPackage);
  void OnPreviewPressed(Event* e);

  Composite* mPreviewArea;
  PreviewWidget* mPreview;
  Text* mName;
  Text* mAuthor;
  Text* mSize;
  Text* mDate;
  Text* mVersionBuilt;
  Text* mDescription;

  TextButton* mPreviewButton;

  HandleOf<ToolTip> mToolTip;

  ContentPackage* mContentPackage;
};

//---------------------------------------------------------- Content Import View
class ContentImportView : public Composite
{
public:
  typedef ContentImportView ZilchSelf;
  ContentImportView(Composite* parent);
  ~ContentImportView();

  void SetImportButton(TextButton* button);

private:
  friend class ContentStoreSource;
  void OnWebResponse(WebResponseEvent* e);
  void LoadContentPackages(StringParam fileName);
  void OnDataSelectionFinal(Event* e);
  void OnImportPressed(Event* e);

  void LoadLocalContentPackages();

  ContentPackage* GetContentPackage(StringParam packageName);

  DataSource* mSource;
  ContentPackageTileView* mTileView;
  ContentPackagePreview* mPreview;

  HashMap<String, ContentPackage*> mContentPackagesMap;
  Array<ContentPackage*> mContentPackages;
  ThreadedWebRequest mContentListRequest;
};

//---------------------------------------------------------------- Content Store
class ContentStore : public Composite
{
public:
  typedef ContentStore ZilchSelf;

  static void Open();

  ContentStore(Composite* parent);
  ~ContentStore();

  void UpdateTransform() override;

private:
  void OnUploadPressed(Event* e);

  ContentImportView* mImportView;
  ContentPackageExporter* mExporter;

  /// The Bottom row.
  Composite* mBottomArea;
  Element* mBottomBar;
  Element* mIcon;

  /// Used for changing to the upload view, and canceling from the upload view.
  TextButton* mUploadButton;
  TextButton* mPrimaryButton;
  static ContentStore* mOpenStore;
};

}//namespace Zero
