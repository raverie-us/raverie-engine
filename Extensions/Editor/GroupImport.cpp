///////////////////////////////////////////////////////////////////////////////
///
/// \file GroupImport.cpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void RunGroupImport(ImportOptions& options)
{
  ContentLibrary* library = options.mLibrary;

  Array<ContentItem*> contentToBuild;

  Array<String> filesToExport(options.mFiles);
  if(options.mConflictOptions && options.mConflictOptions->GetAction() == ConflictAction::Replace)
    filesToExport.Append(options.mConflictedFiles.All());

  // For every file imported
  for(uint fileIndex = 0; fileIndex < filesToExport.Size(); ++fileIndex)
  {
    String fullPath = filesToExport[fileIndex];
    String fileName = FilePath::GetFileName(fullPath);
    String storedfilename = FilePath::Combine(library->SourcePath, fileName);

    //Add the content item
    AddContentItemInfo addContent;
    addContent.FileName = fileName;
    addContent.Library = library;
    addContent.ExternalFile = fullPath;
    addContent.OnContentFileConflict = ContentFileConflict::Replace;
    addContent.Options = &options;

    Status addItem;

    ContentItem* newContentItem = Z::gContentSystem->AddContentItemToLibrary(addItem, addContent);

    //If the content add succeeded
    if(addItem.Succeeded())
      contentToBuild.PushBack(newContentItem);
    else
      DoNotifyError("Failed Import", addItem.Message);
  }

  // Now that all the content has been added. Build and load them for use.

  // Build all new content items
  ResourceLibrary* resourceLibrary = Z::gResources->GetResourceLibrary(library->Name);

  ResourcePackage package;
  Status status;
  Z::gContentSystem->BuildContentItems(status, contentToBuild, package);
  DoNotifyStatus(status);

  // Load all resource generated into the active resource library
  Z::gResources->ReloadPackage(resourceLibrary, &package);

  // Do editor side importing
  DoEditorSideImporting(&package, &options);

  // Compile all scripts
  ZilchManager::GetInstance()->Compile();

  if(!contentToBuild.Empty() && status.Succeeded())
    DoNotify("Content Imported", "Content has been added to the project", "BigPlus");
  else if (status.Failed())
    DoNotify("Content Import", "Content failed to be added to the project", "Error");
}

void GroupImport()
{
  ImportCallback* importCallback = new ImportCallback();
  importCallback->Open();
}

void OpenGroupImport(Array<String>& files)
{
  ContentLibrary* library = Z::gEditor->mProjectLibrary;
  ImportOptions* options = new ImportOptions();
  options->Initialize(files, library);

  if (options->ShouldAutoImport())
  {
    RunGroupImport(*options);
  }
  else
  {
    //Create the import options window
    Editor* editor = Z::gEditor;
    Window* window = new Window(Z::gEditor);
    window->SetTitle("Group Import Options");

    GroupImportWindow* import = new GroupImportWindow(window, options);

    window->SetSize(Pixels(540, 500.0f));
    window->SetTranslation(Pixels(10, 10, 0));

    CenterToWindow(Z::gEditor, window, false);
  }
}

void LoadFilesDroppedOnViewport(MouseEvent* event, Viewport* viewport, Space* space, Cog* droppedOn, Array<String>& files)
{
  if (files.Empty())
    return;

  {
    //Check for project file load and load if true
    String fileName = files[0];
    String extension = FilePath::GetExtension(fileName);
    if (extension == "zeroproj")
    {
      OpenProjectFile(fileName);
      return;
    }
    else if (extension == "data")
    {
      Z::gEditor->OpenTextFile(fileName);
      return;
    }
    else if (extension == "zeropack")
    {
      ContentImporter::OpenImportWindow(fileName);
      return;
    }
  }

  //Attempt to add files as resources to the project's library
  ContentLibrary* library = Z::gEditor->mProjectLibrary;

  if (library == NULL)
  {
    DoNotifyError("Failed", "No valid content library to add content");
    return;
  }

  // The user could've dragged a file in. Recursively find all files in the given path.
  // Might need to be updated later to deal with multiple files of the same name in different directories...
  Array<String> allFiles;
  for(size_t i = 0; i < files.Size(); ++i)
    FindFilesRecursively(files[i], allFiles);
  
  OpenGroupImport(allFiles);
}

//------------------------------------------------------------------------ GroupImportWindow
GroupImportWindow::GroupImportWindow(Composite* parent, ImportOptions* options)
  : Composite(parent)
{
  mOptions = options;

  mParentWindow = parent;
  this->SetLayout(CreateStackLayout());

  Composite* top = new Composite(this);
  top->SetLayout(CreateRowLayout());
  top->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);

  {
    {
      Composite* left = new Composite(top);
      left->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);
      left->SetLayout(CreateStackLayout());
      mPropertyView = new PropertyView(left);
      mPropertyView->SetObject(mOptions);
      mPropertyView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
      mPropertyView->Rebuild();
    }

    {
      Composite* right = new Composite(top);
      right->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);
      right->SetLayout(CreateStackLayout());

      Label* label = new Label(right);
      label->SetText("Files to import...");

      mListBox = new ListBox(right);
      mListBox->SetDataSource(&mStrings);
      mListBox->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
    }
  }

  Composite* buttonRow = new Composite(this);
  buttonRow->SetLayout(CreateRowLayout());
  {
    mImportButton = new TextButton(buttonRow);
    mImportButton->SetText("Import All");
    mCancelButton = new TextButton(buttonRow);
    mCancelButton->SetText("Cancel All");
  }
    
  ConnectThisTo(mImportButton, Events::ButtonPressed, OnPressed);
  ConnectThisTo(mCancelButton, Events::ButtonPressed, OnCancel);
  ConnectThisTo(mOptions, Events::ImportOptionsModified, OnOptionsModified);

  UpdateListBoxSource();
}

float GroupImportWindow::GetPropertyGridHeight()
{
  uint propertyCount = 0;
    
  // METAREFACTOR - Confirm AllProperties.Size() will have the same results as the old Properties array
  // Adding 1 to each for the 
  if(mOptions->mImageOptions)
    propertyCount += ZilchVirtualTypeId(mOptions->mImageOptions)->AllProperties.Size() + 1;
  if(mOptions->mGeometryOptions)
    propertyCount += ZilchVirtualTypeId(mOptions->mGeometryOptions)->AllProperties.Size() + 1;
  if(mOptions->mAudioOptions)
    propertyCount += ZilchVirtualTypeId(mOptions->mAudioOptions)->AllProperties.Size() + 1;
  if(mOptions->mConflictOptions)
    propertyCount += ZilchVirtualTypeId(mOptions->mConflictOptions)->AllProperties.Size() + 1;

  return (float)propertyCount * 20.0f;
}

void GroupImportWindow::OnOptionsModified(Event* event)
{
  UpdateListBoxSource();
  return;
  mOptions->BuildOptions();

  float time = 0.2f;

  float height = GetPropertyGridHeight();
  Vec2 windowSize = Pixels(540, height + 60.0f);

  Vec2 propertyGridSize = Pixels(200,GetPropertyGridHeight() + 9.0f);

  AnimateToSize(mPropertyView, propertyGridSize, time);
  AnimateToSize(mParentWindow, windowSize, time);

  if(mOptions->mConflictOptions->mAction == ConflictAction::Replace)
  {
    ActionSequence* sequence = new ActionSequence(this);
    sequence->Add(new ActionDelay(time));
    sequence->Add(new CallAction<GroupImportWindow, &GroupImportWindow::RebuildTree>(this));
  }
  else
  {
    RebuildTree();
  }
}

void GroupImportWindow::RebuildTree()
{
  mPropertyView->Invalidate();
}

void GroupImportWindow::UpdateListBoxSource()
{
  mStrings.Strings.Clear();
  for(uint i = 0; i < mOptions->mFiles.Size(); ++i)
  {
    String file = FilePath::GetFileName(mOptions->mFiles[i]);
    mStrings.Strings.PushBack(String::Format("(Add) %s", file.c_str()));
  }

  for(uint i = 0; i < mOptions->mConflictedFiles.Size(); ++i)
  {
    String file = FilePath::GetFileName(mOptions->mConflictedFiles[i]);
    mStrings.Strings.PushBack(String::Format("(Update) %s", file.c_str()));
    mListBox->HighlightItem(mOptions->mFiles.Size() + i);
  }
}

void GroupImportWindow::OnPressed(Event* event)
{
  RunGroupImport(*mOptions);
  float time = 0.5f;
  AnimateTo(mParentWindow, Pixels(2000.0f, 200.0f, 0), mParentWindow->GetSize() * 0.5f, time);

  ActionSequence* sequence = new ActionSequence(mParentWindow);
  sequence->Add(new ActionDelay(time));
  sequence->Add(DestroyAction(mParentWindow));
}

void GroupImportWindow::OnCancel(Event* event)
{
  mParentWindow->Destroy();
}

//------------------------------------------------------------------------ ImportCallback
void ImportCallback::Open()
{
  //Open the open file dialog
  FileDialogConfig config;
  config.EventName = "OnFileSelected";
  config.CallbackObject = this;
  config.Title = "Select resource";
  config.AddFilter("ResourceFile", "*.*");
  config.Flags |= FileDialogFlags::MultiSelect;

  ConnectThisTo(this, config.EventName, OnFilesSelected);
  Z::gEngine->has(OsShell)->OpenFile(config);
}

void ImportCallback::OnFilesSelected(OsFileSelection* fileSelection)
{
  OpenGroupImport(fileSelection->Files);
  delete this;
}

}//namespace Zero
