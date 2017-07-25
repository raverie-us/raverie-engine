///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentPackageWidget.cpp
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ContentImportUi
{
const cstr cLocation = "EditorUi/ContentImporter";
Tweakable(Vec4, ToolTipBorderColor, Vec4(1,1,1,1), cLocation);
}

//---------------------------------------------------- Content Entry Name Sorter
template <typename Comparer>
struct ContentEntryNameSorter
{
  Comparer mComparer;
  ContentEntryNameSorter(Comparer comparer) : mComparer(comparer) {}

  bool operator()(ContentPackageEntry* left, ContentPackageEntry* right)
  {
    String lowerLeft = left->File.ToLower();
    String lowerRight = right->File.ToLower();
    return mComparer(lowerLeft, lowerRight);
  }
};

//----------------------------------------------- Content Entry Extension Sorter
template <typename Comparer>
struct ContentEntryExtensionSorter
{
  Comparer mComparer;
  ContentEntryExtensionSorter(Comparer comparer) : mComparer(comparer) {}

  bool operator()(ContentPackageEntry* left, ContentPackageEntry* right)
  {
    String leftExtension = FilePath::GetExtension(left->File);
    String rightExtension = FilePath::GetExtension(right->File);
    return mComparer(leftExtension, rightExtension);
  }
};

//-------------------------------------------------- Content Entry Active Sorter
template <typename Comparer>
struct ContentEntryActiveSorter
{
  Comparer mComparer;
  ContentEntryActiveSorter(Comparer comparer) : mComparer(comparer) {}

  bool operator()(ContentPackageEntry* left, ContentPackageEntry* right)
  {
    return mComparer(left->Active, right->Active);
  }
};

//------------------------------------------------------ Content Importer Source
struct ContentImporterSource : public DataSource
{
  ContentPackageListing* mListing;

  //****************************************************************************
  ContentImporterSource(ContentPackageListing* listing) : mListing(listing) { }

  //****************************************************************************
  DataEntry* GetRoot() override
  {
    return (DataEntry*)mListing;
  }

  //****************************************************************************
  DataEntry* ToEntry(DataIndex index) override
  {
    return (DataEntry*)index.Id;
  }

  //****************************************************************************
  DataIndex ToIndex(DataEntry* dataEntry) override
  {
    return DataIndex((u64)dataEntry);
  }

  //****************************************************************************
  DataEntry* Parent(DataEntry* dataEntry) override
  {
    return mListing;
  }

  //****************************************************************************
  uint ChildCount(DataEntry* dataEntry) override
  {
    if(dataEntry == mListing)
      return mListing->Entries.Size();
    else
      return 0;
  }

  //****************************************************************************
  DataEntry* GetChild(DataEntry* dataEntry, uint index, DataEntry* prev)override
  {
     ContentPackageListing* listing = (ContentPackageListing*)dataEntry;
     return (DataEntry*)listing->SortedEntries[index];
  }

  //****************************************************************************
  bool IsExpandable(DataEntry* dataEntry) override
  {
    return dataEntry == mListing;
  }

  //****************************************************************************
  void GetData(DataEntry* dataEntry, Any& variant, StringParam column)override
  {
    // Ignore the root
    if(dataEntry == mListing)
      return;

    ContentPackageEntry* entry = (ContentPackageEntry*)dataEntry;
    if(!column.Empty())
    {
      if(column == CommonColumns::Name)
      {
        variant = String(FilePath::GetFileNameWithoutExtension(entry->File));
      }
      else if(column == CommonColumns::Icon)
      {
        if(entry->Conflicted)
          variant = String("WarningIcon");
        else
          variant = String("ResourceIcon");
      }
      else if(column == "Type")
      {
        variant = String(FilePath::GetExtension(entry->File));
      }
      else if(column == "Import")
      {
        variant = entry->Active;
      }
    }
  }

  //****************************************************************************
  bool SetData(DataEntry* dataEntry, AnyParam variant, StringParam column) override
  {
    ContentPackageEntry* entry = (ContentPackageEntry*)dataEntry;
    if(!column.Empty())
    {
      if(column == "Import")
      {
        entry->Active = variant.Get<bool>();
        return true;
      }
    }
    return false;
  }

  //****************************************************************************
  void Sort(DataEntry* dataEntry, StringParam column, bool flip) override
  {
    // Can only sort the root
    if(dataEntry != NULL)
      return;

    #define GetSorter(type, comparer) type<comparer>(comparer())

    if(flip)
    {
      if(column == CommonColumns::Name)
        Zero::Sort(mListing->SortedEntries.All(), GetSorter(ContentEntryNameSorter, greater<String>));
      else if(column == CommonColumns::Type)
        Zero::Sort(mListing->SortedEntries.All(), GetSorter(ContentEntryExtensionSorter, greater<String>));
      else if(column == "Import")
        Zero::Sort(mListing->SortedEntries.All(), GetSorter(ContentEntryActiveSorter, greater<bool>));
    }
    else
    {
      if(column == CommonColumns::Name)
        Zero::Sort(mListing->SortedEntries.All(), GetSorter(ContentEntryNameSorter, less<String>));
      else if(column == CommonColumns::Type)
        Zero::Sort(mListing->SortedEntries.All(), GetSorter(ContentEntryExtensionSorter, less<String>));
      else if(column == "Import")
        Zero::Sort(mListing->SortedEntries.All(), GetSorter(ContentEntryActiveSorter, less<bool>));
    }
  }
};

//------------------------------------------------------------- Content Importer
//******************************************************************************
ContentImporter::ContentImporter(Composite* parent)
  :Composite(parent)
{
  this->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom,
                                    Pixels(0, 2), Thickness::cZero));

  mTreeView = new TreeView(this);
  mTreeView->SetMinSize(Pixels(200, 200));
  mTreeView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);

  SetTreeFormatting();

  // Create the data source
  mSource = new ContentImporterSource(&mListing);
  mTreeView->SetDataSource(mSource);
  ConnectThisTo(mTreeView, Events::MouseEnterRow, OnMouseEnterTreeRow);
  ConnectThisTo(mTreeView, Events::MouseExitRow, OnMouseExitTreeRow);

  mTagEditor = new TagEditor(this);
  mTagEditor->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(45.0f));
  mTagEditor->GetTagChain()->mSorted = false;

  Composite* buttonRow = new Composite(this);
  buttonRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight,
                                         Vec2::cZero, Thickness::cZero));
  {
    TextButton* importButton = new TextButton(buttonRow);
    importButton->SetText("Import");
    importButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(80));
    ConnectThisTo(importButton, Events::ButtonPressed, OnImport);

    Composite* temp = new Composite(buttonRow);
    temp->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

    TextButton* cancelButton = new TextButton(buttonRow);
    cancelButton->SetText("Cancel");
    cancelButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(80));
    ConnectThisTo(cancelButton, Events::ButtonPressed, OnCancel);
  }

  ConnectThisTo(this, "OnImportFileSelected", OnImportFileSelected);
}

//******************************************************************************
void ContentImporter::OpenImportWindow(StringParam fileName)
{
  Window* window = new Window(Z::gEditor);
  window->SetTitle("Import Content Package");
  ContentImporter* widget = new ContentImporter(window);
  window->SizeToContents();
  window->SetSize(Pixels(340,490));
  CenterToWindow(Z::gEditor, window, false);
  widget->LoadListingFromFile(fileName);
  window->MoveToFront();
}

//******************************************************************************
ContentImporter::~ContentImporter()
{
  mToolTip.SafeDestroy();
  SafeDelete(mSource);
}

//******************************************************************************
void ContentImporter::LoadListingFromFile(StringParam filename)
{
  mFileName = filename;
  LoadContentPackageListing(mListing, filename);
  #define GetSorter(type, comparer) type<comparer>(comparer())
  Sort(mListing.SortedEntries.All(), GetSorter(ContentEntryActiveSorter, less<bool>));
  mTreeView->Refresh();

  String defaultTag = FilePath::GetFileNameWithoutExtension(filename);
  mTagEditor->GetTagChain()->AddTag(defaultTag, true);
}

//******************************************************************************
void ContentImporter::SetTreeFormatting()
{
  TreeFormatting formatting;
  formatting.Flags.SetFlag(FormatFlags::ShowHeaders);

  // Icon column
  ColumnFormat* format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = CommonColumns::Icon;
  format->ColumnType = ColumnType::Fixed;
  format->FixedSize = Pixels(16, 20);
  format->Editable = false;
  format->CustomEditor = cDefaultIconEditor;

  // Name column
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = CommonColumns::Name;
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 3;
  format->HeaderName = "Name";
  format->Editable = false;

  // Type column
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = CommonColumns::Type;
  format->ColumnType = ColumnType::Flex;
  format->FlexSize = 1;
  format->HeaderName = "Type";
  format->Editable = false;

  // Import column
  format = &formatting.Columns.PushBack();
  format->Index = formatting.Columns.Size() - 1;
  format->Name = "Import";
  format->ColumnType = ColumnType::Fixed;
  format->FixedSize = Pixels(60, 20);
  format->Editable = false;
  format->HeaderName = "Import";
  format->CustomEditor = cDefaultBooleanEditor;

  mTreeView->SetFormat(formatting);
}

//******************************************************************************
void ContentImporter::OnImport(Event* event)
{
  // Add the tags to the listing
  mTagEditor->GetTagChain()->GetTags(mListing.Tags);

  ImportContentPackageListing(mListing, Z::gEditor->mProjectLibrary, mFileName);

  Z::gEditor->mLibrary->SetSearchTags(mListing.Tags);

  CloseTabContaining(this);
}

//******************************************************************************
void ContentImporter::OnCancel(Event* event)
{
  CloseTabContaining(this);
}

//******************************************************************************
void ContentImporter::OnImportFileSelected(OsFileSelection* event)
{
  if(event->Success)
    LoadListingFromFile(event->Files[0]);
  else
    CloseTabContaining(this);
}

//******************************************************************************
void ContentImporter::OnMouseEnterTreeRow(TreeEvent* e)
{
  mToolTip.SafeDestroy();

  DataIndex index = e->Row->mIndex;
  DataSource* dataSource = mTreeView->GetDataSource();
  ContentPackageEntry* entry = (ContentPackageEntry*)dataSource->ToEntry(index);

  // Create the tooltip if it's conflicted
  if(entry->Conflicted)
  {
    ToolTip* toolTip = new ToolTip(e->Row);
    toolTip->SetText("Content already exists under this name. Importing this "
                     "will overwrite the existing content item.");
    toolTip->SetColor(ToolTipColor::Yellow);

    // Position the tooltip
    ToolTipPlacement placement;
    placement.SetScreenRect(e->Row->GetScreenRect());
    placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left,
                          IndicatorSide::Bottom, IndicatorSide::Top);
    toolTip->SetArrowTipTranslation(placement);

    mToolTip = toolTip;
  }
}

//******************************************************************************
void ContentImporter::OnMouseExitTreeRow(TreeEvent* e)
{
  mToolTip.SafeDestroy();
}

//******************************************************************************
void ImportContentPackage()
{
  Window* window = new Window(Z::gEditor);
  window->SetTitle("Import Content Package");
  ContentImporter* widget = new ContentImporter(window);
  window->SizeToContents();
  CenterToWindow(Z::gEditor, window, false);

  //Open the open file dialog
  FileDialogConfig config;
  config.EventName = "OnImportFileSelected";
  config.CallbackObject = widget;
  config.Title = "Zero Package";
  config.AddFilter("Zero Pack File", "*.zeropack");
  Z::gEngine->has(OsShell)->OpenFile(config);
}

}//namespace Zero
