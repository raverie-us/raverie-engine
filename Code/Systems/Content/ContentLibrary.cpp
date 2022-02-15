// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(ContentLibrary, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
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
  forRange (ContentItem* item, ContentItems.Values())
    item->SaveMetaFile();
}

String ContentLibrary::GetOutputPath()
{
  // to avoid conflicts of same named libraries in the out directory,
  // include the hash as part of the output file name.
  Guid hashOfLibraryIdAndLocation = LibraryId ^ (u64)SourcePath.Hash();
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
  // remove from the map
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

void ContentLibrary::BuildListing(ResourceListing& listing)
{
  forRange (ContentItem* node, ContentItems.Values())
  {
    node->BuildListing(listing);
  }
}

} // namespace Zero
