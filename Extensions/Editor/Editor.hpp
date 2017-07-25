///////////////////////////////////////////////////////////////////////////////
///
/// \file Editor.hpp
/// Declaration of the Editor classes.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------- Forward Declarations
class ConsoleUi;
class LoadingWindow;
class ToolControl;
class LibraryView;
class ContentLibrary;
class RuntimeEditorImpl;
class PropertyView;
class Level;
class StressTestDialog;
class ObjectView;
class CommandManager;
class UpdateEvent;
class CommandCaptureContextEvent;
class DocumentEditor;
class BugReporter;
class MainPropertyView;
class SavingEvent;
class MainWindow;
class Editor;
class MessageBoxEvent;
class ZilchCompiledEvent;
class CogCommandManager;
class EventDirectoryWatcher;
class FileEditEvent;
class CodeTranslatorListener;
class WebBrowserWidget;
class CameraViewport;
class EditorViewport;
class FindTextDialog;

//------------------------------------------------------------------------------
namespace Events
{
  DeclareEvent(EditorChangeSpace);
  DeclareEvent(UnloadProject);
  DeclareEvent(LoadedProject);
}//namespace Events

//----------------------------------------------------------------- Editor Event
class EditorEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EditorEvent(Editor* editor);
  Editor* mEditor;
};

//----------------------------------------------------------------------- Editor
DeclareEnum2(EditorMode, Mode2D, Mode3D);
DeclareEnum2(PlayGameOptions, SingleInstance, MultipleInstances);

/// Main editor Object.
class Editor : public MultiDock
{
public:
  // Meta Initialization
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Editor(Composite* parent);
  ~Editor();

  /// Space that is being edited
  Space* GetEditSpace();
  /// Invoked by EditorViewport when it becomes active
  void SetEditSpace(Space* space);
  /// Sets the active space for the main EditorViewport
  void SetEditorViewportSpace(Space* space);

  /// Focus on the passed in space
  void SetFocus(Space* space);

  /// Current level being edited on the edit space
  Level* GetEditLevel();
  /// Get starting level for game.
  Level* GetStartingLevel();

  Widget* ShowConsole();
  Widget* ToggleConsole();
  Widget* ShowBrowser();
  Widget* ShowMarket();

  /// Selects a tool with the given name.
  void SelectTool(StringParam toolName);

  /// Sets the edit mode for the current levels editor camera controller
  void SetEditMode(EditorMode::Enum mode);
  /// Gets the edit mode for the current levels editor camera controller
  EditorMode::Enum GetEditMode();

  void OnProjectFileModified(FileEditEvent* e);

  OsWindow* mOsWindow;
  MainWindow* mMainWindow;

  Widget* ShowWindow(StringParam name);
  Window* AddManagedWidget(Widget* widget, DockArea::Enum dockArea, bool visible = true);

  void CreateDockableWindow(StringParam windowName, CameraViewport* cameraViewport,
                            Vec2Param initialSize, bool destroySpaceOnClose,
                            DockArea::Enum dockMode = DockArea::Floating);

  //User Configuration object.
  Cog* mConfig;
  // Current loaded Project object.
  HandleOf<Cog> mProject;
  
  EventDirectoryWatcher* mProjectDirectoryWatcher;
  
  /// Get the project of current game.
  Cog* GetProjectCog();
  /// Simple helper to get the path of the current project. Returns an empty string if no project is loaded.
  String GetProjectPath();

  /// Current game session being edited
  HandleOf<GameSession> mEditGame;
  HandleOf<Space> mActiveSpace;
  HandleOf<Level> mEditLevel;

  HandleOf<EditorViewport> mEditorViewport;
  Array<EditorViewport*> mEditInGameViewports;

  /// Current Project Primary content library.
  ContentLibrary* mProjectLibrary;

  /// Editor Tools
  ToolControl* Tools;

  PropertyView* GetPropertyView();
  MainPropertyView* mMainPropertyView;

  //Core Editor Widgets
  LibraryView* mLibrary;
  ConsoleUi* mConsole;
  LoadingWindow* mLoading;
  ObjectView* mObjectView;

  //Commands
  CommandManager* mCommands;
  CogCommandManager* mCogCommands;

  //Bugs
  BugReporter* mBugReporter;

  //Find
  FindTextDialog* mFindTextDialog;

  //Stress Tests
  StressTestDialog* mStressTestDialog;

  //Desync
  Window* mDesyncWindow;

  // For re-parenting on copy and paste
  CogId mLastCopiedParent;
  String mLastCopy;
  // For now, we're only storing the first copied objects
  // location in the hierarchy. The reason for this is I'm not exactly sure
  // what would be best when copying multiple objects.
  uint mPasteHierarchyIndex;

  // This should ideally be a component on the editor
  CodeTranslatorListener* mCodeTranslatorListener;

  //------------------------------------------------------------ Selection
  HandleOf<MetaSelection> mSelection;
  MetaSelection* GetSelection();
  void OnSelectionFinal(SelectionChangedEvent* event);

  //------------------------------------------------------------ Edit

  // Edit a Resource Object
  void EditResource(Resource* resource);

  // Add a new resource
  void AddResource();

  // Add a new resource type of given type
  void AddResourceType(BoundType* resourceType);

  // Open a text file if it is a resource it will open as that resource
  virtual DocumentEditor* OpenTextFileAuto(StringParam file) = 0;
  // Open a string in the text editor for debugging data dumps etc
  virtual DocumentEditor* OpenTextString(StringParam name, StringParam text, StringParam extension = String()) = 0;
  // Open a text file for text editing
  virtual DocumentEditor* OpenTextFile(StringParam filename) = 0;
  // Open a document resource for text editing
  virtual DocumentEditor* OpenDocumentResource(DocumentResource* docResource) = 0;

  //Undo last change.
  void Undo();

  //Redo last undo.
  void Redo();

  //Save all objects.
  Status SaveAll(bool showNotify);

  bool TakeProjectScreenshot();

  /// The main operation queue used by the editor.
  OperationQueue* GetOperationQueue() { return mQueue; }
  HandleOf<OperationQueue> mQueue;

  /// To re-initialize script objects, we need to remove all live script that
  /// run in editor, then re-add them after the scripts have been compiled.
  void OnScriptsCompiledPrePatch(ZilchCompileEvent* e);
  void OnScriptsCompiledPostPatch(ZilchCompileEvent* e);

  /// Removes all live zilch objects on all game sessions. It will add all
  /// removal operations to the mReInitializeQueue so they can all be
  /// re-added at a later point.
  void TearDownZilchStateOnGames(HashSet<ResourceLibrary*>& modifiedLibraries);
  
  /// We tear down live Zilch objects and proxies that should become live
  /// objects. This is usually done on PreScriptCompile. However, the first
  /// time scripts are compiled, if we do it on PreScriptCompile, we wouldn't
  /// remove proxies (as we won't know anything about Zilch types yet).
  /// Because of this, on the first compile, we tear down the zilch state
  /// when the scripts have finished compiling.
  /// Currently, this is only to fix an issue with RunInEditor scripts
  /// on the GameSession. 
  bool mFirstCompile;

  /// The easiest way to re-initialize all script components is to use
  /// operations to remove, then undo to re-add them.
  OperationQueue mReInitializeQueue;

  /// Re-initializing the scripts will cause the space to be modified, so we
  /// need to store the modified state of all spaces to restore it properly.
  /// Otherwise, modifying a script would always mark a space as modified.
  HashMap<Space*, bool> mSpaceModifiedStates;

//Internals
  void OnSaveCheck(SavingEvent* event);
  RuntimeEditorImpl* mRuntimeEditorImpl;
  void SelectOnly(HandleParam object);
  void SelectPrimary(HandleParam object);
  virtual void OnEngineUpdate(UpdateEvent* event) {}
  void Update();
  void ExecuteCommand(StringParam commandName);
  Composite* OpenSearchWindow(Widget* returnFocus, bool noBorder = false);
  Space* CreateNewSpace(uint flags);
  void OnCaptureContext(CommandCaptureContextEvent* event);
  typedef Array<HandleOf<GameSession> > GameArray;
  typedef GameArray::range GameRange;
  void DisplayGameSession(StringParam name, GameSession* gameSession);
  GameSession* PlayGame(PlayGameOptions::Type options);
  GameSession* PlaySingleGame();
  GameSession* PlayNewGame();
  void ZoomOnGame(GameSession* gameSession);
  void EditGameSpaces();
  void DestroyGames();
  void PauseGame();
  void StopGame();
  void StepGame();
  bool AreGamesRunning();
  GameRange GetGames();
  void ClearInvalidGames();
  void SelectGame();
  void SelectSpace();
  GameSession* GetEditGameSession();
  GameSession* CreateDefaultGameSession();
  GameSession* EditorCreateGameSession(uint flags);
  void SetMainPropertyViewObject(Object* object);
  void LoadDefaultLevel();
  void ProjectLoaded();
  bool RequestQuit(bool isRestart);
  void OnSaveQuitMessageBox(MessageBoxEvent* event);
  void OnSaveRestartMessageBox(MessageBoxEvent* event);
  GameArray mGames;
  bool mGamePending;

  Array<CogId> mSelectionGizmos;
  UniquePointer<EventObject> mSimpleDebuggerListener;
};

namespace Z
{
  extern Editor* gEditor;
}//namespace Z


}//namespace Zero
