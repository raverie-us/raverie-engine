// MIT Licensed (see LICENSE.md).
#pragma once

namespace Scintilla
{
struct SCNotification;
class SurfaceImpl;
struct SelectionRange;
} // namespace Scintilla

namespace Zero
{

namespace Events
{
DeclareEvent(CharacterAdded);
DeclareEvent(TextEditorModified);
} // namespace Events

class ScintillaZero;
class KeyboardTextEvent;
class KeyboardEvent;
class UpdateEvent;
class PropertyEvent;
class TextEditorConfig;
class TextEditorHotspot;
class ScintillaWidget;

class TextEditorEvent : public Event
{
public:
  ZilchDeclareType(TextEditorEvent, TypeCopyMode::ReferenceType);
  char Added;
};

DeclareEnum3(LinePosition, Beginning, Middle, End);

// CustomIndicator is always last.
DeclareEnum7(Lexer, Text, Cpp, Python, Console, Shader, Zilch, SpirV);

DeclareEnum13(IndicatorStyle,
              Plain,
              Squiggle,
              Tt,
              Diagonal,
              Strike,
              Hidden,
              Box,
              Roundbox,
              Straightbox,
              Dash,
              Dots,
              Squigglelow,
              Dotbox);

/// Text editor widget.
class TextEditor : public BaseScrollArea
{
public:
  ZilchDeclareType(TextEditor, TypeCopyMode::ReferenceType);

  TextEditor(Composite* parent);
  ~TextEditor();

  // Select the lexer which preforms syntax highlighting and auto completion.
  void SetLexer(uint lexer);

  // Select a color scheme.
  void SetColorScheme(ColorScheme& scheme);
  void UpdateColorScheme();
  void SetCommonLexerStyles(ColorScheme& scheme);

  void SetTextStyle(uint pos, uint length, uint style);

  // Get the length of the document.
  int GetLength();

  // Get the number of lines in this document.
  int GetLineCount();

  // Get the number of lines, including the the additional lines that would
  // fit in the client over-scroll area.  Note: The over-scroll area does not
  // contain editable, numbered text lines.
  int GetClientLineCount();

  // Has the document been modified?
  bool IsModified();

  // Read Only prevents all modification.
  void SetReadOnly(bool value);
  bool GetReadOnly();

  // Set whether backspace unindents or not
  void SetBackspaceUnindents(bool value);

  // Get how much a given line is indented
  int GetLineIndentation(int line);

  // Get/set the current position of the cursor.
  int GetCurrentPosition();
  void SetCurrentPosition(int pos);

  // Get current line of the cursor.
  int GetCurrentLine();

  // Goes to a position scrolling it into view.
  void GoToPosition(int position);

  // Goes to a line scrolling it into view.
  void GoToLine(int line);

  // Scrolls the view until the line is visible
  void MakePositionVisible(int position);
  void MakeLineVisible(int line);

  // Get the line that the text position is on.
  int GetLineFromPosition(int position);

  // Get the text position of the beginning of a line.
  int GetPositionFromLine(int line);

  // Get the length of the line.
  int GetLineLength(int line);

  // Get/set the width of a tab (as a multiple of spaces).
  int GetTabWidth();
  void SetTabWidth(int width);

  // Returns a string that is either a tab character, or the number of spaces
  // based on tab width
  String GetTabStyleAsString();

  // Get the start/end position of a word within the given position
  int GetWordStartPosition(int position, bool onlyWordCharacters = true);
  int GetWordEndPosition(int position, bool onlyWordCharacters = true);

  // Get the start positions of the words at every caret
  void GetAllWordStartPositions(Array<int>& startPositions);

  // Get the locations of every caret
  void GetAllCaretPositions(Array<int>& caretPositions);

  void AdvanceCaretsToEnd();

  // Get the start of the selection.
  int GetSelectionStart();

  // Get end of selection.
  int GetSelectionEnd();

  // Goto line and select region.
  void GotoAndSelect(int start, int end);

  // Select a range of text
  void Select(int start, int end);

  // Set the selection
  void SetSelectionStartAndLength(int start, int length = 0);

  LinePosition::Enum GetLinePositionInfo();

  void GetLineText(int line, char* buffer, uint size);
  int GetCurrentLineText(char* buffer, uint size);

  String GetLineText(int line);
  String GetCurrentLineText();

  StringRange GetSelectedText();

  // Clear all text
  void ClearAll();

  // This command clears any saved undo or redo history.
  // It also sets the save point to the start of the undo buffer,
  // so the document will appear to be unmodified.
  void ClearUndo();

  // Set that current state of the document is unmodified.
  void SetSavePoint();

  // Set all text in the document.
  void SetAllText(StringRange text, bool sendEvents = true);
  StringRange GetAllText();

  // Replace the main selection
  void ReplaceSelection(StringParam text, bool sendEvents = true);

  // Insert and remove text at specific location
  void InsertText(int pos, const char* text);
  void RemoveRange(int pos, int length);

  // Append text works in read only
  void Append(StringRange text);

  Array<StringRange> GetSelections();
  void BlockComment(cstr comment);

  // Queing up undo actions
  void StartUndo();
  void EndUndo();

  void InsertAutoCompleteText(const char* text, int length, int removeCount, int charOffset);

  // Set an annotation.
  void SetAnnotation(int lineNumber, StringParam message, bool goToLine = true);
  // Clear all annotations
  void ClearAnnotations();

  // The maximum number of supported indicators (what the index is for)
  static const int MaxIndicators = 32;
  // Set the position of an indicator
  void SetIndicatorStart(int index, int start);
  void SetIndicatorEnd(int index, int end);
  // Set the style of an indicator
  void SetIndicatorStyle(int index, IndicatorStyle::Enum style);

  void SetMarkerColors(int marker, int foreground, int background);
  void SetMarkerForegroundColor(int marker, int foreground);
  void SetMarkerBackgroundColor(int marker, int background);
  void SetMarker(int line, int type);
  void ClearMarker(int line, int type);
  bool MarkerExists(int line, int type);
  int GetMarkerMask(int line);
  int GetNextMarker(int lineStart, int type);
  int GetPreviousMarker(int lineStart, int type);
  int GetNextMarkerMask(int lineStart, int markerMask);
  int GetPreviousMarkerMask(int lineStart, int markerMask);

  static const int InstructionMarker = 2;
  static const int BreakPointMarker = 1;

  void OnConfigChanged(PropertyEvent* event);
  void OnConfigPropertyChanged(PropertyEvent* event);
  void OnColorSchemeChanged(ObjectEvent* event);

  void OnTextTyped(KeyboardTextEvent* event);

  void OnMouseDown(MouseEvent* event);
  void OnRightMouseDown(MouseEvent* event);
  void OnMouseMove(MouseEvent* event);
  void OnMouseUp(MouseEvent* event);
  void OnMouseFileDrop(MouseFileDropEvent* event);
  void OnMouseScroll(MouseEvent* event);

  void OnCut(ClipboardEvent* event);
  void OnCopy(ClipboardEvent* event);
  void OnPaste(ClipboardEvent* event);

  void OnFocusOut(FocusEvent* event);
  void OnFocusIn(FocusEvent* event);
  void OnMouseExit(MouseEvent* event);

  void SetFontSize(int size);

  bool TakeFocusOverride() override;
  void ClearAllReadOnly();
  void SetWordWrap(bool enabled);
  void EnableLineNumbers(bool enabled);
  void EnableScrollPastEnd(bool enabled);
  uint GetLineHeight();

  void OnUpdate(UpdateEvent* event);

  // Update text body highlights and scroll well indicators for cursors &
  // highlights.
  void UpdateTextMatchHighlighting();

  // Create/Populate scroll-well indicators associated with this TextEditor.
  //   - Note: 'indicators' will be cleared the re-populated based on 'ranges'.
  void UpdateIndicators(Array<Rectangle>& indicators,
                        const std::vector<Scintilla::SelectionRange>& ranges,
                        Vec4Param indicatorColor,
                        Vec2Param minIndicatorHeight,
                        float indicatorWidth,
                        float indicatorOffsetX);

  Vec3 GetScreenPositionOfCursor();
  Vec3 GetScreenPositionFromCursor(int cursor);
  int GetCursorFromScreenPosition(Vec2Param screenPos);
  void UpdateTransform();

  void UpdateArea(ScrollUpdate::Enum type);
  void UseTextEditorConfig();
  TextEditorConfig* GetConfig();
  virtual void UpdateConfig(TextEditorConfig* config);
  void UpdateMargins(ColorScheme& scheme);

  // Scroll Area Functions
  Vec2 GetClientSize() override;
  void SetClientSize(Vec2 newSize) override{};

  void GetText(int start, int end, char* buffer, int bufferSize);
  String GetText(int start, int end);
  int GetRuneAt(int position);
  // Event Processing
  intptr_t SendEditor(unsigned int Msg, u64 wParam = 0, s64 lParam = 0);
  void SetAStyle(int style, ByteColor fore, ByteColor back, int size = 0, cstr face = 0);
  // Send to derived classes
  virtual void OnNotify(Scintilla::SCNotification& scn);

  virtual void BreakpointsClicked(int line, int position)
  {
  }

  virtual ICodeEditor* GetCodeEditor()
  {
    return nullptr;
  }

  virtual void ScriptError(ScriptEvent* event)
  {
  }

public:
  friend class ScintillaWidget;
  friend class ScintillaZero;

  ScintillaZero* mScintilla;
  ScintillaWidget* mScinWidget;

  HashMap<int, String> AnnotationLines;

  uint mTime;
  uint mTickTime;
  // Used to block events when setting text.
  bool mSendEvents;
  // Line numbers
  bool mLineNumbers;
  bool mLineNumberMargin;
  int mTotalMargins;
  // Break point margin
  bool mBreakpoints;
  // Folding margin
  bool mFolding;
  // Several code paths (per frame) can cause indicators to update.
  // But, it only needs to happen once per frame as updating indicators
  // requires a texture upload.
  bool mIndicatorsRequireUpdate;
  // Highlight all instances of text matching the current selection.
  bool mTextMatchHighlighting;
  // Highlight mode is either partial text, or whole text.
  bool mHighlightPartialTextMatch;
  // Current font Size
  int mFontSize;
  // The lexer we're using
  Lexer::Enum mLexer;
  // Color Scheme
  String mColorSchemeName;
  // Active hotspots
  Array<TextEditorHotspot*> mHotspots;

  PixelBuffer* mIndicators;
  TextureView* mIndicatorDisplay;

protected:
  virtual void OnFocusIn()
  {
  }
  virtual void OnFocusOut(){};
  virtual void OnKeyDown(KeyboardEvent* event);
};

} // namespace Zero
