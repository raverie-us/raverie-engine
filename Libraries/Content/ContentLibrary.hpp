// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
class ResourcePackage;

DeclareEnum2(LibraryState, Enumerated, Loaded);

/// A Content Library is a collection of content items. Each
/// content library is represented by an OS folder on disk.
class ContentLibrary : public Object
{
public:
  ZilchDeclareType(ContentLibrary, TypeCopyMode::ReferenceType);

  ContentLibrary();
  ~ContentLibrary();

  // Name of content library.
  String Name;

  // Source path for content library.
  String SourcePath;

  // Library listing file name.
  String LibraryFile;

  // Unique Id for this content library.
  ResourceId LibraryId;

  // Source control type for this content library;
  String SourceControlType;

  // Save content library file.
  // Does not save contained content items meta or data.
  void Save();

  // Save all content files meta files
  void SaveAllContentItemMeta();

  // Load content library file.
  bool Load();

  // Get output path.
  String GetOutputPath();

  // Library Protection
  bool GetReadOnly()
  {
    return mReadOnly;
  }
  bool GetWritable()
  {
    return !mReadOnly;
  }

  // Find a content item by file name (Not the full path).
  ContentItem* FindContentItemByFileName(StringParam filename);

  void Serialize(Serializer& stream);
  void BuildListing(ResourceListing& listing);

  // Resources in this library hashed by unique file id
  typedef HashMap<String, ContentItem*> ContentMapType;

  ContentMapType::valuerange GetContentItems()
  {
    return ContentItems.Values();
  }

private:
  // Internals
  // Add a content item to this library.
  void AddContentItem(ContentItem* contentItem);
  // Remove a content item from this library.
  bool RemoveContentItem(ContentItem* contentItem);

  bool mReadOnly;

  ContentMapType ContentItems;

  friend class ContentItem;
  friend class ContentSystem;
};

} // namespace Zero
