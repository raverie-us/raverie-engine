///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentPackage.cpp
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ContentPackageListing::~ContentPackageListing()
{
  DeleteObjectsInContainer(SortedEntries);
}

void ContentPackageListing::AddEntry(ContentPackageEntry* entry)
{
  Entries.Insert(entry->File, entry);
  SortedEntries.PushBack(entry);
}

void BuildContentPackageListingFromLibrary(ContentPackageListing& listing, ContentLibrary* library)
{
  // Set location to library folder
  listing.Location = library->SourcePath;

  // Add a content items in library
  forRange(ContentItem* item, library->GetContentItems())
  {
    String filename = item->GetFullPath();
    
    ContentPackageEntry* entry = new ContentPackageEntry();
    entry->Active = false;
    entry->File = item->Filename;
    entry->Size = GetFileSize(filename);
    entry->Conflicted = false;
    listing.AddEntry(entry);
  }
}

const String metaExt = "meta";

void LoadContentPackageListing(ContentPackageListing& listing, StringParam filename)
{
  // Open up the read only the header information
  Archive archive(ArchiveMode::Decompressing);
  archive.ReadZipFile(ArchiveReadFlags::Entries, filename);

  // For every file in the archive
  forRange(ArchiveEntry& archiveEntry, archive.GetEntries())
  {
    StringRange entryName = archiveEntry.Name;
    // Meta files are in the library but not content items
    if(!(FilePath::GetExtension(entryName) == metaExt))
    {
      bool conflicted = Z::gContentSystem->FindContentItemByFileName(entryName) != nullptr;

      // Only active by default if it's not conflicted
      bool active = !conflicted;

      // Add the content item
      ContentPackageEntry* entry = new ContentPackageEntry();
      entry->Active = active;
      entry->File = archiveEntry.Name;
      entry->Size = archiveEntry.Full.Size;
      entry->Conflicted = conflicted;
      listing.AddEntry(entry);
    }
  }
}

void ExportContentPackageListing(ContentPackageListing& listing, StringParam filename)
{
  // Start compressing files to an archive
  Archive archive(ArchiveMode::Compressing);

  forRange(ContentPackageEntry* entry, listing.Entries.Values())
  {
    if(entry->Active)
    {
      String filename = entry->File;
      String metaFilename = BuildString(filename, ".meta");

      // Add the file
      String fullPath = FilePath::Combine(listing.Location, filename);
      archive.AddFile(fullPath, filename);

      // Add the meta file
      String metaFullPath = FilePath::Combine(listing.Location, metaFilename);
      archive.AddFile(metaFullPath, metaFilename);
    }
  }

  // Write out the zip file
  archive.WriteZipFile(filename);
}

void ImportContentPackageListing(ContentPackageListing& listing, ContentLibrary* library, StringParam filename)
{
  // Reopen file so we can read entries
  File archiveFile;
  archiveFile.Open(filename.c_str(), FileMode::Read, FileAccessPattern::Sequential);

  // Open the archive and read entries
  Archive archive(ArchiveMode::Decompressing);
  archive.ReadZip(ArchiveReadFlags::Entries, archiveFile);

  // Copy them directly to the content library folder
  String destFolder = library->SourcePath;
  Array<ContentItem*> newContent;

  forRange(ArchiveEntry& archiveEntry, archive.GetEntries())
  {
    ContentPackageEntry* entry = listing.Entries.FindValue(archiveEntry.Name, nullptr);

    if(entry == nullptr || !entry->Active)
      continue;

    // Extract file and meta file 
    String fileName = entry->File;
    String metaFileName = BuildString(fileName, ".meta");

    String destFile = FilePath::Combine(destFolder, fileName);
    String metaFile = FilePath::Combine(destFolder, metaFileName);

    // Extract the files from the file object
    archive.Extract(archiveFile, fileName, destFile);
    archive.Extract(archiveFile, metaFileName, metaFile);

    // Attempt to add it to the content library
    AddContentItemInfo addContent;
    addContent.FileName = fileName;
    addContent.Library = library;
    addContent.Tags = listing.Tags;
    addContent.OnContentFileConflict = ContentFileConflict::Replace;

    Status status;
    ContentItem* newContentItem = Z::gContentSystem->AddContentItemToLibrary(status, addContent);
    DoNotifyStatus(status);

    // If the add succeeded store for loading
    if(status.Succeeded())
      newContent.PushBack(newContentItem);
    else
      DoNotifyStatus(status); 
  }

  // Build all new content items
  ResourceLibrary* resourceLibrary = Z::gResources->GetResourceLibrary(library->Name);

  ResourcePackage package;
  Status status;
  Z::gContentSystem->BuildContentItems(status, newContent, package);
  DoNotifyStatus(status);

  // Load all resource generated into the active resource library
  Z::gResources->ReloadPackage(resourceLibrary, &package);

}

}//namespace Zero
