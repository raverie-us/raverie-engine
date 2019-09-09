// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
/// Component to store the copyright info of a content item.
class ContentCopyright : public ContentComponent
{
public:
  ZilchDeclareType(ContentCopyright, TypeCopyMode::ReferenceType);

  String Owner;
  String Date;

  void Serialize(Serializer& stream);
};

/// Stores the history in mercurial of this content item.
class ContentHistory : public ContentComponent
{
public:
  ZilchDeclareType(ContentHistory, TypeCopyMode::ReferenceType);

  Array<Revision> mRevisions;

  void Initialize(ContentComposition* item);
};

/// Stores any user notes about the content item.
class ContentNotes : public ContentComponent
{
public:
  ZilchDeclareType(ContentNotes, TypeCopyMode::ReferenceType);

  String Notes;

  void Serialize(Serializer& stream);
};

/// Stores any options for the content item that will effect the item's
/// uses in editor vs in game.
class ContentEditorOptions : public ContentComponent
{
public:
  ZilchDeclareType(ContentEditorOptions, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;
  void Initialize(ContentComposition* item) override;

  bool mShowInEditor;
};

/// When added
class ResourceTemplate : public ContentComponent
{
public:
  ZilchDeclareType(ResourceTemplate, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream);

  String mDisplayName;
  String mDescription;
  uint mSortWeight;
  String mCategory;
  uint mCategorySortWeight;
};
} // namespace Zero
