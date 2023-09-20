// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Forward Declarations
struct SearchViewResult;
struct SearchData;
class ScrollArea;
class SearchProvider;
class SearchViewElement;
class TextBox;
class KeyboardEvent;
class SearchView;
class TagChainTextBox;

namespace Events
{
DeclareEvent(SearchCanceled);
DeclareEvent(SearchPreview);
DeclareEvent(SearchCompleted);
DeclareEvent(AlternateSearchCompleted);
} // namespace Events

/// Event Sent by the search view..
class SearchViewEvent : public Event
{
public:
  RaverieDeclareType(SearchViewEvent, TypeCopyMode::ReferenceType);

  SearchView* View;
  SearchViewResult* Element;
};

/// Sent when alternate conditions to a completed search have triggered.
class AlternateSearchCompletedEvent : public Event
{
public:
  RaverieDeclareType(AlternateSearchCompletedEvent, TypeCopyMode::ReferenceType);

  String mSearchText;
};

// Boost priority to make differing result types appear above other result
// types. The higher the range boost value, the higher the sort priority order.
namespace SearchViewResultPriority
{

enum PriorityRange
{
  CommandBegin = 1000,
  LibraryBegin = 2000,
  TagBegin = 3000
};
}

/// Possible search match element
struct SearchViewResult
{
public:
  SearchViewResult() : Data(nullptr)
  {
  }
  // Provider that added this result
  SearchProvider* Interface;
  // Name match Priority
  int Priority;
  // Match User Data
  void* Data;
  // Handle to object
  Handle ObjectHandle;
  // Name of the Match
  String Name;
  // Whether or not the result is considered valid (displayed grayed out if not)
  Status mStatus;
};

class SearchProviderFilter
{
public:
  virtual bool FilterResult(SearchViewResult& result) = 0;
};

template <typename T, bool (T::*Func)(SearchViewResult& result)>
class SearchProviderFilterMethod : public SearchProviderFilter
{
public:
  SearchProviderFilterMethod(T* instance) : mInstance(instance)
  {
  }

  bool FilterResult(SearchViewResult& result) override
  {
    return (mInstance->*Func)(result);
  }

  T* mInstance;
};

/// Interface to providing search elements and execute matching.
class SearchProvider : public EventObject
{
public:
  SearchProvider(StringParam providerType = "TypeInvalid") : mFilter(nullptr), mProviderType(providerType)
  {
  }

  // Virtual Destructor for cleanup
  virtual ~SearchProvider()
  {
  }
  // Collect Search Results
  virtual void Search(SearchData& search)
  {
  }
  // Get an element's display type
  virtual String GetElementType(SearchViewResult& element)
  {
    return String();
  }
  // Get an element's display name
  virtual String GetElementName(SearchViewResult& element)
  {
    return element.Name;
  }
  // Get a small icon to display
  virtual String GetIcon(SearchViewResult& element)
  {
    return String();
  }
  // Used for tags element does not end search
  virtual bool OnMatch(SearchView* searchView, SearchViewResult& element)
  {
    return true;
  }
  // Run auto completed command
  virtual void RunCommand(SearchView* searchView, SearchViewResult& element){};
  // Create a preview widget
  virtual Composite* CreatePreview(Composite* parent, SearchViewResult& element)
  {
    return nullptr;
  }
  // Add a widget to the SearchView's TextBox preview widget stack.
  virtual bool AddToAlternatePreview(SearchData* search, Composite* searchPreviewWidget)
  {
    return false;
  }
  // Execute functionality hinted at the by the SearchPreview ToolTip.
  virtual void AttemptAlternateSearchCompleted()
  {
  }

  virtual bool FilterResult(SearchViewResult& result);

  // Get an element's display name with its provider's display type
  String GetElementNameAndSearchType(SearchViewResult& element)
  {
    ReturnIf(mProviderType == "TypeInvalid", mProviderType, "Provider type must be specified.");
    return BuildString(GetElementName(element), "(", mProviderType, ")");
  }

  template <typename T, bool (T::*Func)(SearchViewResult& result)>
  void SetCallbackFilter(T* instance)
  {
    mFilter = new SearchProviderFilterMethod<T, Func>(instance);
  }

  // Helps differentiate name-collisions on list items.
  String mProviderType;

  // This allows for an extra step that filters what a search provider would
  // normally return.
  SearchProviderFilter* mFilter;
};

/// All data needed to preform a search
struct SearchData
{
  ~SearchData();

  void Search();

  // Output Data
  void AddAvailableTagsToResults();
  void Sort();
  void ClearSearchProviders();

  // Search Providers
  Array<SearchProvider*> SearchProviders;

  // String being searched
  String SearchString;
  // Tags active for this search
  TagList ActiveTags;
  // Meta types active for this search
  Array<BoundType*> ActiveMeta;

  // Tags found in this search
  TagList AvailableTags;
  // Search results
  Array<SearchViewResult> Results;
};

class SearchViewElement : public Composite
{
public:
  RaverieDeclareType(SearchViewElement, TypeCopyMode::ReferenceType);

  SearchViewElement(Composite* parent);
  void OnMouseUp(MouseEvent* event);
  void OnMouseMove(MouseEvent* event);
  void Setup(SearchView* view, uint index, bool selected, SearchViewResult& element);
  void UpdateTransform() override;

  SearchView* mView;
  Text* mName;
  Text* mType;
  Element* mBackground;
  uint mIndex;
};

/// Search view searches for results with given providers.
class SearchView : public Composite
{
public:
  RaverieDeclareType(SearchView, TypeCopyMode::ReferenceType);

  SearchView(Composite* parent);
  ~SearchView();

  /// Start a search with a string
  void Search(StringParam text);
  void OnDestroy() override;

  // Add a tag to the search and clear text.
  void AddTag(StringParam tag, bool removeable = true);
  void AddHiddenTag(StringParam tag);

  void AddMetaType(BoundType* meta);

  bool TakeFocusOverride() override;

  // Active Search Data.
  SearchData* mSearch;

private:
  void UpdateTransform() override;

  WidgetRect GetToolTipRect();
  void PositionToolTip();
  void PositionAlternateToolTip();

  void ResolveVerticalToolTipOverlap();

  void OnSearchDataModified(Event* e);
  void OnSearchKeyPreview(KeyboardEvent* e);
  void ClearToolTips();
  void Canceled();
  void Selected();
  void BuildResults();
  void MoveSelection(int index);
  void SetSelection(int index);
  void OnEnter(ObjectEvent* event);
  void OnKeyPressed(KeyboardEvent* event);
  void OnSearchTipCommand(KeyboardEvent* event);
  void OnScrollUpdated(Event* e);

  // Active Elements
  Array<SearchViewElement*> mElements;
  // Current Preview Object
  HandleOf<ToolTip> mToolTip;
  HandleOf<ToolTip> mAlternateToolTip;
  uint mSelectedIndex;
  // Searching Text Box
  TagChainTextBox* mSearchBar;
  // Results area
  ScrollArea* mArea;
  Element* mBackground;

  // Whether or not we sent out an event saying we completed or failed.
  bool mEventTerminated;

  friend class SearchViewElement;
};

/// Returns true if there are any tags that would reject this entry.
bool CheckTags(HashSet<String>& testTags, BoundType* type);

/// Check tags and if passes add all other tags.
bool CheckAndAddTags(SearchData& search, BoundType* type);

/// Logic for simple filter if the tag is present or there
/// are no active tags add the tag and return true.
bool CheckAndAddSingleTag(SearchData& search, StringParam tag);

// Create a simple text box preview.
Composite* CreateTextPreview(Composite* parent, StringParam text);

/// Returns true if there are any tags that would reject this entry.
template <typename StringContainer>
bool CheckTags(HashSet<String>& testTags, StringContainer& tags)
{
  // No tags always accept
  if (testTags.Empty())
    return true;

  // Tags and no tags on this always false
  if (!testTags.Empty() && tags.Empty())
    return false;

  // There must be no tag that rejects
  // this object
  uint foundTags = 0;

  forRange (String str, tags.All())
  {
    if (testTags.Contains(str))
      ++foundTags;
  }

  return !(testTags.Size() > foundTags);
}

/// Check tags and if passes add all other tags.
template <typename StringContainer>
bool CheckAndAddTags(SearchData& search, StringContainer& tags)
{
  if (CheckTags<StringContainer>(search.ActiveTags, tags))
  {
    forRange (String& tag, tags.All())
      search.AvailableTags.Insert(tag);
    return true;
  }
  else
    return false;
}

} // namespace Raverie
