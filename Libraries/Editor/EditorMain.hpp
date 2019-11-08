// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class EditorMain : public Editor
{
public:
  ZilchDeclareType(EditorMain, TypeCopyMode::ReferenceType);
  float mTimeSinceEscape;
  bool mDisableInput;

  EditorMain(Composite* parent, OsWindow* window);
  ~EditorMain();

  void OnPackagedBuilt(ContentSystemEvent* event);
  bool LoadPackage(Cog* projectCog, ContentLibrary* library, ResourcePackage* package);

  void OnEngineUpdate(UpdateEvent* event);
  void OnClosing(HandleableEvent* event);
  void OnMouseDown(MouseEvent* mouseEvent);
  void OnCutCopyPaste(ClipboardEvent* event);
  void OnKeyDown(KeyboardEvent* keyEvent);
  void ShowTools(CommandEvent* event);
  void ShowLibrary(CommandEvent* event);
  void ToggleConsole(CommandEvent* event);
  void ShowConsole(CommandEvent* event);
  void HideConsole(CommandEvent* event);
  void ShowMarket(CommandEvent* event);
  void ShowChat(CommandEvent* event);
  void ShowObjects(CommandEvent* event);
  void ShowAnimator(CommandEvent* event);
  void ShowHotKeyEditor(CommandEvent* event);
  void ShowOperationHistroy(CommandEvent* event);
  void ShowBroadPhaseTracker(CommandEvent* event);
  void ShowProperties(CommandEvent* event);
  void ShowConfig(CommandEvent* event);
  void ShowProject(CommandEvent* event);
  void SelectTweakables(CommandEvent* event);
  void ShowFindNext(CommandEvent* event);
  void ShowFindAll(CommandEvent* event);
  void ShowReplaceNext(CommandEvent* event);
  void ShowReplaceAll(CommandEvent* event);
  void ShowBugReporter(CommandEvent* event);
  void EditColorScheme(CommandEvent* event);
  void ClearConsole(CommandEvent* event);
  void OnNameActivated(TypeEvent* event);
  void ShowLibrary(StringParam libraryName);
  void ShowCoreLibrary(CommandEvent* event);
  void ShowVolumeMeter(CommandEvent* event);
  void ShowSoundNodeGraph(CommandEvent* event);
  void ShowRenderGroupHierarchies(CommandEvent* event);
  LibraryView* CreateLibraryView(bool showCore, bool autoDock = true);
  void AttachDocumentEditor(StringParam name, DocumentEditor* docEditor);
  DocumentEditor* OpenTextString(StringParam name, StringParam text, StringParam extension = String()) override;
  DocumentEditor* OpenTextFile(StringParam filename) override;
  DocumentEditor* OpenDocumentResource(DocumentResource* docResource) override;
  DocumentEditor* OpenTextFileAuto(StringParam file) override;
  void OnScriptError(ScriptEvent* event);
  void OnDebuggerPaused(ScriptEvent* event);
  void OnDebuggerResumed(ScriptEvent* event);

  void OnBlockingTaskStart(BlockingTaskEvent* event);
  void OnBlockingTaskFinish(Event* event);

  void OnNotifyEvent(NotifyEvent* event);
  void StressTest(CommandEvent* event);
  void OnMainClick(MouseEvent* event);

  Array<ResourcePackage*> PackagesToLoad;
};

} // namespace Zero
