///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentStore.cpp
/// 
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//******************************************************************************
String GetDownloadFolder()
{
  return FilePath::Combine(GetTemporaryDirectory(), "ZeroDownloads");
}

//******************************************************************************
String GetLocalContentStoreDirectory()
{
  String dataDir = Z::gEngine->GetConfigCog()->has(MainConfig)->DataDirectory;
  return FilePath::Combine(dataDir, "ContentStore");
}

//---------------------------------------------------- Content Store Data Source
class ContentStoreSource : public DataSource
{
public:
  ContentPackage mRoot;
  Array<ContentPackage*>& mEntries;

  //****************************************************************************
  ContentStoreSource(ContentImportView* contentStore)
    : mEntries(contentStore->mContentPackages)
  {
    
  }

  //****************************************************************************
  DataEntry* GetRoot() override
  {
    return &mRoot;
  }

  //****************************************************************************
  DataEntry* ToEntry(DataIndex index) override
  {
    if(index == cRootIndex || (uint)index.Id >= mEntries.Size())
      return &mRoot;
    return mEntries[(uint)index.Id];
  }

  //****************************************************************************
  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    if(dataEntry == &mRoot)
      return cRootIndex;
    ContentPackage* entry = (ContentPackage*)dataEntry;
    uint index = mEntries.FindIndex(entry);
    return DataIndex(index);
  }

  //****************************************************************************
  Handle ToHandle(DataEntry* dataEntry) override
  {
    return (ContentPackage*)dataEntry;
  }
  
  //****************************************************************************
  DataEntry* Parent(DataEntry* dataEntry) override
  {
    if(dataEntry == &mRoot)
      return NULL;
    return &mRoot;
  }

  //****************************************************************************
  uint ChildCount(DataEntry* dataEntry) override
  {
    if(dataEntry == &mRoot)
      return mEntries.Size();
    return 0;
  }

  //****************************************************************************
  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev) override
  {
    if(dataEntry == &mRoot)
      return mEntries[index];
    return NULL;
  }
  
  //****************************************************************************
  bool IsExpandable(DataEntry* dataEntry) override
  {
    return false;
  }

  //****************************************************************************
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column) override
  {
    ContentPackage* entry = (ContentPackage*)dataEntry;
    if (column == CommonColumns::Name)
      variant = entry->mName;
    else
      variant = entry;
  }

  //****************************************************************************
  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    return false;
  }
};

//------------------------------------------------------ Content Package Preview
class PackagePreviewWidget : public PreviewWidget
{
public:
  typedef PackagePreviewWidget ZilchSelf;
  ContentPackage* mPackage;
  TextureView* mImage;

  //****************************************************************************
  PackagePreviewWidget(ContentPackage* package, Composite* parent)
    : PreviewWidget(parent), mPackage(package)
  {
    ConnectThisTo(package, Events::TextureLoaded, OnTextureLoaded);
    mImage = new TextureView(this);
    mImage->SetTexture(package->mPreview);
  }

  //****************************************************************************
  void UpdateTransform()
  {
    mImage->SetSize(mSize);
    PreviewWidget::UpdateTransform();
  }

  //****************************************************************************
  void OnTextureLoaded(Event* e)
  {
    mImage->SetTexture(mPackage->mPreview);
  }
};

//---------------------------------------------------- Content Package Tile View
//******************************************************************************
TileViewWidget* ContentPackageTileView::CreateTileViewWidget(Composite* parent,
                 StringParam name, HandleParam instance, DataIndex index,
                 PreviewImportance::Enum minImportance)
{
  if(ContentPackage* package = instance.Get<ContentPackage*>())
  {
    PackagePreviewWidget* preview = new PackagePreviewWidget(package, parent);
    preview->mName = package->mName;
    return new TileViewWidget(parent, this, preview, index);
  }
  return NULL;
}

//------------------------------------------------------ Content Package Preview
class PreviewDataEntry : public Composite
{
public:
  //****************************************************************************
  PreviewDataEntry(Composite* parent, StringParam label, Text*& rhs, Vec4Param color)
    : Composite(parent)
  {
    SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));
    mBackground = CreateAttached<Element>(cWhiteSquare);
    mBackground->SetColor(color);
    Text* nameLabel = new Text(this, cText);
    nameLabel->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    nameLabel->SetText(label);

    rhs = new Text(this, cText);
    rhs->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    rhs->mAlign = TextAlign::Right;
  }

  //****************************************************************************
  void UpdateTransform() override
  {
    mBackground->SetSize(mSize);
    Composite::UpdateTransform();
  }

  Element* mBackground;
};

//******************************************************************************
ContentPackagePreview::ContentPackagePreview(Composite* parent)
  : Composite(parent)
{
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0,0), Thickness(Pixels(4,0,0,0))));

  // Create the area where the preview exists
  mPreviewArea = new Composite(this);
  mPreviewArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 3.0f);
  mPreviewArea->SetLayout(CreateRatioLayout());
  {
    Resource* temp = SpriteSourceManager::GetInstance()->GetDefault();
    mPreview = ResourcePreview::CreatePreviewWidget(mPreviewArea, "Default", temp);
    mPreview->SizeToContents();
  }

  // Content Package data area
  Composite* infoArea = new Composite(this);
  infoArea->SetSizing(SizeAxis::Y, SizePolicy::Flex, 2);
  infoArea->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0,1), Thickness(0, 0, 0, 0)));
  {
    new PreviewDataEntry(infoArea, "Name: ", mName, TreeViewValidUi::PrimaryColor);
    new PreviewDataEntry(infoArea, "Author: ", mAuthor, TreeViewValidUi::SecondaryColor);
    new PreviewDataEntry(infoArea, "Size: ", mSize, TreeViewValidUi::PrimaryColor);
    new PreviewDataEntry(infoArea, "Published: ", mDate, TreeViewValidUi::SecondaryColor);
    new PreviewDataEntry(infoArea, "Zero Version: ", mVersionBuilt, TreeViewValidUi::PrimaryColor);
    
    Spacer* spacer = new Spacer(infoArea);
    spacer->SetSize(Pixels(1,10));

    Composite* test = new Composite(infoArea);
    test->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
    mDescription = new Text(test, cText);
    mDescription->SetMultiLine(true);
    mDescription->SetSize(Pixels(280, 80));
    mDescription->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    mDescription->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

    Composite* previewArea = new Composite(infoArea);
    previewArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(0,0,0,2)));
    {
      Spacer* spacer = new Spacer(previewArea);
      spacer->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);

      mPreviewButton = new TextButton(previewArea);
      mPreviewButton->SetText("Preview");
      mPreviewButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(120));
      mPreviewButton->SetVisible(false);
      ConnectThisTo(mPreviewButton, Events::ButtonPressed, OnPreviewPressed);
    }
  }
}

//******************************************************************************
ContentPackagePreview::~ContentPackagePreview()
{
  mToolTip.SafeDestroy();
}

//******************************************************************************
void ContentPackagePreview::UpdateTransform()
{
  // Always place the tooltip to the right of the version text
  if(ToolTip* toolTip = mToolTip)
  {
    ToolTipPlacement placement;
    placement.SetScreenRect(mVersionBuilt->GetScreenRect());
    placement.SetPriority(IndicatorSide::Right, IndicatorSide::Right, 
                          IndicatorSide::Right, IndicatorSide::Right);
    toolTip->SetArrowTipTranslation(placement);
  }

  Composite::UpdateTransform();
}

//******************************************************************************
void ContentPackagePreview::ViewContentPackage(ContentPackage* contentPackage)
{
  mToolTip.SafeDestroy();
  mContentPackage = contentPackage;

  bool visible = (contentPackage != NULL && !contentPackage->mName.Empty());
  mName->SetActive(visible);
  mAuthor->SetActive(visible);
  mSize->SetActive(visible);
  mDate->SetActive(visible);
  mVersionBuilt->SetActive(visible);
  mDescription->SetActive(visible);
  mPreviewButton->SetVisible(visible);

  if(contentPackage)
  {
    mName->SetText(contentPackage->mName);
    mAuthor->SetText(contentPackage->mAuthor);
    mDate->SetText(contentPackage->mDate);
    mVersionBuilt->SetText(ToString(contentPackage->mVersionBuilt));
    mDescription->SetText(contentPackage->mDescription);

    mVersionBuilt->SetColor(Vec4(1,1,1,1));

    // If the content package was built in a newer version of the editor,
    // we need to warn them
    uint revisionNumber = GetRevisionNumber();
    if(revisionNumber < contentPackage->mVersionBuilt)
    {
      const Vec4 cErrorColor = Vec4(1, 0.1f, 0.1f, 1);
      mVersionBuilt->SetColor(cErrorColor);

      // Create a tooltip warning them about an older version
      ToolTip* toolTip = new ToolTip(this);
      toolTip->SetDestroyOnMouseExit(false);
      toolTip->SetColor(ToolTipColor::Red);
      toolTip->SetText("Warning! This content package was built in a newer "
                       "version of the Zero Engine. This may cause errors to "
                       "occur, and it's recommended that you update to the newest version.");

      // The tooltip will be placed in the UpdateTransform function
      mToolTip = toolTip;
    }

    // Display the size of the content package in a readable format
    uint bytes = contentPackage->mSize;
    mSize->SetText(HumanReadableFileSize(bytes));

    mPreview->Destroy();
    mPreview = new PackagePreviewWidget(contentPackage, mPreviewArea);
  }
  MarkAsNeedsUpdate();
}

//******************************************************************************
void OnPackagePreviewCallback(BackgroundTask* task, Job* job)
{
  DownloadTaskJob* downloadJob = (DownloadTaskJob*)job;
  String data = downloadJob->mData;
  if(data == "File not found")
    return;

  String contentPackageName = downloadJob->mName;

  // The location to extract the project to
  String folderName = BuildString(contentPackageName, "Package");
  String location = FilePath::Combine(GetTemporaryDirectory(), folderName);

  // Read the data from the request into a buffer
  ByteBufferBlock buffer((byte*)data.Data(), data.SizeInBytes(), false);

  // Extract the archive to the given location
  Archive projectArchive(ArchiveMode::Decompressing);
  projectArchive.ReadBuffer(ArchiveReadFlags::Enum(ArchiveReadFlags::Entries |
                                               ArchiveReadFlags::Data), buffer);
  projectArchive.ExportToDirectory(ArchiveExportMode::OverwriteIfNewer, location);

  // The location of the project file that was extracted
  String projectLocation;// = FilePath::Combine(location, BuildString(contentPackageName, ".zeroproj"));

  const String cZeroProjExtension = "zeroproj";

  // Walk through all the entries and look for the .zeroproj
  forRange(ArchiveEntry entry, projectArchive.GetEntries())
  {
    StringRange extension = FilePath::GetExtension(entry.Name);
    if(extension == cZeroProjExtension)
    {
      projectLocation = FilePath::Combine(location, entry.Name);
      break;
    }
  }

  // Open a new instance of the Zero engine and load the downloaded project
  Os::SystemOpenFile(GetApplication().c_str(), Os::Verb::Open, projectLocation.c_str());
}

//******************************************************************************
void ContentPackagePreview::OnPreviewPressed(Event* e)
{
  // Store the new project in the config file
  String url = String::Format("http://zero.digipen.edu/ContentStore/ContentStore.php?"
                              "Command=RequestPackagePreviewProject&FileName=%s",
                              mContentPackage->mName.c_str());

  String name = BuildString(mContentPackage->mName, "Preview");
  BackgroundTask* task = DownloadTaskJob::DownloadToBuffer(url, name);
  task->mCallback = &OnPackagePreviewCallback;
}

//******************************************************************************
void OnPackageDownloadCallback(BackgroundTask* task, Job* job)
{
  DownloadTaskJob* downloadJob = (DownloadTaskJob*)job;
  String location = FilePath::Combine(GetTemporaryDirectory(), downloadJob->mName);
  WriteStringRangeToFile(location, downloadJob->mData);

  ContentImporter::OpenImportWindow(location);
}

//---------------------------------------------------------- Content Import View
//******************************************************************************
ContentImportView::ContentImportView(Composite* parent)
  : Composite(parent)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(4,0,4,0)));

  Composite* searchArea = new Composite(this);
  searchArea->SetLayout(CreateStackLayout());
  searchArea->SetSizing(SizeAxis::X, SizePolicy::Flex, 3.0f);
  {
    mTileView = new ContentPackageTileView(searchArea);
    mTileView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
    mSource = new ContentStoreSource(this);
    mTileView->SetDataSource(mSource);
    mTileView->SetItemSizePercent(0.7f);

    ConnectThisTo(mTileView->GetSelection(), Events::DataSelectionFinal, OnDataSelectionFinal);
  }

  Splitter* splitter = new Splitter(this);
  splitter->SetInteractive(false);

  mPreview = new ContentPackagePreview(this);
  mPreview->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(280));

  mMinSize = Pixels(600, 300);

  const String cRequestUrl = "http://zero.digipen.edu/ContentStore/ContentStore.php?Command=RequestContentList";

  // Load local packages
  LoadLocalContentPackages();

  // Get the package list
  ConnectThisTo(&mContentListRequest, Events::WebResponse, OnWebResponse);
  mContentListRequest.mUrl = cRequestUrl;
  mContentListRequest.Run();
}

//******************************************************************************
ContentImportView::~ContentImportView()
{
  DeleteObjectsInContainer(mContentPackages);
  SafeDelete(mSource);
}

//******************************************************************************
void ContentImportView::SetImportButton(TextButton* button)
{
  ConnectThisTo(button, Events::ButtonPressed, OnImportPressed);
}

//******************************************************************************
void ContentImportView::OnWebResponse(WebResponseEvent* e)
{
  if(e->ResponseCode == WebResponseCode::OK)
  {
    DataTreeLoader loader;
    Status status;
    loader.OpenBuffer(status, e->Data);

    if(status.Failed())
      return;

    PolymorphicNode rootNode;
    loader.GetPolymorphic(rootNode);

    PolymorphicNode packageNode;
    while(loader.GetPolymorphic(packageNode))
    {
      // We need to serialize the package to get the name before trying
      // to get the final package
      ContentPackage tempPackage;
      tempPackage.Serialize(loader);
      loader.EndPolymorphic();

      ContentPackage* package = GetContentPackage(tempPackage.mName);
      *package = tempPackage;
      package->mLocal = false;

      String url = String::Format("http://zero.digipen.edu/ContentStore/ContentStore.php?"
                                  "Command=RequestPackagePreview&FileName=%s", package->mName.c_str());
      package->LoadStreamedTexture(url);
    }
    loader.EndPolymorphic();

    MarkAsNeedsUpdate();
  }
}

//******************************************************************************
void ContentImportView::LoadContentPackages(StringParam fileName)
{
  DataTreeLoader loader;
  Status status;
  loader.OpenFile(status, fileName);

  if(status.Failed())
    return;

  PolymorphicNode rootNode;
  loader.GetPolymorphic(rootNode);

  PolymorphicNode packageNode;
  while(loader.GetPolymorphic(packageNode))
  {
    ContentPackage* package = new ContentPackage();
    package->Serialize(loader);
    mContentPackages.PushBack(package);
    loader.EndPolymorphic();
  }
  loader.EndPolymorphic();
}

//******************************************************************************
void ContentImportView::OnDataSelectionFinal(Event* e)
{
  // Get the selected indices
  Array<DataIndex> selectedIndices;
  mTileView->GetSelection()->GetSelected(selectedIndices);

  if(selectedIndices.Empty())
  {
    mPreview->ViewContentPackage(NULL);
    return;
  }

  ContentPackage* selectedPackage = (ContentPackage*)mSource->ToEntry(selectedIndices.Front());
  mPreview->ViewContentPackage(selectedPackage);
}

//******************************************************************************
void ContentImportView::OnImportPressed(Event* e)
{
  // Ignore the event if we're not active
  if(!GetActive())
    return;

  Array<DataIndex> selectedIndices;
  mTileView->GetSelection()->GetSelected(selectedIndices);

  if(selectedIndices.Empty())
    return;

  DataIndex selected = selectedIndices.Front();
  ContentPackage* package = (ContentPackage*)mSource->ToEntry(selected);

  if(package->mLocal)
  {
    String directory = GetLocalContentStoreDirectory();
    String location = FilePath::CombineWithExtension(directory, package->mName, ".zeropack");
    ContentImporter::OpenImportWindow(location);
  }
  else
  {
    String url = String::Format("http://zero.digipen.edu/ContentStore/ContentStore.php?"
      "Command=RequestContentPackageDownload&FileName=%s", package->mName.c_str());
    BackgroundTask* task = DownloadTaskJob::DownloadToBuffer(url, package->mName);
    task->mCallback = &OnPackageDownloadCallback;
  }
}

//******************************************************************************
void ContentImportView::LoadLocalContentPackages()
{
  const String cMetaExtension = "meta";
  String directory = GetLocalContentStoreDirectory();

  // Walk each meta file in the directory and create a content package
  FileRange fileRange(directory);
  while(!fileRange.Empty())
  {
    String fileName = fileRange.Front();
    String extension = FilePath::GetExtension(fileName);

    // Only look at the meta files
    if(extension == cMetaExtension)
    {
      // Get the content package
      String packageName = FilePath::GetFileNameWithoutExtension(fileName);
      ContentPackage* package = GetContentPackage(packageName);

      // Load the file
      DataTreeLoader loader;
      Status status;
      loader.OpenFile(status, FilePath::Combine(directory, fileName));

      // Serialize the data into the content package
      PolymorphicNode node;
      loader.GetPolymorphic(node);
      package->Serialize(loader);
      loader.EndPolymorphic();

      // Attempt to load the preview image for this content package
      String previewName = BuildString(package->mName, "Preview.png");
      package->LoadLocalTexture(FilePath::Combine(directory, previewName));

      loader.Close();
    }

    fileRange.PopFront();
  }
}

//******************************************************************************
ContentPackage* ContentImportView::GetContentPackage(StringParam packageName)
{
  ContentPackage* package = mContentPackagesMap.FindValue(packageName, NULL);
  if(package == NULL)
  {
    package = new ContentPackage();
    mContentPackages.PushBack(package);
    mContentPackagesMap.Insert(packageName, package);
  }

  return package;
}

//---------------------------------------------------------------- Content Store
ContentStore* ContentStore::mOpenStore = NULL;

//******************************************************************************
void ContentStore::Open()
{
  if(mOpenStore != NULL)
    return;

  Window* window = new Window(Z::gEditor);
  window->mClientPadding.Bottom = 0.0f;
  window->mClientPadding.Left = 0.0f;
  window->mClientPadding.Right = 0.0f;
  mOpenStore = new ContentStore(window);
  mOpenStore->SetName("TheVoid");
  mOpenStore->SetHideOnClose(true);
  window->SetSize(Pixels(1100,520));

  Vec3 center = GetCenterPosition(Z::gEditor, window);
  window->SetTranslation(Vec3(center.x, -Pixels(520),0));
  AnimateTo(window, center, Pixels(1100, 520));
}

//******************************************************************************
ContentStore::ContentStore(Composite* parent)
  : Composite(parent)
{
  SetLayout(CreateStackLayout());

  // Create the import view
  mImportView = new ContentImportView(this);
  mImportView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);

  // Create the upload view, but disable it as the import view is shown by default
  mExporter = new ContentPackageExporter(this);
  mExporter->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
  mExporter->SetActive(false);

  mBottomArea = new Composite(this);
  mBottomArea->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(27));
  mBottomArea->SetLayout(CreateStackLayout(LayoutDirection::RightToLeft, Pixels(4,0), Thickness(1,2,1,2)));
  {
    mBottomBar = mBottomArea->CreateAttached<Element>(cWhiteSquare);
    mBottomBar->SetColor(Vec4(0.1f, 0.1f, 0.1f,1));
    mBottomBar->SetNotInLayout(true);
    mBottomBar->SetInteractive(false);

    mPrimaryButton = new TextButton(mBottomArea);
    mPrimaryButton->SetText("Import Selected...");
    mPrimaryButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(280));

    Spacer* spacer = new Spacer(mBottomArea);
    spacer->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);

    mUploadButton = new TextButton(mBottomArea);
    mUploadButton->SetText("Upload...");
    mUploadButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(135));
    ConnectThisTo(mUploadButton, Events::ButtonPressed, OnUploadPressed);

    mIcon = mBottomArea->CreateAttached<Element>("TheVoidFull");
    Composite* iconComposite = new Composite(mBottomArea);
    iconComposite->SetSizing(SizeAxis::X, SizePolicy::Fixed, mIcon->GetMinSize().x);
  }

  mImportView->SetImportButton(mPrimaryButton);

  // For, the upload UI is for devs only
  if(Z::gEngine->GetConfigCog()->has(DeveloperConfig) == NULL)
    mUploadButton->SetVisible(false);
}

//******************************************************************************
ContentStore::~ContentStore()
{
  mOpenStore = NULL;
}

//******************************************************************************
void ContentStore::UpdateTransform()
{
  mBottomBar->SetSize(mBottomArea->GetSize());
  mIcon->SetTranslation(Pixels(2,0,0));
   
  Composite::UpdateTransform();
}

//******************************************************************************
void ContentStore::OnUploadPressed(Event* e)
{
  bool importActive = mImportView->GetActive();
  mImportView->SetActive(!importActive);
  mExporter->SetActive(importActive);
  mImportView->MarkAsNeedsUpdate();
  mExporter->MarkAsNeedsUpdate();

  if(importActive)
  {
    mUploadButton->SetText("Cancel");
    mPrimaryButton->SetText("Upload Content Items...");
  }
  else
  {
    mUploadButton->SetText("Upload...");
    mPrimaryButton->SetText("Import Selected...");
  }

  MarkAsNeedsUpdate();
}

}//namespace Zero
