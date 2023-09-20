// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void DispatchCodeTranslatorScriptError(StringParam eventId,
                                       StringParam shortMessage,
                                       StringParam fullMessage,
                                       const Raverie::CodeLocation& location)
{
  // This should only happen when a composite has a raverie error. Figure out how
  // to report later?
  if (location.CodeUserData == nullptr)
    return;

  Resource* resource = (Resource*)location.CodeUserData;
  DocumentResource* documentResource = Type::DynamicCast<DocumentResource*>(resource);
  if (resource == nullptr)
    return;

  ScriptEvent e;
  e.Script = documentResource;
  e.Message = shortMessage;
  e.Location = location;
  Z::gResources->DispatchEvent(eventId, &e);

  Console::Print(Filter::DefaultFilter, "%s", fullMessage.c_str());
}

// Simple error callback function for the compositor.
// Sends out the event to display an inline error in the script file.
void OnRaverieFragmentCompilationError(Raverie::ErrorEvent* e)
{
  String shortMessage = e->ExactError;
  String fullMessage = e->GetFormattedMessage(Raverie::MessageFormat::Python);
  DispatchCodeTranslatorScriptError(Events::SyntaxError, shortMessage, fullMessage, e->Location);
}

void OnRaverieFragmentTranslationError(TranslationErrorEvent* e)
{
  String fullMessage = e->GetFormattedMessage(Raverie::MessageFormat::Python);
  DispatchCodeTranslatorScriptError(Events::SyntaxError, e->mShortMessage, fullMessage, e->mLocation);
}

TranslatedShaderScriptEditor::TranslatedShaderScriptEditor(Composite* parent) : ScriptEditor(parent)
{
  // mShaderGenerator = nullptr;
  ConnectThisTo(RaverieManager::GetInstance(), Events::ScriptsCompiledPostPatch, OnBuild);
}

TranslatedShaderScriptEditor::~TranslatedShaderScriptEditor()
{
  // delete mShaderGenerator;
}

void TranslatedShaderScriptEditor::Build()
{
  Vec2 scrollPercent = GetScrolledPercentage();
  SetAllText(OnTranslate());
  SetScrolledPercentage(scrollPercent);
}
void TranslatedShaderScriptEditor::OnBuild(Event* e)
{
  Build();
}

String TranslatedShaderScriptEditor::OnTranslate()
{
  return "";
}

FragmentFileTranslatorScriptEditor::FragmentFileTranslatorScriptEditor(Composite* parent) :
    TranslatedShaderScriptEditor(parent)
{
  mFragment = nullptr;
}

void FragmentFileTranslatorScriptEditor::SetResource(RaverieFragment* fragment)
{
  mFragment = fragment;
  ListenForModified(mFragment);
}

String FragmentFileTranslatorScriptEditor::OnTranslate()
{
  return String();
}

RaverieCompositorScriptEditor::RaverieCompositorScriptEditor(Composite* parent) : TranslatedShaderScriptEditor(parent)
{
  mMaterial = nullptr;
}

void RaverieCompositorScriptEditor::SetResource(Material* material)
{
  mMaterial = material;
  ListenForModified(mMaterial);
}

String RaverieCompositorScriptEditor::OnTranslate()
{
  return String();
}

TranslatedRaverieCompositorScriptEditor::TranslatedRaverieCompositorScriptEditor(Composite* parent) :
    TranslatedShaderScriptEditor(parent)
{
  mMaterial = nullptr;
  mDisplayMode = TranslationDisplayMode::Full;
}

void TranslatedRaverieCompositorScriptEditor::SetResource(Material* material)
{
  mMaterial = material;
  ListenForModified(mMaterial);
}

String TranslatedRaverieCompositorScriptEditor::OnTranslate()
{
  return String();
}

void TranslatedRaverieCompositorScriptEditor::SetDisplayMode(TranslationDisplayMode::Enum displayMode)
{
  mDisplayMode = displayMode;
}

BaseSplitScriptEditor::BaseSplitScriptEditor(Composite* parent) : Composite(parent)
{
  // Can't called setup now as virtual functions won't properly be setup yet
}

void BaseSplitScriptEditor::Setup()
{
  SetLayout(CreateRowLayout());

  mSourceEditor = new ScriptEditor(this);
  mSplitter = new Splitter(this);
  SetTranslatedEditor();

  mSourceEditor->SetSizing(SizeAxis::X, SizePolicy::Flex, 200);
  mSplitter->SetSizing(SizeAxis::X, SizePolicy::Fixed, 4);
  mTranslatedEditor->SetSizing(SizeAxis::X, SizePolicy::Flex, 200);

  ConnectThisTo(Z::gEditor, Events::Save, OnSaveCheck);
  ConnectThisTo(RaverieManager::GetInstance(), Events::ScriptsCompiledPostPatch, OnBuild);
  ConnectThisTo(mTranslatedEditor, Events::LeftMouseDown, OnLeftMouseDown);
}

void BaseSplitScriptEditor::SetTranslatedEditor()
{
}

void BaseSplitScriptEditor::SaveCheck()
{
}

void BaseSplitScriptEditor::Build()
{
}

void BaseSplitScriptEditor::SetLexer(uint lexer)
{
  mSourceEditor->SetLexer(Lexer::Raverie);
  mTranslatedEditor->SetLexer(lexer);
}

void BaseSplitScriptEditor::OnSaveCheck(SavingEvent* e)
{
  SaveCheck();
}

void BaseSplitScriptEditor::OnBuild(Event* e)
{
  Build();
}

void BaseSplitScriptEditor::OnLeftMouseDown(MouseEvent* e)
{
}

FragmentSplitScriptEditor::FragmentSplitScriptEditor(Composite* parent) : BaseSplitScriptEditor(parent)
{
  // This has to be called here so that the virtual functions will be properly
  // set
  Setup();
}

void FragmentSplitScriptEditor::SetTranslatedEditor()
{
  mTranslatedEditor = new FragmentFileTranslatorScriptEditor(this);
}

void FragmentSplitScriptEditor::SaveCheck()
{
  if (mSourceEditor->IsModified())
    mFragment->mText = mSourceEditor->GetAllText();
}

void FragmentSplitScriptEditor::Build()
{
  mSourceEditor->SetAllText(mFragment->mText);
  mTranslatedEditor->Build();
}


void FragmentSplitScriptEditor::SetResource(RaverieFragment* fragment)
{
  mFragment = fragment;
  FragmentFileTranslatorScriptEditor* editor = (FragmentFileTranslatorScriptEditor*)mTranslatedEditor;
  editor->SetResource(fragment);
}

MaterialSplitScriptEditor::MaterialSplitScriptEditor(Composite* parent) : BaseSplitScriptEditor(parent)
{
  mDisplayMode = TranslationDisplayMode::Pixel;
  // This has to be called here so that the virtual functions will be properly
  // set
  Setup();
}

void MaterialSplitScriptEditor::SetTranslatedEditor()
{
  TranslatedRaverieCompositorScriptEditor* editor = new TranslatedRaverieCompositorScriptEditor(this);
  editor->mDisplayMode = mDisplayMode;
  mTranslatedEditor = editor;
}

void MaterialSplitScriptEditor::Build()
{
  mTranslatedEditor->Build();
}

void MaterialSplitScriptEditor::SetResource(Material* material)
{
  mMaterial = material;
  TranslatedRaverieCompositorScriptEditor* editor = (TranslatedRaverieCompositorScriptEditor*)mTranslatedEditor;
  editor->SetResource(material);
}

void MaterialSplitScriptEditor::SetDisplayMode(TranslationDisplayMode::Enum displayMode)
{
  mDisplayMode = displayMode;
  TranslatedRaverieCompositorScriptEditor* editor = (TranslatedRaverieCompositorScriptEditor*)mTranslatedEditor;
  editor->SetDisplayMode(mDisplayMode);
}

// Tries to find a script window of a given name, if it cannot be found then it
// is created.
template <typename ScriptEditorType>
ScriptEditorType* CreateScriptWindow(Editor* editor, StringParam name, uint lexerType)
{
  // get the center window (where we add all tabs to)
  Window* centerWindow = editor->GetCenterWindow();

  // find the tab for this translation if we already had it open
  WindowTabEvent event;
  event.Name = name;
  centerWindow->DispatchBubble(Events::TabFind, &event);

  ScriptEditorType* scriptEditor = nullptr;
  if (event.TabWidgetFound)
  {
    scriptEditor = (ScriptEditorType*)event.TabWidgetFound;
  }
  else
  {
    // if it doesn't exist then create the window, set the lexers,
    // give the window reference to the source script (so it can save it)
    scriptEditor = new ScriptEditorType(centerWindow);
    scriptEditor->SetName(name);
    Z::gEditor->AddManagedWidget(scriptEditor, DockArea::Center, true);
  }

  scriptEditor->SetLexer(lexerType);
  return scriptEditor;
}

struct ShaderFileBuilder
{
  String CreateFileWithContents(StringParam fileContents, StringParam className)
  {
    return fileContents;
  }

  Resource* GetResourceByName(StringParam name)
  {
    return RaverieFragmentManager::GetInstance()->GetResource(name, ResourceNotFound::ReturnNull);
  }
};

template <typename Functor>
void RunCodeTranslator(
    Editor* editor, Space* space, CodeTranslator* translator, Functor customFileBuilder, StringParam rerunCommand)
{
  // get the center window (where we add all tabs to)
  Window* centerWindow = editor->GetCenterWindow();

  // get all of the translated files
  HashMap<String, String> files;
  translator->Translate(files);

  auto fileRange = files.All();
  for (; !fileRange.Empty(); fileRange.PopFront())
  {
    // get the item name
    auto item = fileRange.Front();
    // get the translated code and file name
    String translatedCode = item.second;
    String fileName = item.first;
    // the file name is the full file path, so strip of all of the path
    StringRange found = fileName.FindLastOf('\\');
    String name = fileName.SubString(found.End(), fileName.End());
    // and then strip of the .z
    found = name.FindLastOf('.');
    String className = name.SubString(name.Begin(), found.Begin());

    // Try to find the original file's tab. We don't want all script files
    // to open, only open the ones that we were editing
    //(aka if we find the tab for the file then we open)
    WindowTabEvent* origFileEvent = new WindowTabEvent();
    Resource* resource = customFileBuilder.GetResourceByName(className);
    origFileEvent->SearchObject = resource;
    origFileEvent->Name = className;
    centerWindow->DispatchBubble(Events::TabFind, origFileEvent);
    if (origFileEvent->TabWidgetFound == nullptr)
      continue;

    // make a name for the tab with fileName : Translated
    name = String::Format("%s : Translated", name.c_str());

    // get the script that this represented
    DocumentResource* script = (DocumentResource*)resource;

    // find the tab for this translation if we already had it open
    WindowTabEvent event;
    event.Name = name;
    centerWindow->DispatchBubble(Events::TabFind, &event);

    CodeSplitWindow* splitWindow;
    if (event.TabWidgetFound)
    {
      splitWindow = (CodeSplitWindow*)event.TabWidgetFound;
    }
    else
    {
      // if it doesn't exist then create the window, set the lexers,
      // give the window reference to the source script (so it can save it)
      splitWindow = new CodeSplitWindow(centerWindow);
      splitWindow->mCommandToRunOnSave = rerunCommand;
      splitWindow->SetName(name);
      splitWindow->mSourceResource = script;
      splitWindow->SetLexers(translator);
      Z::gEditor->AddManagedWidget(splitWindow, DockArea::Center, true);
    }
    // make sure the scroll bar stays approximately where it is
    Vec2 scrollPercent = splitWindow->mSourceText->GetScrolledPercentage();
    splitWindow->mSourceText->SetAllText(script->LoadTextData());
    splitWindow->mSourceText->SetScrolledPercentage(scrollPercent);

    String fileText = customFileBuilder.CreateFileWithContents(translatedCode, className);

    scrollPercent = splitWindow->mTranslatedText->GetScrolledPercentage();
    splitWindow->mTranslatedText->SetAllText(fileText);
    splitWindow->mTranslatedText->SetScrolledPercentage(scrollPercent);
  }
}

CodeTranslatorListener::CodeTranslatorListener()
{
  ConnectThisTo(Z::gEditor, "ComposeRaverieMaterial", OnComposeRaverieMaterial);
  ConnectThisTo(Z::gEditor, "TranslateRaverieFragment", OnTranslateRaverieFragment);
  ConnectThisTo(Z::gEditor, "TranslateRaveriePixelMaterial", OnTranslateRaveriePixelFragment);
  ConnectThisTo(Z::gEditor, "TranslateRaverieGeometryMaterial", OnTranslateRaverieGeometryFragment);
  ConnectThisTo(Z::gEditor, "TranslateRaverieVertexMaterial", OnTranslateRaverieVertexFragment);
}

template <typename EditorType>
void CreateFragmentTranslationWindow(ObjectEvent* e, StringParam baseWindowName)
{
  Editor* editor = Z::gEditor;
  RaverieFragment* fragment = (RaverieFragment*)e->Source;
  EditorType* scriptEditor =
      CreateScriptWindow<EditorType>(editor, BuildString(baseWindowName, fragment->Name), Lexer::Shader);
  scriptEditor->SetResource(fragment);
  scriptEditor->Build();
}

template <typename EditorType>
void CreateMaterialTranslationWindow(ObjectEvent* e,
                                     TranslationDisplayMode::Enum displayMode,
                                     StringParam baseWindowName)
{
  Editor* editor = Z::gEditor;
  Material* material = (Material*)e->Source;
  EditorType* scriptEditor =
      CreateScriptWindow<EditorType>(editor, BuildString(baseWindowName, material->Name), Lexer::Shader);
  scriptEditor->SetDisplayMode(displayMode);
  scriptEditor->SetResource(material);
  scriptEditor->Build();
}

void CodeTranslatorListener::OnComposeRaverieMaterial(ObjectEvent* e)
{
  Editor* editor = Z::gEditor;
  Material* material = (Material*)e->Source;
  RaverieCompositorScriptEditor* scriptEditor = CreateScriptWindow<RaverieCompositorScriptEditor>(
      editor, BuildString("CompositedMat", material->Name), Lexer::Raverie);
  scriptEditor->SetResource(material);
  scriptEditor->Build();
}

void CodeTranslatorListener::OnTranslateRaverieFragment(ObjectEvent* e)
{
  CreateFragmentTranslationWindow<FragmentFileTranslatorScriptEditor>(e, "Translated");
}

void CodeTranslatorListener::OnTranslateRaveriePixelFragment(ObjectEvent* e)
{
  CreateMaterialTranslationWindow<TranslatedRaverieCompositorScriptEditor>(
      e, TranslationDisplayMode::Pixel, "TranslatedPixelMat");
}

void CodeTranslatorListener::OnTranslateRaverieGeometryFragment(ObjectEvent* e)
{
  CreateMaterialTranslationWindow<TranslatedRaverieCompositorScriptEditor>(
      e, TranslationDisplayMode::Geometry, "TranslatedGeometryMat");
}

void CodeTranslatorListener::OnTranslateRaverieVertexFragment(ObjectEvent* e)
{
  CreateMaterialTranslationWindow<TranslatedRaverieCompositorScriptEditor>(
      e, TranslationDisplayMode::Vertex, "TranslatedVertexMat");
}

void CodeTranslatorListener::OnTranslateRaverieFragmentWithLineNumbers(ObjectEvent* e)
{
  CreateFragmentTranslationWindow<FragmentSplitScriptEditor>(e, "Translated");
}

void CodeTranslatorListener::OnTranslateRaveriePixelFragmentWithLineNumbers(ObjectEvent* e)
{
  CreateMaterialTranslationWindow<MaterialSplitScriptEditor>(e, TranslationDisplayMode::Pixel, "TranslatedPixelMat");
}

void CodeTranslatorListener::OnTranslateRaverieVertexFragmentWithLineNumbers(ObjectEvent* e)
{
  CreateMaterialTranslationWindow<MaterialSplitScriptEditor>(e, TranslationDisplayMode::Vertex, "TranslatedVertexMat");
}

void CreateShaderTranslationDebugHelper(Editor* editor)
{
  ShaderTranslationDebugHelper* translatorWindow = new ShaderTranslationDebugHelper(editor);
  editor->AddManagedWidget(translatorWindow, DockArea::Center, true);
  translatorWindow->SetSize(Vec2(500, 500));
}

void BindCodeTranslatorCommands(Cog* configCog, CommandManager* commands)
{
  // Ideally change this to add as a component later
  Z::gEditor->mCodeTranslatorListener = new CodeTranslatorListener();
  commands->AddCommand("DebugShaderTranslation", BindCommandFunction(CreateShaderTranslationDebugHelper));
}

} // namespace Raverie
