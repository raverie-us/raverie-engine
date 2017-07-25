///////////////////////////////////////////////////////////////////////////////
///
/// \file Tags.cpp
/// Implementation of the Tags composite.
/// 
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace TagsUi
{
const cstr cLocation = "EditorUi/Controls/Tags";
Tweakable(Vec4, BackgroundColor,   Vec4(0.65,0.65,0.65, 1), cLocation);
Tweakable(Vec4, SearchIconBgColor, Vec4(1,1,1,1),           cLocation);
Tweakable(Vec4, SearchIconColor,   Vec4(1,1,1,1),           cLocation);
Tweakable(Vec4, SearchIconColorRight,   Vec4(1,1,1,1),      cLocation);
}

namespace Events
{
  DefineEvent(TagDeleted);
  DefineEvent(TagsModified);
  DefineEvent(SearchDataModified);
}

ZilchDefineType(TagEvent, builder, type)
{
}

//-------------------------------------------------------------------------- Tag
//******************************************************************************
TagLabel::TagLabel(Composite* parent, StringParam name, bool removeable) : Composite(parent)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnterBackground);
  ConnectThisTo(this, Events::MouseExit, OnMouseExitBackground);

  mName = new Text(this, cText);
  mName->SetText(name);

  mRemoveable = removeable;

  mDeleteButton = CreateAttached<Element>("RemoveX");
  mDeleteButton->SetActive(removeable);
  ConnectThisTo(mDeleteButton, Events::LeftMouseUp, OnDelete);
}

//******************************************************************************
void TagLabel::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBackground->SetColor(TagsUi::BackgroundColor);
  mName->SetTranslation(Pixels(2,0,0));
  mName->SetSize(mSize - Pixels(16,0));

  mDeleteButton->SetTranslation(Vec3(mSize.x - Pixels(16),Pixels(0),0));
  mDeleteButton->SetSize(Pixels(16,16));
  
  Composite::UpdateTransform();
}

//******************************************************************************
Vec2 TagLabel::GetDesiredSize()
{
  const Vec2 cExtraWidth = Pixels(16, -1);

  // Measure the text
  Vec2 textSize = mName->mFont->MeasureText(mName->GetText(), 1.0f);
 
  return textSize + cExtraWidth;
}

//******************************************************************************
String TagLabel::GetName()
{
  return mName->GetText();
}

//******************************************************************************
void TagLabel::OnMouseEnterBackground(MouseEvent* e)
{
  mBackground->SetColor(Vec4(0.91f, 0.478f, 0.196f, 1));
}

//******************************************************************************
void TagLabel::OnMouseExitBackground(MouseEvent* e)
{
  mBackground->SetColor(TagsUi::BackgroundColor);
}

//******************************************************************************
void TagLabel::OnDelete(MouseEvent* e)
{
  TagEvent tagEvent;
  tagEvent.mTag = this;
  tagEvent.mTagName = mName->GetText();
  GetDispatcher()->Dispatch(Events::TagDeleted, &tagEvent);
}

//----------------------------------------------------------------- TagChainBase
//******************************************************************************
TagChainBase::TagChainBase(Composite* parent) : Composite(parent)
{
  const String cDefinitionSet = "Tags";
  mDefSet = mDefSet->GetDefinitionSet(cDefinitionSet);
  mSorted = false;
}

struct SortTag
{
  bool operator()(TagLabel* left, TagLabel* right)
  {
    // Un-removable items always come first
    // If neither are removable, sort by name
    if(!left->mRemoveable && !right->mRemoveable)
      return left->GetName() < right->GetName();

    if(!left->mRemoveable)
      return true;
    else if(!right->mRemoveable)
      return false;

    // If neither are removable, sort by name
    return left->GetName() < right->GetName();
  }
};

//******************************************************************************
bool TagChainBase::AddTag(StringParam tagName, bool removeable, bool sendsEvents)
{
  if(ContainsTag(tagName))
    return false;

  TagLabel* tag = new TagLabel(this, tagName, removeable);
  ConnectThisTo(tag, Events::TagDeleted, OnTagDeleted);
  mTags.PushBack(tag);
  if(mSorted)
    Sort(mTags.All(), SortTag());
  MarkAsNeedsUpdate();
  GetParent()->MarkAsNeedsUpdate();
  if(sendsEvents)
    Modified();

  return true;
}

//******************************************************************************
void TagChainBase::GetTags(Array<String>& tags, bool includeNonRemoveable)
{
  uint tagCount = mTags.Size();
  tags.Resize(tagCount);
  for(uint i = 0; i < tagCount; ++i)
  {
    TagLabel* tag = mTags[i];
    if(includeNonRemoveable || tag->mRemoveable)
      tags[i] = tag->GetName();
  }
}

//******************************************************************************
void TagChainBase::GetTags(HashSet<String>& tags, bool includeNonRemoveable)
{
  forRange(TagLabel* tag, mTags.All())
  {
    if(includeNonRemoveable || tag->mRemoveable)
      tags.Insert(tag->GetName());
  }
}

//******************************************************************************
void TagChainBase::ClearTags()
{
  forRange(TagLabel* label, mTags.All())
  {
    label->Destroy();
  }

  mTags.Clear();
}

//******************************************************************************
bool TagChainBase::ContainsTag(StringParam tag)
{
  forRange(TagLabel* label, mTags.All())
  {
    if(label->GetName() == tag)
      return true;
  }

  return false;
}

//******************************************************************************
void TagChainBase::OnTagDeleted(TagEvent* e)
{
  mTags.EraseValueError(e->mTag);
  e->mTag->Destroy();
  GetDispatcher()->Dispatch(Events::TagDeleted, e);
  Modified();
}

//******************************************************************************
void TagChainBase::Modified()
{
  Event event;
  DispatchBubble(Events::TagsModified, &event);
}

//--------------------------------------------------------------------- TagChain
//******************************************************************************
TagChain::TagChain(Composite* parent) : TagChainBase(parent)
{
  mBackground = CreateAttached<Element>("TagBoxBackground");
  SetClipping(true);
}

//******************************************************************************
void TagChain::UpdateTransform()
{
  mBackground->SetSize(mSize);

  Vec3 cOffset = Pixels(3,3,0);

  Vec3 currPos = Vec3::cZero;

  for(uint i = 0; i < mTags.Size(); ++i)
  {
    TagLabel* tag = mTags[i];

    Vec2 tagSize = tag->GetDesiredSize();

    if(cOffset.x + currPos.x + tagSize.x > mSize.x)
    {
      currPos.x = 0.0f;
      currPos.y += Pixels(17);
    }

    tag->SetTranslationAndSize(cOffset + currPos, tagSize);

    currPos.x += tagSize.x + Pixels(3);
  }
  Composite::UpdateTransform();
}

//******************************************************************************
float TagChain::GetDesiredHeight()
{
  Vec3 cOffset = Pixels(3,3,0);
  Vec3 currPos = Vec3::cZero;
  for(uint i = 0; i < mTags.Size(); ++i)
  {
    TagLabel* tag = mTags[i];

    Vec2 tagSize = tag->GetDesiredSize();

    if(cOffset.x + currPos.x + tagSize.x > mSize.x)
    {
      currPos.x = 0.0f;
      currPos.y += Pixels(17);
    }

    currPos.x += tagSize.x + Pixels(3);
  }
  return cOffset.y + currPos.y + Pixels(20);
}

//-------------------------------------------------------------- TagChainTextBox
//******************************************************************************
TagChainTextBox::TagChainTextBox(Composite* parent) : TagChainBase(parent)
{
  mSearchBar = new TextBox(this, "IconTextBox");
  mSearchBar->SetEditable(true);
  mSearchBar->mEditTextField->SetClearFocus(false);

  // We want to keep focus when they press 'Enter'
  mSearchBar->mEditTextField->mEnterClearFocus = false;
  mSearchBar->SetHintText("search...");

  mMagnifyingGlassBg = CreateAttached<Element>(cWhiteSquare);
  mMagnifyingGlassBg->SetActive(false);
  mMagnifyingGlass = CreateAttached<Element>("MainSearch");
  ConnectThisTo(mSearchBar, Events::TextChanged, OnSearchBoxChanged);
  ConnectThisTo(mSearchBar, Events::KeyPreview, OnSearchBoxKeyPreview);
  ConnectThisTo(this, Events::TagDeleted, OnTagDeleted);

  mSearchIndex = 0;
  mAddTagsOnEnter = true;

  SetSearchIconSide(SearchIconSide::Left);

  SetClipping(true);
}

//******************************************************************************
void TagChainTextBox::UpdateTransform()
{
  Vec3 currTagPos = Vec3::cZero;

  if(mSearchIconSide == SearchIconSide::Left)
  {
    mMagnifyingGlass->SetTranslation(Pixels(1,1,0));

    mMagnifyingGlassBg->SetVisible(false);

    mSearchBar->SetSize(mSize);

    // Offset the tags by the search icon
    currTagPos = Pixels(28,1,0);
  }
  else
  {
    mMagnifyingGlassBg->SetVisible(true);
    mMagnifyingGlassBg->SetColor(TagsUi::SearchIconBgColor);

    Vec2 iconSize = mMagnifyingGlass->GetSize();
    Vec3 iconPos(mSize.x - iconSize.x, 0, 0);

    mMagnifyingGlass->SetTranslation(iconPos - Pixels(2,0,0));
    mMagnifyingGlassBg->SetTranslation(iconPos - Pixels(2,0,0));

    mSearchBar->SetSize(mSize - Vec2(iconSize.x + Pixels(2), 0));

    currTagPos = Pixels(2,1,0);
  }

  for(uint i = 0; i < mTags.Size(); ++i)
  {
    TagLabel* tag = mTags[i];

    Vec2 tagSize = tag->GetDesiredSize();

    if(currTagPos.x + tagSize.x > mSize.x)
    {
      currTagPos.x = 0.0f;
      currTagPos.y += Pixels(17);
    }

    tag->SetTranslationAndSize(currTagPos, tagSize);

    currTagPos.x += tagSize.x + Pixels(3);
  }

  mSearchBar->mTextOffset = currTagPos.x;

  TagChainBase::UpdateTransform();
}

Vec2 TagChainTextBox::GetMinSize()
{
  return Vec2(20, 20);
}

//******************************************************************************
bool TagChainTextBox::AddTag(StringParam tagName, bool removeable, 
                             bool sendsEvents)
{
  // do not add Root tags to search
  if(tagName == "Root")
  {
    Error("Attempting to add Root Library Data Entry tag to search");
    return false;
  }
  // Attempt to add the tag
  bool added = TagChainBase::AddTag(tagName, removeable, sendsEvents);
  if(added)
  {
    mSearch.ActiveTags.Insert(tagName);
    mSearchBar->SetText(String());
    mSearch.SearchString = String();
    mLastSearchText = String();
    Refresh();
  }

  return added;
}

//******************************************************************************
void TagChainTextBox::ClearSearch()
{
  mSearchBar->SetText(String());
  mSearch.SearchString = String();
  mLastSearchText = String();

  Refresh();
}

//******************************************************************************
void TagChainTextBox::ClearTags()
{
  // Remove all tags from the active search tags
  forRange(TagLabel* label, mTags.All())
  {
    mSearch.ActiveTags.Erase(label->GetName());
  }

  TagChainBase::ClearTags();
}

//******************************************************************************
void TagChainTextBox::Refresh()
{
  mSearch.Results.Clear();
  mSearch.AvailableTags.Clear();

  mSearch.Search();

  mSearch.AddAvailableTagsToResults();
  mSearch.Sort();

  Event event;
  GetDispatcher()->Dispatch(Events::SearchDataModified, &event);
}

//******************************************************************************
bool TagChainTextBox::TakeFocusOverride()
{
  mSearchBar->TakeFocus();
  return true;
}

//******************************************************************************
void TagChainTextBox::SetSearchIconSide(SearchIconSide::Type side)
{
  mSearchIconSide = side;
  MarkAsNeedsUpdate();
}

//******************************************************************************
void TagChainTextBox::OnTagDeleted(TagEvent* e)
{
  mSearch.ActiveTags.Erase(e->mTagName);

  Refresh();
}

//******************************************************************************
void TagChainTextBox::OnSearchBoxChanged(Event* e)
{
  String text = mSearchBar->GetText();

  // Only submit if the last character entered was a space
  if(text.ComputeRuneCount() == 1 && text.Front() == ' ')
  {
    mSearchBar->SetText(String());
    return;
  }

  mSearch.SearchString = text;
  Refresh();

  mLastSearchText = mSearchBar->GetText();
}

//******************************************************************************
void TagChainTextBox::OnSearchBoxKeyPreview(KeyboardEvent* e)
{
  if(e->Key == Keys::Escape)
  {
    ClearTags();
    mSearchBar->SetText(String());
    mSearch.SearchString.Clear();
    mLastSearchText.Clear();
    Modified();
    Refresh();
    e->Handled = true;
  }
  // If backspace is pressed and there are no characters, attempt
  // to delete the last tag in the chain
  else if(e->Key == Keys::Back && mLastSearchText.Empty() && e->State == KeyState::Down)
  {
    if(mTags.Size() > 0)
    {
      TagLabel* tag = mTags.Back();
      String tagName = tag->GetName();
      mSearch.ActiveTags.Erase(tagName);
      mSearchBar->SetText(tagName);
      mSearchBar->mEditTextField->SetEditCaretPos(tagName.ComputeRuneCount());
      mSearch.SearchString = tagName;
      mLastSearchText = tagName;
      mTags.PopBack();
      tag->Destroy();
      Modified();
      Refresh();
      e->Handled = true;
    }
  }
  else if(mAddTagsOnEnter && e->Key == Keys::Enter)
  {
    String text = mSearchBar->GetText();

    if(!mSearch.Results.Empty())
    {
      // I cannot assume the search index is valid until the tile view
      // data source refactor
      uint searchIndex = mSearchIndex;
      if(searchIndex >= mSearch.Results.Size())
        searchIndex = 0;
      SearchViewResult& result = mSearch.Results[searchIndex];

      if(result.Interface->GetType(result) == "Tag")
      {
        AddTag(result.Name, true);
        mSearchBar->SetText(String());
        mSearch.SearchString = String();
        mLastSearchText = String();

        Refresh();
        e->Handled = true;
        mSearchIndex = 0;
      }
    }
  }
}

//-------------------------------------------------------------------- TagEditor
//******************************************************************************
TagEditor::TagEditor(Composite* parent) : Composite(parent)
{
  mTagChain = new TagChain(this);
  mTagChain->SetSize(Pixels(100, 60));
  mTagChain->mSorted = true;

  Spacer* spacer = new Spacer(this);
  spacer->SetSize(Pixels(1,1));

  mTextBox = new TextBox(this);
  mTextBox->SetSize(Pixels(100, 20));
  mTextBox->SetEditable(true);
  mTextBox->mEditTextField->mEnterClearFocus = false;
  mTextBox->SetHintText("Add tags...");
  ConnectThisTo(mTextBox, Events::TextChanged, OnSearchBoxChanged);
  ConnectThisTo(mTextBox, Events::TextEnter, OnSearchBoxSubmitted);
}

const float cTextBoxSize = Pixels(20);

//******************************************************************************
void TagEditor::UpdateTransform()
{
  float tagChainHeight = mSize.y - cTextBoxSize - Pixels(3);
  tagChainHeight = Math::Max(Pixels(20), tagChainHeight);
  mTagChain->SetSize(Vec2(mSize.x, tagChainHeight));

  mTextBox->SetTranslation(Vec3(0, tagChainHeight + Pixels(1), 0));
  mTextBox->SetSize(Vec2(mSize.x, cTextBoxSize));

  Composite::UpdateTransform();
}

//******************************************************************************
float TagEditor::GetDesiredHeight()
{
  return mTagChain->GetDesiredHeight() + cTextBoxSize + Pixels(1);
}

//******************************************************************************
TagChain* TagEditor::GetTagChain()
{
  return mTagChain;
}

//******************************************************************************
void TagEditor::SubmitText(StringParam text)
{
  Status status;
  if(!IsValidName(text, status))
  {
    DoNotify("Invalid Tag", status.Message, "Warning");
    return;
  }

  mTagChain->AddTag(text, true);
  mTextBox->SetText(String());
  Modified();
}

//******************************************************************************
void TagEditor::OnSearchBoxChanged(Event* e)
{
  String text = mTextBox->GetText();

  if(!text.Empty() && text.Back() == ' ')
    SubmitText(text);
}

//******************************************************************************
void TagEditor::OnSearchBoxSubmitted(Event* e)
{
  String text = mTextBox->GetText();

  if(!text.Empty())
    SubmitText(text);
}

//---------------------------------------------------------- Resource Tag Editor
//******************************************************************************
ResourceTagEditor::ResourceTagEditor(Composite* parent)
  : TagEditor(parent)
{
  ConnectThisTo(mTagChain, Events::TagDeleted, OnTagDeleted);
}

//******************************************************************************
void ResourceTagEditor::EditResource(Resource* resource)
{
  Array<Resource*> resources(1);
  resources[0] = resource;
  EditResources(resources);
}

//******************************************************************************
void ResourceTagEditor::EditResources(Array<Resource*>& resources)
{
  mResources.Clear();
  mTagChain->ClearTags();

  // The uint is the count, the bool is whether or not it's removable
  typedef Pair<uint, bool> TagEntry;
  HashMap<String, TagEntry> tagEntries;

  // We want to find tags that are shared by all of the given tags
  forRange(Resource* resource, resources.All())
  {
    // Add the resource
    mResources.PushBack(resource);

    // Get the tags for this resource
    Array<String> coreTags, userTags;
    resource->GetTags(coreTags, userTags);

    // Create core tags
    forRange(String& coreTag, coreTags.All())
    {
      TagEntry* entry = tagEntries.FindPointer(coreTag);
      // Increase the counter if it exists, otherwise Insert it
      if(entry)
        entry->first += 1;
      else
        tagEntries.Insert(coreTag, TagEntry(1, false));
    }

    // Create core tags
    forRange(String& userTag, userTags.All())
    {
      TagEntry* entry = tagEntries.FindPointer(userTag);
      // Increase the counter if it exists, otherwise Insert it
      if(entry)
        entry->first += 1;
      else
        tagEntries.Insert(userTag, TagEntry(1, true));
    }
  }

  typedef HashMap<String, TagEntry>::pair PairType;
  forRange(PairType tagEntry, tagEntries.All())
  {
    TagEntry& entry = tagEntry.second;
    
    if(entry.first >= resources.Size())
      mTagChain->AddTag(tagEntry.first, entry.second, false);
  }
}

//******************************************************************************
void ResourceTagEditor::Modified()
{
  HashSet<String> tags;
  mTagChain->GetTags(tags, false);

  for(uint i = 0; i < mResources.Size(); ++i)
  {
    Resource* resource = mResources[i];

    // The resource could have been deleted
    if(!resource)
      continue;

    resource->mContentItem->SetTags(tags);
    resource->mContentItem->SaveContent();

    ResourceEvent e;
    e.Manager = resource->GetManager();
    e.EventResource = resource;
    resource->GetManager()->DispatchEvent(Events::ResourceTagsModified, &e);
  }
}

//******************************************************************************
void ResourceTagEditor::OnTagDeleted(TagEvent* e)
{
  for(uint i = 0; i < mResources.Size(); ++i)
  {
    Resource* resource = mResources[i];

    // The resource could have been deleted
    if(!resource)
      continue;

    ContentItem* contentItem = resource->mContentItem;
    ContentTags* contentTags = contentItem->has(ContentTags);
    if(contentTags)
    {
      contentTags->mTags.Erase(e->mTagName);
      contentItem->SaveContent();

      ResourceEvent e;
      e.Manager = resource->GetManager();
      e.EventResource = resource;
      resource->GetManager()->DispatchEvent(Events::ResourceTagsModified, &e);
    }
  }
}

}// namespace Zero
