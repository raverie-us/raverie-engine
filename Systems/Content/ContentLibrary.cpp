///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentLibrary.hpp
///
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(ContentLibrary, builder, type)
{
}

ContentLibrary::ContentLibrary()
{
  mReadOnly = false;
}

ContentLibrary::~ContentLibrary()
{
  DeleteObjectsInContainer(ContentItems);
}

void ContentLibrary::Save()
{
  SaveToDataFile(*this, LibraryFile);
}

bool ContentLibrary::Load()
{
  return LoadFromDataFile(*this, LibraryFile);
}

void ContentLibrary::SaveAllContentItemMeta()
{
  forRange(ContentItem* item, ContentItems.Values())
    item->SaveMetaFile();
}

String ContentLibrary::GetOutputPath()
{
  //to avoid conflicts of same named libraries in the out directory,
  //include the hash as part of the output file name.
  Guid hashOfLibraryIdAndLocation = LibraryId ^ SourcePath.Hash();
  String hexHash = StripHex0x(ToString(hashOfLibraryIdAndLocation));
  String fileName = BuildString(Name, hexHash);
  return FilePath::Combine(Z::gContentSystem->ContentOutputPath, fileName);
}

ContentItem* ContentLibrary::FindContentItemByFileName(StringParam filename)
{
  String fullPath = FilePath::Combine(SourcePath, filename);
  String uniqueId = UniqueFileId(fullPath);
  return ContentItems.FindValue(uniqueId, nullptr);
}

void ContentLibrary::AddContentItem(ContentItem* contentItem)
{
  ContentItems[contentItem->UniqueFileId] = contentItem;
}

bool ContentLibrary::RemoveContentItem(ContentItem* contentItem)
{
  //remove from the map
  ContentItems.Erase(contentItem->UniqueFileId);

  return true;
}

void ContentLibrary::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeNameDefault(LibraryId, ResourceId(0));
  SerializeNameDefault(SourceControlType, String());
  SerializeNameDefault(mReadOnly, false);
}

void ContentLibrary::BuildContent(BuildOptions& buildOptions)
{
  Z::gEngine->LoadingStart();

  //Begin building content
  buildOptions.BuildStatus = BuildStatus::Running;

  int itemsToBuild = ContentItems.Size();
  int itemsBuilt = 0;

  String currentOperation = "Processing";
  String currentTask = this->Name;
  ContentMapType::valuerange itemsToProcess = ContentItems.Values();

  //Continue until finished or canceled
  while(!itemsToProcess.Empty() && buildOptions.BuildStatus == BuildStatus::Running)
  {
    ContentItem* item = itemsToProcess.Front();

    //ZPrint("Building %s\n", item->Filename.c_str());

    if(buildOptions.SendProgress)
    {
      float progress = float(itemsBuilt + 1) / float(itemsToBuild);
      Z::gEngine->LoadingUpdate(currentOperation, currentTask, item->Filename, ProgressType::Normal, progress);
    }

    //Build the content item
    item->BuildContent(buildOptions);

    if(buildOptions.Failure)
    {
      ZPrint("Content Build Failed, %s\n", buildOptions.Message.c_str());
      buildOptions.Failure = false;
      buildOptions.Message = String();
    }
    else
      ++itemsBuilt;

    itemsToProcess.PopFront();
  }

  //if all of the items were built, then the build completed successfully
  if(itemsBuilt == itemsToBuild)
  {
    buildOptions.BuildStatus = BuildStatus::Completed;
  }
  else
  {
    buildOptions.BuildStatus = BuildStatus::Failed;
    String message = String::Format("Failed to build content library '%s'", this->Name.c_str());
    DoNotifyError("Content Library build failed", message);
  }

  Z::gEngine->LoadingFinish();
}

void ContentLibrary::BuildListing(ResourceListing& listing)
{
  forRange(ContentItem* node, ContentItems.Values())
  {
    node->BuildListing(listing);
  }
}

void ContentLibrary::SetPaths(BuildOptions& buildOptions)
{
  buildOptions.SourcePath = this->SourcePath;
  buildOptions.OutputPath = GetOutputPath();
}

}//namespace Zero
