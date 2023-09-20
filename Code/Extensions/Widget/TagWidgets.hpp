// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
namespace Events
{
DeclareEvent(TagDeleted);
DeclareEvent(TagsModified);
DeclareEvent(SearchDataModified);
} // namespace Events

class TagEvent : public Event
{
public:
  RaverieDeclareType(TagEvent, TypeCopyMode::ReferenceType);
  TagLabel* mTag;
  String mTagName;
};

class TagLabel : public Composite
{
public:
  typedef TagLabel self_type;
  typedef TagLabel RaverieSelf;

  /// Constructor.
  TagLabel(Composite* parent, StringParam name, bool removeable);

  /// Widget interface.
  void UpdateTransform() override;

  Vec2 GetDesiredSize();

  String GetName();

  bool mRemoveable;

private:
  /// Event Response.
  void OnMouseEnterBackground(MouseEvent* e);
  void OnMouseExitBackground(MouseEvent* e);
  void OnDelete(MouseEvent* e);

  Text* mName;
  Element* mDeleteButton;
  Element* mBackground;
};

class TagChainBase : public Composite
{
public:
  typedef TagChainBase RaverieSelf;

  /// Constructor.
  TagChainBase(Composite* parent);

  virtual bool AddTag(StringParam tagName, bool removeable, bool sendsEvents = true);
  void GetTags(Array<String>& tags, bool includeNonRemoveable = true);
  void GetTags(HashSet<String>& tags, bool includeNonRemoveable = true);
  void ClearTags();

  /// Returns whether or not the given tag is contained in this tag chain.
  bool ContainsTag(StringParam tag);

  bool mSorted;

protected:
  void OnTagDeleted(TagEvent* e);
  void Modified();

  Array<TagLabel*> mTags;
};

class TagChain : public TagChainBase
{
public:
  typedef TagChain RaverieSelf;

  /// Constructor.
  TagChain(Composite* parent);

  /// Widget interface.
  void UpdateTransform() override;

  float GetDesiredHeight(Vec2Param& sizeConstraint);

private:
  Element* mBackground;
  Vec2 mLastSize;
};

DeclareEnum2(SearchIconSide, Left, Right);

class TagChainTextBox : public TagChainBase
{
public:
  typedef TagChainTextBox RaverieSelf;

  /// Constructor.
  TagChainTextBox(Composite* parent);

  /// Widget interface.
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  bool AddTag(StringParam tagName, bool removeable, bool sendsEvents = true) override;
  void ClearSearch();
  void ClearTags();

  void Refresh();
  bool TakeFocusOverride() override;

  String GetText()
  {
    return mSearchBar->GetText();
  }

  void SetSearchIconSide(SearchIconSide::Type side);

  /// Active Search Data.
  SearchData mSearch;
  uint mSearchIndex;

  bool mAddTagsOnEnter;

  TextBox* mSearchBar;

private:
  void OnTagDeleted(TagEvent* e);

  void OnSearchBoxChanged(Event* e);
  void OnSearchBoxKeyPreview(KeyboardEvent* e);

  SearchIconSide::Type mSearchIconSide;
  Element* mMagnifyingGlassBg;
  Element* mMagnifyingGlass;
  String mLastSearchText;

  /// Results area.
  ScrollArea* mArea;
};

class TagEditor : public Composite
{
public:
  typedef TagEditor RaverieSelf;

  /// Constructor.
  TagEditor(Composite* parent);

  /// Widget interface.
  void UpdateTransform() override;
  Vec2 Measure(LayoutArea& data) override;

  float GetDesiredHeight(Vec2Param& sizeConstraint);

  TagChain* GetTagChain();

  float GetAxisSize(SizeAxis::Enum axis);
  void SetAxisSize(SizeAxis::Enum axis, float size);

  void SetIsAnimating(bool state);

protected:
  /// Validates the given text as a tag and adds it.
  void SubmitText(StringParam text);

  virtual void Modified()
  {
  }

  /// Event response.
  void OnSearchBoxChanged(Event* e);
  void OnSearchBoxSubmitted(Event* e);

  bool mIsAnimating;
  TagChain* mTagChain;
  TextBox* mTextBox;
  IconButton* mAddButton;
};

class ResourceTagEditor : public TagEditor
{
public:
  typedef ResourceTagEditor RaverieSelf;

  /// Constructor.
  ResourceTagEditor(Composite* parent);

  /// Edits the given content item.
  void EditResource(Resource* resource);
  void EditResources(Array<Resource*>& resources);

  void CleanTagEditor();

private:
  /// Saves the content item.
  void Modified();
  void OnTagDeleted(TagEvent* e);

  Array<HandleOf<Resource>> mResources;
};

} // namespace Raverie
