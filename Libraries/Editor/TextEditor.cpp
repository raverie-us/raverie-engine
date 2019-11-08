// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "ScintillaPlatformZero.hpp"

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
} // namespace Events

ZilchDefineType(TextEditorEvent, builder, type)
{
}

// ScintillaWidget Inner Scintilla Widget
class ScintillaWidget : public Widget
{
public:
  ScintillaWidget(Composite* parent);
  ~ScintillaWidget();
  ScintillaZero* mScintilla;
  Scintilla::SurfaceImpl mSurface;
  void RenderUpdate(ViewBlock& viewBlock,
                    FrameBlock& frameBlock,
                    Mat4Param parentTx,
                    ColorTransform colorTx,
                    WidgetRect clipRect) override;
};

// To prevent interference the set of indicators is divided up into
// a range for use by lexers: [0 .. 7], a range for use by containers:
// [8 = INDIC_CONTAINER .. 31 = INDIC_IME-1], and a range for IME
// indicators: [32 = INDIC_IME .. 35 = INDIC_IME_MAX].
//
//  - Note: "Container" refers to a class of type 'ScintillaBase'.
//          So, 'ScintillaZero' is a container.
namespace ScintillaCustomIndicators
{

enum
{
  TextMatchHighlight = INDIC_CONTAINER
};
}

class ScintillaZero : public Scintilla::ScintillaBase
{
public:
  friend class TextEditor;
  friend class ScintillaWidget;

  ScintillaZero();
  virtual ~ScintillaZero();
  // Method to skip Scintilla's AutoComplete logic
  virtual int KeyCommand(unsigned int iMessage);

  // Scintilla Interface
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
  void NotifyParent(Scintilla::SCNotification scn) override;
  void NotifyDoubleClick(Scintilla::Point pt, bool shift, bool ctrl, bool alt) override;
  // We don't use Scinitilla's built in Cut/Copy/Paste
  // because we handle them specially to match browser behavior.
  void Copy() override;
  bool CanPaste() override;
  void Paste() override;
  void CreateCallTipWindow(Scintilla::PRectangle rc) override;
  void AddToPopUp(const char* label, int cmd = 0, bool enabled = true) override;
  void ClaimSelection() override;
  void CopyToClipboard(const Scintilla::SelectionText& selectedText) override;

  void
  InsertPasteText(const char* text, int len, Scintilla::SelectionPosition selStart, bool isRectangular, bool isLine);
  uint SendEditor(unsigned int Msg, unsigned long wParam = 0, long lParam = 0);
  void InsertAutoCompleteText(const char* text, int length, int removeCount, int charOffset);

public:
  TextEditor* mOwner;
  bool mMouseCapture;

  std::vector<Scintilla::SelectionRange> mHighlightRanges;

  Array<Rectangle> mHighlightIndicators;
  Array<Rectangle> mCursorIndicators;

protected:
  void Clear();
  void NewLine();
  void MoveSelectedLinesUp();
  void MoveSelectedLinesDown();

private:
  void MoveSelection(Scintilla::SelectionRange& selection, int dir, bool extend);

  bool IsSelected(Scintilla::SelectionRange& range, int* endSelectionOut);
  bool FindTextNotSelected(int start, int end, const char* text, Scintilla::SelectionRange& newSel);

  void ClearHighlightRanges();
  void UpdateHighlightIndicators();
  void ProcessTextMatch(char*& text, int* begin, int* end);
  void HighlightMatchingText(int begin, int end, const char* text);
  void HighlightAllTextInstances(int begin, int end, const char* text);

  ScintillaZero& operator=(const ScintillaZero&);
};

void TextFromSelectionRange(int start, int end, char bufferOut[]);

ScintillaWidget::ScintillaWidget(Composite* parent) : Widget(parent), mSurface(this)
{
  // Prevent scrolling issue when scintilla is first created.
  mSize = Pixels(1000, 500);
}

ScintillaWidget::~ScintillaWidget()
{
  SafeDelete(mScintilla);
}

void ScintillaWidget::RenderUpdate(
    ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, WidgetRect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  mSurface.mViewBlock = &viewBlock;
  mSurface.mFrameBlock = &frameBlock;
  mSurface.mViewNode = nullptr;
  mSurface.mColor = colorTx.ColorMultiply;
  mSurface.mBaseRect = clipRect;

  // Setting clip rect here because scintilla is not getting the correct client
  // rect
  mSurface.mClipRect = WidgetRect::PointAndSize(Vec2(parentTx.m30, parentTx.m31), mSize);

  Scintilla::PRectangle rcPaint = mScintilla->GetClientRectangle();
  mScintilla->Paint(&mSurface, rcPaint);
}

static int KeyTranslate(int keyIn)
{
  switch (keyIn)
  {
  case Zero::Keys::Down:
    return SCK_DOWN;
  case Zero::Keys::Up:
    return SCK_UP;
  case Zero::Keys::Left:
    return SCK_LEFT;
  case Zero::Keys::Right:
    return SCK_RIGHT;
  case Zero::Keys::Home:
    return SCK_HOME;
  case Zero::Keys::End:
    return SCK_END;
  case Zero::Keys::PageUp:
    return SCK_PRIOR;
  case Zero::Keys::PageDown:
    return SCK_NEXT;
  case Zero::Keys::Delete:
    return SCK_DELETE;
  case Zero::Keys::Escape:
    return SCK_ESCAPE;
  case Zero::Keys::Back:
    return SCK_BACK;
  case Zero::Keys::Tab:
    return SCK_TAB;
  case Zero::Keys::Enter:
    return SCK_RETURN;
  case Zero::Keys::Alt:
    return SCK_MENU;
  default:
    return keyIn;
  }
}

const int LineNumberMargin = 0;
const int DebuggingMargin = 1;
const int FoldingMargin = 2;

const float cTextEditorVScrollWellWidth = 9.0f;
const float cTextEditorVScrollSliderWidth = 6.0f;
const float cTextEditorVScrollIndicatorWidth = 2.0f;

const float cTextEditorVScrollIndicatorMinHeight = 4.0f;
const float cTextEditorVScrollCursorHeight = 2.0f;

const float cTextEditorVScrollSliderOffset = 2.0f;
const float cTextEditorVScrollIndicatorOffset = 0.0f;

ZilchDefineType(TextEditor, builder, type)
{
}

TextEditor::TextEditor(Composite* parent) : BaseScrollArea(parent)
{
  mTotalMargins = 0;
  mLexer = Lexer::Zilch;

  Scintilla_LinkLexers();
  mScinWidget = new ScintillaWidget(this);
  mScinWidget->SetTakeFocusMode(FocusMode::Hard);
  mScintilla = new ScintillaZero();
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
  mIndicatorsRequireUpdate = false;
  mTextMatchHighlighting = true;
  mHighlightPartialTextMatch = false;
  mMinSize = Vec2(50, 50);

  // Read the current config settings
  UseTextEditorConfig();

  mIndicators = new PixelBuffer();
  mIndicatorDisplay = new TextureView(this);
  mIndicatorDisplay->SetTexture(mIndicators->Image);
  mIndicatorDisplay->SetInteractive(false);
  mIndicatorDisplay->SizeToContents();

  SetScrollWellSize(0, cTextEditorVScrollWellWidth);
  SetScrollSliderSize(0, cTextEditorVScrollSliderWidth);
  SetScrollSliderOffset(0, cTextEditorVScrollSliderOffset);

  Cog* configCog = Z::gEngine->GetConfigCog();
  TextEditorConfig* textConfig = configCog->has(TextEditorConfig);
  mFontSize = textConfig->FontSize;
  ColorScheme* colorScheme = GetColorScheme();
  ConnectThisTo(colorScheme, Events::ColorSchemeChanged, OnColorSchemeChanged);

  // Connections
  ConnectThisTo(mScinWidget, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(mScinWidget, Events::LeftMouseUp, OnMouseUp);
  ConnectThisTo(mScinWidget, Events::KeyDown, OnKeyDown);
  ConnectThisTo(mScinWidget, Events::KeyRepeated, OnKeyDown);

  ConnectThisTo(mScinWidget, Events::Cut, OnCut);
  ConnectThisTo(mScinWidget, Events::Copy, OnCopy);
  ConnectThisTo(mScinWidget, Events::Paste, OnPaste);

  ConnectThisTo(mScinWidget, Events::MouseScroll, OnMouseScroll);
  ConnectThisTo(mScinWidget, Events::FocusLost, OnFocusOut);
  ConnectThisTo(mScinWidget, Events::MouseExitHierarchy, OnMouseExit);
  ConnectThisTo(mScinWidget, Events::FocusGained, OnFocusIn);
  ConnectThisTo(mScinWidget, Events::RightMouseDown, OnRightMouseDown);
  ConnectThisTo(mScinWidget, Events::MouseMove, OnMouseMove);
  ConnectThisTo(mScinWidget, Events::MouseFileDrop, OnMouseFileDrop);
  ConnectThisTo(mScinWidget, Events::TextTyped, OnTextTyped);

  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnUpdate);

  ConnectThisTo(textConfig, Events::PropertyModified, OnConfigPropertyChanged);

  // Clear margins
  SendEditor(SCI_SETMARGINWIDTHN, LineNumberMargin, 0);
  SendEditor(SCI_SETMARGINWIDTHN, FoldingMargin, 0);
  SendEditor(SCI_SETMARGINWIDTHN, DebuggingMargin, 10);

  // Track width or scroll bars will not show up properly
  SendEditor(SCI_SETSCROLLWIDTHTRACKING, true);
  // Start the scroll width as 100
  SendEditor(SCI_SETSCROLLWIDTH, 10);
  // Allow scrolling past the end
  // SendEditor(SCI_SETENDATLASTLINE, false);

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
  SafeDelete(mIndicators);
}

void TextEditor::SetLexer(uint lexer)
{
  SendEditor(SCI_SETVIEWWS, SCWS_INVISIBLE);
  mLineNumbers = false;
  mBreakpoints = false;

  switch (lexer)
  {
  case Lexer::Cpp:
  {
    const char cppKeywords[] = "class const do else extern false float \
        for if in inline int matrix out pass \
        return register static string struct  \
        true typedef false uniform \
        vector void volatile while uint";

    SendEditor(SCI_SETLEXER, SCLEX_CPP, 0);
    SendEditor(SCI_SETKEYWORDS, 0, (uptr_t)cppKeywords);

    mLineNumbers = true;

    break;
  }

  case Lexer::SpirV:
  {
    StringBuilder opCodesBuilder;
    Array<String> opCodeNames = GetOpcodeNames();
    for (size_t i = 0; i < opCodeNames.Size(); ++i)
    {
      String opName = opCodeNames[i];
      // OpCode names don't include the "Op" in the beginning to manually add
      // it.
      opCodesBuilder.AppendFormat("Op%s ", opName.c_str());
    }
    String opCodes = opCodesBuilder.ToString();

    // Hardcoded as there's not clean tools right now to parse this from spirv.
    // In the interest of time this was manually grabbed from the header but
    // should eventually be updated.
    const char languages[] = "Unknown ESSL GLSL OpenCL_C OpenCL_CPP HLSL";
    const char executionModels[] = "Vertex TessellationControl TessellationEvaluation Geometry Fragment "
                                   "GLCompute Kernel";
    const char addressingModels[] = "Logical Physical32 Physical64";
    const char storageClasses[] = "UniformConstant Input Uniform Output Workgroup CrossWorkgroup Private "
                                  "Function Generic PushConstant AtomicCounter Image StorageBuffer";
    const char decorations[] = "RelaxedPrecision SpecId Block BufferBlock RowMajor ColMajor "
                               "ArrayStride MatrixStride GLSLShared GLSLPacked CPacked BuiltIn "
                               "NoPerspective Flat Patch Centroid Sample Invariant Restrict Aliased "
                               "Volatile Constant Coherent NonWritable NonReadable Uniform "
                               "SaturatedConversion Stream Location Component Index Binding "
                               "DescriptorSet Offset XfbBuffer XfbStride FuncParamAttr FPRoundingMode "
                               "FPFastMathMode LinkageAttributes NoContraction InputAttachmentIndex "
                               "Alignment MaxByteOffset AlignmentId MaxByteOffsetId ExplicitInterpAMD "
                               "OverrideCoverageNV PassthroughNV ViewportRelativeNV "
                               "SecondaryViewportRelativeNV NonUniformEXT HlslCounterBufferGOOGLE "
                               "HlslSemanticGOOGLE";
    const char builtIns[] = "Position PointSize ClipDistance CullDistance VertexId InstanceId "
                            "PrimitiveId InvocationId Layer ViewportIndex TessLevelOuter "
                            "TessLevelInner TessCoord PatchVertices FragCoord PointCoord "
                            "FrontFacing SampleId SamplePosition SampleMask FragDepth "
                            "HelperInvocation NumWorkgroups WorkgroupSize WorkgroupId "
                            "LocalInvocationId GlobalInvocationId LocalInvocationIndex WorkDim "
                            "GlobalSize EnqueuedWorkgroupSize GlobalOffset GlobalLinearId "
                            "SubgroupSize SubgroupMaxSize NumSubgroups NumEnqueuedSubgroups "
                            "SubgroupId SubgroupLocalInvocationId VertexIndex InstanceIndex "
                            "SubgroupEqMask SubgroupEqMaskKHR SubgroupGeMask SubgroupGeMaskKHR "
                            "SubgroupGtMask SubgroupGtMaskKHR SubgroupLeMask SubgroupLeMaskKHR "
                            "SubgroupLtMask SubgroupLtMaskKHR BaseVertex BaseInstance DrawIndex "
                            "DeviceIndex ViewIndex BaryCoordNoPerspAMD BaryCoordNoPerspCentroidAMD "
                            "BaryCoordNoPerspSampleAMD BaryCoordSmoothAMD "
                            "BaryCoordSmoothCentroidAMD BaryCoordSmoothSampleAMD "
                            "BaryCoordPullModelAMD FragStencilRefEXT ViewportMaskNV "
                            "SecondaryPositionNV SecondaryViewportMaskNV PositionPerViewNV "
                            "ViewportMaskPerViewNV FullyCoveredEXT";
    const char capabilities[] = "Matrix Shader Geometry Tessellation Addresses Linkage Kernel Vector16 "
                                "Float16Buffer Float16 Float64 Int64 Int64Atomics ImageBasic "
                                "ImageReadWrite ImageMipmap Pipes Groups DeviceEnqueue LiteralSampler "
                                "AtomicStorage Int16 TessellationPointSize GeometryPointSize "
                                "ImageGatherExtended StorageImageMultisample "
                                "UniformBufferArrayDynamicIndexing SampledImageArrayDynamicIndexing "
                                "StorageBufferArrayDynamicIndexing StorageImageArrayDynamicIndexing "
                                "ClipDistance CullDistance ImageCubeArray SampleRateShading ImageRect "
                                "SampledRect GenericPointer Int8 InputAttachment SparseResidency "
                                "MinLod Sampled1D Image1D SampledCubeArray SampledBuffer ImageBuffer "
                                "ImageMSArray StorageImageExtendedFormats ImageQuery DerivativeControl "
                                "InterpolationFunction TransformFeedback GeometryStreams "
                                "StorageImageReadWithoutFormat StorageImageWriteWithoutFormat "
                                "MultiViewport SubgroupDispatch NamedBarrier PipeStorage "
                                "GroupNonUniform GroupNonUniformVote GroupNonUniformArithmetic "
                                "GroupNonUniformBallot GroupNonUniformShuffle "
                                "GroupNonUniformShuffleRelative GroupNonUniformClustered "
                                "GroupNonUniformQuad SubgroupBallotKHR DrawParameters SubgroupVoteKHR "
                                "StorageBuffer16BitAccess StorageUniformBufferBlock16 StorageUniform16 "
                                "UniformAndStorageBuffer16BitAccess StoragePushConstant16 "
                                "StorageInputOutput16 DeviceGroup MultiView "
                                "VariablePointersStorageBuffer VariablePointers AtomicStorageOps "
                                "SampleMaskPostDepthCoverage StorageBuffer8BitAccess "
                                "UniformAndStorageBuffer8BitAccess StoragePushConstant8 "
                                "Float16ImageAMD ImageGatherBiasLodAMD FragmentMaskAMD "
                                "StencilExportEXT ImageReadWriteLodAMD SampleMaskOverrideCoverageNV "
                                "GeometryShaderPassthroughNV ShaderViewportIndexLayerEXT "
                                "ShaderViewportIndexLayerNV ShaderViewportMaskNV ShaderStereoViewNV "
                                "PerViewAttributesNV FragmentFullyCoveredEXT "
                                "GroupNonUniformPartitionedNV ShaderNonUniformEXT "
                                "RuntimeDescriptorArrayEXT InputAttachmentArrayDynamicIndexingEXT "
                                "UniformTexelBufferArrayDynamicIndexingEXT "
                                "StorageTexelBufferArrayDynamicIndexingEXT "
                                "UniformBufferArrayNonUniformIndexingEXT "
                                "SampledImageArrayNonUniformIndexingEXT "
                                "StorageBufferArrayNonUniformIndexingEXT "
                                "StorageImageArrayNonUniformIndexingEXT "
                                "InputAttachmentArrayNonUniformIndexingEXT "
                                "UniformTexelBufferArrayNonUniformIndexingEXT "
                                "StorageTexelBufferArrayNonUniformIndexingEXT SubgroupShuffleINTEL "
                                "SubgroupBufferBlockIOINTEL SubgroupImageBlockIOINTEL";

    StringBuilder specialBuilder;
    specialBuilder.Append(languages);
    specialBuilder.Append(" ");
    specialBuilder.Append(executionModels);
    specialBuilder.Append(" ");
    specialBuilder.Append(addressingModels);
    specialBuilder.Append(" ");
    specialBuilder.Append(storageClasses);
    specialBuilder.Append(" ");
    specialBuilder.Append(decorations);
    specialBuilder.Append(" ");
    specialBuilder.Append(builtIns);
    specialBuilder.Append(" ");
    specialBuilder.Append(capabilities);
    String specialWords = specialBuilder.ToString();

    SendEditor(SCI_SETLEXER, SCLEX_CPP, 0);
    SendEditor(SCI_SETKEYWORDS, 0, (uptr_t)opCodes.c_str());
    SendEditor(SCI_SETKEYWORDS, 1, (uptr_t)specialWords.c_str());

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
    // Use python to highlight 'quotes'
    SendEditor(SCI_SETLEXER, (uptr_t)SCLEX_CONTAINER, 0);
    // No keywords
    SendEditor(SCI_SETKEYWORDS, 0, (sptr_t) "");
    SendEditor(SCI_SETKEYWORDS, 1, (sptr_t) "");

    mLineNumbers = false;
    break;
  }

  case Lexer::Python:
  {
    const char pythonKeywords[] = "and del for is raise assert elif from lambda return "
                                  "break else global not try class except if or while "
                                  "continue exec import pass yield def finally in print show";

    const char pythonSpecial[] = "self event True False None";

    SendEditor(SCI_SETLEXER, (uptr_t)SCLEX_PYTHON, 0);
    SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)pythonKeywords);
    SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)pythonSpecial);

    // View White space
    SendEditor(SCI_SETVIEWWS, SCWS_VISIBLEALWAYS);
    mLineNumbers = true;

    break;
  }

  case Lexer::Text:
  {
    SendEditor(SCI_SETLEXER, (uptr_t)SCLEX_CONTAINER, 0);
    SendEditor(SCI_SETKEYWORDS, 0, (sptr_t) "");
    SendEditor(SCI_SETKEYWORDS, 1, (sptr_t) "");
    SendEditor(SCI_SETVIEWWS, SCWS_INVISIBLE);
    break;
  }

  case Lexer::Zilch:
  {
    const char zilchKeywords[] = "abstract alias alignof as assert Assign auto base break case catch "
                                 "checked "
                                 "class compare const constructor continue copy decrement default "
                                 "delegate delete "
                                 "destructor do dynamic else enum explicit export extern false finally "
                                 "fixed "
                                 "flags for foreach friend function get global goto if immutable "
                                 "implicit import in include "
                                 "increment inline interface internal is local lock loop module mutable "
                                 "namespace new "
                                 "null operator out override package params partial positional private "
                                 "protected public "
                                 "readonly ref register require return sealed sends set signed sizeof "
                                 "stackalloc static "
                                 "struct switch throw true try typedef typeid typename typeof type "
                                 "unchecked unsafe unsigned "
                                 "using var virtual volatile where while yield timeout scope debug";

    const char zilchSpecial[] = "this value event";

    SendEditor(SCI_SETLEXER, (uptr_t)SCLEX_CPP, 0);
    SendEditor(SCI_SETKEYWORDS, 0, (sptr_t)zilchKeywords);
    SendEditor(SCI_SETKEYWORDS, 1, (sptr_t)zilchSpecial);
    SendEditor(SCI_SETVIEWWS, SCWS_INVISIBLE);
    mLineNumbers = true;
    mBreakpoints = true;
    break;
  }
  }

  mLexer = (Lexer::Enum)lexer;
  UpdateColorScheme();
}

void TextEditor::UpdateMargins(ColorScheme& scheme)
{
  mTotalMargins = 0;

  if (mBreakpoints)
  {
    // For Debugging Set up markers
    mTotalMargins += 16;
    SendEditor(SCI_SETMARGINTYPEN, DebuggingMargin, SC_MARGIN_SYMBOL);
    SendEditor(SCI_SETMARGINWIDTHN, DebuggingMargin, 16);
    SendEditor(SCI_SETMARGINSENSITIVEN, DebuggingMargin, true);
    SendEditor(SCI_SETMARGINMASKN, DebuggingMargin, 0x000000FF);

    SendEditor(SCI_MARKERDEFINE, BreakPointMarker, SC_MARK_CIRCLE);
    SendEditor(SCI_MARKERSETFORE, BreakPointMarker, Red);
    SendEditor(SCI_MARKERSETBACK, BreakPointMarker, Red);

    SendEditor(SCI_MARKERDEFINE, InstructionMarker, SC_MARK_SHORTARROW);
    SendEditor(SCI_MARKERSETFORE, InstructionMarker, Yellow);
    SendEditor(SCI_MARKERSETBACK, InstructionMarker, Yellow);
  }
  else
  {
    mTotalMargins += 10;
    SendEditor(SCI_SETMARGINWIDTHN, DebuggingMargin, 10);
  }

  // Set up code folding
  if (mFolding)
  {
    mTotalMargins += 16;

    // Enable folding
    SendEditor(SCI_SETPROPERTY, (u64) "fold", (s64) "1");
    SendEditor(SCI_SETPROPERTY, (u64) "fold.compact", (s64) "1");
    SendEditor(SCI_SETPROPERTY, (u64) "fold.comment", (s64) "1");
    SendEditor(SCI_SETPROPERTY, (u64) "fold.preprocessor", (s64) "1");

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
    SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);

    // Set all colors for folding markers
    for (int i = SC_MARKNUM_FOLDEREND; i <= SC_MARKNUM_FOLDEROPEN; ++i)
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

  if (mLineNumbers && mLineNumberMargin)
  {
    uint width = SendEditor(SCI_TEXTWIDTH, STYLE_LINENUMBER, (intptr_t) "9999");
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
  if (enabled)
  {
    SendEditor(SCI_SETWRAPMODE, SC_WRAP_WORD, 0);
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
  return SendEditor(SCI_GETREADONLY) != 0;
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
  if (GetReadOnly())
  {
    SetReadOnly(false);
    SendEditor(SCI_APPENDTEXT, text.ComputeRuneCount(), (intptr_t)text.Data());
    SetReadOnly(true);
  }
  else
  {
    SendEditor(SCI_APPENDTEXT, text.ComputeRuneCount(), (intptr_t)text.Data());
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
  if (config)
  {
    ConnectThisTo(config, Events::PropertyModified, OnConfigChanged);
    this->UpdateConfig(config);
  }
}

void TextEditor::OnConfigChanged(PropertyEvent* event)
{
  TextEditorConfig* config = GetConfig();
  this->UpdateConfig(config);
}

void TextEditor::OnConfigPropertyChanged(PropertyEvent* event)
{
  // All text editors use text match highlighting settings.  If the the text
  // editor needs to use ALL TextEditorConfig properties, then
  // 'UseTextEditorConfig' should be called after text editor init/construction.
  String name = event->mProperty.GetLeafPropertyName();
  if (name != "TextMatchHighlighting" && name != "HighlightPartialTextMatch")
    return;

  TextEditorConfig* config = GetConfig();

  mTextMatchHighlighting = config->TextMatchHighlighting;
  mHighlightPartialTextMatch = config->HighlightPartialTextMatch;
  if (!mTextMatchHighlighting)
  {
    mScintilla->ClearHighlightRanges();
    mScintilla->mHighlightIndicators.Clear();
    mScintilla->mCursorIndicators.Clear();
  }
}

void TextEditor::OnColorSchemeChanged(ObjectEvent* event)
{
  UpdateColorScheme();
}

TextEditorConfig* TextEditor::GetConfig()
{
  // Check if the editor is present for when this is called from the launcher
  if (Z::gEditor)
  {
    auto config = Z::gEditor->mConfig->has(TextEditorConfig);
    ErrorIf(config == nullptr, "The config should always have a TextEditorConfig component");
    return config;
  }
  return nullptr;
}

void TextEditor::UpdateConfig(TextEditorConfig* textConfig)
{
  mColorSchemeName = textConfig->ColorScheme;

  mFontSize = textConfig->FontSize;
  mFolding = textConfig->CodeFolding;
  mLineNumberMargin = textConfig->LineNumbers;

  mTextMatchHighlighting = textConfig->TextMatchHighlighting;
  mHighlightPartialTextMatch = textConfig->HighlightPartialTextMatch;
  if (!mTextMatchHighlighting)
  {
    mScintilla->ClearHighlightRanges();
    mScintilla->mHighlightIndicators.Clear();
    mScintilla->mCursorIndicators.Clear();
  }

  SetLexer(mLexer);
  UpdateColorScheme();

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
  // character > 255 is quick UTF8 fix
  if (IsGraph(r) || r == Keys::Space || r > 255)
  {
    byte utf8Bytes[4];
    int bytesRead = UTF8::UnpackUtf8RuneIntoBuffer(r, utf8Bytes);
    mScintilla->AddCharUTF((char*)utf8Bytes, bytesRead);
  }
  else if (Keyboard::Instance->KeyIsDown(Keys::Shift))
  {
    // Note: Ctrl + A to select all text will not result in useful highlight
    //       criteria.  So, no need to check for it.  Additionally, for text
    //       highlighting criteria, Shift +: Up, Down, PageUp, and PageDown
    //       produces invalid text selections.  If the selection anchor and
    //       caret are on different lines, then highlight criteria is invalid.
    bool usingHighlightModifier = Keyboard::Instance->KeyIsDown(Keys::Left);
    usingHighlightModifier |= Keyboard::Instance->KeyIsDown(Keys::Right);
    usingHighlightModifier |= Keyboard::Instance->KeyIsDown(Keys::Home);
    usingHighlightModifier |= Keyboard::Instance->KeyIsDown(Keys::End);

    if (usingHighlightModifier)
      return;
  }

  // Text selection was cancelled.
  if (Keyboard::Instance->IsAnyNonModifierDown())
  {
    mScintilla->ClearHighlightRanges();
    mScintilla->mHighlightIndicators.Clear();
  }
}

void TextEditor::OnRightMouseDown(MouseEvent* event)
{
  Vec2 p = ToLocal(event->Position);
  mScintilla->ButtonDown(Scintilla::Point(p.x, p.y), mTime, event->ShiftPressed, event->CtrlPressed, event->AltPressed);
  mScintilla->ButtonUp(Scintilla::Point(p.x, p.y), mTime, false);
}

void TextEditor::OnMouseDown(MouseEvent* event)
{
  Vec2 p = ToLocal(event->Position);
  mScintilla->ButtonDown(Scintilla::Point(p.x, p.y), mTime, event->ShiftPressed, event->CtrlPressed, event->AltPressed);
}

void TextEditor::OnMouseMove(MouseEvent* event)
{
  Vec2 p = ToLocal(event->Position);
  mScintilla->ButtonMove(Scintilla::Point(p.x, p.y));
}

void TextEditor::OnMouseUp(MouseEvent* event)
{
  Vec2 p = ToLocal(event->Position);
  mScintilla->ButtonUp(Scintilla::Point(p.x, p.y), mTime, false);
}

void TextEditor::OnMouseFileDrop(MouseFileDropEvent* event)
{
}

void TextEditor::UpdateArea(ScrollUpdate::Enum type)
{
  if (type != ScrollUpdate::Auto)
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
  uint lines = GetClientLineCount();
  uint size = SendEditor(SCI_TEXTHEIGHT);
  uint width = mScintilla->scrollWidth;

  // Add the size of the margins and some buffer
  // space size of the scroll area
  width += mTotalMargins + Pixels(20);

  return Pixels(width, lines * size);
}

// 'OnUpdate' ticks will cause 'mScinWidget' to get marked for updating.
// So, as intended, 'UpdateTransform' will be triggered at least every tick,
// besides normal triggered updates due to resizing, scrolling, etc...
void TextEditor::UpdateTransform()
{
  UpdateScrollBars();

  mScinWidget->SetSize(mVisibleSize);
  mScinWidget->mScintilla->ChangeSize();

  mIndicatorsRequireUpdate = true;

  BaseScrollArea::UpdateTransform();
}

// Not included in color styles
#define SCE_ERROR 30

#define SCE_LINK 31

void TextEditor::UpdateColorScheme()
{
  ColorScheme* colors = GetColorScheme();
  if (colors)
    SetColorScheme(*colors);
}

void TextEditor::SetColorScheme(ColorScheme& scheme)
{
  // Copies global style to all others
  SendEditor(SCI_STYLECLEARALL);

  // Set the overall default style for text (foreground and background color)
  SetAStyle(STYLE_DEFAULT, ToByteColor(scheme.Default), ToByteColor(scheme.Background), mFontSize, "Inconsolata");

  // Set style for Line number / gutter
  SetAStyle(STYLE_LINENUMBER, ToByteColor(scheme.GutterText), ToByteColor(scheme.Gutter));

  // Set the color of any whitespace identifiers (the . where spaces are, and ->
  // where tabs are)
  SendEditor(SCI_SETWHITESPACEFORE, true, ToByteColor(scheme.Whitespace));

  // Set the color when we select text
  SendEditor(SCI_SETSELBACK, true, ToByteColor(scheme.Selection));

  // Set the color of the current line our caret is on
  SendEditor(SCI_SETCARETLINEBACK, ToByteColor(scheme.LineSelection));

  // Set the caret color to be the default text color
  SendEditor(SCI_SETCARETFORE, ToByteColor(scheme.Default));
  SendEditor(SCI_SETADDITIONALCARETFORE, ToByteColor(scheme.Default));

  uint background = ToByteColor(scheme.Background);

  switch (mLexer)
  {
  case Lexer::Zilch:
  {
    SetCommonLexerStyles(scheme);

    SetAStyle(SCE_C_CHARACTER, ToByteColor(scheme.Default),
              background); // Character literals: 'c'
    break;
  }
  case Lexer::Shader:
  case Lexer::SpirV:
  case Lexer::Cpp:
  {
    SetCommonLexerStyles(scheme);
    break;
  }

  case Lexer::Console:
  case Lexer::Text:
  {
    SetAStyle(SCE_P_DEFAULT, ToByteColor(scheme.Default), background); // Default text
    SetAStyle(SCE_P_IDENTIFIER, ToByteColor(scheme.Default),
              background); // Identifiers: self.Space

    SetAStyle(SCE_P_COMMENTBLOCK, ToByteColor(scheme.Default),
              background); // Block comments: /* */, """
    SetAStyle(SCE_P_COMMENTLINE, ToByteColor(scheme.Default),
              background); // Line comments: #, //
    SetAStyle(SCE_P_TRIPLE, ToByteColor(scheme.Default),
              background); // Triple comment
    SetAStyle(SCE_P_TRIPLEDOUBLE, ToByteColor(scheme.Default),
              background);                                               // Triple comment
    SetAStyle(SCE_P_STRINGEOL, ToByteColor(scheme.Default), background); // ?
    SetAStyle(SCE_P_DECORATOR, ToByteColor(scheme.Default), background); // ?

    SetAStyle(SCE_P_NUMBER, ToByteColor(scheme.Number),
              background); // Number literals: 5, 3.9
    SetAStyle(SCE_P_CHARACTER, ToByteColor(scheme.Default),
              background); // Character literals: 'c'
    SetAStyle(SCE_P_STRING, ToByteColor(scheme.StringLiteral),
              background); // String literals: "hello world"

    SetAStyle(SCE_P_CLASSNAME, ToByteColor(scheme.Default),
              background); // Class names: RigidBody, Model
    SetAStyle(SCE_P_DEFNAME, ToByteColor(scheme.Default),
              background); // Function names: OnLogicUpdate
    SetAStyle(SCE_P_OPERATOR, ToByteColor(scheme.Default),
              background); // Operators: () += .

    SetAStyle(SCE_P_WORD, ToByteColor(scheme.Default),
              background); // Keywords: class, if
    SetAStyle(SCE_P_WORD2, ToByteColor(scheme.Default),
              background); // Context keywords: self, this, value

    SetAStyle(SCE_ERROR, ToByteColor(scheme.Error),
              background); // Errors / exceptions

    SetAStyle(SCE_LINK, ToByteColor(scheme.Link), background); // Link text

    SendEditor(SCI_STYLESETUNDERLINE, SCE_LINK, true);
    SendEditor(SCI_STYLESETHOTSPOT, SCE_LINK, true);
    break;
  }

  case Lexer::Python:
  {
    SetAStyle(SCE_P_DEFAULT, ToByteColor(scheme.Default), background); // Default text
    SetAStyle(SCE_P_IDENTIFIER, ToByteColor(scheme.Default),
              background); // Identifiers: self.Space

    SetAStyle(SCE_P_COMMENTBLOCK, ToByteColor(scheme.Comment),
              background); // Block comments: /* */, """
    SetAStyle(SCE_P_COMMENTLINE, ToByteColor(scheme.Comment),
              background); // Line comments: #, //
    SetAStyle(SCE_P_TRIPLE, ToByteColor(scheme.Comment),
              background); // Triple comment
    SetAStyle(SCE_P_TRIPLEDOUBLE, ToByteColor(scheme.Comment),
              background);                                               // Triple comment
    SetAStyle(SCE_P_STRINGEOL, ToByteColor(scheme.Comment), background); // ?
    SetAStyle(SCE_P_DECORATOR, ToByteColor(scheme.Comment), background); // ?

    SetAStyle(SCE_P_NUMBER, ToByteColor(scheme.Number),
              background); // Number literals: 5, 3.9
    SetAStyle(SCE_P_CHARACTER, ToByteColor(scheme.StringLiteral),
              background); // Character literals: 'c'
    SetAStyle(SCE_P_STRING, ToByteColor(scheme.StringLiteral),
              background); // String literals: "hello world"

    SetAStyle(SCE_P_CLASSNAME, ToByteColor(scheme.ClassName),
              background); // Class names: RigidBody, Model
    SetAStyle(SCE_P_DEFNAME, ToByteColor(scheme.FunctionName),
              background); // Function names: OnLogicUpdate
    SetAStyle(SCE_P_OPERATOR, ToByteColor(scheme.Operator),
              background); // Operators: () += .

    SetAStyle(SCE_P_WORD, ToByteColor(scheme.Keyword),
              background); // Keywords: class, if
    SetAStyle(SCE_P_WORD2, ToByteColor(scheme.SpecialWords),
              background); // Context keywords: self, this, value

    SetAStyle(SCE_ERROR, ToByteColor(scheme.Error),
              background); // Errors / exceptions

    SetAStyle(SCE_LINK, ToByteColor(scheme.Link), background); // Link text

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

  SetAStyle(SCE_C_DEFAULT, ToByteColor(scheme.Default), background); // Default text
  SetAStyle(SCE_C_IDENTIFIER, ToByteColor(scheme.Default),
            background); // Identifiers: self.Space

  SetAStyle(SCE_C_COMMENT, ToByteColor(scheme.Comment),
            background); // Block comments: /* */, """
  SetAStyle(SCE_C_COMMENTDOC, ToByteColor(scheme.Comment),
            background); // Block comments: /* */, """
  SetAStyle(SCE_C_COMMENTLINE, ToByteColor(scheme.Comment),
            background); // Line comments: #, //
  SetAStyle(SCE_C_COMMENTLINEDOC, ToByteColor(scheme.Comment),
            background);                                                      // Line comments: #, //
  SetAStyle(SCE_C_UUID, ToByteColor(scheme.Comment), background);             // ?
  SetAStyle(SCE_C_STRINGEOL, ToByteColor(scheme.Comment), background);        // ?
  SetAStyle(SCE_C_VERBATIM, ToByteColor(scheme.Comment), background);         // ?
  SetAStyle(SCE_C_TRIPLEVERBATIM, ToByteColor(scheme.Comment), background);   // ?
  SetAStyle(SCE_C_HASHQUOTEDSTRING, ToByteColor(scheme.Comment), background); // ?
  SetAStyle(SCE_C_REGEX, ToByteColor(scheme.Comment), background);            // ?

  SetAStyle(SCE_C_NUMBER, ToByteColor(scheme.Number),
            background); // Number literals: 5, 3.9
  SetAStyle(SCE_C_CHARACTER, ToByteColor(scheme.StringLiteral),
            background); // Character literals: 'c'
  SetAStyle(SCE_C_STRING, ToByteColor(scheme.StringLiteral),
            background); // String literals: "hello world"
  SetAStyle(SCE_C_STRINGRAW, ToByteColor(scheme.StringLiteral),
            background); // String literals: "hello world"

  SetAStyle(SCE_C_GLOBALCLASS, ToByteColor(scheme.ClassName),
            background); // Class names: RigidBody, Model
  SetAStyle(SCE_C_OPERATOR, ToByteColor(scheme.Operator),
            background); // Operators: () += .

  SetAStyle(SCE_C_WORD, ToByteColor(scheme.Keyword),
            background); // Keywords: class, if
  SetAStyle(SCE_C_COMMENTDOCKEYWORD, ToByteColor(scheme.Keyword),
            background); // Documentation keywords
  SetAStyle(SCE_C_PREPROCESSOR, ToByteColor(scheme.Directive),
            background); // Preprocessor directives
  SetAStyle(SCE_C_WORD2, ToByteColor(scheme.SpecialWords),
            background); // Context keywords: self, this, value

  SetAStyle(SCE_ERROR, ToByteColor(scheme.Error), background); // Errors / exceptions
  SetAStyle(SCE_C_COMMENTDOCKEYWORDERROR, ToByteColor(scheme.Error),
            background); // Documentation errors
}

void TextEditor::OnKeyDown(KeyboardEvent* event)
{
  if (event->Key == Keys::Back || event->Key == Keys::Delete)
    ClearAnnotations();

  // Prevent scintilla from processing these keys
  if (event->Key >= Keys::F1 && event->Key <= Keys::F12)
    return;

  int key = KeyTranslate(event->Key);

  bool consumed = false;
  int handled = mScintilla->KeyDown(key, event->ShiftPressed, event->CtrlPressed, event->AltPressed, &consumed);

  // Check to see if the event has been consumed to prevent other widgets
  // from doing things
  if (consumed)
    event->Handled = true;

  if (event->Key == Keys::D && event->ShiftPressed && event->CtrlPressed)
  {
    mScintilla->WndProc(SCI_LINEDUPLICATE, 0, 0);
    // Get the current caret position and find the line that was just duplicated
    int line = GetLineFromPosition(mScintilla->SelectionStart().Position());
    // If that line duplicated is not in view scroll it into view
    MakeLineVisible(line);
  }

  if (event->Key == Keys::Down && event->ShiftPressed && event->CtrlPressed)
    mScintilla->WndProc(SCI_MOVESELECTEDLINESDOWN, 0, 0);

  if (event->Key == Keys::Up && event->ShiftPressed && event->CtrlPressed)
    mScintilla->WndProc(SCI_MOVESELECTEDLINESUP, 0, 0);

  if (event->CtrlPressed)
  {
    if (event->Key == Keys::Minus)
    {
      SetFontSize(mFontSize - 1);
      // ref T3197
      // SetFontSize internally calls SetColorScheme, which is required.
      // It is being called a second time here because of an underlying
      // bug that is currently outside the scope of fixing, but this fixes
      // the behavioral problems without breaking anything else.
      // The issue is described in the above phabricator issue.
      // The reason I found this fix for the change in behavior is that while
      // ctrl+plus and ctrl+minus didn't work consistently and scrollwheel did,
      // which is calling SetColorScheme after SetFontSize.
      // I suspect it has something to do with us responding to windows message
      // pump events and then scintilla settings being set using the windows
      // message pump also - Dane Curbow
      UpdateColorScheme();
    }
    if (event->Key == Keys::Equal)
    {
      SetFontSize(mFontSize + 1);
      // ref T3197
      UpdateColorScheme();
    }
    // if(event->Key == Keys::R)
    //  SetWordWrap(true);
  }

  // If not a keyboard short cut handle the key
  if (!(event->AltPressed || event->CtrlPressed))
    event->Handled = true;
}

void TextEditor::SetFontSize(int size)
{
  mFontSize = Math::Max(size, cMinFontSize);
  UpdateColorScheme();
}

void TextEditor::OnCut(ClipboardEvent* event)
{
  OnCopy(event);
  mScintilla->Cut();
}

void TextEditor::OnCopy(ClipboardEvent* event)
{
  if (mScintilla->sel.Empty())
  {
    // Copy the line that the main caret is on
    Scintilla::SelectionText selectedText;
    mScintilla->CopySelectionRange(&selectedText, true);
    event->SetText(String(selectedText.s, selectedText.s + selectedText.len));
    return;
  }

  // Get newline string
  char newLine[3] = {0};
  if (mScintilla->pdoc->eolMode != SC_EOL_LF)
    strcat(newLine, "\r");
  if (mScintilla->pdoc->eolMode != SC_EOL_CR)
    strcat(newLine, "\n");
  int newLineLength = strlen(newLine);

  // Sort caret order so text is copied top-down
  std::vector<Scintilla::SelectionRange> rangesInOrder = mScintilla->sel.RangesCopy();
  std::sort(rangesInOrder.begin(), rangesInOrder.end());

  StringBuilder builder;

  // Copy characters from every caret
  for (uint i = 0; i < rangesInOrder.size() - 1; ++i)
  {
    Scintilla::SelectionRange& selection = rangesInOrder[i];

    int start = selection.Start().Position();
    int end = selection.End().Position();
    for (int charIndex = start; charIndex < end; ++charIndex)
      builder.Append(mScintilla->pdoc->CharAt(charIndex));
    builder.Append(StringRange(newLine, newLine + newLineLength));
  }

  // Last or only caret, no additional newline
  Scintilla::SelectionRange& selection = rangesInOrder.back();
  for (int charIndex = selection.Start().Position(); charIndex < selection.End().Position(); ++charIndex)
    builder.Append(mScintilla->pdoc->CharAt(charIndex));

  String text = builder.ToString();
  event->SetText(text);
}

void TextEditor::OnPaste(ClipboardEvent* event)
{
  Scintilla::UndoGroup ug(mScintilla->pdoc);

  Zero::String clipboardText = event->GetText();

  mScintilla->ClearSelection(mScintilla->multiPasteMode == SC_MULTIPASTE_EACH);

  // Get newline string
  char newLineChars[3] = {0};
  if (mScintilla->pdoc->eolMode != SC_EOL_LF)
    strcat(newLineChars, "\r");
  if (mScintilla->pdoc->eolMode != SC_EOL_CR)
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
    StringRange newLineGroup =
        clipboardRange.SubString(clipboardRange.Begin(), clipboardRange.Begin() + newLineRuneCount);
    if (newLineGroup == newLine)
    {
      ++newLineCount;
      linePositions.PushBack(newLineGroup.End());
    }
    clipboardRange.PopFront();
  }

  // Sort caret order to match clipboard for multi-caret copies
  std::vector<Scintilla::SelectionRange>& ranges = mScintilla->sel.GetRanges();
  std::sort(ranges.begin(), ranges.end());

  if (newLineCount == ranges.size() - 1)
  {
    // Paste respective chunks at each caret delimited by newline
    int offset = 0;
    for (uint i = 0; i < ranges.size(); ++i)
    {
      Scintilla::SelectionRange& selection = ranges[i];

      mScintilla->InsertSpace(selection.Start().Position(), selection.Start().VirtualSpace());
      selection.ClearVirtualSpace();

      StringIterator start = linePositions[i];
      StringIterator end =
          (i + 1 < linePositions.Size()) ? (linePositions[i + 1] - newLineRuneCount) : clipboardText.End();
      String lineChunk(start, end);
      mScintilla->pdoc->InsertString(selection.Start().Position(), lineChunk.Data(), lineChunk.SizeInBytes());
    }
  }
  else
  {
    // Paste text at each caret
    for (uint i = 0; i < ranges.size(); ++i)
    {
      Scintilla::SelectionRange& selection = ranges[i];

      mScintilla->InsertSpace(selection.Start().Position(), selection.Start().VirtualSpace());
      selection.ClearVirtualSpace();
      mScintilla->pdoc->InsertString(selection.Start().Position(), clipboardText.c_str(), clipboardText.SizeInBytes());
    }
  }
}

void TextEditor::OnMouseScroll(MouseEvent* event)
{
  OsShell* shell = Z::gEngine->has(OsShell);
  int scroll = (int)shell->GetScrollLineCount();
  scroll = int(scroll * event->Scroll.y);

  // vertical scroll
  if (!event->CtrlPressed && !event->ShiftPressed)
    mScintilla->ScrollTo(mScintilla->topLine + scroll * -1);
  // horizontal scroll when holding shift
  else if (!event->CtrlPressed && event->ShiftPressed)
    mScintilla->HorizontalScrollTo(mScintilla->xOffset + scroll * -10);
  // change font size while holding control and scrolling
  else
  {
    mFontSize = Math::Clamp(int(mFontSize + scroll), cMinFontSize, cMaxFontSize);
    SetFontSize(mFontSize);
    UpdateColorScheme();
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

intptr_t TextEditor::SendEditor(unsigned int Msg, u64 wParam, s64 lParam)
{
  return (intptr_t)mScintilla->WndProc(Msg, (uptr_t)wParam, (sptr_t)lParam);
}

void TextEditor::GetText(int start, int end, char* buffer, int bufferSize)
{
  // SCI_GETTEXTRANGE requires that the buffer have space for a null terminator
  int size = end - start;
  ReturnIf(size >= bufferSize, , "The buffer was too small for the text");

  Scintilla::Sci_TextRange textRange;
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
  return StringRange(buffer, buffer + size);
}

int TextEditor::GetRuneAt(int position)
{
  return SendEditor(SCI_GETCHARAT, position);
}

DeclareEnum3(CommentMode, Auto, Comment, Uncomment);

void TextEditor::BlockComment(cstr comment)
{
  int commentLen = strlen(comment);

  // Get the selection information
  int selectionStart = SendEditor(SCI_GETSELECTIONSTART);
  int selectionEnd = SendEditor(SCI_GETSELECTIONEND);
  int selStartLine = SendEditor(SCI_LINEFROMPOSITION, selectionStart);
  int selEndLine = SendEditor(SCI_LINEFROMPOSITION, selectionEnd);

  // Get all of the current caret positions
  Array<int> caretPositions;
  GetAllCaretPositions(caretPositions);

  // Get the lines that have active carets without duplicate entries
  HashSet<int> linesSelected;
  forRange (int caretPos, caretPositions.All())
  {
    int line = SendEditor(SCI_LINEFROMPOSITION, caretPos);
    linesSelected.Insert(line);
  }

  // If there is a selection get all the lines selected
  for (int currentLine = selStartLine; currentLine <= selEndLine; ++currentLine)
    linesSelected.Insert(currentLine);

  // Buffer for testing text for comments
  const int bufferSize = commentLen + 1;
  char* buffer = (char*)alloca(bufferSize);

  CommentMode::Enum commentMode = CommentMode::Auto;

  // Check lines for indent level and comment mode
  int minIndentLevel = INT_MAX;
  forRange (int currentLine, linesSelected.All())
  {
    int lineStart = SendEditor(SCI_POSITIONFROMLINE, currentLine);
    int lineEnd = SendEditor(SCI_GETLINEENDPOSITION, currentLine);
    int lineIndent = SendEditor(SCI_GETLINEINDENTPOSITION, currentLine);

    // Skip blank lines.
    if (lineIndent == lineEnd)
      continue;

    if (commentMode == CommentMode::Auto)
    {
      GetText(lineIndent, lineIndent + commentLen, buffer, bufferSize);
      // Is it a comment? Then uncomment everything.
      if (strncmp(buffer, comment, commentLen) == 0)
        commentMode = CommentMode::Uncomment;
      else
        commentMode = CommentMode::Comment;
    }

    minIndentLevel = Math::Min(lineIndent - lineStart, minIndentLevel);
  }

  // Comment and uncomment all the lines
  SendEditor(SCI_BEGINUNDOACTION);
  forRange (int currentLine, linesSelected.All())
  {
    int lineStart = SendEditor(SCI_POSITIONFROMLINE, currentLine);
    int lineEnd = SendEditor(SCI_GETLINEENDPOSITION, currentLine);
    int lineIndent = SendEditor(SCI_GETLINEINDENTPOSITION, currentLine);

    // Skip blank lines.
    if (lineIndent == lineEnd)
      continue;

    if (commentMode == CommentMode::Comment)
    {
      // comment the line inserting as min indent level
      SendEditor(SCI_INSERTTEXT, lineStart + minIndentLevel, (sptr_t)comment);
    }
    else
    {
      // Get the text where a comment would be present on this line
      GetText(lineIndent, lineIndent + commentLen, buffer, bufferSize);
      // Is it a comment?
      if (strncmp(buffer, comment, commentLen) == 0)
      {
        // Remove the comment
        RemoveRange(lineIndent, commentLen);
      }
    }
  }
  SendEditor(SCI_ENDUNDOACTION);
}

void TextEditor::SetAStyle(int style, ByteColor fore, ByteColor back, int size, cstr face)
{
  SendEditor(SCI_STYLESETFORE, style, fore);
  SendEditor(SCI_STYLESETBACK, style, back);

  if (size >= 1)
    SendEditor(SCI_STYLESETSIZE, style, size);

  if (face)
    SendEditor(SCI_STYLESETFONT, style, (sptr_t)(face));
}

void TextEditor::OnUpdate(UpdateEvent* event)
{
  ErrorIf(event->RealDt < 0.0f, "The RealDt was negative");
  uint msTime = (uint)((int)(event->RealDt * 1000.0f));
  mTickTime += msTime;
  mTime += msTime;
  if (mTickTime > uint(mScintilla->timer.ticksToWait))
  {
    mScinWidget->MarkAsNeedsUpdate();

    // tick to blink caret
    mScintilla->Tick();
    mTickTime = 0;
  }

  if (mIndicatorsRequireUpdate)
  {
    mIndicatorsRequireUpdate = false;
    UpdateTextMatchHighlighting();
  }
}

void TextEditor::UpdateTextMatchHighlighting()
{
  ScrollBar* verticalBar = GetVerticalScrollBar();
  Vec2 bufferSize = verticalBar->mSize + verticalBar->mBackground->mSize;
  Vec3 position = verticalBar->mBackground->mTranslation;
  ByteColor color = ToByteColor(verticalBar->mBackground->GetColor());

  if (verticalBar->mVisible)
  {
    mIndicators->Resize(verticalBar->mSize.x + bufferSize.x, bufferSize.y, false, false, Color::White, false);

    mIndicatorDisplay->SetTranslation(Pixels(position.x + 1, position.y, position.z));
    mIndicatorDisplay->SetSize(Pixels(bufferSize.x, bufferSize.y));
  }

  mIndicators->Clear(byte(0));
  mScintilla->UpdateHighlightIndicators();

  // Call after 'UpdateHighlightIndicators' so that cursor indicators
  // draw on top of the highlight indicators.
  UpdateIndicators(mScintilla->mCursorIndicators,
                   mScintilla->sel.GetRanges(),
                   Vec4(1, 1, 1, 0.85f),
                   Vec2(cTextEditorVScrollCursorHeight, 0),
                   0,
                   0);

  mIndicators->Upload();
}

void TextEditor::UpdateIndicators(Array<Rectangle>& indicators,
                                  const std::vector<Scintilla::SelectionRange>& ranges,
                                  Vec4Param indicatorColor,
                                  Vec2Param minIndicatorHeight,
                                  float indicatorWidth,
                                  float indicatorOffsetX)
{
  ScrollBar* verticalBar = GetVerticalScrollBar();
  if (!verticalBar->mVisible)
    return;

  indicators.Clear();

  // No indicators?  Then don't do anything.
  if (ranges.empty() || !mTextMatchHighlighting)
    return;

  float lineCount = GetClientLineCount();
  float barHeight = verticalBar->mBackground->mSize.y;

  Vec2 size;
  if (indicatorWidth == 0)
    size.x = Math::Floor(verticalBar->mSize.x + verticalBar->mBackground->mSize.x);
  else
    size.x = indicatorWidth;

  // If minIndicatorHeight.y == 0, then the height will default to
  // minIndicatorHeight.x, which is intended behavior.
  float indicatorHeight = minIndicatorHeight.y * (barHeight * 1.0f / lineCount);
  size.y = Math::Floor(Math::Max(minIndicatorHeight.x, indicatorHeight));

  float top;

  int count = ranges.size();
  for (int i = 0; i < count; ++i)
  {
    top = Math::Floor(barHeight * GetLineFromPosition(ranges[i].anchor.Position()) / lineCount);

    Rectangle& rect = indicators.PushBack();
    rect.Min = Vec2(indicatorOffsetX, top);
    rect.Max = Vec2(indicatorOffsetX + size.x,
                    top + size.y - 1.0f); // - 1 to make it zero-indexed

    mIndicators->FillRect(rect.Min, rect.Max, ToByteColor(indicatorColor));
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
  // When updating the position in a document change the corresponding scroll
  // position
  MakePositionVisible(position);
}

void TextEditor::GoToLine(int lineNumber)
{
  SendEditor(SCI_GOTOLINE, lineNumber);
  // When updating the position in a document change the corresponding scroll
  // position
  MakeLineVisible(lineNumber);
}

void TextEditor::MakePositionVisible(int position)
{
  // Update transform is called to make sure the widget has the correct
  // flag set on which scroll bars are visible so when ScrollAreaToView
  // is called it properly scrolls the specified area into view.
  UpdateTransform();

  Vec2 clientSize = GetClientSize();

  int column = SendEditor(SCI_GETCOLUMN, position);
  int line = SendEditor(SCI_LINEFROMPOSITION, position);
  int textHeight = SendEditor(SCI_TEXTHEIGHT, line);

  float xPos = column;
  float yPos = line * textHeight;

  // With the way we use scintilla SCI_TEXTWIDTH always returns 0
  Vec2 minPos(xPos, yPos);
  Vec2 maxPos(xPos, yPos + textHeight);

  ScrollAreaToView(minPos, maxPos, false);
}

void TextEditor::MakeLineVisible(int line)
{
  int position = SendEditor(SCI_POSITIONFROMLINE, line);
  MakePositionVisible(position);
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

  // Read the current line into a buffer, and get the cursor position
  // (lineOffset) into that line
  const int BufferSize = 8192;
  char buffer[BufferSize];
  int lineOffset = GetCurrentLineText(buffer, BufferSize);

  // Default the position to be in the middle
  LinePosition::Enum position = LinePosition::Middle;

  // Assume that everything after the cursor is whitespace (until the end of the
  // line)...
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

  // Basically, if everything after the cursor was empty (then we are really at
  // the end)
  if (isAllSpaceAfterCursor)
    position = LinePosition::End;

  // Assume that everything before the cursor is whitespace (until the end of
  // the line)...
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

  // Basically, if everything before the cursor was empty (then we are really at
  // the beginning) Note that it can be both at the beginning and end (if the
  // line is empty), but beginning takes precedence
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
  int position = SendEditor(SCI_GETCURRENTPOS);
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

int TextEditor::GetClientLineCount()
{
  // Editable line count of document.
  int lineCount = GetLineCount();
  // Current height of text.
  int textHeight = SendEditor(SCI_TEXTHEIGHT);
  // If enabled, additional line count that would fit in the over-scroll client
  // area.
  lineCount += !mScintilla->endAtLastLine * mVisibleSize.y / textHeight;

  return lineCount;
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
  SendEditor(SCI_ADDTEXT, text.SizeInBytes(), (intptr_t)text.Data());
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
  if (line == -1)
    SendEditor(SCI_MARKERDELETEALL, type);
  else
    SendEditor(SCI_MARKERDELETE, line, type);
}

int TextEditor::GetNextMarker(int lineStart, int type)
{
  return GetNextMarkerMask(lineStart, (1 << type));
}

int TextEditor::GetPreviousMarker(int lineStart, int type)
{
  return GetPreviousMarkerMask(lineStart, (1 << type));
}

int TextEditor::GetNextMarkerMask(int lineStart, int markerMask)
{
  return SendEditor(SCI_MARKERNEXT, lineStart, markerMask);
}

int TextEditor::GetPreviousMarkerMask(int lineStart, int markerMask)
{
  return SendEditor(SCI_MARKERPREVIOUS, lineStart, markerMask);
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

bool TextEditor::MarkerExists(int line, int type)
{
  return (GetMarkerMask(line) & (1 << type)) != 0;
}

int TextEditor::GetMarkerMask(int line)
{
  return SendEditor(SCI_MARKERGET, line);
  ;
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

void TextEditor::SetAnnotation(int lineNumber, StringParam errorMessage, bool goToLine)
{
  String wrappedMessage = WordWrap(errorMessage, 80);

  // Don't set annotations on this line if we already have one there with the
  // same text
  String& existingMessage = AnnotationLines[lineNumber];
  if (existingMessage != wrappedMessage)
  {
    existingMessage = wrappedMessage;

    // ANNOTATION_STANDARD //ANNOTATION_BOXED
    SendEditor(SCI_ANNOTATIONSETVISIBLE, ANNOTATION_STANDARD);
    SendEditor(SCI_ANNOTATIONSETTEXT, lineNumber, (intptr_t)wrappedMessage.c_str());
    SendEditor(SCI_ANNOTATIONSETSTYLE, lineNumber, SCE_ERROR);
  }

  if (goToLine)
  {
    // Go to one past the line, otherwise we might not be able to
    // see the annotations that are after the current line
    GoToLine(lineNumber + 1);
    // SendEditor(SCI_SCROLLCARET);
  }
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
  const int lineNumber = SendEditor(SCI_LINEFROMPOSITION, position, 0);

  switch (notify.nmhdr.code)
  {
  case SCN_MODIFIED:
  {
  }
  break;

  case SCN_MARGINCLICK:
  {
    if (margin == FoldingMargin)
      SendEditor(SCI_TOGGLEFOLD, lineNumber, 0);
    else if (margin == DebuggingMargin)
      BreakpointsClicked(lineNumber, position);
  }
  break;

  case SCN_AUTOCSELECTION:
  case SCN_AUTOCCANCELLED:
  {
    SendEditor(SCI_AUTOCSETFILLUPS, 0, (s64) "");
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
    bool shouldSendEvent =
        (notify.nmhdr.code == SCN_MODIFIED && notify.modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)) ||
        (notify.nmhdr.code == SCN_CHARADDED || notify.nmhdr.code == SCN_KEY);

    if (shouldSendEvent)
    {
      // We have to get the line from the current caret position because some
      // Scintilla on notify messages do not fill out the position/line modified
      // causing incorrect behavior when attempting to scroll off screen
      // modification into view
      MakeLineVisible(GetLineFromPosition(mScintilla->SelectionStart().Position()));

      Event event;
      this->GetDispatcher()->Dispatch(Events::TextEditorModified, &event);
    }
  }
}

// Implement Scintilla in Zero Ui
ScintillaZero::ScintillaZero()
{
  mMouseCapture = false;

  // Overwrite Scintilla's default behavior of mapping cut/copy/paste.
  // We do this since now the operating system (or browser) sends cut/copy/paste externally.
  kmap.AssignCmdKey(SCK_DELETE, SCI_SHIFT, SCI_NULL); // SCI_CUT
  kmap.AssignCmdKey(SCK_INSERT, SCI_SHIFT, SCI_NULL); // SCI_PASTE
  kmap.AssignCmdKey(SCK_INSERT, SCI_CTRL, SCI_NULL);  // SCI_COPY

  kmap.AssignCmdKey('X', SCI_CTRL, SCI_NULL); // SCI_CUT
  kmap.AssignCmdKey('C', SCI_CTRL, SCI_NULL); // SCI_COPY
  kmap.AssignCmdKey('V', SCI_CTRL, SCI_NULL); // SCI_PASTE
}

ScintillaZero::~ScintillaZero()
{
}

void ScintillaZero::Clear()
{
  if (sel.Empty())
  {
    bool singleVirtual = false;
    if ((sel.Count() == 1) && !RangeContainsProtected(sel.MainCaret(), sel.MainCaret() + 1) &&
        sel.RangeMain().Start().VirtualSpace())
    {
      singleVirtual = true;
    }
    Scintilla::UndoGroup ug(pdoc, (sel.Count() > 1) || singleVirtual);
    for (size_t r = 0; r < sel.Count(); r++)
    {
      if (!RangeContainsProtected(sel.Range(r).caret.Position(), sel.Range(r).caret.Position() + 1))
      {
        if (sel.Range(r).Start().VirtualSpace())
        {
          if (sel.Range(r).anchor < sel.Range(r).caret)
            sel.Range(r) = Scintilla::SelectionPosition(
                InsertSpace(sel.Range(r).anchor.Position(), sel.Range(r).anchor.VirtualSpace()));
          else
            sel.Range(r) = Scintilla::SelectionPosition(
                InsertSpace(sel.Range(r).caret.Position(), sel.Range(r).caret.VirtualSpace()));
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
  Scintilla::UndoGroup ug(pdoc);

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
  Scintilla::UndoGroup ug(pdoc);

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
  Scintilla::UndoGroup ug(pdoc);

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

      Scintilla::SelectionRange& selection = sel.Range(i);
      for (size_t j = 0; j < i; ++j)
      {
        Scintilla::SelectionRange& other = sel.Range(j);

        if (other.caret.Position() < selection.anchor.Position() &&
            other.anchor.Position() > selection.caret.Position())
        {
          Scintilla::SelectionPosition anchor(std::max(other.anchor.Position(), selection.anchor.Position()));
          Scintilla::SelectionPosition caret(std::min(other.caret.Position(), selection.caret.Position()));
          Scintilla::SelectionRange newRange(caret, anchor);

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

      Scintilla::SelectionRange& selection = sel.Range(i);
      for (size_t j = 0; j < i; ++j)
      {
        Scintilla::SelectionRange& other = sel.Range(j);

        if (other.caret.Position() > selection.anchor.Position() &&
            other.anchor.Position() < selection.caret.Position())
        {
          Scintilla::SelectionPosition anchor(std::min(other.anchor.Position(), selection.anchor.Position()));
          Scintilla::SelectionPosition caret(std::max(other.caret.Position(), selection.caret.Position()));
          Scintilla::SelectionRange newRange(caret, anchor);

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

      Scintilla::SelectionRange newSel;
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

void ScintillaZero::MoveSelection(Scintilla::SelectionRange& selection, int pos, bool extend)
{
  Scintilla::SelectionPosition newPos(pos);

  newPos = ClampPositionIntoDocument(newPos);
  newPos = MovePositionOutsideChar(newPos, pos - selection.caret.Position());

  selection.caret.SetPosition(newPos.Position());
  if (!extend)
    selection.anchor = selection.caret;
}

bool ScintillaZero::IsSelected(Scintilla::SelectionRange& range, int* endSelectionOut)
{
  for (size_t i = 0; i < sel.Count(); ++i)
  {
    Scintilla::SelectionRange& current = sel.Range(i);
    if (range == current)
    {
      *endSelectionOut = std::max(current.caret.Position(), current.anchor.Position());
      return true;
    }
  }

  return false;
}

bool ScintillaZero::FindTextNotSelected(int start, int end, const char* text, Scintilla::SelectionRange& newSel)
{
  int length = strlen(text);
  while (start < end)
  {
    int lengthFound = length;
    int pos = pdoc->FindText(start,
                             end,
                             text,
                             true,
                             false,
                             false,
                             false,
                             0,
                             &lengthFound,
                             std::unique_ptr<Scintilla::CaseFolder>(CaseFolderForEncoding()).get());
    if (pos == -1)
      return false;

    newSel = Scintilla::SelectionRange(pos + lengthFound, pos);

    if (!IsSelected(newSel, &start))
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
  // This logic is now handled in OnCopy to be more browser like,
  // since we cannot explicitly access the clipboard.
}

void ScintillaZero::Paste()
{
}

bool ScintillaZero::CanPaste()
{
  return true;
}

void ScintillaZero::ClaimSelection()
{
}

void ScintillaZero::ClearHighlightRanges()
{
  if (mHighlightRanges.empty())
    return;

  const uint indicator = ScintillaCustomIndicators::TextMatchHighlight;
  const uint LexerAgnosticDefaultStyle = 0;

  // Select the custom indicator.
  SendEditor(SCI_SETINDICATORCURRENT, indicator);

  // SCI_INDICATORCLEARRANGE, reset the value over the entire document body to
  // 0.
  //   - Note1: A rangeValue == 0, regardless of lexer type, is the default
  //   value for
  //           the entire document.
  //   - Note2: Filling a range that is the entire document body, doesn't
  //   actually
  //           walk the entire document's text.  Rather, all non-zero style
  //           ranges [ie, previous highlight indicators] are set to a style of
  //           0.  Then they are all collapsed to a single range as they are
  //           now all the same style, 0.
  pdoc->DecorationFillRange(0, LexerAgnosticDefaultStyle, pdoc->Length());

  mHighlightRanges.clear();
}

void ScintillaZero::UpdateHighlightIndicators()
{
  // Cleanup old highlighting incase main selection has changed.
  ClearHighlightRanges();

  Scintilla::SelectionRange& main = sel.RangeMain();

  int lineAnchor = SendEditor(SCI_LINEFROMPOSITION, main.anchor.Position());
  int lineCaret = SendEditor(SCI_LINEFROMPOSITION, main.caret.Position());

  Scintilla::SelectionRange& rectangle = sel.Rectangular();
  int rectAnchor = SendEditor(SCI_LINEFROMPOSITION, rectangle.anchor.Position());
  int rectCaret = SendEditor(SCI_LINEFROMPOSITION, rectangle.caret.Position());

  // 1) Text matching is globally disabled.
  // 2) Nothing in selection.
  // 3) Selection line-wraps.
  // 4) Selection is rectangular.
  // 5) Selection has virtual space.
  if (!mOwner->mTextMatchHighlighting || main.Empty() || lineAnchor != lineCaret || rectAnchor != rectCaret ||
      main.anchor.VirtualSpace() || main.caret.VirtualSpace())
  {
    mHighlightIndicators.Clear();
    return;
  }

  int begin = std::min(main.caret.Position(), main.anchor.Position());
  int end = std::max(main.caret.Position(), main.anchor.Position());
  int size = end - begin;

  // All white space is not valid criteria for text highlighting.
  bool isWhiteSpace = true;
  for (int i = 0; i < size; ++i)
  {
    Rune r = (char)SendEditor(SCI_GETCHARAT, begin + i);
    isWhiteSpace &= UTF8::IsWhiteSpace(r);
  }

  if (isWhiteSpace)
  {
    mHighlightIndicators.Clear();
    return;
  }

  // +1 for null terminator.
  char* text = (char*)alloca(size + 1);

  pdoc->GetCharRange(text, begin, size);
  text[size] = 0;

  // Remove white space, determine if partial text or whole text match, etc...
  ProcessTextMatch(text, &begin, &end);
  // Invalid partial or whole text
  if (text == nullptr)
    return;

  HighlightMatchingText(begin, end, text);
  mOwner->UpdateIndicators(mHighlightIndicators,
                           mHighlightRanges,
                           GetColorScheme()->TextMatchIndicator,
                           Vec2(cTextEditorVScrollIndicatorMinHeight, 1),
                           cTextEditorVScrollIndicatorWidth,
                           cTextEditorVScrollIndicatorOffset);
}

void ScintillaZero::ProcessTextMatch(char*& text, int* begin, int* end)
{
  // Partial text match requires no processing.
  if (mOwner->mHighlightPartialTextMatch)
  {
    int size = *end - *begin;
    pdoc->GetCharRange(text, *begin, size);
    text[size] = 0;

    return;
  }

  String string(text);

  StringRange trimmed = string.TrimStart();
  trimmed = trimmed.TrimEnd();

  int startDelta = trimmed.Begin().Data() - string.Begin().Data();

  *begin += startDelta;
  *end -= string.End().Data() - trimmed.End().Data();

  int start = *begin;
  int stop = *end;
  int size = stop - start;

  text = &text[startDelta];

  Rune first = (char)SendEditor(SCI_GETCHARAT, start);

  bool isAlphaNumeric = true;
  bool isOperator = true;

  for (int i = 0; i < size; ++i)
  {
    Rune r = (char)SendEditor(SCI_GETCHARAT, start + i);

    // isalnum, Note: isalnum != !ispunct
    isAlphaNumeric &= UTF8::IsAlphaNumeric(r);
    // isgraph && !isalnum
    isOperator &= UTF8::IsPunctuation(r);
  }

  int lineBegin = SendEditor(SCI_LINEFROMPOSITION, start);
  int lineEnd = SendEditor(SCI_LINEFROMPOSITION, stop - 1);
  int lineBeginNeighbor = SendEditor(SCI_LINEFROMPOSITION, start - 1);
  int lineEndNeighbor = SendEditor(SCI_LINEFROMPOSITION, stop);

  Rune a = (char)SendEditor(SCI_GETCHARAT, start - 1);
  Rune b = (char)SendEditor(SCI_GETCHARAT, stop);

  // 1) Is 'begin' at the start of the document?
  // 2) Is begin at the start of a line?
  bool beginValid = start == 0 || lineBegin != lineBeginNeighbor;
  // 3) Is begin's previous char whitespace or an alpha numeric?
  bool opBeginValid = start > 0 && (UTF8::IsWhiteSpace(a) || UTF8::IsAlphaNumeric(a));

  // 1) Is 'end' at the end of the document?
  // 2) Is end at the end of a line?
  bool endValid = stop == pdoc->Length() || lineEnd != lineEndNeighbor;
  // 3) Is end whitespace or alpha numeric?
  bool opEndValid = stop < pdoc->Length() && (UTF8::IsWhiteSpace(b) || UTF8::IsAlphaNumeric(b));

  // Check for chained operator chars surrounded by either whitespace or alpha
  // numerics.
  if (isOperator && (beginValid || opBeginValid) && (endValid || opEndValid))
  {
    pdoc->GetCharRange(text, start, size);
    text[size] = 0;

    return;
  }

  // Mix of operators and alpha-numerics not allowed.
  if (!isAlphaNumeric)
  {
    text = nullptr;
    return;
  }

  // 3) Is begin's previous char whitespace or an operator?
  beginValid |= (start > 0 && (UTF8::IsWhiteSpace(a) || UTF8::IsPunctuation(a)));
  // 3) Is end whitespace or an operator?
  endValid |= (stop < pdoc->Length() && (UTF8::IsWhiteSpace(b) || UTF8::IsPunctuation(b)));

  if (!beginValid || !endValid)
  {
    text = nullptr;
    return;
  }

  pdoc->GetCharRange(text, start, size);
  text[size] = 0;
}

void ScintillaZero::HighlightMatchingText(int begin, int end, const char* text)
{
  const uint indicator = ScintillaCustomIndicators::TextMatchHighlight;

  int outlineAlpha = GetColorScheme()->TextMatchOutlineAlpha * CS::MaxByte;
  Vec4 matchColor = GetColorScheme()->TextMatchHighlight * CS::MaxByte;
  Scintilla::ColourDesired color(matchColor.x, matchColor.y, matchColor.z);

  // Select the custom indicator.
  SendEditor(SCI_SETINDICATORCURRENT, indicator);

  // Note: the following lines, with appended comments, could be
  // done with a call to 'SendEditor' using the defined symbol
  // in each line's respective comment.  However, they all call
  // 'InvalidateStyleRedraw'.  Yet, it only needs to be called once
  // after setting all desired indicator params.
  vs.indicators[indicator].style = (s64)IndicatorStyle::Roundbox; // SCI_INDICSETSTYLE
  vs.indicators[indicator].under = true;                          // SCI_INDICSETUNDER
  vs.indicators[indicator].fore = color;                          // SCI_INDICSETFORE
  vs.indicators[indicator].outlineAlpha = outlineAlpha;           // SCI_INDICSETOUTLINEALPHA
  vs.indicators[indicator].fillAlpha = matchColor.w;              // SCI_INDICSETALPHA
  InvalidateStyleRedraw();

  // Search the document
  HighlightAllTextInstances(begin, end, text);
}

void ScintillaZero::HighlightAllTextInstances(int begin, int end, const char* text)
{
  // rangeValue == 0 is reserved as the default value for the entire document.
  // rangeValue == 1 is reserved for the main 'text' selected.
  int rangeValue = 2;

  // Document portion before 'text'.
  int start = 0;
  int stop = begin;

  int length = strlen(text);

  // +1 for null terminator.
  char* match = (char*)alloca(length + 1);
  memcpy(match, text, length + 1);

  const uint indicator = ScintillaCustomIndicators::TextMatchHighlight;

  // There is the portion of the document to search before 'text',
  // and the portion of the document to search after 'text'.
  for (int portion = 0; portion < 2; ++portion)
  {
    while (start < stop)
    {
      int lengthFound = length;
      int pos = pdoc->FindText(start,
                               stop,
                               text,
                               true,
                               false,
                               false,
                               false,
                               0,
                               &lengthFound,
                               std::unique_ptr<Scintilla::CaseFolder>(CaseFolderForEncoding()).get());
      if (pos == -1)
        break;

      int anchor = std::min(pos, pos + lengthFound);
      int caret = std::max(pos, pos + lengthFound);

      // Adhere to partial vs whole match, 'FindText' returns only partial
      // matches.
      char* candidate = match;
      ProcessTextMatch(candidate, &anchor, &caret);
      if (candidate == nullptr)
      {
        // Next search position.
        start = caret;
        continue;
      }

      Scintilla::SelectionRange range(caret, anchor);

      if (!IsSelected(range, &caret))
      {
        // Select the custom indicator.
        SendEditor(SCI_SETINDICATORCURRENT, indicator);

        // Assign a value to this instance of 'text'.  Useful if needed
        // to recall/find this instance later.
        SendEditor(SCI_SETINDICATORVALUE, rangeValue++);
        // Decorate the the instance of the text range with the current,
        // internal, indicator style.
        SendEditor(SCI_INDICATORFILLRANGE, anchor, lengthFound);

        mHighlightRanges.push_back(range);
      }

      // Next search position.
      start = caret;
    }

    // Document portion after 'text'.
    start = end;
    stop = pdoc->Length();
  }
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

void ScintillaZero::NotifyParent(Scintilla::SCNotification scn)
{
  mOwner->OnNotify(scn);
}

void ScintillaZero::SetMouseCapture(bool captured)
{
  mMouseCapture = captured;

  if (captured)
    mOwner->mScinWidget->CaptureMouse();
  else
    mOwner->mScinWidget->ReleaseMouseCapture();
}

bool ScintillaZero::HaveMouseCapture()
{
  return mMouseCapture;
}

void ScintillaZero::CreateCallTipWindow(Scintilla::PRectangle rc)
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
    char* text = reinterpret_cast<char*>(lParam);
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
    // Easiest way to make the current auto complete logic work without being
    // refactored
    std::vector<Scintilla::SelectionRange>& ranges = sel.GetRanges();
    std::sort(ranges.begin(), ranges.end());
    return ranges.front().Start().Position();
  }
  break;
  }

  return 0;
}

void ScintillaZero::InsertAutoCompleteText(const char* text, int length, int removeCount, int charOffset)
{
  Scintilla::UndoGroup ug(pdoc);

  for (uint i = 0; i < sel.Count(); ++i)
  {
    int start = sel.Range(i).Start().Position() - charOffset;
    pdoc->InsertString(start, text, length);
    pdoc->DeleteChars(start - removeCount, removeCount);
  }
}

void ScintillaZero::AddToPopUp(const char* label, int cmd /*= 0*/, bool enabled /*= true*/)
{
}

void ScintillaZero::UpdateSystemCaret()
{
  ScintillaBase::UpdateSystemCaret();
}

void ScintillaZero::CopyToClipboard(const Scintilla::SelectionText& selectedText)
{
}

void ScintillaZero::NotifyDoubleClick(Scintilla::Point pt, bool shift, bool ctrl, bool alt)
{
}

} // namespace Zero
