///////////////////////////////////////////////////////////////////////////////
///
/// \file TextEditor.cpp
/// Implementation of the TextEditor Widget.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "Scintilla/ScintillaPrecompiled.hpp"

const ByteColor ColorBlack = ByteColorRGBA(0, 0, 0, 0xFF);
const ByteColor Yellow = ByteColorRGBA(0xFF, 0xFF, 0, 0xFF);
const ByteColor Red = ByteColorRGBA(163, 21, 21, 0xFF);
const int cMinFontSize = 8;
const int cMaxFontSize = 128;

namespace Zero
{

class TextEditor;

namespace Events
{
  DefineEvent(CharacterAdded);
  DefineEvent(TextEditorModified);
}//namespace Events

ZilchDefineType(TextEditorEvent, builder, type)
{
}

 //------------------------------------------------------------- Scintilla Widget
 //ScintillaWidget Inner Scintilla Widget
class ScintillaWidget : public Widget
{
public:
  ScintillaWidget(Composite* parent);
  ~ScintillaWidget();
  ScintillaZero* mScintilla;
  Scintilla::SurfaceImpl mSurface;
  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;
};

//------------------------------------------------------------ ScintillaZero
class ScintillaZero : public ScintillaBase
{
public:
  ScintillaZero();
  virtual ~ScintillaZero();
  // Method to skip Scintilla's AutoComplete logic
  virtual int KeyCommand(unsigned int iMessage);

  //Scintilla Interface
  void Initialise() override;
  void Finalise() override;
  sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) override;
  void SetTicking(bool on) override;
  void SetMouseCapture(bool on) override;
  bool HaveMouseCapture() override;
  void UpdateSystemCaret() override;
  void SetVerticalScrollPos() override;
  void SetHorizontalScrollPos() override;
  bool ModifyScrollBars(int nMax, int nPage) override;
  void NotifyChange() override;
  void NotifyFocus(bool focus) override;
  void NotifyParent(SCNotification scn) override;
  void NotifyDoubleClick(Point pt, bool shift, bool ctrl, bool alt) override;
  void Copy() override;
  bool CanPaste() override;
  void Paste() override;
  void CreateCallTipWindow(PRectangle rc) override;
  void AddToPopUp(const char *label, int cmd = 0, bool enabled = true) override;
  void ClaimSelection() override;
  void CopyToClipboard(const SelectionText &selectedText) override;

  void InsertPasteText(const char *text, int len, SelectionPosition selStart,
                       bool isRectangular, bool isLine);
  uint SendEditor(unsigned int Msg, unsigned long wParam = 0, long lParam = 0);
  void InsertAutoCompleteText(const char* text, int length, int removeCount, int charOffset);
  TextEditor* mOwner;
  bool mMouseCapture;
  friend class TextEditor;
  friend class ScintillaWidget;
protected:
  void Clear();
  void NewLine();
  void MoveSelectedLinesUp();
  void MoveSelectedLinesDown();
private:
  void MoveSelection(SelectionRange& selection, int dir, bool extend);
  bool FindTextNotSelected(int start, int end, const char* text, SelectionRange& newSel);
  ScintillaZero& operator = (const ScintillaZero &);
};

//------------------------------------------------------------- Scintilla Widget
ScintillaWidget::ScintillaWidget(Composite* parent)
  : Widget(parent)
  , mSurface(this)
{
  //Prevent scrolling issue when scintilla is first created.
  mSize = Pixels(1000,500);
}

ScintillaWidget::~ScintillaWidget()
{
  SafeDelete(mScintilla);
}

void ScintillaWidget::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  mSurface.mViewBlock = &viewBlock;
  mSurface.mFrameBlock = &frameBlock;
  mSurface.mViewNode = nullptr;
  mSurface.mColor = colorTx.ColorMultiply;
  mSurface.mBaseRect = clipRect;

  // Setting clip rect here because scintilla is not getting the correct client rect
  mSurface.mClipRect = Rect::PointAndSize(Vec2(parentTx.m30, parentTx.m31), mSize);

  PRectangle rcPaint = mScintilla->GetClientRectangle();
  mScintilla->Paint(&mSurface, rcPaint);
}

static int KeyTranslate(int keyIn)
{
  switch (keyIn)
  {
  case Zero::Keys::Down:      return SCK_DOWN;
  case Zero::Keys::Up:        return SCK_UP;
  case Zero::Keys::Left:      return SCK_LEFT;
  case Zero::Keys::Right:     return SCK_RIGHT;
  case Zero::Keys::Home:      return SCK_HOME;
  case Zero::Keys::End:       return SCK_END;
  case Zero::Keys::PageUp:    return SCK_PRIOR;
  case Zero::Keys::PageDown:  return SCK_NEXT;
  case Zero::Keys::Delete:    return SCK_DELETE;
  case Zero::Keys::Escape:    return SCK_ESCAPE;
  case Zero::Keys::Back:      return SCK_BACK;
  case Zero::Keys::Tab:       return SCK_TAB;
  case Zero::Keys::Enter:     return SCK_RETURN;
  case Zero::Keys::Alt:       return SCK_MENU;
  default:                        return keyIn;
  }

}

const int LineNumberMargin = 0;
const int DebuggingMargin = 1;
const int FoldingMargin = 2;

const int DebugInstructionIndex = 2;
const int DebugBreakPointIndex = 1;


//------------------------------------------------------------------ Text Editor
ZilchDefineType(TextEditor, builder, type)
{

}

TextEditor::TextEditor(Composite* parent)
  : BaseScrollArea(parent)
{
  Scintilla_LinkLexers();
  mScinWidget = new ScintillaWidget(this);
  mScinWidget->SetTakeFocusMode(FocusMode::Hard);
  mScintilla =  new ScintillaZero();
  mScintilla->Initialise();
  mScintilla->wMain = mScinWidget;
  mScintilla->mOwner = this;
  mScinWidget->mScintilla = mScintilla;
  mTickTime = 0;
  mTime = 0;
  mFontSize = 13;
  mLineNumbers = true;
  mBreakpoints = false;
  mSendEvents = true;
  mLineNumberMargin = true;
  mFolding = false;
  mMinSize = Vec2(50, 50);

  Cog* configCog = Z::gEngine->GetConfigCog();
  TextEditorConfig* textConfig = configCog->has(TextEditorConfig);
  mFontSize = textConfig->FontSize;
  ColorScheme* colorScheme = GetColorScheme();
  ConnectThisTo(colorScheme, Events::ColorSchemeChanged, OnColorSchemeChanged);

  //Connections
  ConnectThisTo(mScinWidget, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(mScinWidget, Events::LeftMouseUp, OnMouseUp);
  ConnectThisTo(mScinWidget, Events::KeyDown, OnKeyDown);
  ConnectThisTo(mScinWidget, Events::KeyRepeated, OnKeyDown);

  ConnectThisTo(mScinWidget, Events::KeyUp, OnKeyUp);
  ConnectThisTo(mScinWidget, Events::MouseScroll, OnMouseScroll);
  ConnectThisTo(mScinWidget, Events::FocusLost, OnFocusOut);
  ConnectThisTo(mScinWidget, Events::MouseExitHierarchy, OnMouseExit);
  ConnectThisTo(mScinWidget, Events::FocusGained, OnFocusIn);
  ConnectThisTo(mScinWidget, Events::RightMouseDown, OnRightMouseDown);
  ConnectThisTo(mScinWidget, Events::MouseMove, OnMouseMove);
  ConnectThisTo(mScinWidget, Events::MouseDrop, OnMouseDrop);
  ConnectThisTo(mScinWidget, Events::TextTyped, OnTextTyped);

  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);

  // Clear margins
  SendEditor(SCI_SETMARGINWIDTHN, LineNumberMargin, 0);
  SendEditor(SCI_SETMARGINWIDTHN, FoldingMargin, 0);
  SendEditor(SCI_SETMARGINWIDTHN, DebuggingMargin, 10);

  // Track width or scroll bars will not show up properly
  SendEditor(SCI_SETSCROLLWIDTHTRACKING, true);
  // Start the scroll width as 100
  SendEditor(SCI_SETSCROLLWIDTH, 10);
  // Allow scrolling past the end
  //SendEditor(SCI_SETENDATLASTLINE, false);

  // Blink at 300
  SendEditor(SCI_SETCARETPERIOD, 300);
  SendEditor(SCI_SETCARETLINEVISIBLE, true);

  /// Tabs are four spaces width and Insert spaces instead of tabs
  SendEditor(SCI_SETTABWIDTH, 4);
  SendEditor(SCI_SETUSETABS, 0);

  // Allow multi select areas and typing
  SendEditor(SCI_SETMULTIPLESELECTION, true);
  SendEditor(SCI_SETADDITIONALSELECTIONTYPING, true);
  SendEditor(SCI_SETMULTIPASTE, SC_MULTIPASTE_EACH);
  SendEditor(SCI_SETVIRTUALSPACEOPTIONS, true);

  // Set the lexer (this will also set the default color scheme)
  SetLexer(Lexer::Text);
  SetLexer(Lexer::Text);

  // Set codepage for UTF8 Support
  SendEditor(SCI_SETCODEPAGE, SC_CP_UTF8);
}

TextEditor::~TextEditor()
{
  DeleteObjectsInContainer(mHotspots);
}

void TextEditor::SetLexer(uint lexer)
{
  SendEditor(SCI_SETVIEWWS, SCWS_INVISIBLE);
  mLineNumbers = false;
  mBreakpoints = false;

  switch(lexer)
  {
    case Lexer::Cpp:
    {
      const char cppKeywords[] =
        "class const do else extern false float \
        for if in inline int matrix out pass \
        return register static string struct  \
        true typedef false uniform \
        vector void volatile while uint";

      SendEditor(SCI_SETLEXER, SCLEX_CPP, 0);
      SendEditor(SCI_SETKEYWORDS, 0, (uptr_t)cppKeywords);

      mLineNumbers = true;

      break;
    }

    case Lexer::Shader:
    {
      const char cppKeywords[] =
      "break continue if else switch return for while do typedef namespace true false compile BlendState const void \
      struct static extern register volatile inline target nointerpolation shared uniform varying attribute row_major column_major snorm \
      unorm bool bool1 bool2 bool3 bool4 int int1 int2 int3 int4 uint uint1 uint2 uint3 uint4 half half1 half2 half3 \
      half4 float float1 float2 float3 float4 double double1 double2 double3 double4 matrix bool1x1 bool1x2 bool1x3 \
      bool1x4 bool2x1 bool2x2 bool2x3 bool2x4 bool3x1 bool3x2 bool3x3 bool3x4 bool4x1 bool4x2 bool4x3 bool4x4 int1x1 \
      int1x2 int1x3 int1x4 int2x1 int2x2 int2x3 int2x4 int3x1 int3x2 int3x3 int3x4 int4x1 int4x2 int4x3 int4x4 uint1x1 \
      uint1x2 uint1x3 uint1x4 uint2x1 uint2x2 uint2x3 uint2x4 uint3x1 uint3x2 uint3x3 uint3x4 uint4x1 uint4x2 uint4x3 \
      uint4x4 half1x1 half1x2 half1x3 half1x4 half2x1 half2x2 half2x3 half2x4 half3x1 half3x2 half3x3 half3x4 half4x1 \
      half4x2 half4x3 half4x4 float1x1 float1x2 float1x3 float1x4 float2x1 float2x2 float2x3 float2x4 float3x1 float3x2 \
      float3x3 float3x4 float4x1 float4x2 float4x3 float4x4 double1x1 double1x2 double1x3 double1x4 double2x1 double2x2 \
      double2x3 double2x4 double3x1 double3x2 double3x3 double3x4 double4x1 double4x2 double4x3 double4x4 texture Texture \
      texture1D Texture1D Texture1DArray texture2D Texture2D Texture2DArray Texture2DMS Texture2DMSArray texture3D Texture3D\
      textureCUBE TextureCube sampler sampler1D sampler2D sampler3D samplerCUBE sampler_state cbuffer technique technique10 \
      VertexShader PixelShader pass string auto case catch char class const_cast default delete dynamic_cast enum explicit \
      friend goto long mutable new operator private protected public reinterpret_cast short signed sizeof static_cast \
      template this throw try typename union unsigned using virtual";

      SendEditor(SCI_SETLEXER, SCLEX_CPP, 0);
      SendEditor(SCI_SETKEYWORDS, 0, (uptr_t)cppKeywords);

      mLineNumbers = true;

      break;
    }

    case Lexer::Console:
    {
      //Use python to highlight 'quotes'
      SendEditor(SCI_SETLEXER, (uptr_t)SCLEX_CONTAINER, 0);
      //No keywords
      SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)"");
      SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)"");

      mLineNumbers = false;
      break;
    }

    case Lexer::Python:
    {
      const char pythonKeywords[] =
        "and del for is raise assert elif from lambda return "
        "break else global not try class except if or while "
        "continue exec import pass yield def finally in print show";

      const char pythonSpecial[] =
        "self event True False None";

      SendEditor(SCI_SETLEXER, (uptr_t)SCLEX_PYTHON, 0);
      SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)pythonKeywords);
      SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)pythonSpecial);

      //View White space
      SendEditor(SCI_SETVIEWWS, SCWS_VISIBLEALWAYS);
      mLineNumbers = true;

      break;
    }

    case Lexer::Text:
    {
      SendEditor(SCI_SETLEXER, (uptr_t)SCLEX_CONTAINER, 0);
      SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)"");
      SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)"");
      SendEditor(SCI_SETVIEWWS, SCWS_INVISIBLE);
      break;
    }

    case Lexer::Zilch:
    {
      const char zilchKeywords[] =
        "abstract alias alignof as assert Assign auto base break case catch checked "
        "class compare const constructor continue copy decrement default delegate delete "
        "destructor do dynamic else enum explicit export extern false finally fixed "
        "flags for foreach friend function get global goto if immutable implicit import in include "
        "increment inline interface internal is local lock loop module mutable namespace new "
        "null operator out override package params partial positional private protected public "
        "readonly ref register require return sealed sends set signed sizeof stackalloc static "
        "struct switch throw true try typedef typeid typename typeof type unchecked unsafe unsigned "
        "using var virtual volatile where while yield timeout scope debug";

      const char zilchSpecial[] =
        "this value event";

      SendEditor(SCI_SETLEXER, (uptr_t)SCLEX_CPP, 0);
      SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)zilchKeywords);
      SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)zilchSpecial);
      SendEditor(SCI_SETVIEWWS, SCWS_INVISIBLE);
      mLineNumbers = true;
      //mBreakpoints = true;
      break;
    }
  }

  mLexer = (Lexer::Enum)lexer;
  SetColorScheme(*GetColorScheme());
}

void TextEditor::UpdateMargins(ColorScheme& scheme)
{
  mTotalMargins = 0;

  if(mBreakpoints)
  {
    //For Debugging Set up markers
    mTotalMargins += 16;
    SendEditor(SCI_SETMARGINTYPEN, DebuggingMargin, SC_MARGIN_SYMBOL);
    SendEditor(SCI_SETMARGINWIDTHN, DebuggingMargin, 16);
    SendEditor(SCI_SETMARGINSENSITIVEN, DebuggingMargin, true);
    SendEditor(SCI_SETMARGINMASKN, DebuggingMargin, 0x000000FF);

    SendEditor(SCI_MARKERDEFINE, DebugBreakPointIndex, SC_MARK_ROUNDRECT);
    SendEditor(SCI_MARKERSETFORE, DebugBreakPointIndex, ColorBlack);
    SendEditor(SCI_MARKERSETBACK, DebugBreakPointIndex, Red);

    SendEditor(SCI_MARKERDEFINE, DebugInstructionIndex, SC_MARK_ROUNDRECT);
    SendEditor(SCI_MARKERSETFORE, DebugInstructionIndex, ColorBlack);
    SendEditor(SCI_MARKERSETBACK, DebugInstructionIndex, Yellow);
  }
  else
  {
    mTotalMargins += 10;
    SendEditor(SCI_SETMARGINWIDTHN, DebuggingMargin, 10);
  }

  // Set up code folding
  if(mFolding)
  {
    mTotalMargins += 16;

    // Enable folding
    SendEditor(SCI_SETPROPERTY, (u64)"fold", (s64)"1");
    SendEditor(SCI_SETPROPERTY, (u64)"fold.compact", (s64)"1");
    SendEditor(SCI_SETPROPERTY, (u64)"fold.comment", (s64)"1");
    SendEditor(SCI_SETPROPERTY, (u64)"fold.preprocessor", (s64) "1");

    // Set up fold margin
    SendEditor(SCI_SETMARGINTYPEN, FoldingMargin, SC_MARGIN_SYMBOL);
    SendEditor(SCI_SETMARGINMASKN, FoldingMargin, SC_MASK_FOLDERS);
    SendEditor(SCI_SETMARGINWIDTHN, FoldingMargin, 16);
    SendEditor(SCI_SETMARGINSENSITIVEN, FoldingMargin, 1);

    // Do the box tree style
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL , SC_MARK_TCORNER);

    // Set all colors for folding markers
    for(int i = SC_MARKNUM_FOLDEREND;i<=SC_MARKNUM_FOLDEROPEN;++i)
    {
      // Fore and background text are reversed
      SendEditor(SCI_MARKERSETFORE, i, ToByteColor(scheme.Gutter));
      SendEditor(SCI_MARKERSETBACK, i, ToByteColor(scheme.GutterText));
    }

    // 16  Draw line below if not expanded
    SendEditor(SCI_SETFOLDFLAGS, 16, 0);
  }
  else
  {
    SendEditor(SCI_SETMARGINWIDTHN, FoldingMargin, 0);
    SendEditor(SCI_SETFOLDFLAGS, 0, 0);
    int lineCount = GetLineCount();
    for (int line = 0; line < lineCount; ++line)
      if (SendEditor(SCI_GETFOLDEXPANDED, line, 0) == 0)
        SendEditor(SCI_TOGGLEFOLD, line, 0);
  }

  if(mLineNumbers && mLineNumberMargin)
  {
    uint width = SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, (long)"9999");
    width = Math::Max(width, uint(20));
    mTotalMargins += width;
    SendEditor(SCI_SETMARGINWIDTHN, LineNumberMargin, width);
  }
  else
  {
    SendEditor(SCI_SETMARGINWIDTHN, LineNumberMargin, 0);
  }
}

void TextEditor::SetWordWrap(bool enabled)
{
  if(enabled)
  {
    SendEditor(SCI_SETWRAPMODE, SC_WRAP_CHAR, 0);
  }
  else
  {
    SendEditor(SCI_SETWRAPMODE, SC_WRAP_NONE, 0);
  }
}

void TextEditor::EnableLineNumbers(bool enabled)
{
  mLineNumbers = enabled;
  UpdateMargins(*GetColorScheme());
}

void TextEditor::EnableScrollPastEnd(bool enabled)
{
  mScintilla->endAtLastLine = !enabled;
}

void TextEditor::SetReadOnly(bool value)
{
  SendEditor(SCI_SETREADONLY, value);
}

bool TextEditor::GetReadOnly()
{
  return SendEditor(SCI_GETREADONLY)!=0;
}

void TextEditor::SetBackspaceUnindents(bool value)
{
  SendEditor(SCI_SETBACKSPACEUNINDENTS, (int)value);
}

int TextEditor::GetLineIndentation(int line)
{
  return SendEditor(SCI_GETLINEINDENTATION, line);
}

void TextEditor::Append(StringRange text)
{
  if(GetReadOnly())
  {
    SetReadOnly(false);
    SendEditor(SCI_APPENDTEXT, text.ComputeRuneCount(), (long)text.Data());
    SetReadOnly(true);
  }
  else
  {
     SendEditor(SCI_APPENDTEXT, text.ComputeRuneCount(), (long)text.Data());
  }
}

void TextEditor::SetTextStyle(uint pos, uint length, uint style)
{
  SendEditor(SCI_STARTSTYLING, pos, 0xFFFFFF);
  SendEditor(SCI_SETSTYLING, length, style);
}

void TextEditor::StartUndo()
{
  SendEditor(SCI_BEGINUNDOACTION);
}

void TextEditor::EndUndo()
{
  SendEditor(SCI_ENDUNDOACTION);
}

void TextEditor::InsertAutoCompleteText(const char* text, int length, int removeCount, int charOffset)
{
  mScintilla->InsertAutoCompleteText(text, length, removeCount, charOffset);
}

void TextEditor::UseTextEditorConfig()
{
  TextEditorConfig* config = GetConfig();
  ConnectThisTo(config, Events::PropertyModified, OnConfigChanged);
  this->UpdateConfig(config);
}

void TextEditor::OnConfigChanged(PropertyEvent* event)
{
  TextEditorConfig* config = GetConfig();
  this->UpdateConfig(config);
}

void TextEditor::OnColorSchemeChanged(ObjectEvent* event)
{
  SetColorScheme(*GetColorScheme());
}

TextEditorConfig* TextEditor::GetConfig()
{
  auto config = Z::gEditor->mConfig->has(TextEditorConfig);
  ErrorIf(config == nullptr, "The config should always have a TextEditorConfig component");
  return config;
}

void TextEditor::UpdateConfig(TextEditorConfig* textConfig)
{
  mColorSchemeName = textConfig->ColorScheme;

  mFontSize = textConfig->FontSize;
  mFolding = textConfig->CodeFolding;
  mLineNumberMargin = textConfig->LineNumbers;

  SetLexer(mLexer);
  SetColorScheme(*GetColorScheme());

  if (textConfig->ShowWhiteSpace)
    SendEditor(SCI_SETVIEWWS, SCWS_VISIBLEALWAYS);
  else
    SendEditor(SCI_SETVIEWWS, SCWS_INVISIBLE);

  if (textConfig->TabWidth == TabWidth::TwoSpaces)
    SendEditor(SCI_SETTABWIDTH, 2);
  else
    SendEditor(SCI_SETTABWIDTH, 4);
}

void TextEditor::OnTextTyped(KeyboardTextEvent* event)
{
  Rune r = event->mRune;
  //character > 255 is quick UTF8 fix
  if( IsGraph(r) || r == Keys::Space || r > 255)
  {
    byte utf8Bytes[4];
    int bytesRead = UTF8::UnpackUtf8RuneIntoBuffer(r, utf8Bytes);
    mScintilla->AddCharUTF((char*)utf8Bytes, bytesRead);
  }
}


void TextEditor::OnRightMouseDown(MouseEvent* event)
{
  Vec2 p = ToLocal(event->Position);
  mScintilla->ButtonDown(Point(p.x, p.y), mTime , event->ShiftPressed, event->CtrlPressed, event->AltPressed);
  mScintilla->ButtonUp(Point(p.x, p.y), mTime, false);
}

void TextEditor::OnMouseDown(MouseEvent* event)
{
  Vec2 p = ToLocal(event->Position);
  mScintilla->ButtonDown(Point(p.x, p.y), mTime , event->ShiftPressed, event->CtrlPressed, event->AltPressed);
}

void TextEditor::OnMouseMove(MouseEvent* event)
{
  Vec2 p = ToLocal(event->Position);
  mScintilla->ButtonMove(Point(p.x, p.y));
}

void TextEditor::OnMouseUp(MouseEvent* event)
{
  Vec2 p = ToLocal(event->Position);
  mScintilla->ButtonUp(Point(p.x, p.y), mTime, false);
}

void TextEditor::OnMouseDrop(MouseEvent* event)
{
}

void TextEditor::UpdateArea(ScrollUpdate::Enum type)
{
  if(type != ScrollUpdate::Auto)
  {
    uint size = SendEditor(SCI_TEXTHEIGHT);
    Vec2 clientOffset = mClientOffset;
    mScintilla->topLine = -(clientOffset.y) / float(size);
    mScintilla->xOffset = -(clientOffset.x);
  }
}

uint TextEditor::GetLineHeight()
{
  return SendEditor(SCI_TEXTHEIGHT);
}

Vec2 TextEditor::GetClientSize()
{
  uint lines = SendEditor(SCI_GETLINECOUNT);
  uint size = SendEditor(SCI_TEXTHEIGHT);
  uint width = mScintilla->scrollWidth;

  // Add the size of the margins and some buffer
  // space size of the scroll area
  width += mTotalMargins + Pixels(20);

  uint additionalLineCount = mScintilla->endAtLastLine ? 0 : (uint)mVisibleSize.y / size;

  return Pixels(width, (lines + additionalLineCount) * size);
}

void TextEditor::UpdateTransform()
{
  UpdateScrollBars();

  mScinWidget->SetSize( mVisibleSize );

  mScinWidget->mScintilla->ChangeSize();

  BaseScrollArea::UpdateTransform();
}

// Not included in color styles
#define SCE_ERROR 30

#define SCE_LINK 31


void TextEditor::SetColorScheme(ColorScheme& scheme)
{
  // Copies global style to all others
  SendEditor(SCI_STYLECLEARALL);
  
  // Set the overall default style for text (foreground and background color)
  SetAStyle(STYLE_DEFAULT, ToByteColor(scheme.Default), ToByteColor(scheme.Background), mFontSize, "Inconsolata");

  // Set style for Line number / gutter
  SetAStyle(STYLE_LINENUMBER, ToByteColor(scheme.GutterText), ToByteColor(scheme.Gutter));

  // Set the color of any whitespace identifiers (the . where spaces are, and -> where tabs are)
  SendEditor(SCI_SETWHITESPACEFORE, true, ToByteColor(scheme.Whitespace));

  // Set the color when we select text
  SendEditor(SCI_SETSELBACK, true, ToByteColor(scheme.Selection));

  // Set the color of the current line our caret is on
  SendEditor(SCI_SETCARETLINEBACK, ToByteColor(scheme.LineSelection));

  // Set the caret color to be the default text color
  SendEditor(SCI_SETCARETFORE, ToByteColor(scheme.Default));
  SendEditor(SCI_SETADDITIONALCARETFORE, ToByteColor(scheme.Default));

  uint background = ToByteColor(scheme.Background);

  switch(mLexer)
  {
    case Lexer::Zilch:
    {
      SetCommonLexerStyles(scheme);
      
      SetAStyle(SCE_C_CHARACTER,              ToByteColor(scheme.Default), background);       // Character literals: 'c'
      break;
    }
    case Lexer::Shader:
    case Lexer::Cpp:
    {
      SetCommonLexerStyles(scheme);
      break;
    }

    case Lexer::Console:
    case Lexer::Text:
    {
      SetAStyle(SCE_P_DEFAULT,                ToByteColor(scheme.Default), background);       // Default text
      SetAStyle(SCE_P_IDENTIFIER,             ToByteColor(scheme.Default), background);       // Identifiers: self.Space

      SetAStyle(SCE_P_COMMENTBLOCK,           ToByteColor(scheme.Default), background);       // Block comments: /* */, """
      SetAStyle(SCE_P_COMMENTLINE,            ToByteColor(scheme.Default), background);       // Line comments: #, //
      SetAStyle(SCE_P_TRIPLE,                 ToByteColor(scheme.Default), background);       // Triple comment
      SetAStyle(SCE_P_TRIPLEDOUBLE,           ToByteColor(scheme.Default), background);       // Triple comment
      SetAStyle(SCE_P_STRINGEOL,              ToByteColor(scheme.Default), background);       // ?
      SetAStyle(SCE_P_DECORATOR,              ToByteColor(scheme.Default), background);       // ?

      SetAStyle(SCE_P_NUMBER,                 ToByteColor(scheme.Number), background);        // Number literals: 5, 3.9
      SetAStyle(SCE_P_CHARACTER,              ToByteColor(scheme.Default), background);       // Character literals: 'c'
      SetAStyle(SCE_P_STRING,                 ToByteColor(scheme.StringLiteral), background); // String literals: "hello world"

      SetAStyle(SCE_P_CLASSNAME,              ToByteColor(scheme.Default), background);       // Class names: RigidBody, Model
      SetAStyle(SCE_P_DEFNAME,                ToByteColor(scheme.Default), background);       // Function names: OnLogicUpdate
      SetAStyle(SCE_P_OPERATOR,               ToByteColor(scheme.Default), background);       // Operators: () += .

      SetAStyle(SCE_P_WORD,                   ToByteColor(scheme.Default), background);       // Keywords: class, if
      SetAStyle(SCE_P_WORD2,                  ToByteColor(scheme.Default), background);       // Context keywords: self, this, value

      SetAStyle(SCE_ERROR,                    ToByteColor(scheme.Error), background);         // Errors / exceptions

      SetAStyle(SCE_LINK,                     ToByteColor(scheme.Link), background);          // Link text

      SendEditor(SCI_STYLESETUNDERLINE, SCE_LINK, true);
      SendEditor(SCI_STYLESETHOTSPOT, SCE_LINK, true);
      break;
    }

    case Lexer::Python:
    {
      SetAStyle(SCE_P_DEFAULT,                ToByteColor(scheme.Default), background);       // Default text
      SetAStyle(SCE_P_IDENTIFIER,             ToByteColor(scheme.Default), background);       // Identifiers: self.Space

      SetAStyle(SCE_P_COMMENTBLOCK,           ToByteColor(scheme.Comment), background);       // Block comments: /* */, """
      SetAStyle(SCE_P_COMMENTLINE,            ToByteColor(scheme.Comment), background);       // Line comments: #, //
      SetAStyle(SCE_P_TRIPLE,                 ToByteColor(scheme.Comment), background);       // Triple comment
      SetAStyle(SCE_P_TRIPLEDOUBLE,           ToByteColor(scheme.Comment), background);       // Triple comment
      SetAStyle(SCE_P_STRINGEOL,              ToByteColor(scheme.Comment), background);       // ?
      SetAStyle(SCE_P_DECORATOR,              ToByteColor(scheme.Comment), background);       // ?

      SetAStyle(SCE_P_NUMBER,                 ToByteColor(scheme.Number), background);        // Number literals: 5, 3.9
      SetAStyle(SCE_P_CHARACTER,              ToByteColor(scheme.StringLiteral), background); // Character literals: 'c'
      SetAStyle(SCE_P_STRING,                 ToByteColor(scheme.StringLiteral), background); // String literals: "hello world"

      SetAStyle(SCE_P_CLASSNAME,              ToByteColor(scheme.ClassName), background);     // Class names: RigidBody, Model
      SetAStyle(SCE_P_DEFNAME,                ToByteColor(scheme.FunctionName), background);  // Function names: OnLogicUpdate
      SetAStyle(SCE_P_OPERATOR,               ToByteColor(scheme.Operator), background);      // Operators: () += .

      SetAStyle(SCE_P_WORD,                   ToByteColor(scheme.Keyword), background);       // Keywords: class, if
      SetAStyle(SCE_P_WORD2,                  ToByteColor(scheme.SpecialWords), background);  // Context keywords: self, this, value

      SetAStyle(SCE_ERROR,                    ToByteColor(scheme.Error), background);         // Errors / exceptions

      SetAStyle(SCE_LINK,                     ToByteColor(scheme.Link), background);          // Link text

      SendEditor(SCI_STYLESETUNDERLINE, SCE_LINK, true);
      SendEditor(SCI_STYLESETHOTSPOT, SCE_LINK, true);
      break;
    }
      
    default:
    break;
  }

  UpdateMargins(scheme);
}

void TextEditor::SetCommonLexerStyles(ColorScheme& scheme)
{
  uint background = ToByteColor(scheme.Background);

  SetAStyle(SCE_C_DEFAULT,                ToByteColor(scheme.Default), background);       // Default text
  SetAStyle(SCE_C_IDENTIFIER,             ToByteColor(scheme.Default), background);       // Identifiers: self.Space

  SetAStyle(SCE_C_COMMENT,                ToByteColor(scheme.Comment), background);       // Block comments: /* */, """
  SetAStyle(SCE_C_COMMENTDOC,             ToByteColor(scheme.Comment), background);       // Block comments: /* */, """
  SetAStyle(SCE_C_COMMENTLINE,            ToByteColor(scheme.Comment), background);       // Line comments: #, //
  SetAStyle(SCE_C_COMMENTLINEDOC,         ToByteColor(scheme.Comment), background);       // Line comments: #, //
  SetAStyle(SCE_C_UUID,                   ToByteColor(scheme.Comment), background);       // ?
  SetAStyle(SCE_C_STRINGEOL,              ToByteColor(scheme.Comment), background);       // ?
  SetAStyle(SCE_C_VERBATIM,               ToByteColor(scheme.Comment), background);       // ?
  SetAStyle(SCE_C_TRIPLEVERBATIM,         ToByteColor(scheme.Comment), background);       // ?
  SetAStyle(SCE_C_HASHQUOTEDSTRING,       ToByteColor(scheme.Comment), background);       // ?
  SetAStyle(SCE_C_REGEX,                  ToByteColor(scheme.Comment), background);       // ?

  SetAStyle(SCE_C_NUMBER,                 ToByteColor(scheme.Number), background);        // Number literals: 5, 3.9
  SetAStyle(SCE_C_CHARACTER,              ToByteColor(scheme.StringLiteral), background); // Character literals: 'c'
  SetAStyle(SCE_C_STRING,                 ToByteColor(scheme.StringLiteral), background); // String literals: "hello world"
  SetAStyle(SCE_C_STRINGRAW,              ToByteColor(scheme.StringLiteral), background); // String literals: "hello world"

  SetAStyle(SCE_C_GLOBALCLASS,            ToByteColor(scheme.ClassName), background);     // Class names: RigidBody, Model
  SetAStyle(SCE_C_OPERATOR,               ToByteColor(scheme.Operator), background);      // Operators: () += .

  SetAStyle(SCE_C_WORD,                   ToByteColor(scheme.Keyword), background);       // Keywords: class, if
  SetAStyle(SCE_C_COMMENTDOCKEYWORD,      ToByteColor(scheme.Keyword), background);       // Documentation keywords
  SetAStyle(SCE_C_PREPROCESSOR,           ToByteColor(scheme.Directive), background);     // Preprocessor directives
  SetAStyle(SCE_C_WORD2,                  ToByteColor(scheme.SpecialWords), background);  // Context keywords: self, this, value

  SetAStyle(SCE_ERROR,                    ToByteColor(scheme.Error), background);         // Errors / exceptions
  SetAStyle(SCE_C_COMMENTDOCKEYWORDERROR, ToByteColor(scheme.Error), background);         // Documentation errors
}

void TextEditor::OnKeyDown(KeyboardEvent* event)
{
  if(event->Key == Keys::Back || event->Key == Keys::Delete)
    ClearAnnotations();

  // Prevent scintilla from processing these keys
  if (event->Key >= Keys::F1 && event->Key <= Keys::F12)
    return;

  int key = KeyTranslate(event->Key);

  bool consumed = false;
  int handled = mScintilla->KeyDown(key, event->ShiftPressed, event->CtrlPressed,
                      event->AltPressed, &consumed);

  // Check to see if the event has been consumed to prevent other widgets
  // from doing things
  if(consumed)
    event->Handled = true;

  if (event->Key == Keys::D && event->ShiftPressed && event->CtrlPressed)
    mScintilla->WndProc(SCI_LINEDUPLICATE, 0, 0);

  if (event->Key == Keys::Down && event->ShiftPressed && event->CtrlPressed)
    mScintilla->WndProc(SCI_MOVESELECTEDLINESDOWN, 0, 0);

  if (event->Key == Keys::Up && event->ShiftPressed && event->CtrlPressed)
    mScintilla->WndProc(SCI_MOVESELECTEDLINESUP, 0, 0);

  if(event->CtrlPressed)
  {
    if(event->Key == Keys::Minus)
    {
      SetFontSize(mFontSize-1);
      // ref T3197
      // SetFontSize internally calls SetColorScheme, which is required.
      // It is being called a second time here because of an underlying 
      // bug that is currently outside the scope of fixing, but this fixes
      // the behavioral problems without breaking anything else.
      // The issue is described in the above phabricator issue.
      // The reason I found this fix for the change in behavior is that while
      // ctrl+plus and ctrl+minus didn't work consistently and scrollwheel did,
      // which is calling SetColorScheme after SetFontSize.
      // I suspect it has something to do with us responding to windows message pump events
      // and then scintilla settings being set using the windows message pump also - Dane Curbow
      SetColorScheme(*GetColorScheme());
    }
    if(event->Key == Keys::Equal)
    {
      SetFontSize(mFontSize+1);
      // ref T3197
      SetColorScheme(*GetColorScheme());
    }
    //if(event->Key == Keys::R)
    //  SetWordWrap(true);
  }

  // If not a keyboard short cut handle the key
  if (!(event->AltPressed || event->CtrlPressed))
    event->Handled = true;
}

void TextEditor::SetFontSize(int size)
{
  mFontSize = Math::Max(size, cMinFontSize);
  SetColorScheme(*GetColorScheme());
}

void TextEditor::OnKeyUp(KeyboardEvent* event)
{
  //
}

void TextEditor::OnMouseScroll(MouseEvent* event)
{
  // vertical scroll
  if(!event->CtrlPressed && !event->ShiftPressed)
    mScintilla->ScrollTo(mScintilla->topLine + event->Scroll.y * -1);
  // horizontal scroll when holding shift
  else if(!event->CtrlPressed && event->ShiftPressed)
    mScintilla->HorizontalScrollTo(mScintilla->xOffset + event->Scroll.y * -10);
  // change font size while holding control and scrolling
  else
  {
    mFontSize = Math::Clamp(int(mFontSize + event->Scroll.y), cMinFontSize, cMaxFontSize);
    SetFontSize(mFontSize);
    SetColorScheme(*GetColorScheme());
  }
}

void TextEditor::OnFocusOut(FocusEvent* event)
{
  mScintilla->SetFocusState(false);
  this->OnFocusOut();
}

void TextEditor::OnFocusIn(FocusEvent* event)
{
  mScintilla->SetFocusState(true);
  this->OnFocusIn();
}

void TextEditor::OnMouseExit(MouseEvent* event)
{
  Z::gMouse->SetCursor(Cursor::Arrow);
}

bool TextEditor::TakeFocusOverride()
{
  // Transfer focus to sub widget
  mScinWidget->HardTakeFocus();
  return true;
}

int TextEditor::SendEditor(unsigned int Msg, u64 wParam, s64 lParam)
{
  return (int)mScintilla->WndProc(Msg, (uptr_t)wParam, (sptr_t)lParam);
}

void TextEditor::GetText(int start, int end, char* buffer, int bufferSize)
{
  // SCI_GETTEXTRANGE requires that the buffer have space for a null terminator
  int size = end - start;
  ReturnIf(size >= bufferSize,, "The buffer was too small for the text");

  Sci_TextRange textRange;
  textRange.chrg.cpMin = start;
  textRange.chrg.cpMax = end;
  textRange.lpstrText = buffer;
  SendEditor(SCI_GETTEXTRANGE, 0, (sptr_t)&textRange);
}

String TextEditor::GetText(int start, int end)
{
  int size = end - start;
  int bufferSize = size + 1;
  char* buffer = (char*)alloca(bufferSize);
  this->GetText(start, end, buffer, bufferSize);
  return StringRange(buffer, buffer+size);
}

int TextEditor::GetRuneAt(int position)
{
  return SendEditor(SCI_GETCHARAT, position);
}

DeclareEnum3(CommentMode, Auto, Comment, Uncomment);

void TextEditor::BlockComment(cstr comment)
{
  int commentLen = strlen(comment);

  //Get the selection information
  int selectionStart = SendEditor(SCI_GETSELECTIONSTART);
  int selectionEnd = SendEditor(SCI_GETSELECTIONEND);
  int caretPosition = SendEditor(SCI_GETCURRENTPOS);
  int selStartLine = SendEditor(SCI_LINEFROMPOSITION, selectionStart);
  int selEndLine = SendEditor(SCI_LINEFROMPOSITION, selectionEnd);
  int lengthDoc = SendEditor(SCI_GETLENGTH);

  //Buffer for testing text for comments
  const int bufferSize = commentLen+1;
  char* buffer = (char*)alloca(bufferSize);

  CommentMode::Enum commentMode = CommentMode::Auto;

  //Check lines for indent level and comment mode
  int minIndentLevel = INT_MAX;
  for(int currentLine=selStartLine;currentLine<=selEndLine;++currentLine)
  {
    int lineStart = SendEditor(SCI_POSITIONFROMLINE, currentLine);
    int lineEnd = SendEditor(SCI_GETLINEENDPOSITION, currentLine);
    int lineIndent = SendEditor(SCI_GETLINEINDENTPOSITION, currentLine);

    //Skip blank lines.
    if(lineIndent==lineEnd)
      continue;

    if(commentMode==CommentMode::Auto)
    {
      GetText(lineIndent, lineIndent+commentLen, buffer, bufferSize);
      //Is it a comment? Then uncomment everything.
      if(strncmp(buffer, comment, commentLen)==0)
        commentMode = CommentMode::Uncomment;
      else
        commentMode = CommentMode::Comment;
    }

    minIndentLevel = Math::Min(lineIndent - lineStart, minIndentLevel);
  }

  //Comment and uncomment all the lines
  SendEditor(SCI_BEGINUNDOACTION);
  for(int currentLine=selStartLine;currentLine<=selEndLine;++currentLine)
  {
    int lineStart = SendEditor(SCI_POSITIONFROMLINE, currentLine);
    int lineEnd = SendEditor(SCI_GETLINEENDPOSITION, currentLine);
    int lineIndent = SendEditor(SCI_GETLINEINDENTPOSITION, currentLine);

    //Skip blank lines.
    if(lineIndent==lineEnd)
      continue;

    if(commentMode == CommentMode::Comment)
    {
      //comment the line inserting as min indent level
      SendEditor(SCI_INSERTTEXT, lineStart+minIndentLevel, (sptr_t)comment);
    }
    else
    {
      //Un comment the line by replacing the comment
      GetText(lineIndent, lineIndent+commentLen, buffer, bufferSize);
      //Is it a comment?
      if(strncmp(buffer, comment, commentLen)==0)
      {
        //replace with nothing
        SendEditor(SCI_SETSEL, lineIndent, lineIndent + commentLen);
        SendEditor(SCI_REPLACESEL, 0, (sptr_t)"");
      }
    }
  }
  SendEditor(SCI_ENDUNDOACTION);
}

void TextEditor::SetAStyle(int style, ByteColor fore, ByteColor back, int size, cstr face)
{
  SendEditor(SCI_STYLESETFORE, style, fore);
  SendEditor(SCI_STYLESETBACK, style, back);

  if(size >= 1)
    SendEditor(SCI_STYLESETSIZE, style, size);

  if(face)
    SendEditor(SCI_STYLESETFONT, style, (sptr_t)(face));
}

void TextEditor::OnUpdate(UpdateEvent* event)
{
  uint msTime = event->RealDt * 1000.0f;
  mTickTime += msTime;
  mTime += msTime;
  if(mTickTime > uint(mScintilla->timer.ticksToWait))
  {
    mScinWidget->MarkAsNeedsUpdate();

    //tick to blink caret
    mScintilla->Tick();
    mTickTime = 0;
  }
}

void TextEditor::SetSavePoint()
{
  SendEditor(SCI_SETSAVEPOINT);
}

void TextEditor::ClearAll()
{
  SendEditor(SCI_CLEARALL);
}

void TextEditor::ClearAllReadOnly()
{
  SendEditor(SCI_SETREADONLY, 0);
  SendEditor(SCI_CLEARALL);
  SendEditor(SCI_SETREADONLY, 1);
}

void TextEditor::ClearUndo()
{
  SendEditor(SCI_EMPTYUNDOBUFFER);
}

bool TextEditor::IsModified()
{
  return SendEditor(SCI_GETMODIFY) != 0;
}

void TextEditor::GoToPosition(int position)
{
  SendEditor(SCI_GOTOPOS, position);
}

void TextEditor::GoToLine(int lineNumber)
{
  SendEditor(SCI_ENSUREVISIBLEENFORCEPOLICY, lineNumber);
  SendEditor(SCI_GOTOLINE, lineNumber);
}

int TextEditor::GetLineFromPosition(int position)
{
  return SendEditor(SCI_LINEFROMPOSITION, position);
}

int TextEditor::GetPositionFromLine(int lineNumber)
{
  return SendEditor(SCI_POSITIONFROMLINE, lineNumber);
}

LinePosition::Enum TextEditor::GetLinePositionInfo()
{
  // Get the current line and its length
  int currentLine = GetCurrentLine();
  int lineLength = GetLineLength(currentLine);

  // Read the current line into a buffer, and get the cursor position (lineOffset) into that line
  const int BufferSize = 8192;
  char buffer[BufferSize];
  int lineOffset = GetCurrentLineText(buffer, BufferSize);

  // Default the position to be in the middle
  LinePosition::Enum position = LinePosition::Middle;

  // Assume that everything after the cursor is whitespace (until the end of the line)...
  bool isAllSpaceAfterCursor = true;

  // Loop from the cursor position to the end of the line
  for (int i = lineOffset; i < lineLength; ++i)
  {
    // If the current character is not a space...
    if (!IsSpace(buffer[i]))
    {
      // It's not all whitespace!
      isAllSpaceAfterCursor = false;
      break;
    }
  }

  // Basically, if everything after the cursor was empty (then we are really at the end)
  if (isAllSpaceAfterCursor)
    position = LinePosition::End;

  // Assume that everything before the cursor is whitespace (until the end of the line)...
  bool isAllSpaceBeforeCursor = true;

  // Loop from the beginning of the line to the cursor position
  for (int i = 0; i < lineOffset; ++i)
  {
    // If the current character is not a space...
    if (!IsSpace(buffer[i]))
    {
      // It's not all whitespace!
      isAllSpaceBeforeCursor = false;
      break;
    }
  }

  // Basically, if everything before the cursor was empty (then we are really at the beginning)
  // Note that it can be both at the beginning and end (if the line is empty), but beginning takes precedence
  if (isAllSpaceBeforeCursor)
    position = LinePosition::Beginning;

  // Return the line position
  return position;
}

int TextEditor::GetLength()
{
  return SendEditor(SCI_GETLENGTH);
}

int TextEditor::GetCurrentLine()
{
  int position =  SendEditor(SCI_GETCURRENTPOS);
  return GetLineFromPosition(position);
}

int TextEditor::GetLineLength(int line)
{
  return SendEditor(SCI_LINELENGTH, line);
}

int TextEditor::GetTabWidth()
{
  return SendEditor(SCI_GETTABWIDTH);
}

String TextEditor::GetTabStyleAsString()
{
  int tabWidth = GetTabWidth();

  // We should discover here whether we should use tabs or spaces
  return String::Repeat(' ', tabWidth);
}

void TextEditor::SetTabWidth(int width)
{
  SendEditor(SCI_SETTABWIDTH, width);
}

int TextEditor::GetWordStartPosition(int position, bool onlyWordCharacters)
{
  return SendEditor(SCI_WORDSTARTPOSITION, position, onlyWordCharacters);
}

int TextEditor::GetWordEndPosition(int position, bool onlyWordCharacters)
{
  return SendEditor(SCI_WORDENDPOSITION, position, onlyWordCharacters);
}

void TextEditor::GetAllWordStartPositions(Array<int>& startPositions)
{
  for (size_t i = 0; i < mScintilla->sel.Count(); ++i)
    startPositions.PushBack(GetWordStartPosition(mScintilla->sel.Range(i).caret.Position()));
}

void TextEditor::GetAllCaretPositions(Array<int>& caretPositions)
{
  for (size_t i = 0; i < mScintilla->sel.Count(); ++i)
    caretPositions.PushBack(mScintilla->sel.Range(i).caret.Position());
}

void TextEditor::AdvanceCaretsToEnd()
{
  SendEditor(SCI_LINEEND);
}

int TextEditor::GetLineCount()
{
  return SendEditor(SCI_GETLINECOUNT);
}

int TextEditor::GetCurrentPosition()
{
  return SendEditor(SCI_GETCURRENTPOS);
}

void TextEditor::SetCurrentPosition(int pos)
{
  SendEditor(SCI_SETCURRENTPOS, pos);
  SendEditor(SCI_SCROLLCARET);
}

void TextEditor::SetAllText(StringRange text, bool sendEvents)
{
  mSendEvents = sendEvents;
  ClearAll();
  SendEditor(SCI_ADDTEXT, text.SizeInBytes(), (long)text.Data());
  mSendEvents = true;
}

StringRange TextEditor::GetAllText()
{
  int length = GetLength();
  char* data = (char*)SendEditor(SCI_GETCHARACTERPOINTER);
  return StringRange(data, data + length);
}

void TextEditor::ClearMarker(int line, int type)
{
  if(line == -1)
    SendEditor(SCI_MARKERDELETEALL, type);
  else
    SendEditor(SCI_MARKERDELETE, line, type);
}

void TextEditor::SetMarkerForegroundColor(int marker, int foreground)
{
  SendEditor(SCI_MARKERSETFORE, marker, foreground);
}

void TextEditor::SetMarkerBackgroundColor(int marker, int background)
{
  SendEditor(SCI_MARKERSETBACK, marker, background);
}

void TextEditor::SetMarkerColors(int marker, int foreground, int background)
{
  SendEditor(SCI_MARKERSETFORE, marker, foreground);
  SendEditor(SCI_MARKERSETBACK, marker, background);
}

void TextEditor::SetMarker(int line, int type)
{
  SendEditor(SCI_MARKERADD, line, type);
}

Vec3 TextEditor::GetScreenPositionOfCursor()
{
  return GetScreenPositionFromCursor(GetCurrentPosition());
}

Vec3 TextEditor::GetScreenPositionFromCursor(int cursor)
{
  int x = SendEditor(SCI_POINTXFROMPOSITION, 0, cursor);
  int y = SendEditor(SCI_POINTYFROMPOSITION, 0, cursor);

  Vec3 pv = Vec3(x, y, 0);

  pv += this->GetScreenPosition();
  return pv;
}

int TextEditor::GetCursorFromScreenPosition(Vec2Param screenPos)
{
  Vec2 localPos = screenPos - Math::ToVector2(this->GetScreenPosition());

  return SendEditor(SCI_POSITIONFROMPOINTCLOSE, (int)localPos.x, (int)localPos.y);
}

void TextEditor::SetAnnotation(int lineNumber, StringParam errorMessage)
{
  String wrappedMessage = WordWrap(errorMessage, 80);

  AnnotationLines.Insert(lineNumber);
  //ANNOTATION_STANDARD //ANNOTATION_BOXED
  SendEditor(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD);
  SendEditor(SCI_ANNOTATIONSETTEXT, lineNumber, (long)wrappedMessage.c_str());
  SendEditor(SCI_ANNOTATIONSETSTYLE, lineNumber, SCE_ERROR);

  //Go to one past the line, otherwise we might not be able to
  //see the annotations that are after the current line
  GoToLine(lineNumber + 1);
  //SendEditor(SCI_SCROLLCARET);
}

void TextEditor::ReplaceSelection(StringParam text, bool sendEvents)
{
  mSendEvents = sendEvents;
  SendEditor(SCI_REPLACESEL, 0, (sptr_t)text.c_str());
  mSendEvents = true;
}

void TextEditor::InsertText(int pos, const char* text)
{
  SendEditor(SCI_INSERTTEXT, pos, reinterpret_cast<s64>(text));
}

void TextEditor::RemoveRange(int pos, int length)
{
  mScintilla->pdoc->DeleteChars(pos, length);
}

int TextEditor::GetSelectionStart()
{
  return SendEditor(SCI_GETSELECTIONNSTART, 0);
}

int TextEditor::GetSelectionEnd()
{
  return SendEditor(SCI_GETSELECTIONNEND, 0);
}

void TextEditor::SetSelectionStartAndLength(int start, int length)
{
  SendEditor(SCI_SETSELECTIONNSTART, 0, start);
  SendEditor(SCI_SETSELECTIONNEND, 0, start + length);
}

void TextEditor::Select(int start, int end)
{
  SendEditor(SCI_SETSELECTIONNSTART, 0, start);
  SendEditor(SCI_SETSELECTIONNEND, 0, end);
}

void TextEditor::GotoAndSelect(int start, int end)
{
  GoToPosition(start);
  Select(start, end);
}

StringRange TextEditor::GetSelectedText()
{
  StringRange all = GetAllText();
  cstr selectionStart = all.Data() + GetSelectionStart();
  cstr selectionEnd = all.Data() + GetSelectionEnd();
  return StringRange(all.mOriginalString, selectionStart, selectionEnd);
}

Array<StringRange> TextEditor::GetSelections()
{
  Error("NOT IMPLEMENTED YET");
  return Array<StringRange>();
}

int TextEditor::GetCurrentLineText(char* buffer, uint size)
{
  return SendEditor(SCI_GETCURLINE, size, (sptr_t)buffer);
}

void TextEditor::GetLineText(int line, char* buffer, uint size)
{
  int length = GetLineLength(line);

  ReturnIf(length >= int(size), , "The output buffer is not big enough to fit the line (including null!)");

  SendEditor(SCI_GETLINE, line, (sptr_t)buffer);

  buffer[length] = '\0';
}

String TextEditor::GetLineText(int line)
{
  int length = GetLineLength(line);

  char* buffer = (char*)alloca(length + 1);

  GetLineText(line, buffer, length + 1);

  String result(buffer, length);

  return result;
}

String TextEditor::GetCurrentLineText()
{
  return GetLineText(GetCurrentLine());
}

void TextEditor::ClearAnnotations()
{
  AnnotationLines.Clear();
  SendEditor(SCI_ANNOTATIONCLEARALL);
}

void TextEditor::SetIndicatorStart(int index, int start)
{
  SendEditor(SCI_INDICSETFORE, index, 0x007f00);
  SendEditor(SCI_INDICATORSTART, index, start);
}

void TextEditor::SetIndicatorEnd(int index, int end)
{
  SendEditor(SCI_INDICATOREND, index, end);
}

void TextEditor::SetIndicatorStyle(int index, IndicatorStyle::Enum style)
{
  SendEditor(SCI_INDICSETSTYLE, index, (s64)style);
}

void TextEditor::OnNotify(Scintilla::SCNotification& notify)
{
  const int modifiers = notify.modifiers;
  const int position = notify.position;
  const int margin = notify.margin;
  const int line_number = SendEditor(SCI_LINEFROMPOSITION, position, 0);

  switch(notify.nmhdr.code)
  {
  case SCN_MODIFIED:
    {
    }
    break;

  case SCN_MARGINCLICK:
    {
      if(margin == FoldingMargin)
        SendEditor(SCI_TOGGLEFOLD, line_number, 0);
    }
    break;

  case SCN_AUTOCSELECTION:
  case SCN_AUTOCCANCELLED:
    {
      SendEditor(SCI_AUTOCSETFILLUPS, 0, (s64)"");
    }
    break;
  case SCN_HOTSPOTRELEASECLICK:
    {
      TextEditorHotspot::ClickHotspotsAt(this, position);
    }
    break;
  case SCN_CHARADDED:
    {
      TextEditorEvent textEvent;
      textEvent.Added = notify.ch;
      this->GetDispatcher()->Dispatch(Events::CharacterAdded, &textEvent);
    }
    break;
  }

  // Send out modified events
  if (mSendEvents == true)
  {
    bool shouldSendEvent = (notify.nmhdr.code == SCN_MODIFIED && notify.modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) ||
                           (notify.nmhdr.code == SCN_CHARADDED || notify.nmhdr.code == SCN_KEY);

    if (shouldSendEvent)
    {
      Event event;
      this->GetDispatcher()->Dispatch(Events::TextEditorModified, &event);
    }
  }
}


//------------------------------------------------------------ ScintillaZero
//Implement Scintilla in Zero Ui
ScintillaZero::ScintillaZero()
{
  mMouseCapture = false;
}

ScintillaZero::~ScintillaZero()
{
}

void ScintillaZero::Clear()
{
  if (sel.Empty())
  {
    bool singleVirtual = false;
    if ((sel.Count() == 1) && !RangeContainsProtected(sel.MainCaret(), sel.MainCaret() + 1) && sel.RangeMain().Start().VirtualSpace())
    {
      singleVirtual = true;
    }
    UndoGroup ug(pdoc, (sel.Count() > 1) || singleVirtual);
    for (size_t r=0; r<sel.Count(); r++)
    {
      if (!RangeContainsProtected(sel.Range(r).caret.Position(), sel.Range(r).caret.Position() + 1))
      {
        if (sel.Range(r).Start().VirtualSpace())
        {
          if (sel.Range(r).anchor < sel.Range(r).caret)
            sel.Range(r) = SelectionPosition(InsertSpace(sel.Range(r).anchor.Position(), sel.Range(r).anchor.VirtualSpace()));
          else
            sel.Range(r) = SelectionPosition(InsertSpace(sel.Range(r).caret.Position(), sel.Range(r).caret.VirtualSpace()));
        }
        else
        {
          pdoc->DelChar(sel.Range(r).caret.Position());
          sel.Range(r).ClearVirtualSpace();
        }
      }
      else
      {
        sel.Range(r).ClearVirtualSpace();
      }
    }
  }
  else
  {
    ClearSelection();
  }
  sel.RemoveDuplicates();
}

void ScintillaZero::NewLine()
{
  UndoGroup ug(pdoc);

  ClearSelection();
  const char* eol = StringFromEOLMode(pdoc->eolMode);

  for (size_t i = 0; i < sel.Count(); ++i)
    pdoc->InsertCString(sel.Range(i).caret.Position(), eol);

  while (*eol)
  {
    NotifyChar(*eol);
    if (recordingMacro)
    {
      char txt[2];
      txt[0] = *eol;
      txt[1] = '\0';
      NotifyMacroRecord(SCI_REPLACESEL, 0, reinterpret_cast<sptr_t>(txt));
    }
    eol++;
  }

  SetLastXChosen();
  SetScrollBars();
  EnsureCaretVisible();
  // Avoid blinking during rapid typing:
  ShowCaretAtCurrentPosition();
}

void ScintillaZero::MoveSelectedLinesUp()
{
  UndoGroup ug(pdoc);

  // Get every line that every selection touches
  Array<int> selectionLines;
  for (size_t i = 0; i < sel.Count(); ++i)
  {
    int anchorLine = pdoc->LineFromPosition(sel.Range(i).anchor.Position());
    int caretLine = pdoc->LineFromPosition(sel.Range(i).caret.Position());

    int dir = anchorLine <= caretLine ? 1 : -1;
    for (int line = anchorLine; line - dir != caretLine; line += dir)
      selectionLines.PushBack(line);
  }

  // Going to swap lines around selections from top to bottom
  Sort(selectionLines.All(), less<int>());

  for (size_t i = 0; i < selectionLines.Size(); ++i)
  {
    // Line above current group of selections
    int fromLine = selectionLines[i] - 1;

    // Walk until there is a line gap between groups of selections
    while (i + 1 < selectionLines.Size() && selectionLines[i + 1] - 1 <= selectionLines[i])
      ++i;

    // Line below current group of selections
    int toLine = selectionLines[i] + 1;

    // Already at the top of the document
    if (fromLine < 0)
      continue;

    // Get text from the line below selection and delete it
    int start = pdoc->LineStart(fromLine);
    int end = pdoc->LineStart(fromLine + 1);
    int length = end - start;

    char lineText[1024] = {0};
    pdoc->GetCharRange(lineText, start, length);
    pdoc->DeleteChars(start, length);

     // Deleted a line above selection, so Insert line has been offset
    --toLine;

    // Reinsert text at the line below selection
    if (toLine == pdoc->LinesTotal())
    {
      // Remove succeeding newline for bottom of document case
      while (lineText[length - 1] == '\n' || lineText[length - 1] == '\r')
      {
        lineText[length - 1] = '\0';
        --length;
      }

      pdoc->InsertCString(pdoc->LineEnd(pdoc->LinesTotal() - 1), StringFromEOLMode(pdoc->eolMode));
      // Correct caret if on end of document
      for (size_t j = 0; j < sel.Count(); ++j)
      {
        if (sel.Range(j).anchor.Position() == pdoc->LineEnd(pdoc->LinesTotal() - 1))
          sel.Range(j).anchor.Add(-(int)strlen(StringFromEOLMode(pdoc->eolMode)));
        if (sel.Range(j).caret.Position() == pdoc->LineEnd(pdoc->LinesTotal() - 1))
          sel.Range(j).caret.Add(-(int)strlen(StringFromEOLMode(pdoc->eolMode)));
      }
      pdoc->InsertCString(pdoc->LineEnd(pdoc->LinesTotal() - 1), lineText);
    }
    else
    {
      pdoc->InsertCString(pdoc->LineStart(toLine), lineText);
    }
  }
}

void ScintillaZero::MoveSelectedLinesDown()
{
  UndoGroup ug(pdoc);

  // Get every line that every selection touches
  Array<int> selectionLines;
  for (size_t i = 0; i < sel.Count(); ++i)
  {
    int anchorLine = pdoc->LineFromPosition(sel.Range(i).anchor.Position());
    int caretLine = pdoc->LineFromPosition(sel.Range(i).caret.Position());

    int dir = anchorLine <= caretLine ? 1 : -1;
    for (int line = anchorLine; line - dir != caretLine; line += dir)
      selectionLines.PushBack(line);
  }

  // Going to swap lines around selections from bottom to top
  Sort(selectionLines.All(), greater<int>());

  for (size_t i = 0; i < selectionLines.Size(); ++i)
  {
    // Line below current group of selections
    int fromLine = selectionLines[i] + 1;

    // Walk until there is a line gap between groups of selections
    while (i + 1 < selectionLines.Size() && selectionLines[i + 1] + 1 >= selectionLines[i])
      ++i;

    // Line above current group of selections
    int toLine = selectionLines[i] - 1;

    // Already at the bottom of the document
    if (fromLine == pdoc->LinesTotal())
      continue;

    // Get text from the line below selection and delete it
    int start = pdoc->LineEnd(fromLine - 1);
    int end = pdoc->LineEnd(fromLine);
    int length = end - start;

    char lineText[1024] = {0};
    pdoc->GetCharRange(lineText, start, length);
    pdoc->DeleteChars(start, length);

    // Reinsert text at the line above selection
    if (toLine < 0)
    {
      char* text = lineText;
      // Remove preceding newline for top of document case
      while (*text == '\n' || *text == '\r')
        ++text;

      pdoc->InsertCString(0, text);
      pdoc->InsertCString(strlen(text), StringFromEOLMode(pdoc->eolMode));
    }
    else
    {
      pdoc->InsertCString(pdoc->LineEnd(toLine), lineText);
    }
  }
}

int ScintillaZero::KeyCommand(unsigned int iMessage)
{
  switch (iMessage)
  {
    case SCI_CANCEL:
    {
      CancelModes();
      MovePositionTo(sel.MainCaret());
    }
    break;

    case SCI_TAB:
    {
      return Editor::KeyCommand(iMessage);
    }
    break;

    case SCI_NEWLINE:
    {
      NewLine();
    }
    break;

    case SCI_CHARLEFT:
    {
      if (SelectionEmpty() || sel.MoveExtends())
      {
        for (size_t i = 0; i < sel.Count(); ++i)
          MoveSelection(sel.Range(i), sel.Range(i).caret.Position() - 1, false);
      }
      else
      {
        for (size_t i = 0; i < sel.Count(); ++i)
        {
          int caret = sel.Range(i).caret.Position();
          int anchor = sel.Range(i).anchor.Position();
          MoveSelection(sel.Range(i), caret < anchor ? caret : anchor, false);
        }
      }
    }
    break;

    case SCI_CHARRIGHT:
    {
      if (SelectionEmpty() || sel.MoveExtends())
      {
        for (size_t i = 0; i < sel.Count(); ++i)
          MoveSelection(sel.Range(i), sel.Range(i).caret.Position() + 1, false);
      }
      else
      {
        for (size_t i = 0; i < sel.Count(); ++i)
        {
          int caret = sel.Range(i).caret.Position();
          int anchor = sel.Range(i).anchor.Position();
          MoveSelection(sel.Range(i), caret > anchor ? caret : anchor, false);
        }
      }
    }
    break;

    case SCI_CHARLEFTEXTEND:
    {
      for (size_t i = 0; i < sel.Count(); ++i)
      {
        MoveSelection(sel.Range(i), sel.Range(i).caret.Position() - 1, true);

        SelectionRange& selection = sel.Range(i);
        for (size_t j = 0; j < i; ++j)
        {
          SelectionRange& other = sel.Range(j);

          if (other.caret.Position() < selection.anchor.Position() &&
              other.anchor.Position() > selection.caret.Position())
          {
            SelectionPosition anchor(std::max(other.anchor.Position(), selection.anchor.Position()));
            SelectionPosition caret(std::min(other.caret.Position(), selection.caret.Position()));
            SelectionRange newRange(caret, anchor);

            selection = newRange;
            other = newRange;
          }
        }
      }
    }
    break;

    case SCI_CHARRIGHTEXTEND:
    {
      for (size_t i = 0; i < sel.Count(); ++i)
      {
        MoveSelection(sel.Range(i), sel.Range(i).caret.Position() + 1, true);

        SelectionRange& selection = sel.Range(i);
        for (size_t j = 0; j < i; ++j)
        {
          SelectionRange& other = sel.Range(j);

          if (other.caret.Position() > selection.anchor.Position() &&
              other.anchor.Position() < selection.caret.Position())
          {
            SelectionPosition anchor(std::min(other.anchor.Position(), selection.anchor.Position()));
            SelectionPosition caret(std::max(other.caret.Position(), selection.caret.Position()));
            SelectionRange newRange(caret, anchor);

            selection = newRange;
            other = newRange;
          }
        }
      }
    }
    break;

    case SCI_WORDLEFT:
    {
      for (size_t i = 0; i < sel.Count(); ++i)
        MoveSelection(sel.Range(i), pdoc->ExtendWordSelect(sel.Range(i).caret.Position(), -1), false);
    }
    break;

    case SCI_WORDRIGHT:
    {
      for (size_t i = 0; i < sel.Count(); ++i)
        MoveSelection(sel.Range(i), pdoc->ExtendWordSelect(sel.Range(i).caret.Position(), 1), false);
    }
    break;

    case SCI_WORDLEFTEXTEND:
    {
      for (size_t i = 0; i < sel.Count(); ++i)
        MoveSelection(sel.Range(i), pdoc->ExtendWordSelect(sel.Range(i).caret.Position(), -1), true);
    }
    break;

    case SCI_WORDRIGHTEXTEND:
    {
      for (size_t i = 0; i < sel.Count(); ++i)
        MoveSelection(sel.Range(i), pdoc->ExtendWordSelect(sel.Range(i).caret.Position(), 1), true);
    }
    break;

    case SCI_VCHOME:
    {
      for (size_t i = 0; i < sel.Count(); ++i)
        MoveSelection(sel.Range(i), pdoc->VCHomePosition(sel.Range(i).caret.Position()), false);
    }
    break;

    case SCI_LINEEND:
    {
      for (size_t i = 0; i < sel.Count(); ++i)
        MoveSelection(sel.Range(i), pdoc->LineEndPosition(sel.Range(i).caret.Position()), false);
    }
    break;

    case SCI_VCHOMEEXTEND:
    {
      for (size_t i = 0; i < sel.Count(); ++i)
        MoveSelection(sel.Range(i), pdoc->VCHomePosition(sel.Range(i).caret.Position()), true);
    }
    break;

    case SCI_LINEENDEXTEND:
    {
      for (size_t i = 0; i < sel.Count(); ++i)
        MoveSelection(sel.Range(i), pdoc->LineEndPosition(sel.Range(i).caret.Position()), true);
    }
    break;

    case SCI_SELECTIONDUPLICATE:
    {
      if (!sel.Empty())
      {
        int start = std::min(sel.RangeMain().caret.Position(), sel.RangeMain().anchor.Position());
        int end = std::max(sel.RangeMain().caret.Position(), sel.RangeMain().anchor.Position());
        int size = end - start;
        char* text = (char*)alloca(size + 1);
        text[size] = 0;

        pdoc->GetCharRange(text, start, size);

        SelectionRange newSel;
        if (FindTextNotSelected(end, pdoc->Length(), text, newSel))
          sel.AddSelection(newSel);
        else if (FindTextNotSelected(0, start, text, newSel))
          sel.AddSelection(newSel);
      }
    }
    break;

    case SCI_LINESCROLLDOWN:
      ScrollTo(topLine + 1);
      return 0;

    case SCI_LINESCROLLUP:
      ScrollTo(topLine - 1);
      return 0;

    default:
      return ScintillaBase::KeyCommand(iMessage);
  }

  // Whenever a selection moves
  sel.selType = Scintilla::Selection::selStream;

  AutoCompleteCancel();

  sel.RemoveDuplicates();

  ShowCaretAtCurrentPosition();

  int currentLine = pdoc->LineFromPosition(sel.RangeMain().caret.Position());
  if (currentLine >= wrapStart)
    WrapLines(true, -1);
  XYScrollPosition newXY = XYScrollToMakeVisible(true, true, true);
  SetXYScroll(newXY);

  if (highlightDelimiter.NeedsDrawing(currentLine))
    RedrawSelMargin();

  SetLastXChosen();

  return 0;
}

void ScintillaZero::MoveSelection(SelectionRange& selection, int pos, bool extend)
{
  SelectionPosition newPos(pos);

  newPos = ClampPositionIntoDocument(newPos);
  newPos = MovePositionOutsideChar(newPos, pos - selection.caret.Position());

  selection.caret.SetPosition(newPos.Position());
  if (!extend)
    selection.anchor = selection.caret;
}

bool ScintillaZero::FindTextNotSelected(int start, int end, const char* text, SelectionRange& newSel)
{
  int length = strlen(text);
  while (start < end)
  {
    int lengthFound = length;
    int pos = pdoc->FindText(start, end, text, true, false, false, false, 0, &lengthFound, std::auto_ptr<CaseFolder>(CaseFolderForEncoding()).get());
    if (pos == -1)
      return false;

    newSel = SelectionRange(pos + lengthFound, pos);

    bool notSelected = true;
    for (size_t i = 0; i < sel.Count(); ++i)
    {
      SelectionRange& selection = sel.Range(i);

      if (newSel == selection)
      {
        notSelected = false;
        start = std::max(selection.caret.Position(), selection.anchor.Position());
        break;
      }
    }

    if (notSelected)
      return true;
  }

  return false;
}

void ScintillaZero::Initialise()
{
}

void ScintillaZero::Finalise()
{
  ScintillaBase::Finalise();
}

void ScintillaZero::SetVerticalScrollPos()
{
  int textHeight = SendEditor(SCI_TEXTHEIGHT);
  mOwner->mClientOffset.y = -Pixels(topLine * textHeight);
  mOwner->MarkAsNeedsUpdate();
}

void ScintillaZero::SetHorizontalScrollPos()
{
  mOwner->mClientOffset.x = -float(xOffset);
  mOwner->MarkAsNeedsUpdate();
}

bool ScintillaZero::ModifyScrollBars(int nMax, int nPage)
{
  return true;
}

void ScintillaZero::Copy()
{
  if(sel.Empty())
  {
    // Copy the line that the main caret is on
    SelectionText selectedText;
    CopySelectionRange(&selectedText, true);
    Z::gEngine->has(Zero::OsShell)->SetClipboardText(StringRange(selectedText.s, selectedText.s + selectedText.len));
    return;
  }

  // Get newline string
  char newLine[3] = {0};
  if (pdoc->eolMode != SC_EOL_LF)
    strcat(newLine, "\r");
  if (pdoc->eolMode != SC_EOL_CR)
    strcat(newLine, "\n");
  int newLineLength = strlen(newLine);

  // Sort caret order so text is copied top-down
  ScVector<SelectionRange> rangesInOrder = sel.RangesCopy();
  std::sort(rangesInOrder.begin(), rangesInOrder.end());

  StringBuilder builder;

  // Copy characters from every caret
  for (uint i = 0; i < rangesInOrder.size() - 1; ++i)
  {
    SelectionRange& selection = rangesInOrder[i];

    int start = selection.Start().Position();
    int end = selection.End().Position();
    for (int charIndex = start; charIndex < end; ++charIndex)
      builder.Append(pdoc->CharAt(charIndex));
    builder.Append(StringRange(newLine, newLine + newLineLength));
  }

  // Last or only caret, no additional newline
  SelectionRange& selection = rangesInOrder.back();
  for (int charIndex = selection.Start().Position(); charIndex < selection.End().Position(); ++charIndex)
    builder.Append(pdoc->CharAt(charIndex));

  String text = builder.ToString();
  Z::gEngine->has(Zero::OsShell)->SetClipboardText(text.All());
}

void ScintillaZero::Paste()
{
  UndoGroup ug(pdoc);

  Zero::String clipboardText = Z::gEngine->has(Zero::OsShell)->GetClipboardText();

  ClearSelection(multiPasteMode == SC_MULTIPASTE_EACH);

  // Get newline string
  char newLineChars[3] = {0};
  if (pdoc->eolMode != SC_EOL_LF)
    strcat(newLineChars, "\r");
  if (pdoc->eolMode != SC_EOL_CR)
    strcat(newLineChars, "\n");
  String newLine(newLineChars);

  uint newLineCount = 0;
  Array<StringIterator> linePositions;
  linePositions.PushBack(clipboardText.Begin());
  
  Zero::StringRange clipboardRange = clipboardText;
  int newLineRuneCount = newLine.ComputeRuneCount();

  // check for newlines for multiselect positions within the text to paste
  while (!clipboardRange.Empty())
  {
    StringRange newLineGroup = clipboardRange.SubString(clipboardRange.Begin(), clipboardRange.Begin() + newLineRuneCount);
    if (newLineGroup == newLine)
    {
      ++newLineCount;
      linePositions.PushBack(newLineGroup.End());
    }
    clipboardRange.PopFront();
  }

  // Sort caret order to match clipboard for multi-caret copies
  ScVector<SelectionRange>& ranges = sel.GetRanges();
  std::sort(ranges.begin(), ranges.end());

  if (newLineCount == ranges.size() - 1)
  {
    // Paste respective chunks at each caret delimited by newline
    int offset = 0;
    for (uint i = 0; i < ranges.size(); ++i)
    {
      SelectionRange& selection = ranges[i];

      InsertSpace(selection.Start().Position(), selection.Start().VirtualSpace());
      selection.ClearVirtualSpace();
     
      StringIterator start = linePositions[i];
      StringIterator end = (i + 1 < linePositions.Size()) ? (linePositions[i + 1] - newLineRuneCount) : clipboardText.End();
      String lineChunk(start, end);
      pdoc->InsertString(selection.Start().Position(), lineChunk.Data(), lineChunk.SizeInBytes());
    }
  }
  else
  {
    // Paste text at each caret
    for (uint i = 0; i < ranges.size(); ++i)
    {
      SelectionRange& selection = ranges[i];

      InsertSpace(selection.Start().Position(), selection.Start().VirtualSpace());
      selection.ClearVirtualSpace();
      pdoc->InsertString(selection.Start().Position(), clipboardText.c_str(), clipboardText.SizeInBytes());
    }
  }
}

bool ScintillaZero::CanPaste()
{
  return true;
}

void ScintillaZero::ClaimSelection()
{
}

void ScintillaZero::NotifyChange()
{
  mOwner->mScinWidget->MarkAsNeedsUpdate();
}

void ScintillaZero::NotifyFocus(bool focus)
{
}

void ScintillaZero::SetTicking(bool on)
{
  if (timer.ticking != on)
    timer.ticking = on;
  timer.ticksToWait = caret.period;
}

uint ScintillaZero::SendEditor(unsigned int Msg, unsigned long wParam, long lParam)
{
  return WndProc(Msg, wParam, lParam);
}

void ScintillaZero::NotifyParent(SCNotification scn)
{
  mOwner->OnNotify(scn);
}

void ScintillaZero::SetMouseCapture(bool captured)
{
  mMouseCapture = captured;

  if(captured)
    mOwner->mScinWidget->CaptureMouse();
  else
    mOwner->mScinWidget->ReleaseMouseCapture();
}

bool ScintillaZero::HaveMouseCapture()
{
  return mMouseCapture;
}

void ScintillaZero::CreateCallTipWindow(PRectangle rc)
{
}

sptr_t ScintillaZero::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
  switch (iMessage)
  {
    case SCI_CLEAR:
    {
      Clear();
    }
    break;

    case SCI_INSERTTEXT:
    {
       if (lParam == 0)
         return 0;

       int insertPos = wParam;
       if (insertPos == -1)
         insertPos = CurrentPosition();

       int newCurrent = CurrentPosition();
       char *text = reinterpret_cast<char*>(lParam);
       pdoc->InsertCString(insertPos, text);
    }
    break;

    case SCI_MOVESELECTEDLINESUP:
    {
      MoveSelectedLinesUp();
    }
    break;

    case SCI_MOVESELECTEDLINESDOWN:
    {
      MoveSelectedLinesDown();
    }
    break;

    case SCI_GETCURRENTPOS:
    {
      // Easiest way to make the current auto complete logic work without being refactored
      ScVector<SelectionRange>& ranges = sel.GetRanges();
      std::sort(ranges.begin(), ranges.end());
      return ranges.front().Start().Position();
    }
    break;
  }

  return 0;
}

void ScintillaZero::InsertAutoCompleteText(const char* text, int length, int removeCount, int charOffset)
{
  UndoGroup ug(pdoc);

  for (uint i = 0; i < sel.Count(); ++i)
  {
    int start = sel.Range(i).Start().Position() - charOffset;
    pdoc->InsertString(start, text, length);
    pdoc->DeleteChars(start - removeCount, removeCount);
  }
}

void ScintillaZero::AddToPopUp(const char *label, int cmd /*= 0*/, bool enabled /*= true*/)
{
}

void ScintillaZero::UpdateSystemCaret()
{
  ScintillaBase::UpdateSystemCaret();
}

void ScintillaZero::CopyToClipboard(const SelectionText &selectedText)
{
}

void ScintillaZero::NotifyDoubleClick(Point pt, bool shift, bool ctrl, bool alt)
{
}

}
