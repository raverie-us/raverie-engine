// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace UploaderUi
{
const cstr cLocation = "EditorUi/ContentStore/Uploader";
Tweakable(Vec4, MissingColor, Vec4(1, 1, 1, 1), cLocation);
} // namespace UploaderUi

class ContentUploadSource : public DataSource
{
public:
  ContentItem* mRoot;
  Array<ContentItem*>& mEntries;

  ContentUploadSource(Array<ContentItem*>& resources) : mEntries(resources)
  {
    mRoot = (ContentItem*)0x123456;
  }

  DataEntry* GetRoot() override
  {
    return mRoot;
  }

  DataEntry* ToEntry(DataIndex index) override
  {
    if (index == cRootIndex || (uint)index.Id >= mEntries.Size())
      return mRoot;
    return mEntries[(uint)index.Id];
  }

  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    if (dataEntry == mRoot)
      return cRootIndex;
    ContentItem* entry = (ContentItem*)dataEntry;
    uint index = mEntries.FindIndex(entry);
    return DataIndex(index);
  }

  Handle ToHandle(DataEntry* dataEntry) override
  {
    return (ContentItem*)dataEntry;
  }

  DataEntry* Parent(DataEntry* dataEntry) override
  {
    if (dataEntry == mRoot)
      return nullptr;
    return &mRoot;
  }

  uint ChildCount(DataEntry* dataEntry) override
  {
    if (dataEntry == mRoot)
      return mEntries.Size();
    return 0;
  }

  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    if (dataEntry == mRoot)
      return mEntries[index];
    return nullptr;
  }

  bool IsExpandable(DataEntry* dataEntry) override
  {
    return false;
  }

  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    ContentItem* contentItem = (ContentItem*)dataEntry;
    if (column == CommonColumns::Name)
    {
      String name = FilePath::GetFileNameWithoutExtension(contentItem->Filename);
      variant = name;
    }
  }

  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    return false;
  }
};

ContentExportTile::ContentExportTile(Composite* parent,
                                     TileView* tileView,
                                     PreviewWidget* tileWidget,
                                     DataIndex dataIndex,
                                     ContentPackageExporter* uploadView,
                                     ContentItem* contentItem) :
    TileViewWidget(parent, tileView, tileWidget, dataIndex)
{
  mExporter = uploadView;
  mContentItem = contentItem;
  CheckForDependencies();

  if (mMissingDependencies)
  {
    mMissingTextBackground = CreateAttached<Element>(cWhiteSquare);
    mMissingTextBackground->SetColor(UploaderUi::MissingColor);
    mMissingText = new Text(this, cText);
    mMissingText->SetText("Missing Dependencies");
  }

  ConnectThisTo(this, Events::RightClick, OnRightClick);
}

void ContentExportTile::UpdateTransform()
{
  // If there are missing dependencies place the text for it below the tiles
  // title bar
  if (mMissingDependencies)
  {
    Vec2 backgroundSize = Vec2(mSize.x, cTileViewNameOffset);
    mMissingTextBackground->SetSize(backgroundSize);

    mMissingText->SizeToContents();
    // Y offset by the titlebar's height to place below it
    Vec2 textbarOffset = Vec2(0, cTileViewNameOffset);
    WidgetRect textRect = WidgetRect::PointAndSize(textbarOffset, backgroundSize);
    // Place both the text and its background below the titlebar
    PlaceCenterToRect(textRect, mMissingTextBackground);
    PlaceCenterToRect(textRect, mMissingText);

    mMissingText->mTranslation.x = Math::Max(mMissingText->mTranslation.x, 0.0f);
    mMissingText->mSize.x = Math::Min(mMissingText->mSize.x, mSize.x);
  }

  TileViewWidget::UpdateTransform();
}

void ContentExportTile::OnMouseHover(MouseEvent* event)
{
  // do nothing, we don't want the popup as it interferes with the remove pop up
  // when attempting to remove an item or add a dependency
}

void ContentExportTile::CheckForDependencies()
{
  // Store the old state so we don't have an unneeded update transform
  bool oldState = mMissingDependencies;

  HashSet<ContentItem*> missingDependencies;
  GetMissingDependencies(missingDependencies);

  mMissingDependencies = !missingDependencies.Empty();

  if (mMissingDependencies != oldState)
    MarkAsNeedsUpdate();
}

void ContentExportTile::GetMissingDependencies(HashSet<ContentItem*>& missingDependencies)
{
  // Build the listing of all resources made by this content item
  ResourceListing listing;
  mContentItem->BuildListing(listing);

  // All dependent resources
  Handle instance = GetEditObject();

  // Get the dependencies from each resource
  forRange (ResourceEntry& entry, listing.All())
  {
    Resource* resource = Z::gResources->GetResource(entry.mResourceId);
    resource->GetDependencies(missingDependencies, instance);
  }

  // Check for core resources in our dependencies list
  HashSet<ContentItem*> toRemove;
  forRange (ContentItem* item, missingDependencies)
  {
    if (item->mLibrary->GetReadOnly())
      toRemove.Insert(item);
  }

  // Remove core resources from the dependencies list as they don't need to be
  // exported
  forRange (ContentItem* item, toRemove)
  {
    missingDependencies.Erase(item);
  }

  forRange (ContentItem* dependency, missingDependencies.All())
  {
    // If it already contains the content item, we can remove it from the list
    if (mExporter->mContentItems.Contains(dependency))
      toRemove.Insert(dependency);
  }

  // Remove the dependencies already listed in the content items
  forRange (ContentItem* item, toRemove)
  {
    missingDependencies.Erase(item);
  }
}

void ContentExportTile::OnRightClick(MouseEvent* e)
{
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0, 0));

  if (mMissingDependencies)
    ConnectMenu(menu, "Add Dependencies", OnAddDependencies, false);

  ConnectMenu(menu, "Remove", OnRemove, false);
}

void ContentExportTile::OnAddDependencies(Event* e)
{
  HashSet<ContentItem*> missingDependencies;
  GetMissingDependencies(missingDependencies);

  forRange (ContentItem* dependency, missingDependencies.All())
  {
    mExporter->mContentItems.PushBack(dependency);
  }

  CheckForDependencies();

  // Update the tile view display to show our dependency is no longer missing
  mExporter->RefreshTileView();
}

void ContentExportTile::OnRemove(Event* e)
{
  mParent->DispatchBubble("RemoveSelectedItems", e);
}

ContentExporterTileView::ContentExporterTileView(ContentPackageExporter* parent) : TileView(parent)
{
  ConnectThisTo(this, Events::MetaDrop, OnMetaDrop);
  ConnectThisTo(this, Events::MetaDropTest, OnMetaDrop);
  mExporter = parent;
}

Resource* GetFirstResource(ContentItem* contentItem)
{
  // Build the listing of all resources made by this content item
  ResourceListing listing;
  contentItem->BuildListing(listing);

  if (listing.Empty())
    return nullptr;

  ResourceEntry& entry = listing.Front();
  return Z::gResources->GetResource(entry.mResourceId);
}

TileViewWidget* ContentExporterTileView::CreateTileViewWidget(
    Composite* parent, StringParam name, HandleParam instance, DataIndex index, PreviewImportance::Enum minImportance)
{
  // We need to pull out a resource from the content item to display a preview
  ContentItem* contentItem = instance.Get<ContentItem*>();
  Resource* firstResource = GetFirstResource(contentItem);

  PreviewWidget* tileWidget = ResourcePreview::CreatePreviewWidget(parent, name, firstResource, minImportance);
  if (tileWidget == nullptr)
    return nullptr;

  return new ContentExportTile(parent, this, tileWidget, index, mExporter, contentItem);
}

void ContentExporterTileView::OnMetaDrop(MetaDropEvent* e)
{
  if (e->Testing)
  {
    if (Resource* resource = e->Instance.Get<Resource*>())
    {
      if (mExporter->mContentItems.Contains(resource->mContentItem))
        e->Result = "Content item already added to package";
      else
        e->Result = "Add to content package";
    }
  }
  else
  {
    if (Resource* resource = e->Instance.Get<Resource*>())
    {
      if (mExporter->mContentItems.Contains(resource->mContentItem))
        return;

      mExporter->mContentItems.PushBack(resource->mContentItem);

      mExporter->RefreshTileView();
      mExporter->UpdateTransformExternal();
      mExporter->mHintText->SetActive(false);
    }
  }
}

ContentPackageExporter::ContentPackageExporter(Composite* parent) : Composite(parent)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(4, 0, 4, 0)));
  this->SetMinSize(Pixels(800, 600));

  mTileView = new ContentExporterTileView(this);
  mTileView->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  mSource = new ContentUploadSource(mContentItems);
  mTileView->SetDataSource(mSource);
  // This is the smallest size that makes the entire missing dependency text
  // visible
  mTileView->SetItemSize(118.f);

  Splitter* splitter = new Splitter(this);
  splitter->SetInteractive(false);

  Composite* right = new Composite(this);
  right->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(4, 0, 4, 0)));

  mPropertyView = new PropertyView(right);
  mPropertyView->SetObject(&mTempPackage);
  mPropertyView->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(280));
  mPropertyView->Refresh();

  TextButton* button = new TextButton(right);
  button->SetText("Export Package");
  ConnectThisTo(button, Events::ButtonPressed, OnExportPressed);

  // Create the hint text
  mHintText = new Text(this, cText);
  mHintText->SetText("Drag and drop resources from the LibraryView...");
  mHintText->SizeToContents();
  // We will be centering it ourself
  mHintText->SetNotInLayout(true);
  mHintText->SetInteractive(false);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, "OnExportFileSelected", OnExportFileSelected);
  ConnectThisTo(this, "RemoveSelectedItems", RemoveSelectedItems);
}

ContentPackageExporter::~ContentPackageExporter()
{
  SafeDelete(mSource);
}

void ContentPackageExporter::UpdateTransform()
{
  // Center the hint text to the tile view
  WidgetRect tileViewRect = mTileView->GetLocalRect();
  PlaceCenterToRect(tileViewRect, mHintText);

  Composite::UpdateTransform();
}

void ContentPackageExporter::RefreshTileView()
{
  mTileView->SetDataSource(mSource);
  MarkAsNeedsUpdate();
}

void ContentPackageExporter::OnExportPressed(Event* e)
{
  // Ignore the event if we're not active
  if (!GetActive())
    return;

  if (mContentItems.Empty())
  {
    DoNotifyWarning("Cannot export content package", "No content items selected.");
    return;
  }

  FileDialogConfig* config = FileDialogConfig::Create();
  config->EventName = "OnExportFileSelected";
  config->CallbackObject = this;
  config->Title = "Select Content Package File";
  config->AddFilter("Import Pack File", "*.zeropack");
  config->StartingDirectory = GetUserDocumentsDirectory();
  String filename = "Package";
  if (!mTempPackage.mName.Empty())
    filename = mTempPackage.mName;
  config->DefaultFileName = BuildString(filename, ".zeropack");
  Z::gEngine->has(OsShell)->SaveFile(config);

  // Close the export window after exporting a content package
  CloseTabContaining(this);
}

void ContentPackageExporter::OnExportFileSelected(OsFileSelection* e)
{
  if (!e->Success)
    return;

  String packageFile = e->Files[0];

  ContentPackageListing listing;

  // Add all the content items to the listing
  forRange (ContentItem* item, mContentItems.All())
  {
    listing.Location = item->mLibrary->SourcePath;
    String filename = item->GetFullPath();

    ContentPackageEntry* entry = new ContentPackageEntry();
    entry->Active = true;
    entry->File = item->Filename;
    entry->Size = GetFileSize(filename);
    entry->Conflicted = false;
    listing.AddEntry(entry);
  }

  // Write out the package file
  ExportContentPackageListing(listing, packageFile);
  Download(packageFile);

  // If info provided write that out
  if (!mTempPackage.mName.Empty())
  {
    mTempPackage.mDate = GetDate();
    mTempPackage.mSize = GetFileSize(packageFile);
    mTempPackage.mVersionBuilt = GetRevisionNumber();

    // Write out the meta file
    String metaFile = BuildString(packageFile, ".meta");
    Download(metaFile);
    TextSaver saver;
    Status status;
    saver.Open(status, metaFile.c_str());
    if (!status.Failed())
    {
      saver.StartPolymorphic("ContentPackage");
      mTempPackage.Serialize(saver);
      saver.EndPolymorphic();
    }
    saver.Close();
  }
}

void ContentPackageExporter::OnKeyDown(KeyboardEvent* event)
{
  if (event->Key == Keys::Delete)
    RemoveSelectedItems(nullptr);
}

void ContentPackageExporter::RemoveSelectedItems(Event*)
{
  // Remove all currently selected content items in the tile view
  Array<DataIndex> selected;
  DataSelection* selection = mTileView->GetSelection();
  selection->GetSelected(selected);

  Array<ContentItem*> toRemove;
  forRange (DataIndex& index, selected)
  {
    ContentItem* item = (ContentItem*)mSource->ToEntry(index);
    toRemove.PushBack(item);
  }

  forRange (ContentItem* content, toRemove.All())
  {
    mContentItems.EraseValue(content);
  }

  // Clear our selection and update the tile view
  selection->SelectNone();
  RefreshTileView();
}

} // namespace Zero
