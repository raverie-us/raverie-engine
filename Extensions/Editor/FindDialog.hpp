///////////////////////////////////////////////////////////////////////////////
///
/// \file FindDialog.hpp
/// Declaration of the FindDialog class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
// Forward declarations
class TextEditor;
class TextButton;
class TextBox;
class TextCheckBox;
class ComboBox;
class Label;
class DocumentEditor;
class ScriptEditor;


DeclareEnum2(EolDirection, Forwards, Backwards);
DeclareEnum2(Direction, Down, Up);

/// A find dialog to find text in documents
class FindTextDialog : public Composite
{
public:
  // Type-defines
  typedef FindTextDialog            ZilchSelf;
  typedef Array<String>             StrArray;
  typedef ContainerSource<StrArray> StrSource;

  // Constructor
  FindTextDialog(Composite* parent);

  // On Take focus
  bool TakeFocusOverride() override;

  // Clear the context
  void ClearContext();

  // Go into default find next mode
  void DefaultFindNextSettings();

  // Go into default find all mode
  void DefaultFindAllSettings();

  // Go into default replace next mode
  void DefaultReplaceNextSettings();

  // Go into default replace all mode
  void DefaultReplaceAllSettings();

  // A pointer to the only instance of the find dialog
  static FindTextDialog* Instance;

private:

  // A helper function for default modes
  void DefaultLookIn();

  // Occurs when we change the search mode
  void SearchModeChanged(ObjectEvent* event);

  // Occurs when we change the character mode
  void CharacterModeChanged(ObjectEvent* event);

  // This function tracks if we press enter/return in either the find/replace text boxes
  void SubmitTextBox(KeyboardEvent* event, TextBox* textBox);
  void SubmitSearchOnFindReturn(KeyboardEvent* event);
  void SubmitSearchOnReplaceReturn(KeyboardEvent* event);

  // Occurs when we click the go button
  void StartSearch(ObjectEvent* event);

  // A search result
  struct SearchResult
  {
    IntrusiveLink(SearchResult, link);

    size_t  Line;
    String  WholeLine;
    size_t  PositionBegin;
    size_t  PositionEnd;
  };

  // Tells us that a given cursor position is not within a region
  static const size_t CursorNotInRegion = (size_t) -1;

  // Defines a region of text and where it comes from
  struct SearchRegion
  {
    IntrusiveLink(SearchRegion, link);

    // Constructor
    SearchRegion();

    String                FileName;
    DocumentEditor*       Editor;
    DocumentResource*     Resource;
    StringRange           RegionText;
    StringRange           WholeText;
    InList<SearchResult>  Results;
    size_t                CursorPos;
  };

  // Get the search regions based on the options given
  bool GetAndValidateSearchRegions();

  // Get the search regions
  bool GetCurrentDocumentRegions();

  // Get the search regions
  bool GetAllOpenDocumentRegions();

  // Get the search regions
  bool GetEntireProjectRegions();

  // Get the search regions
  bool GetCurrentScopeRegions();

  // Get the search regions
  bool GetSelectedTextRegions();

  // Split the region with the cursor position into two
  void SplitActiveCursorRegion();

  // Take the results of the region search and use them
  void ProcessResults();

  // Perform the actual searching, and fill in the context with the results
  void DoSearchAndGetContext();

  // Get the find regex (this may be different than the find text, due to options)
  String GetFindRegex();

  // Advance an input string to the end of a line
  static const char* MoveToEol(StringRange wholeString, const char* currentPosition, EolDirection::Enum direction);

  // Get the entire line as a string range
  static StringRange GetWholeLine(StringRange wholeString, const char* currentPosition);

  // Count the number of lines in a given string range
  static size_t CountLines(StringRange input);

  // Check if a cursor position is valid within a region
  int ValidCursorPos(StringRange regionText, DocumentEditor* editor);

  // Do the replacement given a script editor
  void DoReplacements(DocumentEditor* scriptEditor, SearchResult* searchResult);

  // Get the next node in a list (respects search direction)
  template <typename T>
  typename T::pointer GetNext(T& list)
  {
    // If we're searching downward... (or we don't care about search direction)
    if (mDirection->GetActive() == false || mDirection->GetSelectedItem() == Direction::Down)
    {
      return &list.Front();
    }
    // If we're searching upward...
    else
    {
      return &list.Back();
    }
  }

  // Pop the next node in a list (respects search direction)
  template <typename T>
  void PopNext(T& list)
  {
    // If we're searching downward... (or we don't care about search direction)
    if (mDirection->GetActive() == false || mDirection->GetSelectedItem() == Direction::Down)
    {
      return list.PopFront();
    }
    // If we're searching upward...
    else
    {
      return list.PopBack();
    }
  }


private:
  //Helpers

  // Add current document region
  bool AddCurrentDocument();

  DocumentEditor* GetEditorForRegion(SearchRegion* region);

  // The context stores information so that when we click
  // search again we can continue from our last search
  struct Context
  {
    // Constructor
    Context();

    // Destructor
    ~Context();

    // Members
    InList<SearchRegion>  Regions;
    size_t                TotalResults;
    Regex                 FindRegex;
  };

  // Store the context (null if the context was ever cleared and needs to be rebuilt)
  Context* mContext;

  // Are we in all mode?
  bool mAllMode;

  // Are we in replace mode?
  bool mReplaceMode;

  // What we want to find 
  TextBox* mFind;

  // Store the replace label
  Label* mReplaceLabel;

  // What we want to replace 
  TextBox* mReplace;

  // Do the action
  TextButton* mGo;

  // What search mode are we in? (find next, find all, replace next, replace all...)
  ComboBox* mSearchMode;
  EnumSourceSpaced mSearchModeSource;

  // What files are we looking in?
  ComboBox* mLookIn;
  EnumSourceSpaced mLookInSource;

  // Store the direction label
  Label* mDirectionLabel;

  // The search direction
  ComboBox* mDirection;
  EnumSource mDirectionSource;

  // Whether we're normally searching, searching with extended characters, or using regular expressions
  ComboBox* mCharacterMode;
  StringSource mCharacterModeArray;

  // Store the regex-flavor label
  Label* mRegexFlavorLabel;

  // The flavor of regular expression that we're using (only available when char-mode is regex)
  ComboBox* mRegexFlavor;
  EnumSource mRegexFlavorSource;

  // Match an entire word only
  TextCheckBox* mMatchWholeWord;

  // Match the word only if it matches the case
  TextCheckBox* mMatchCase;

  // Wrap around (in case we hit the end and find nothing)
  TextCheckBox* mWrapAround;
};

/// A dialog for finding an object in the current level
class FindObjectDialog : public Composite
{
public:
  // Type-defines
  typedef FindObjectDialog ZilchSelf;

  // Constructor
  FindObjectDialog(Composite* parent);

  // Occurs when we click the go button
  void StartSearch(ObjectEvent* event);
  void AddCurrentDocument();
private:

  // The find script
  ScriptEditor* mFind;

  // The replace script
  ScriptEditor* mReplace;

  // Do the action
  TextButton* mGo;

  // Do we search root objects only?
  TextCheckBox* mRootObjectsOnly;
};

}//namespace Zero
