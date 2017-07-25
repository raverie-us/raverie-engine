///////////////////////////////////////////////////////////////////////////////
///
/// \file FindDialog.cpp
/// Implementation of the FindDialog class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

// Defines
#define inifinite_loop for(;;)

/***********************************************************************\
  *- A big note to anyone who reads this file: -*

  The search context is something that could use refactoring, since it
  no longer needs to be a member variable (or allocated for that matter)
  and instead should be something that's on the stack and passed through
  to all the functions. This is because we no longer maintain the context
  through searches, but rather we let the cursor position in the open
  document tell us where next to search.

  This also means that all ClearContext cases can be taken out, even the
  ones used where we check for modified documents.

  On that same note, after considering that context should be a stack
  variable, I then thought that pushing CharEnd and CharBegin forward
  after a replace was no longer necessary. This is not true since
  ReplaceAll still relies on that behavior. (note to self)

  Yet another consideration, we might actually want to keep context
  around since search in selection may rely on it to work. Search in
  selection is currently disabled for this exact reason.
\***********************************************************************/

namespace Zero
{
// A pointer to the only instance of the find dialog
FindTextDialog* FindTextDialog::Instance = NULL;

// Bit fields
static const size_t AllBit = 1;
static const size_t ReplaceBit = 2;

// These enums correspond with combo box options on the find dialog
DeclareEnum4(SearchMode, FindNext, FindAll, ReplaceNext, ReplaceAll);
DeclareEnum3(CharacterMode, Normal, Extended, Regex);
//DeclareEnum4(LookIn, CurrentDocument, AllOpenDocuments, EntireProject, SelectedText);
DeclareEnum4(LookIn, CurrentDocument, AllOpenDocuments, EntireProject, CurrentScope);

// Constructor
FindTextDialog::FindTextDialog(Composite* parent) : Composite(parent)
{
  // Finally, set this to be a stacking layout
  this->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2(0, 2), Thickness::cZero));

  // Looks better at least 250 pixels wide
  mMinSize = Pixels(250, 352);

  // Set the instance pointer
  Instance = this;

  // By default, we have no context
  mContext = NULL;

  // The search modes that we're in
  mAllMode = false;
  mReplaceMode = false;

  // Create a search mode combo box
  new Label(this, cText, "Search Mode:");
  mSearchMode       = new ComboBox(this);
  mSearchModeSource.SetEnum(SearchMode::Names);
  mSearchMode->SetListSource(&mSearchModeSource);
  ConnectThisTo(mSearchMode, Events::ItemSelected, SearchModeChanged);

  // Create the find label and text box
  new Label(this, cText, "Find what: ");
  mFind = new TextBox(this);
  mFind->SetEditable(true);
  ConnectThisTo(mFind, Events::KeyPreview, SubmitSearchOnFindReturn);

  // Create the replace label and text box
  mReplaceLabel = new Label(this, cText, "Replace with: ");
  mReplace = new TextBox(this);
  mReplace->SetEditable(true);
  ConnectThisTo(mReplace, Events::KeyPreview, SubmitSearchOnReplaceReturn);

  // Create the 'go' button that does the searching / replacing
  (new Composite(this))->SetSize(Vec2(0, 2));
  mGo = new TextButton(this);
  mGo->SetText("Go");
  ConnectThisTo(mGo, Events::ButtonPressed, StartSearch);

  // Create a look-in combo box
  new Label(this, cText, "Look in:");
  mLookIn       = new ComboBox(this);
  mLookInSource.SetEnum(LookIn::Names);
  mLookIn->SetListSource(&mLookInSource);

  // Create a direction combo box
  mDirectionLabel  = new Label(this, cText, "Direction:");
  mDirection       = new ComboBox(this);
  mDirectionSource.SetEnum(Direction::Names);
  mDirection->SetListSource(&mDirectionSource);

  // Populate the char-mode array
  mCharacterModeArray.Strings.PushBack("Normal");
  mCharacterModeArray.Strings.PushBack("Extended (\\r, \\n, \\t, \\0, \\x, wildcard*)");
  mCharacterModeArray.Strings.PushBack("Regular Expressions");

  // Create a char-mode combo box
  new Label(this, cText, "Character Mode:");
  mCharacterMode       = new ComboBox(this);
  mCharacterMode->SetListSource(&mCharacterModeArray);
  ConnectThisTo(mCharacterMode, Events::ItemSelected, CharacterModeChanged);

  // Create a regex-flavor combo box
  mRegexFlavorLabel = new Label(this, cText, "Regex Flavor:");
  mRegexFlavor      = new ComboBox(this);
  mRegexFlavorSource.SetEnum(RegexFlavor::Names);
  mRegexFlavor->SetListSource(&mRegexFlavorSource);
  mRegexFlavor->SetActive(false);
  mRegexFlavorLabel->SetActive(false);

  // Create a checkbox for 'match whole word'
  mMatchWholeWord = new TextCheckBox(this);
  mMatchWholeWord->SetText("Match Whole Word");

  // Create a checkbox for 'match case'
  mMatchCase = new TextCheckBox(this);
  mMatchCase->SetText("Match Case");

  // Create a checkbox for 'wrap around' and make it defaulted on
  mWrapAround = new TextCheckBox(this);
  mWrapAround->SetText("Wrap Around");

  // Set all the selected items (we do this after everything is done that way 
  // messages will be handled properly)
  mWrapAround->SetChecked(true);
  mSearchMode->SetSelectedItem(SearchMode::FindNext, true);
  mLookIn->SetSelectedItem(LookIn::CurrentDocument, true);
  mDirection->SetSelectedItem(Direction::Down, true);
  mCharacterMode->SetSelectedItem(CharacterMode::Normal, true);
  mRegexFlavor->SetSelectedItem(RegexFlavor::EcmaScript, true);
}

// Go into default find next mode
void FindTextDialog::DefaultFindNextSettings()
{
  mSearchMode->SetSelectedItem(SearchMode::FindNext, true);
  mLookIn->SetSelectedItem(LookIn::CurrentDocument, true);
  DefaultLookIn();
}

// Go into default find all mode
void FindTextDialog::DefaultFindAllSettings()
{
  mSearchMode->SetSelectedItem(SearchMode::FindAll, true);
  mLookIn->SetSelectedItem(LookIn::EntireProject, true);
  DefaultLookIn();
}

// Go into default replace next mode
void FindTextDialog::DefaultReplaceNextSettings()
{
  mSearchMode->SetSelectedItem(SearchMode::ReplaceNext, true);
  DefaultLookIn();
}

// Go into default replace all mode
void FindTextDialog::DefaultReplaceAllSettings()
{
  mSearchMode->SetSelectedItem(SearchMode::ReplaceAll, true);
  DefaultLookIn();
}

// A helper function for default modes
void FindTextDialog::DefaultLookIn()
{
  DocumentManager* docManager = DocumentManager::GetInstance();

  // If we have no 'current/open' script document...
  if (docManager->CurrentEditor==NULL)
  {
    // Select the entire project as our look in space
    mLookIn->SetSelectedItem(LookIn::EntireProject, true);
  }
  // Otherwise, if we do have a currently open document...
  else
  {
    // Get the selected text from the document and put it in the find window
    String selectedText = docManager->CurrentEditor->GetSelectedText();
    if(!selectedText.Empty())
      mFind->SetText(docManager->CurrentEditor->GetSelectedText());
  }

  // Set focus on the find text
  mFind->TakeFocus();
}

// Occurs when we change the character mode
void FindTextDialog::CharacterModeChanged(ObjectEvent* event)
{
  // Get whether we're in regular expression mode...
  bool inRegex = mCharacterMode->GetSelectedItem() == CharacterMode::Regex;

  // Show the regex flavor combo box if we're in regular expression mode...
  mRegexFlavor->SetActive(inRegex);
  mRegexFlavorLabel->SetActive(inRegex);
}

// Occurs when we change the search mode
void FindTextDialog::SearchModeChanged(ObjectEvent* event)
{
  // Get the selected index
  size_t searchMode = (size_t)mSearchMode->GetSelectedItem();
 
  // Are we in 'all' mode
  mAllMode = (searchMode & AllBit) != 0;

  // Are we in 'replace' mode
  mReplaceMode = (searchMode & ReplaceBit) != 0;

  // Show or hide replacements
  mReplaceLabel->SetActive(mReplaceMode);
  mReplace->SetActive(mReplaceMode);

  // Show or hide direction and word wrap
  mDirection->SetActive(!mAllMode);
  mDirectionLabel->SetActive(!mAllMode);
  mWrapAround->SetActive(!mAllMode);
}

// Get the search regions based on the options given
bool FindTextDialog::GetAndValidateSearchRegions()
{
  // Make sure we're actually trying to find text
  if (mFind->GetText().SizeInBytes() == 0)
  {
    // Show a warning and return that we failed
    DoNotifyWarning("Find/Replace Warning", "The find text was left empty");
    return false;
  }

  // Get the current 'look in' mode
  size_t lookInMode = (size_t)mLookIn->GetSelectedItem();

  // Are the search regions valid
  bool valid = false;

  // Based on the 'look in' mode...
  switch (lookInMode)
  {
    case LookIn::CurrentDocument:
      valid = GetCurrentDocumentRegions();
      break;

    case LookIn::AllOpenDocuments:
      valid = GetAllOpenDocumentRegions();
      break;

    case LookIn::EntireProject:
      valid = GetEntireProjectRegions();
      break;

    case LookIn::CurrentScope:
      valid = GetCurrentScopeRegions();
      break;

    //case LookIn::SelectedText:
    //  valid = GetSelectedTextRegions();
    //  break;
  }

  // Only split the regions if we can even word wrap
  //if (mWrapAround->GetActive())
  {
    // Split the active region, if we have one
    SplitActiveCursorRegion();
  }
  
  // Return if the search regions were valid
  return valid;
}

// Split the region with the cursor position into two
void FindTextDialog::SplitActiveCursorRegion()
{
  // Get a range of all the regions
  InList<SearchRegion>::range regions = mContext->Regions.All();

  // Make a range of all passed regions
  InList<SearchRegion>::range passedRegions = mContext->Regions.All();

  // Loop through all the regions
  while (regions.Empty() == false)
  {
    // Get the current region
    SearchRegion* region = &regions.Front();

    // Iterate to the next region
    regions.PopFront();

    // Set the range of values we've passed up
    passedRegions.end = region;

    // If the cursor is inside the current region
    if (region->CursorPos != CursorNotInRegion)
    {
      // Move all the regions to the end
      if (passedRegions.Empty() == false)
      {
        mContext->Regions.Splice(mContext->Regions.End(), passedRegions);
      }

      // Get the two split string ranges
      StringRange topSplit(region->RegionText.Begin(), region->RegionText.Begin() + region->CursorPos);
      StringRange bottomSplit(region->RegionText.Begin() + region->CursorPos, region->RegionText.End());
    
      // Create the next region that we're going to split
      SearchRegion* topRegion = region;
      SearchRegion* bottomRegion = new SearchRegion();

      // Setup the top and bottom regions
      topRegion->CursorPos      = CursorNotInRegion;
      topRegion->RegionText     = topSplit;
      bottomRegion->RegionText  = bottomSplit;

      //Copy core data
      bottomRegion->WholeText   = topRegion->WholeText;
      bottomRegion->Resource    = topRegion->Resource;
      bottomRegion->Editor      = topRegion->Editor;
      bottomRegion->FileName    = topRegion->FileName;

      // In down mode, we know that the process order is from front to back
      // In up mode, we know that the process order is from back to front
      // Therefore, if we want to search the bottom first in down mode, then we should push it in the array before
      // the top, and likewise, if we want to search the top first in up mode, it should come after the bottom
      // This supports wrap-around mode
      mContext->Regions.InsertBefore(topRegion, bottomRegion);

      // If we're in wrap around mode...
      if (mWrapAround->GetChecked())
      {
        // Unlink the top region, and move it to the end of the list
        // That way, in down mode we search the bottom first
        // and in top mode, we search the top first (since it's reverse)
        mContext->Regions.Erase(topRegion);
        mContext->Regions.PushBack(topRegion);
      }
      // Otherwise, we're not in wrap around mode...
      else
      {
        // If we're searching downward...
        if (mDirection->GetSelectedItem() == Direction::Down)
        {
          // Remove the top
          mContext->Regions.Erase(topRegion);
          delete topRegion;
        }
        // If we're searching upward...
        else
        {
          // Remove the bottom
          mContext->Regions.Erase(bottomRegion);
          delete bottomRegion;
        }
      }
      
      // We split the region, so return out now
      return;
    }
  }
}

// Check if a cursor position is valid within a region
int FindTextDialog::ValidCursorPos(StringRange regionText, DocumentEditor* editor)
{
  // Store the position of the cursor (relative to the start of the region)
  int cursorPos;

  // Get all of the text
  StringRange all = editor->GetAllText();

  // Get the region offset
  int regionOffset = regionText.Data() - all.Data();
  
  // If we're searching downward... (or we don't care about search direction)
  if (mDirection->GetActive() == false || mDirection->GetSelectedItem() == Direction::Down)
  {
    cursorPos = editor->GetSelectionEnd();
  }
  else
  {
    cursorPos = editor->GetSelectionStart();
  }

  // Put the cursor position back into region space
  cursorPos -= regionOffset;

  // If the cursor position is within the bounds of the region
  if (cursorPos >= 0 && cursorPos <= (int)regionText.SizeInBytes())
  {
    // Return the cursor position
    return cursorPos;
  }

  // Otherwise, return that the cursor is not within the region
  return CursorNotInRegion;
}

// Get the search regions
bool FindTextDialog::GetCurrentDocumentRegions()
{
  if(AddCurrentDocument())
  {
    return true;
  }
  else
  {
    // Show an warning and return that we failed
    DoNotifyWarning("Find/Replace Warning", "No document is in focus. Click inside the document to search in it.");
    return false;
  }
}

bool FindTextDialog::AddCurrentDocument()
{
  DocumentManager* docManager = DocumentManager::GetInstance();

  // If we have a current document... otherwise do nothing
  DocumentEditor* editor = docManager->CurrentEditor;
  if(editor)
  {
    // Create a single region
    SearchRegion* region = new SearchRegion();
    region->Editor = editor;
    region->WholeText = editor->GetAllText();
    region->RegionText = region->WholeText;
    region->Resource = editor->GetResource();
    region->CursorPos = ValidCursorPos(region->RegionText, editor);
    mContext->Regions.PushBack(region);
    return true;
  }
  else
  {
    return false;
  }
}

// Get the search regions
bool FindTextDialog::GetAllOpenDocumentRegions()
{
  DocumentManager* docManager = DocumentManager::GetInstance();
  
  DocumentEditor* currentEditor = docManager->CurrentEditor;

  // Get a range of all the document editors
  PodArray<DocumentEditor*>::range range = docManager->Instances.All();

  // Loop through all the instances of script editors
  while (!range.Empty())
  {
    // Get the current document editor
    DocumentEditor* editor = range.Front();

    // Iterate to the next instance
    range.PopFront();

    // If this is the current open document, skip past it since we already added it
    if (editor == currentEditor)
    {
      AddCurrentDocument();
      continue;
    }

    // So long as we have a document and it has a resource...
    if (editor->GetDocument() != NULL)
    {
      // Create a region for this resource
      SearchRegion* region = new SearchRegion();

      // Set the region's resource
      region->Resource = editor->GetDocument()->GetResource();

      // Store the editor
      region->Editor = editor;

      // Point the range at the document text
      region->WholeText = editor->GetAllText();
      region->RegionText = region->WholeText;

      // Output this region
      mContext->Regions.PushBack(region);
    }
  }

  // If no valid documents were open
  if (mContext->Regions.Empty())
  {
    // Show an warning and return that we failed
    DoNotifyWarning("Find/Replace Warning", "No documents are open to be searched.");
    return false;
  }

  // Otherwise, we must have found open documents!
  return true;
}

// Get the search regions
bool FindTextDialog::GetEntireProjectRegions()
{
  DocumentManager* docManager = DocumentManager::GetInstance();

  // Get the current editor (or NULL if we don't have one)
  DocumentEditor* currentEditor = docManager->CurrentEditor;

  // Get a range of all the document resources
  ResourceSystem* resourceSystem = Z::gResources;
  ResourceSystem::TextResourceMap::valuerange range = resourceSystem->TextResources.Values();

  // Loop through all the instances of document resources
  for(;!range.Empty();range.PopFront())
  {
    // Get the current resource
    ResourceId id = range.Front();
    DocumentResource* current = (DocumentResource*)Z::gResources->GetResource(id);

    // Resource has been erased
    if (current == NULL)
      continue;

    // If this is the current open document, skip past it since we already added it
    if (currentEditor && currentEditor->GetResource() == current)
    {
      AddCurrentDocument();
      continue;
    }

    // Get the file name
    String fileName = current->mContentItem->GetFullPath();

    // Create a region for this resource
    SearchRegion* region = new SearchRegion();

    // Set the region's resource
    region->Resource = current;

    // Is their a document with an editor for this resource?
    Document* document = docManager->Documents.FindValue((u64)current->mResourceId, NULL);
    if(document && document->mEditor)
    {
      region->WholeText = document->mEditor->GetAllText();
      region->Editor = document->mEditor;
    }
    else
    {
      region->WholeText = current->LoadTextData();
    }

    region->RegionText = region->WholeText;

    // Output this region
    mContext->Regions.PushBack(region);
  }

  // If no valid script files exist in the project
  if (mContext->Regions.Empty())
  {
    // Show an warning and return that we failed
    DoNotifyWarning("Find/Replace Warning", "No script files exist in the project.");
    return false;
  }

  // Otherwise, we must have found script resources
  return true;
}

struct WhitespaceInfo
{
  size_t  LeadingWhitespace;
  bool    IsAllWhitespace;
};

// Get the number of leading whitespace characters
WhitespaceInfo GetLeadingWhiteSpaceCount(StringRange line)
{
  // Store the result
  WhitespaceInfo info;

  // Store the number of whitespace characters
  info.LeadingWhitespace = 0;

  // Assume that the entire lines is all whitespace
  info.IsAllWhitespace = true;

  // Count the amount of space on the current line
  while (line.Empty() == false)
  {
    // Get the current character
    Rune current = line.Front();

    // If the current character is not an indent space then break out
    if (current != ' ' && current != '\t')
    {
      // Even though it's not an indent space, it still could be a space (newline, etc)
      if (!IsSpace(current))
      {
        // We hit a real character, so the entire line is not whitespace
        info.IsAllWhitespace = false;
      }
      break;
    }

    // Otherwise, increase the white space count
    ++info.LeadingWhitespace;

    // Move to the next character
    line.PopFront();
  }

  // Return the white space count
  return info;
}

// Get the search regions
bool FindTextDialog::GetCurrentScopeRegions()
{

  DocumentManager* docManager = DocumentManager::GetInstance();
  DocumentEditor* currentEditor = docManager->CurrentEditor;

  // If we have a current document... otherwise do nothing
  if (currentEditor)
  {
    // Get all the text
    StringRange all = currentEditor->GetAllText();

    // Get the total number of lines
    int totalLines = currentEditor->GetLineCount();

    // Get the current line
    int lineNumber = currentEditor->GetCurrentLine();

    // Get the entire line that the cursor is on
    String lineString = currentEditor->GetLineText(lineNumber);

    // Get the number of leading whitespace characters
    WhitespaceInfo lineScope = GetLeadingWhiteSpaceCount(lineString);
    
    // Store the line numbers
    int topLineNumber = lineNumber;
    int bottomLineNumber = lineNumber;

    // Temporary store for whitespace info
    WhitespaceInfo temp;

    // Figure out the top line
    do
    {
      // Move the top line upward
      --topLineNumber;

      // Don't go past the beginning
      if (topLineNumber < 0)
        break;

      // Get the current line
      String currentLine = currentEditor->GetLineText(topLineNumber);

      // Get the whitespace information
      temp = GetLeadingWhiteSpaceCount(currentLine);
    }
    // Walk up the lines until we hit one that's of a lesser scope then the current
    while (temp.LeadingWhitespace >= lineScope.LeadingWhitespace || temp.IsAllWhitespace);

    // Figure out the bottom line
    do
    {
      // Move the bottom line downward
      ++bottomLineNumber;

      // Don't go past the end
      if (bottomLineNumber >= totalLines)
        break;

      // Get the current line
      String currentLine = currentEditor->GetLineText(bottomLineNumber);

      // Get the whitespace information
      temp = GetLeadingWhiteSpaceCount(currentLine);
    }
    // Walk up the lines until we hit one that's of a lesser scope then the current
    while (temp.LeadingWhitespace >= lineScope.LeadingWhitespace || temp.IsAllWhitespace);

    // Note: Even though the bottom line points one past the actual block scope, its ok since
    // we use the function GetPositionFromLine, which always gets us the beginning of the line

    // Create a region for this scope
    SearchRegion* region = new SearchRegion();
    region->WholeText = currentEditor->GetAllText();
    region->Editor = currentEditor;

    // Compute the text region based off the lines
    StringIterator regionStart = region->WholeText.Begin() + currentEditor->GetPositionFromLine(topLineNumber);
    StringIterator regionEnd   = region->WholeText.Begin() + currentEditor->GetPositionFromLine(bottomLineNumber);
    region->RegionText = StringRange(regionStart, regionEnd);

    // The cursor is in the middle of block typically, so we have to get the cursor position relative to the region
    region->CursorPos = ValidCursorPos(region->RegionText, currentEditor);

    // Set the resource, and add this region to the context
    region->Resource = currentEditor->GetDocument()->GetResource();
    mContext->Regions.PushBack(region);
  }

  // If no valid selections exist
  if (mContext->Regions.Empty())
  {
    // Show an warning and return that we failed
    DoNotifyWarning("Find/Replace Warning", "The text cursor was not placed in a scope, or the document was not in focus.");
    return false;
  }

  // Otherwise, we must have found open documents!
  return true;
}

// Get the search regions
bool FindTextDialog::GetSelectedTextRegions()
{
  DocumentManager* docManager = DocumentManager::GetInstance();

  DocumentEditor* currentEditor = docManager->CurrentEditor;

  // If we have a current document... otherwise do nothing
  if (currentEditor)
  {
    // Get all the selections in the current document
    Array<StringRange> selections = currentEditor->GetSelections();

    // Loop through all the selections
    for (size_t i = 0; i < selections.Size(); ++i)
    {
      // Create a region for this selection
      SearchRegion* region = new SearchRegion();
      region->Editor = currentEditor;
      region->WholeText = currentEditor->GetAllText();
      region->RegionText = selections[i];
      region->Resource = currentEditor->GetDocument()->GetResource();
      mContext->Regions.PushBack(region);
    }
  }

  // If no valid selections exist
  if (mContext->Regions.Empty())
  {
    // Show an warning and return that we failed
    DoNotifyWarning("Find/Replace Warning", "No selections were found, or the document was not in focus.");
    return false;
  }

  // Otherwise, we must have found open documents!
  return true;
}

// Advance an input string to the end of a line
const char* FindTextDialog::MoveToEol(StringRange wholeString, const char* currentPosition, EolDirection::Enum direction)
{
  // Assume the direction is forward
  int walkOffset = 1;

  // If the direction passed in is backwards...
  if (direction == EolDirection::Backwards)
  {
    // Flip it so we go backwards
    walkOffset = -1;
  }

  // Store the last position
  const char* lastPosition = currentPosition;

  // While we haven't hit the end of the line (or entire string)
  while (wholeString.Contains(currentPosition) &&
         *currentPosition != '\r' &&
         *currentPosition != '\n' &&
         *currentPosition != '\0')
  {
    // Store the last position
    lastPosition = currentPosition;

    // Move in the direction that was given
    currentPosition += walkOffset;
  }

  // Return the advanced input
  return lastPosition;
}

// Get the entire line as a string range
StringRange FindTextDialog::GetWholeLine(StringRange wholeString, const char* currentPosition)
{
  cstr beginByte = MoveToEol(wholeString, currentPosition, EolDirection::Backwards);
  cstr endByte = MoveToEol(wholeString, currentPosition, EolDirection::Forwards) + 1;
  StringRange output(wholeString.mOriginalString, beginByte, endByte);
  return output.Trim();
}

// Count the number of lines in a given string range
size_t FindTextDialog::CountLines(StringRange input)
{
  // Start the line count at zero
  size_t lineCount = 0;
  int debugCount = 0;
  // Loop until we've gone through the whole string
  while (!input.Empty())
  {
    // Get the current character
    Rune current = input.Front();

    // If we hit any kind of new-line character...
    if (current == '\r' || current == '\n')
    {
      // Increment the line count
      ++lineCount;

      // If the current character is a \r, and we have a next character...
      if (current == '\r' && !input.Empty())
      {
        // Get the next character
        input.PopFront();
        Rune next = input.Front();

        // If the next character is a newline or EOF
        if (next == '\n' || next == '\0')
        {
          // Decrement the line counter for the\r and
          // let us count the \n in the next pass
          --lineCount;
          // continue so we don't PopFront again
          continue;
        }
      }
      // If the next character
    }

    // Iterate to the next character
    input.PopFront();
    ++debugCount;
  }

  // Return the counted number of lines
  return lineCount;
}

// Clear the context
void FindTextDialog::ClearContext()
{
  // Destroy the context and set it to null
  delete mContext;
  mContext = NULL;
}

bool FindTextDialog::TakeFocusOverride()
{
  mFind->TakeFocus();
  return true;
}

// Get the find regex (this may be different than the find text, due to options)
String FindTextDialog::GetFindRegex()
{
  // The regex text that we will normally be searching for (disregarding options)
  String innerRegex;
  
  // Based on the selected character mode...
  switch (mCharacterMode->GetSelectedItem())
  {
    case CharacterMode::Normal:
      innerRegex = Regex::Escape(mFind->GetText(), EscapeMode::Normal);
      break;

    case CharacterMode::Extended:
      innerRegex = Regex::Escape(mFind->GetText(), EscapeMode::Extended);
      break;

    case CharacterMode::Regex:
      innerRegex = mFind->GetText();
      break;
  }

  // If we wish to match the whole word only (not always available)
  if (mMatchWholeWord->GetActive() && mMatchWholeWord->GetChecked())
  {
    return BuildString("\\b", innerRegex, "\\b");
  }
  else
  {
    return innerRegex;
  }
}

// Constructor
FindTextDialog::SearchRegion::SearchRegion()
{
  CursorPos = CursorNotInRegion;
  Editor = NULL;
  Resource = NULL;
}

// Constructor
FindTextDialog::Context::Context()
{
  // By default, we haven't found anything
  TotalResults = 0;
}

// Destructor
FindTextDialog::Context::~Context()
{
  // Get a range of all the regions
  InList<SearchRegion>::range regions = Regions.All();

  // Loop through all the regions
  while (regions.Empty() == false)
  {
    // Get the current region
    SearchRegion* region = &regions.Front();

    // Get a range of all results
    InList<SearchResult>::range results = region->Results.All();

    // Loop through all the results
    while (results.Empty() == false)
    {
      // Get the current result
      SearchResult* result = &results.Front();

      // Iterate to the next result
      results.PopFront();

      // Delete the current result
      delete result;
    }

    // Iterate to the next region
    regions.PopFront();

    // Delete the current region
    delete region;
  }
}

// Perform the actual searching, and fill in the context with the results
void FindTextDialog::DoSearchAndGetContext()
{
  // If we already have a context, early out
  if (mContext != NULL)
    return;

  // Create the context, as well as a regular expression from the search parameters
  mContext = new Context();

  // Get all the search regions (if they are not valid... return early)
  if (GetAndValidateSearchRegions() == false)
    return;

  // Produce the final find regex
  String findRegex = GetFindRegex();

  // The flavor of regular expressions that we'll be using
  RegexFlavor::Enum regexFlavor = RegexFlavor::EcmaScript;

  // If we are in fact in regular expression mode...
  if (mCharacterMode->GetSelectedItem() == CharacterMode::Regex)
  {
    // Get the user-selected flavor of the regular expression
    regexFlavor = (RegexFlavor::Enum)mRegexFlavor->GetSelectedItem();
  }

  // If the find regex is not valid...
  if (Regex::Validate(findRegex, regexFlavor, mMatchCase->GetChecked()) == false)
  {
    // Error notification and return early
    DoNotifyError("Find/Replace Error", "The regular expression used in search was not valid. Please report this if you think it's an error.");
    return;
  }

  // Set the find regex
  mContext->FindRegex = Regex(GetFindRegex(), regexFlavor, mMatchCase->GetChecked());

  // Get a range of the regions so we can iterate through it
  InList<SearchRegion>::range regions = mContext->Regions.All();

  // Loop through all the instances of script resources
  while (regions.Empty() == false)
  {
    // Get the current script resource
    SearchRegion* region = &regions.Front();

    // Store the current sub-region (this will change below)
    StringRange subRegion = region->RegionText;

    // Loop until we get no more matches
    inifinite_loop
    {
      // Get all the resulting matches from our search
      Matches matches;
      mContext->FindRegex.Search(subRegion, matches);

      // If we have matches...
      if (matches.Empty() == false)
      {
        // The search result
        SearchResult* result = new SearchResult();
        StringRange matchSub = subRegion.FindFirstOf(matches.Front());
        // Count the number of lines until the text that we found (+1 since line numbers are actually 0 based)
        result->Line = CountLines(StringRange(region->WholeText.Begin(), matchSub.Begin())) + 1;

        // Get the entire line of text
        result->WholeLine = GetWholeLine(region->WholeText, matchSub.Data());

        // Get the offset into the whole text that the character exists at
        result->PositionBegin = matchSub.Begin() - region->WholeText.Begin();
        result->PositionEnd   = matchSub.End()   - region->WholeText.Begin();

        // We found another result...
        ++mContext->TotalResults;

        // Add the result to the current region
        region->Results.PushBack(result);

        // If the end of the match is the same...
        if (matchSub.mEnd == subRegion.Data())
        {
          // Break out early
          break;
        }
        else
        {
          // Move the string forward to the last match
          //matchSub = region->WholeText.FindLastOf(matches.Front());
          subRegion.mBegin = matchSub.mEnd;
        }
      }
      else
      {
        // Otherwise, break out
        break;
      }
    }

    // Iterate to the next instance
    regions.PopFront();

    // If the region has no results, it must be trimmed
    if (region->Results.Empty())
    {
      // Trim the region from the list
      mContext->Regions.Erase(region);
      delete region;
    }
  }

  // If the context has no results
  if (mContext->TotalResults == 0)
  {
    // Notify the user that there were no results
    DoNotifyWarning("Find/Replace Warning", "Nothing was found.");
  }
}

void FindTextDialog::SubmitTextBox(KeyboardEvent* event, TextBox* textBox)
{
  // if the user pressed the enter key
  if (event->Key == Keys::Enter)
  {
    // Begin the search
    StartSearch(NULL);
    event->Handled = true;
    textBox->TakeFocus();
  }
}

void FindTextDialog::SubmitSearchOnFindReturn(KeyboardEvent* event)
{
  SubmitTextBox(event, mFind);
}

void FindTextDialog::SubmitSearchOnReplaceReturn(KeyboardEvent* event)
{
  SubmitTextBox(event, mReplace);
}

// Occurs when we click the go button
void FindTextDialog::StartSearch(ObjectEvent* event)
{
  // Perform the actual search and fill in the context
  DoSearchAndGetContext();

  // If the context has no results
  if (mContext->TotalResults == 0)
  {
    // Clear the context
    ClearContext();
  }
  else
  {
    // Process the context
    ProcessResults();
  }
}

// Get the console's text editor
ConsoleUi* GetConsoleTextBox()
{
  // Show the console window (for now)
  Widget* consoleWidget = Z::gEditor->ShowConsole();

  // If we didn't find the console widget...
  if (consoleWidget == NULL)
  {
    // Use the ui-automator to find the widget
    consoleWidget = FindWidgetByName("Console", UiTraversal::DepthFirst, 0, Z::gEditor->GetRootWidget());
  }

  // Cast the widget and return it (not safe :/)
  return (ConsoleUi*)consoleWidget;
}

DocumentEditor* FindTextDialog::GetEditorForRegion(SearchRegion* region)
{
  //First try the stored editor
  DocumentEditor* editor = region->Editor;

  //If not store editor and the region has a resource request that the editor
  //open it
  if(editor==NULL && region->Resource)
  {
    //No current editor open try to open it as a resource
    editor = Z::gEditor->OpenDocumentResource(region->Resource);
  }

  if(editor==NULL && !region->FileName.Empty())
  {
    editor = Z::gEditor->OpenTextFile(region->FileName);
  }
  
  ErrorIf(editor==NULL, "Can not find editor for this region. ");

  return editor;

}

// Take the results of the region search and use them
void FindTextDialog::ProcessResults()
{
  // Get the selected index
  size_t searchMode = (size_t)mSearchMode->GetSelectedItem();
  
  // Are we in 'all' mode
  mAllMode = (searchMode & AllBit) != 0;

  // Are we in 'replace' mode
  mReplaceMode = (searchMode & ReplaceBit) != 0;


  // If we're in 'all' mode...
  if (mAllMode)
  {
    // A string builder for what we'll print to the console
    StringBuilder foundLines;

    // Get a range of the regions so we can iterate through it
    InList<SearchRegion>::range regions = mContext->Regions.All();

    // Loop through all the instances of script resources
    while (regions.Empty() == false)
    {
      // Get the current script resource
      SearchRegion* region = &regions.Front();

      // Get a range for all the results in the region
      InList<SearchResult>::range results = region->Results.All();

      // Loop through all the results in the region
      while (results.Empty() == false)
      {
        // Get the current result
        SearchResult* result = &results.Front();

        // In All mode, we only want to open documents if we're doing replacements
        if(mReplaceMode)
        {
          DocumentEditor* editor = GetEditorForRegion(region);
          if(editor)
          {
            //Bring the window into focus
            editor->FocusWindow();

            //Select the text
            editor->GotoAndSelect(result->PositionBegin, result->PositionEnd);

            // Do the replacement on the selected text
            // If we are in replace mode...
            DoReplacements(editor, result);
          }
        }


        // Append the file and line number
        foundLines.Append("File \"");
        foundLines.Append(region->Resource->mContentItem->GetFullPath());
        foundLines.Append(String::Format("\", line %d, ", result->Line));
        foundLines.Append(result->WholeLine);
        foundLines.Append("\n");

        // Iterate to the next result
        results.PopFront();
      }

      // Iterate to the next region
      regions.PopFront();
    }

    // Get the console (show it if need be)
    ConsoleUi* console = GetConsoleTextBox();

    // As long as we now have the console widget...
    if (console != NULL)
    {
      // Get all the text
      String consoleStr = foundLines.ToString();

      // Print out to the console
      console->ClearAllReadOnly();
      console->AddLine(String::Format("Search - %d Found", mContext->TotalResults));
      console->AddBlock(consoleStr);
    }

    // Since we just processed 'all', then the context is used up completely
    ClearContext();
  }
  // Otherwise, we're in 'next' mode (not 'all' mode)
  else
  {
    // Get the current region and result
    SearchRegion* region = GetNext(mContext->Regions);
    SearchResult* result = GetNext(region->Results);

    DocumentEditor* editor = GetEditorForRegion(region);

    if(editor)
    {
      //Bring the window into focus
      editor->FocusWindow();

      //Select the text
      editor->GotoAndSelect(result->PositionBegin, result->PositionEnd);

      // If we are in replace mode...
      if (mReplaceMode)
      {
        // Do the replacement on the selected text
        DoReplacements(editor, result);
      }
    }

    // Remove the result and delete it
    PopNext(region->Results);
    delete result;

    // If we have no results left in this current region...
    if (region->Results.Empty())
    {
      // Remove the region and delete it
      PopNext(mContext->Regions);
      delete region;

      // If we have no regions left in this context...
      if (mContext->Regions.Empty())
      {
        // Clear the context
        ClearContext();
      }
    }
  }

  // Clear the context
  // Note that we'd typically want to keep the context around
  // that's how it was originally designed, however, we don't know
  // if a document was added, modified, removed, active document changed, etc
  // This should be OK since we should be building the context based
  // on the current document / cursor position
  ClearContext();
}

// Do the replacement given a DocumentEditor
void FindTextDialog::DoReplacements(DocumentEditor* documentEditor, SearchResult* toReplace)
{
  // Get the start of the text in the editor
  StringIterator start = documentEditor->GetAllText().Begin();

  // Get the replace area
  StringRange replaceArea(start + toReplace->PositionBegin, start + toReplace->PositionEnd);
  
  // Get the replaced text
  String replacedText = mContext->FindRegex.Replace(replaceArea, mReplace->GetText());

  // Replace the current selection
  documentEditor->ReplaceSelection(replacedText, false);

  // Always reselect the replaced text to show the user what was changed
  documentEditor->SetSelectionStartAndLength(toReplace->PositionBegin, replacedText.ComputeRuneCount());

  // Get a range of all the regions
  InList<SearchRegion>::range regions = mContext->Regions.All();

  // Loop through all the instances of script resources
  while (regions.Empty() == false)
  {
    // Get the current script resource
    SearchRegion* region = &regions.Front();

    // If its from the same resource
    if (region->Resource == documentEditor->GetDocument()->GetResource())
    {
      // Get a range for all the results in the region
      InList<SearchResult>::range results = region->Results.All();

      // Loop through all the results in the region
      while (results.Empty() == false)
      {
        // Get the current result
        SearchResult* result = &results.Front();

        // If the result comes after the replacement...
        // Technically this will fail in weird regex cases where the
        // next find is within the replace area... but screw that case! (TODO)
        if (result->PositionBegin > toReplace->PositionEnd)
        {
          // Get the difference in size between the replaced area and the replaced text
          int sizeDifference = replacedText.ComputeRuneCount() - replaceArea.ComputeRuneCount();

          // Update the positions
          result->PositionBegin += sizeDifference;
          result->PositionEnd   += sizeDifference;

          // Update the line too (used in replace all cases)
          result->Line = CountLines(StringRange(region->WholeText.Begin(), region->WholeText.Begin() + result->PositionBegin)) + 1;
        }

        // Iterate to the next result
        results.PopFront();
      }
    }

    // Iterate to the next region
    regions.PopFront();
  }
}
 
}//namespace Zero
