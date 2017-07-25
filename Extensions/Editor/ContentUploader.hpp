///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentUploader.hpp
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
class MetaDropEvent;
class ContentPackageExporter;

//--------------------------------------------------------- ContentExportTile
class ContentExportTile : public TileViewWidget
{
public:
  typedef ContentExportTile ZilchSelf;
  ContentExportTile(Composite* parent, TileView* tileView,
                    PreviewWidget* tileWidget, DataIndex dataIndex,
                    ContentPackageExporter* exporter, ContentItem* contentItem);

  /// Widget Interface.
  void UpdateTransform() override;
  
  //TileViewWidget Event Handlers
  void OnMouseHover(MouseEvent* event) override;

  /// Checks the resource for any dependent resources.
  void CheckForDependencies();

private:
  void GetMissingDependencies(HashSet<ContentItem*>& resources);
  void OnRightClick(MouseEvent* e);
  void OnKeyDown(KeyboardEvent* e);
  void OnAddDependencies(Event* e);
  void OnRemove(Event* e);

  void RemoveContentItem();

  Text* mMissingText;
  ContentPackageExporter* mExporter;
  ContentItem* mContentItem;
  bool mMissingDependencies;
};

//---------------------------------------------------- Content Package Tile View
class ContentExporterTileView : public TileView
{
public:
  typedef ContentExporterTileView ZilchSelf;

  ContentExporterTileView(ContentPackageExporter* parent);
  TileViewWidget* CreateTileViewWidget(Composite* parent, StringParam name,
                                       HandleParam instance, DataIndex index,
                          PreviewImportance::Enum minImportance = PreviewImportance::None) override;

  void OnMetaDrop(MetaDropEvent* e);

  ContentPackageExporter* mExporter;
};

//---------------------------------------------------------- Content Upload View
class ContentPackageExporter : public Composite
{
public:
  typedef ContentPackageExporter ZilchSelf;
  ContentPackageExporter(Composite* parent);
  ~ContentPackageExporter();

  /// Widget interface.
  void UpdateTransform() override;

private:
  void OnExportPressed(Event* e);
  void OnExportFileSelected(OsFileSelection* e);
  void ExportPackageFile(StringParam filename);
  void RefreshTileView();

  friend class ContentExportTile;
  friend class ContentExporterTileView;
  DataSource* mSource;
  ContentExporterTileView* mTileView;
  PropertyView* mPropertyView;

  Array<ContentItem*> mContentItems;
  ThreadedWebRequest mContentListRequest;
  ContentPackage mTempPackage;

  /// When there are no objects in the tile view, this will give them a hint
  /// to drag in items from the library view.
  Text* mHintText;
};

}//namespace Zero
