// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class ContentComposition;
class ImportOptions;
class SourceControl;

struct ContentInitializer
{
  String Name;
  String Filename;
  String Extension;
  String BuilderType;
  ContentLibrary* Library;
  ContentComposition* Item;
  ImportOptions* Options;
  ResourceId AddResourceId;
  String Message;
  String ResourceOwner;
  bool Success;
};

typedef ContentItem* (*MakeContentItem)(ContentInitializer& initializer);
typedef ContentItem* (*UpdateContentItem)(ContentItem* existingContent, ContentInitializer& initializer);
typedef ContentComponent* (*MakeContentComponent)();

struct ContentTypeEntry
{
  ContentTypeEntry() : Meta(NULL), MakeItem(NULL), UpdateItem(NULL)
  {
  }
  ContentTypeEntry(BoundType* meta, MakeContentItem make, UpdateContentItem update = nullptr) :
      Meta(meta),
      MakeItem(make),
      UpdateItem(update)
  {
  }
  BoundType* Meta;
  MakeContentItem MakeItem;
  UpdateContentItem UpdateItem;
};

class ContentComponentFactory
{
public:
  ContentComponent* CreateFromName(StringRange name);

  /// Map from component name to component creation function
  typedef HashMap<String, MakeContentComponent> ContentCreatorMapType;
  ContentCreatorMapType Creators;
};

namespace Events
{
DeclareEvent(PackageBuilt);
} // namespace Events

/// See event declaration above this that show when this event is sent.
class ContentSystemEvent : public Event
{
public:
  ZilchDeclareType(ContentSystemEvent, TypeCopyMode::ReferenceType);

  ContentLibrary* mLibrary;
  HandleOf<ResourcePackage> mPackage;
};

DeclareEnum3(ContentFileConflict, Fail, FindNewName, Replace);
DeclareEnum3(ExtenedAddErrors, General, AlreadyExists, NoContentUpdater);

/// Information used to generate content items.
class AddContentItemInfo
{
public:
  AddContentItemInfo()
  {
    Library = NULL;
    Options = NULL;
    OnContentFileConflict = ContentFileConflict::Fail;
    AddResourceId = (ResourceId)0;
  }

  // Library to add the content item to.
  ContentLibrary* Library;
  // Content item Added.
  ContentItem* AddedContentItem;
  // Import Options
  ImportOptions* Options;
  // Local Name of the file to add.
  String FileName;
  // Default name for the builder. If this field is empty,
  // the name will be based on FileName.
  String Name;
  // When multiple builder types can be used for a file type,
  // this overrides what type of builder should be used.
  // For example, png could be texture or sprite.
  String BuilderType;
  // If file is not in the content library folder the full path to the file.
  String ExternalFile;
  // Is the content system allowed to generate a new name?
  ContentFileConflict::Enum OnContentFileConflict;

  // ResourceId to use 0 for new id
  ResourceId AddResourceId;
  HashSet<String> Tags;
  String ResourceOwner;
};

void InitializeContentSystem();
void ShutdownContentSystem();

/// Content system manages content libraries and content building.
class ContentSystem : public ExplicitSingleton<ContentSystem, EventObject>
{
public:
  ZilchDeclareType(ContentSystem, TypeCopyMode::ReferenceType);

  ContentSystem();
  ~ContentSystem();

  /// Scan all LibrarySearchPaths for libraries.
  void EnumerateLibraries();

  /// Building

  /// Load or Create a library of the given name from the specified directory.
  ContentLibrary* LibraryFromDirectory(Status& status, StringParam name, StringParam directory);

  /// Build the Content Library into a Resource Package.
  HandleOf<ResourcePackage> BuildLibrary(Status& status, ContentLibrary* library, bool sendEvent);

  /// Build ContentItems into Resource Package.
  HandleOf<ResourcePackage>
  BuildContentItems(Status& status, ContentItemArray& toBuild, ContentLibrary* library, bool useJobs);

  /// Build Individual ContentItems into Resource Package.
  HandleOf<ResourcePackage> BuildSingleContentItem(Status& status, ContentItem* contentItem);

  // Content item management

  /// Add a new ContentItem to the given library. See AddContentItemInfo for
  /// details.
  ContentItem* AddContentItemToLibrary(Status& status, AddContentItemInfo& addContent);

  /// Remove the ContentItem from it library.
  bool RemoveContentItemFromLibray(ContentItem* contentItem);

  /// Rename a ContentItem's file.
  bool RenameContentItemFile(ContentItem* contentItem, StringParam newFileName);

  String GetHistoryPath(ContentLibrary* library);

  // Find a content item by file name (Not the full path).
  ContentItem* FindContentItemByFileName(StringParam filename);

  // Internals
  ContentItem* CreateFromName(StringRange name);
  void EnumerateLibrariesInPath(StringParam path);

  ContentComponentFactory ComponentFactory;

  typedef HashMap<String, ContentTypeEntry> ContentCreatorMapType;
  ContentCreatorMapType Creators;
  ContentCreatorMapType CreatorsByExtension;

  /// The extension types to ignore (such as meta).
  HashSet<String> IgnoredExtensions;
  /// Map of component names to meta types.
  typedef HashMap<String, BoundType*> ContentMetaMap;
  ContentMetaMap ContentComponents;

  /// Map of content types to load order (default value is 10)
  typedef HashMap<String, uint> LoadOrderMapType;
  LoadOrderMapType LoadOrderMap;

  /// Map of library names to the library itself.
  /// The name is not the folder name, but the name of the library itself.
  typedef HashMap<String, ContentLibrary*> ContentLibraryMapType;
  ContentLibraryMapType Libraries;

  // Map of active ContentItems for safe handles
  typedef HashMap<ContentItemId, ContentItem*> ContentItemMap;
  ContentItemMap LoadedItems;
  ContentItemId mIdCount;

  /// Where to move deleted content to.
  String HistoryPath;
  bool mHistoryEnabled;

  // These paths are set by the editor
  Array<String> LibrarySearchPaths;
  String ContentOutputPath;
  String PrebuiltContentPath;
  /// Where the tools (curl, crash handler, etc) are located
  String ToolPath;

  HashSet<ContentItemId> mModifiedContentItems;

private:
};

namespace Z
{
extern ContentSystem* gContentSystem;
} // namespace Z

} // namespace Zero
