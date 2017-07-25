///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------ Editor Main
class EditorMain : public Editor
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  float mTimeSinceEscape;
  bool mDisableInput;
  typedef EditorMain ZilchSelf;

  EditorMain(Composite* parent, OsWindow* window);
  ~EditorMain();
  void OnEngineUpdate(UpdateEvent* event);
  void OnClosing(HandleableEvent* event);
  void OnMouseDown(MouseEvent* mouseEvent);
  void OnKeyDown(KeyboardEvent* keyEvent);
  void ShowTools(CommandEvent* event);
  void ShowLibrary(CommandEvent* event);
  void ToggleConsole(CommandEvent* event);
  void ShowBrowser(CommandEvent* event);
  void ShowMarket(CommandEvent* event);
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
  void OnNameActivated(TypeEvent* event);
  void ShowLibrary(StringParam libraryName);
  void ShowCoreLibrary(CommandEvent* event);
  void ShowVolumeMeter(CommandEvent* event);
  void ShowSoundNodeGraph(CommandEvent* event);
  LibraryView* CreateLibraryView(bool showCore, bool autoDock = true);
  void AttachDocumentEditor(StringParam name, DocumentEditor* docEditor);
  DocumentEditor* OpenTextString(StringParam name, StringParam text, StringParam extension = String()) override;
  DocumentEditor* OpenTextFile(StringParam filename) override;
  DocumentEditor* OpenDocumentResource(DocumentResource* docResource) override;
  DocumentEditor* OpenTextFileAuto(StringParam file) override;
  void OnScriptError(DebugEngineEvent* event);
  
  void OnBlockingTaskStart(BlockingTaskEvent* event);
  void OnBlockingTaskFinish(Event* event);

  void OnNotifyEvent(NotifyEvent* event);
  void StressTest(CommandEvent* event);
  void OnMainClick(MouseEvent* event);

};

}//namespace Zero
