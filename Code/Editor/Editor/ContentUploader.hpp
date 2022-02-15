// MIT Licensed (see LICENSE.md).
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

class ContentExportTile : public TileViewWidget
{
public:
  typedef ContentExportTile ZilchSelf;
  ContentExportTile(Composite* parent,
                    TileView* tileView,
                    PreviewWidget* tileWidget,
                    DataIndex dataIndex,
                    ContentPackageExporter* exporter,
                    ContentItem* contentItem);

  /// Widget Interface.
  void UpdateTransform() override;

  // TileViewWidget Event Handlers
  void OnMouseHover(MouseEvent* event) override;

  /// Checks the resource for any dependent resources.
  void CheckForDependencies();

private:
  void GetMissingDependencies(HashSet<ContentItem*>& resources);
  void OnRightClick(MouseEvent* e);
  void OnAddDependencies(Event* e);
  void OnRemove(Event* e);

  Element* mMissingTextBackground;
  Text* mMissingText;
  ContentPackageExporter* mExporter;
  ContentItem* mContentItem;
  bool mMissingDependencies;
};

class ContentExporterTileView : public TileView
{
public:
  typedef ContentExporterTileView ZilchSelf;

  ContentExporterTileView(ContentPackageExporter* parent);
  TileViewWidget* CreateTileViewWidget(Composite* parent,
                                       StringParam name,
                                       HandleParam instance,
                                       DataIndex index,
                                       PreviewImportance::Enum minImportance = PreviewImportance::None) override;

  void OnMetaDrop(MetaDropEvent* e);

  ContentPackageExporter* mExporter;
};

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
  void OnKeyDown(KeyboardEvent* e);
  void RemoveSelectedItems(Event*);

  friend class ContentExportTile;
  friend class ContentExporterTileView;
  DataSource* mSource;
  ContentExporterTileView* mTileView;
  PropertyView* mPropertyView;

  Array<ContentItem*> mContentItems;
  ContentPackage mTempPackage;

  /// When there are no objects in the tile view, this will give them a hint
  /// to drag in items from the library view.
  Text* mHintText;
};

} // namespace Zero
