///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentUploader.cpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace UploaderUi
{
const cstr cLocation = "EditorUi/ContentStore/Uploader";
Tweakable(Vec4, MissingColor, Vec4(1,1,1,1), cLocation);
}

//---------------------------------------------------- Content Store Data Source
class ContentUploadSource : public DataSource
{
public:
  ContentItem* mRoot;
  Array<ContentItem*>& mEntries;

  //****************************************************************************
  ContentUploadSource(Array<ContentItem*>& resources)
    : mEntries(resources)
  {
    mRoot = (ContentItem*)0x123456;
  }

  //****************************************************************************
  DataEntry* GetRoot() override
  {
    return mRoot;
  }

  //****************************************************************************
  DataEntry* ToEntry(DataIndex index) override
  {
    if(index == cRootIndex || (uint)index.Id >= mEntries.Size())
      return mRoot;
    return mEntries[(uint)index.Id];
  }

  //****************************************************************************
  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    if(dataEntry == mRoot)
      return cRootIndex;
    ContentItem* entry = (ContentItem*)dataEntry;
    uint index = mEntries.FindIndex(entry);
    return DataIndex(index);
  }

  //****************************************************************************
  Handle ToHandle(DataEntry* dataEntry) override
  {
    return (ContentItem*)dataEntry;
  }
  
  //****************************************************************************
  DataEntry* Parent(DataEntry* dataEntry) override
  {
    if(dataEntry == mRoot)
      return nullptr;
    return &mRoot;
  }

  //****************************************************************************
  uint ChildCount(DataEntry* dataEntry) override
  {
    if(dataEntry == mRoot)
      return mEntries.Size();
    return 0;
  }

  //****************************************************************************
  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    if(dataEntry == mRoot)
      return mEntries[index];
    return nullptr;
  }
    
  //****************************************************************************
  bool IsExpandable(DataEntry* dataEntry) override
  {
    return false;
  }

  //****************************************************************************
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    ContentItem* contentItem = (ContentItem*)dataEntry;
    if(column == CommonColumns::Name)
    {
      String name = FilePath::GetFileNameWithoutExtension(contentItem->Filename);
      variant = name;
    }
  }

  //****************************************************************************
  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    return false;
  }
};

//--------------------------------------------------------- Content Package Tile
//******************************************************************************
ContentExportTile::ContentExportTile(Composite* parent, TileView* tileView,
                        PreviewWidget* tileWidget, DataIndex dataIndex,
                        ContentPackageExporter* uploadView, ContentItem* contentItem)
 : TileViewWidget(parent, tileView, tileWidget, dataIndex)
{
  mExporter = uploadView;
  mContentItem = contentItem;
  CheckForDependencies();

  mMissingText = new Text(this, cText);
  mMissingText->SetText("Missing dependencies");
  mMissingText->SizeToContents();

  ConnectThisTo(this, Events::RightClick, OnRightClick);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
}

//******************************************************************************
void ContentExportTile::UpdateTransform()
{
  Vec2 textSize = mMissingText->GetMinSize();
  Rect local = GetLocalRect();
  local.SizeY = textSize.y;
  SetClipping(true);
  PlaceCenterToRect(local, mMissingText);
  mMissingText->mTranslation.x = Math::Max(mMissingText->mTranslation.x, Pixels(1));
  mMissingText->mSize.x = Math::Min(textSize.x, mSize.x);
  mMissingText->SetVisible(mMissingDependencies);
  mMissingText->SetColor(UploaderUi::MissingColor);
  if(mMissingDependencies)
  {
    mBackground->SetColor(UploaderUi::MissingColor);
    //mHighlight->SetColor(UploaderUi::MissingColor);
  }

  TileViewWidget::UpdateTransform();
}

void ContentExportTile::OnMouseHover(MouseEvent* event)
{
  // do nothing, we don't want the popup as it interferes with the remove pop up
  // when attempting to remove an item or add a dependency
}

//******************************************************************************
void ContentExportTile::CheckForDependencies()
{
  // Store the old state so we don't have an unneeded update transform
  bool oldState = mMissingDependencies;

  HashSet<ContentItem*> missingDependencies;
  GetMissingDependencies(missingDependencies);

  mMissingDependencies = !missingDependencies.Empty();

  if(mMissingDependencies != oldState)
    MarkAsNeedsUpdate();
}

//******************************************************************************
void ContentExportTile::GetMissingDependencies(HashSet<ContentItem*>& missingDependencies)
{
  // Build the listing of all resources made by this content item
  ResourceListing listing;
  mContentItem->BuildListing(listing);

  // All dependent resources
  Handle instance = GetEditObject();

  // Get the dependencies from each resource
  forRange(ResourceEntry& entry, listing.All())
  {
    Resource* resource = Z::gResources->GetResource(entry.mResourceId);
    resource->GetDependencies(missingDependencies, instance);
  }

  forRange(ContentItem* dependency, missingDependencies.All())
  {
    // If it already Contains the content item, we can remove it from the list
    if(mExporter->mContentItems.Contains(dependency))
      missingDependencies.Erase(dependency);
  }
}

//******************************************************************************
void ContentExportTile::OnRightClick(MouseEvent* e)
{
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0) );

  if(mMissingDependencies)
    ConnectMenu(menu, "Add Dependencies", OnAddDependencies);
  ConnectMenu(menu, "Remove", OnRemove);
}

//******************************************************************************
void ContentExportTile::OnKeyDown(KeyboardEvent* e)
{
  if(e->Key == Keys::Delete)
    RemoveContentItem();
}

//******************************************************************************
void ContentExportTile::OnAddDependencies(Event* e)
{
  HashSet<ContentItem*> missingDependencies;
  GetMissingDependencies(missingDependencies);

  forRange(ContentItem* dependency, missingDependencies.All())
  {
    mExporter->mContentItems.PushBack(dependency);
  }

  mMissingDependencies = false;
  // Update the tile view display to show our dependency is no longer missing
  mExporter->RefreshTileView();
}

//******************************************************************************
void ContentExportTile::OnRemove(Event* e)
{
  RemoveContentItem();
}

//******************************************************************************
void ContentExportTile::RemoveContentItem()
{
  // Remove the item from the list of items to be exported
  mExporter->mContentItems.EraseValueError(mContentItem);
  // Update the tile view display
  mExporter->RefreshTileView();
}

//---------------------------------------------------- Content Package Tile View
//******************************************************************************
ContentExporterTileView::ContentExporterTileView(ContentPackageExporter* parent)
  : TileView(parent)
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

  if(listing.Empty())
    return nullptr;

  ResourceEntry& entry = listing.Front();
  return Z::gResources->GetResource(entry.mResourceId);
}

//******************************************************************************
TileViewWidget* ContentExporterTileView::CreateTileViewWidget(Composite* parent,
                StringParam name, HandleParam instance, DataIndex index,
                PreviewImportance::Enum minImportance)
{
  // We need to pull out a resource from the content item to display a preview
  ContentItem* contentItem = instance.Get<ContentItem*>();
  Resource* firstResource = GetFirstResource(contentItem);


  PreviewWidget* tileWidget = ResourcePreview::CreatePreviewWidget(parent, name,
                                                  firstResource, minImportance);
  if(tileWidget == nullptr)
    return nullptr;

  return new ContentExportTile(parent, this, tileWidget, index,
                               mExporter, contentItem);
}

//******************************************************************************
void ContentExporterTileView::OnMetaDrop(MetaDropEvent* e)
{
  if(e->Testing)
  {
    if(Resource* resource = e->Instance.Get<Resource*>())
    {
      if(mExporter->mContentItems.Contains(resource->mContentItem))
        e->Result = "Content item already added to package";
      else
        e->Result = "Add to content package";
    }
  }
  else
  {
    if(Resource* resource = e->Instance.Get<Resource*>())
    {
      if(mExporter->mContentItems.Contains(resource->mContentItem))
        return;

      mExporter->mContentItems.PushBack(resource->mContentItem);

      mExporter->MarkAsNeedsUpdate();
      mExporter->UpdateTransformExternal();
      mExporter->mHintText->SetActive(false);
    }
  }
}

//---------------------------------------------------------- Content Upload View
//******************************************************************************
ContentPackageExporter::ContentPackageExporter(Composite* parent)
  : Composite(parent)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(4,0,4,0)));
  this->SetMinSize(Pixels(800,600));

  mTileView = new ContentExporterTileView(this);
  mTileView->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  mSource = new ContentUploadSource(mContentItems);
  mTileView->SetDataSource(mSource);

  Splitter* splitter = new Splitter(this);
  splitter->SetInteractive(false);

  Composite* right = new Composite(this);
  right->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(4,0,4,0)));

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

  ConnectThisTo(this, "OnExportFileSelected", OnExportFileSelected);
}

//******************************************************************************
ContentPackageExporter::~ContentPackageExporter()
{
  SafeDelete(mSource);
}

//******************************************************************************
void ContentPackageExporter::UpdateTransform()
{
  // Center the hint text to the tile view
  Rect tileViewRect = mTileView->GetLocalRect();
  PlaceCenterToRect(tileViewRect, mHintText);

  Composite::UpdateTransform();
}

//******************************************************************************
void ContentPackageExporter::OnExportPressed(Event* e)
{
  // Ignore the event if we're not active
  if(!GetActive())
    return;

  if(mContentItems.Empty())
  {
    DoNotifyWarning("Cannot export content package", "No content items selected.");
    return;
  }

  FileDialogConfig config;
  config.EventName = "OnExportFileSelected";
  config.CallbackObject = this;
  config.Title = "Select Content Package File";
  config.AddFilter("Zero Pack File", "*.zeropack");
  config.StartingDirectory = GetUserDocumentsDirectory();
  config.DefaultFileName = "Package.zeropack";
  Z::gEngine->has(OsShell)->SaveFile(config);
}

//******************************************************************************
void ContentPackageExporter::OnExportFileSelected(OsFileSelection* e)
{
  if(!e->Success)
    return;

  String packageFile = e->Files[0];

  ContentPackageListing listing;

  // Add all the content items to the listing
  forRange(ContentItem* item, mContentItems.All())
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

  // If info provided write that out
  if(!mTempPackage.mName.Empty())
  {
    mTempPackage.mDate = GetDate();
    mTempPackage.mSize = GetFileSize(packageFile);
    mTempPackage.mVersionBuilt = GetRevisionNumber();

    // Write out the meta file
    String metaFile = BuildString(packageFile, ".meta");
    TextSaver saver;
    Status status;
    saver.Open(status, metaFile.c_str());
    if(!status.Failed())
    {
      saver.StartPolymorphic("ContentPackage");
      mTempPackage.Serialize(saver);
      saver.EndPolymorphic();
    }
    saver.Close();
  }
}

void ContentPackageExporter::RefreshTileView()
{
  mTileView->SetDataSource(mSource);
  MarkAsNeedsUpdate();
}

}//namespace Zero
