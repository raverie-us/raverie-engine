///////////////////////////////////////////////////////////////////////////////
///
/// \file Editor.cpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"


namespace Zero
{

namespace Z
{
  Editor* gEditor = nullptr;
}

namespace Events
{
  DefineEvent(EditorChangeSpace);
  DefineEvent(UnloadProject);
  DefineEvent(LoadedProject);
}//namespace Events

ZilchDefineType(EditorEvent, builder, type)
{

  ZilchBindFieldProperty(mEditor);
}


EditorEvent::EditorEvent(Editor* editor)
{
  mEditor = editor;
}

//------------------------------------------------------------- Simple Visualize
class RuntimeEditorImpl : public RuntimeEditor
{
public:

  struct EditorIconToAdd
  {
    CogId Object;
    String Icon;
  };

  RuntimeEditorImpl()
  {
    Z::gRuntimeEditor = this;
  }

  ~RuntimeEditorImpl()
  {

  }

  Array<EditorIconToAdd> Proxies;

  void VisualizePending()
  {
  }

  void OnResourceIdConflict(ResourceEntry& entry, Resource* previous) override
  {
    //Only check for duplicate resource ids in the editor. 
    //Content items are not loaded out of editor.
    if(previous->mContentItem && entry.mLibrarySource)
    {
      String prevIdAndName = previous->ResourceIdName;
      String newIdAndName = BuildString(ToString(entry.mResourceId), ":", entry.Name);

      String prevFileName = entry.mLibrarySource->Filename.c_str();
      String newFileName = previous->mContentItem->Filename.c_str();

      ZPrint("Another resource is already mapped to the "
        "resource id '%s',  it was mapped as '%s'.\n", 
        prevIdAndName.c_str(),
        newIdAndName.c_str());

      if(previous->mContentItem == entry.mLibrarySource)
      {
        ZPrint("To fix, remove file '%s' with conflicting Ids.\n", newFileName.c_str()); 
      }
      else
      {
        ZPrint("To fix conflicting Ids, you can remove either '%s' or '%s'\n", 
          prevFileName.c_str(),
          newFileName.c_str());     
      }

      DoNotifyError("Resource Id Conflict. ", 
        "Id conflict, resources fail to load. See console for details or contact zero support team.");
    }
  }

  Level* GetEditingLevel() override
  {
    return Z::gEditor->GetEditLevel();
  }

  Resource* NewResourceOnWrite(ResourceManager* resourceManager, BoundType* type, StringParam property, Space* space,
                               Resource* resource, Archetype* archetype, bool modified) override
  {
    return Zero::NewResourceOnWrite(resourceManager, type, property, space, resource, archetype, modified);
  }

  bool HasFocus(GameSession* game) override
  {
    GameWidget* gameWidget = game->mGameWidget;
    if(gameWidget)
      return gameWidget->HasFocus();
    else
      return false;
  }

  void OpenEditorViewport(Space* space) override
  {
    // Look for existing EditorViewport for this space first and change tab
    SetFocus(space);

    EditorViewport* editorViewport = new EditorViewport(Z::gEditor, OwnerShip::Sharing);
    Z::gEditor->mEditInGameViewports.PushBack(editorViewport);
    Z::gEditor->GetCenterWindow()->AttachAsTab(editorViewport);
    // Set GameSession handle to null to GameWidget does not quit game
    editorViewport->mGameWidget->mGame = nullptr;
    editorViewport->SetTargetSpace(space);
    // Level is already loaded so call this manually, event is not used so just pass null
    editorViewport->OnSpaceLevelLoaded(nullptr);
    editorViewport->SetName(BuildString("Space: ", space->GetName()));

    CameraViewport* cameraViewport = editorViewport->mEditorCamera.Has<CameraViewport>();
    // Override which GameWidget this CameraViewport will attach its viewport to
    cameraViewport->SetGameWidgetOverride(editorViewport->mGameWidget);
    cameraViewport->SetRenderInGame(true);
    cameraViewport->SetRenderToViewport(true);
  }

  void SetFocus(Space* space) override
  {
    Z::gEditor->SetFocus(space);
  }

  Resource* AddResource(ResourceManager* resourceManager, ResourceAdd& resourceAdd) override
  {
    AddNewResource(resourceManager, resourceAdd);
    return resourceAdd.SourceResource;
  }

  void Visualize(Component* component, StringParam icon) override
  {
    if(component->GetSpace()->IsEditorMode())
    {
      EditorIconToAdd toAdd = { component->GetOwner() , icon };
      Proxies.PushBack(toAdd);
    }
  }

  void ShowTextError(StringParam file, int line, StringParam message) override
  {
    DocumentEditor* editor = Z::gEditor->OpenTextFileAuto(file);
    if(editor != NULL)
    {
      editor->SetLexer(Lexer::Shader);
      editor->SetAnnotation(line, message);
    }
  }

  void ShowTextBlock(StringParam name, StringRange text, int line, StringParam message)
  {
    DocumentEditor* editor = Z::gEditor->OpenTextString(name, text);
    editor->SetLexer(Lexer::Shader);
    editor->SetAnnotation(line, message);
  }

  MetaSelection* GetActiveSelection()
  {
    return Z::gEditor->mSelection;
  }
};

//----------------------------------------------------------------------- Editor
ZilchDefineType(Editor, builder, type)
{
  ZilchBindGetterProperty(Actions);
  ZilchBindGetterProperty(EditSpace);
  ZilchBindGetterProperty(EditGameSession);
  ZilchBindGetterProperty(EditLevel);
  ZilchBindGetterProperty(Selection);
  ZilchBindGetterProperty(OperationQueue);
  ZilchBindGetterProperty(ProjectCog);
  ZilchBindMethod(SetFocus);
  ZilchBindMethod(DisplayGameSession);
  ZilchBindMethod(ExecuteCommand);
  ZilchBindMethod(SelectPrimary);
  ZilchBindMethod(PlaySingleGame);
  ZilchBindMethod(PlayNewGame);
  ZilchBindMethod(PauseGame);
  ZilchBindMethod(StopGame);
  ZilchBindMethod(StepGame);
  ZilchBindMethod(EditGameSpaces);
  ZilchBindMethod(AddResource);
  ZilchBindMethod(AddResourceType);
  ZilchBindMethod(ZoomOnGame);
  ZilchBindMethod(SelectTool);
  ZilchBindMethod(SetMainPropertyViewObject);
  ZilchBindMethod(EditResource);
  ZilchBindMethod(CreateDockableWindow);
}

Editor::Editor(Composite* parent)
  : MultiDock(parent)
{
  mRuntimeEditorImpl = new RuntimeEditorImpl();
  Z::gEditor = this;
  mProject = nullptr;
  mSelection = nullptr;
  mProjectLibrary = nullptr;
  mObjectView = nullptr;
  mConfig = nullptr;
  mLibrary = nullptr;
  mConsole = nullptr;
  mLoading = nullptr;
  mCommands = nullptr;
  mFindTextDialog = nullptr;
  mStressTestDialog = nullptr;
  mDesyncWindow = nullptr;
  mBugReporter = nullptr;
  mCodeTranslatorListener = nullptr;
  mFirstCompile = false;
  mProjectDirectoryWatcher = nullptr;
  mSimpleDebuggerListener = nullptr;

  mQueue = new OperationQueue();

  mCogCommands = new CogCommandManager();

  MetaSelection* selection = GetSelection();
  
  ZilchManager* zilchManager = ZilchManager::GetInstance();
  ConnectThisTo(zilchManager, Events::ScriptsCompiledPrePatch, OnScriptsCompiledPrePatch);
  ConnectThisTo(zilchManager, Events::ScriptsCompiledPostPatch, OnScriptsCompiledPostPatch);

  ConnectThisTo(this, Events::CommandCaptureContext, OnCaptureContext);
  ConnectThisTo(selection, Events::SelectionFinal, OnSelectionFinal);
  ConnectThisTo(this, Events::SaveCheck, OnSaveCheck);
  ConnectThisTo(Z::gEngine, Events::EngineUpdate, OnEngineUpdate);

  BoundType* editorMeta = ZilchTypeId(Editor);
  Z::gSystemObjects->Add(this, editorMeta, ObjectCleanup::None);
}

Editor::~Editor()
{
  Z::gEditor = nullptr;
  SafeDelete(mRuntimeEditorImpl);
  SafeDelete(mCogCommands);
  SafeDelete(mProjectDirectoryWatcher);
  SafeDelete(mCodeTranslatorListener);
  mEditGame.SafeDestroy();
  DestroyGames();
}

Level* Editor::GetStartingLevel()
{
  Level* editLevel = GetEditLevel();
  return editLevel;
}

Level* Editor::GetEditLevel()
{
  return mEditLevel;
}

Space* Editor::GetEditSpace()
{
  return mActiveSpace;
}

void Editor::SetEditSpace(Space* space)
{
  mActiveSpace = space;

  EditorEvent editorEvent(this);
  this->GetDispatcher()->Dispatch(Events::EditorChangeSpace, &editorEvent);

  if(mObjectView)
    mObjectView->SetSpace(space);

  CommandManager::GetInstance()->SetContext(space);
}

void Editor::SetEditorViewportSpace(Space* space)
{
  EditorViewport* editorViewport = mEditorViewport;
  if (editorViewport == nullptr)
  {
    editorViewport = new EditorViewport(this, OwnerShip::Owner);
    GetCenterWindow()->AttachAsTab(editorViewport);
    mEditorViewport = editorViewport;
    GetEditGameSession()->mGameWidget = editorViewport->mGameWidget;
  }

  editorViewport->SetTargetSpace(space);
}

void Editor::SetFocus(Space* space)
{
  forRange(EditorViewport* viewport, mEditInGameViewports.All())
  {
    if ((Space*)viewport->mEditSpace == space)
    {
      Window* window = GetWindowContaining(viewport);
      window->mTabArea->SelectTabWith(viewport);
      return;
    }
  }
}

Cog* Editor::GetProjectCog()
{
  return mProject;
}

String Editor::GetProjectPath()
{
  Cog* projectCog = GetProjectCog();
  if(projectCog == nullptr)
    return String();

  ProjectSettings* projectSettings = projectCog->has(ProjectSettings);
  if(projectSettings == nullptr)
    return String();

  return projectSettings->GetProjectFolder();
}

MetaSelection* Editor::GetSelection()
{
  if(MetaSelection* selection = mSelection)
    return selection;
  MetaSelection* selection = new MetaSelection();
  mSelection = selection;
  return selection;
}

void Editor::LoadDefaultLevel()
{
  Cog* projectCog = Z::gEditor->mProject;
  if(projectCog == nullptr)
    return;

  ProjectSettings* project = projectCog->has(ProjectSettings);

  Level* level = nullptr;

  // Load the last level edited
  Cog* configCog = Z::gEngine->GetConfigCog();
  if(EditorConfig* editorConfig = configCog->has(EditorConfig))
  {
    String lastEditedLevel = configCog->has(EditorConfig)->EditingLevel;
    Level* lastEdited = LevelManager::FindOrNull(lastEditedLevel);

    // Is this level from this project?
    if(lastEdited && lastEdited->mContentItem->mLibrary == mProjectLibrary)
      level = lastEdited;
  }

  GameSession* gameSession = GetEditGameSession();

  // If there is DefaultGameSetup use that starting level
  DefaultGameSetup* gameSetup = gameSession->has(DefaultGameSetup);
  if(level == nullptr && gameSetup)
    level = gameSetup->GetStartingLevel();

  // Load the default level for the project
  if(level == nullptr)
    level = LevelManager::FindOrNull(project->DefaultLevel);

  // Do not load core resource, it is not editable
  if (level == LevelManager::GetDefault())
    level = nullptr;

  // Load any level in project
  if(level == nullptr)
  {
    forRange(Resource* resource,  LevelManager::GetInstance()->ResourceIdMap.Values())
    {
      if(resource->mContentItem && resource->mContentItem->mLibrary == mProjectLibrary)
      {
        level = (Level*)resource;
        break;
      }
    }
  }

  // Create a level if project does not have one
  if (level == nullptr)
  {
    ResourceAdd resourceAdd;
    resourceAdd.Library = Z::gEditor->mProjectLibrary;
    resourceAdd.Name = "Level";
    resourceAdd.Template = LevelManager::FindOrNull("TemplateLevel");
    AddNewResource(LevelManager::GetInstance(), resourceAdd);

    level = LevelManager::FindOrNull("Level");
  }

  Z::gEditor->EditResource(level);
}

GameSession* Editor::CreateDefaultGameSession()
{
  //No game session archetype in project create a new one
  Cog* projectCog = Z::gEditor->mProject;
  if(projectCog == nullptr)
    return nullptr;

  ProjectSettings* project = projectCog->has(ProjectSettings);
  // METAREFACTOR - Improper GameSession creation - wait, why does this comment exist, maybe because we removed MetaCreateContext...
  GameSession* game = new GameSession();
  game->SetInEditor(true);
  DefaultGameSetup* gameSetup = new DefaultGameSetup();
  gameSetup->SetDefaults();
  gameSetup->mStartingSpace = ArchetypeManager::Find(project->ProjectSpace);
  gameSetup->mStartingLevel = LevelManager::Find(project->DefaultLevel);
  game->AddComponent(gameSetup);
  ArchetypeManager::GetInstance()->MakeNewArchetypeWith(game, CoreArchetypes::Game);
  return game;
}

GameSession* Editor::EditorCreateGameSession(uint flags)
{
  Archetype* gameSessionArchetype = ArchetypeManager::FindOrNull(CoreArchetypes::Game);

  GameSession* game = nullptr;

  if(gameSessionArchetype == nullptr)
  {
    game = CreateDefaultGameSession();
  }
  else
  {
    game = (GameSession*)Z::gFactory->CreateCheckedType(ZilchTypeId(GameSession), nullptr, CoreArchetypes::Game, flags, nullptr);

    //For some reason we failed to actually create a game session. Maybe the archetype contained the wrong expected type?
    if(game == nullptr)
      game = CreateDefaultGameSession();
    game->SetInEditor(true);
  }

  return game;
}

void Editor::SetMainPropertyViewObject(Object* object)
{
  mMainPropertyView->EditObject(object, true);
}

void Editor::ProjectLoaded()
{
  Cog* projectCog = Z::gEditor->mProject;
  if(projectCog == nullptr)
    return;

  ProjectSettings* project = projectCog->has(ProjectSettings);

  String message = String::Format("Project '%s' has been loaded.", project->ProjectName.c_str());

  // The first time a project is loaded, game session needs to
  // update its references named "Space" and "Level" 
  // so always mark game session as modified so it is saved
  // at least once.
  //GameSession* gameSession = GetEditGameSession();
  //gameSession->ModifiedFromArchetype();

  mLibrary->View(mProjectLibrary, project->ProjectResourceLibrary);

  // Set the project so project commands will work
  CommandManager::GetInstance()->SetContext(project);

  SafeDelete(mProjectDirectoryWatcher);
  mProjectDirectoryWatcher = new EventDirectoryWatcher(mProjectLibrary->SourcePath);
  ConnectThisTo(mProjectDirectoryWatcher, Events::FileModified, OnProjectFileModified);
  ConnectThisTo(mProjectDirectoryWatcher, Events::FileRenamed, OnProjectFileModified);

  ObjectEvent event(projectCog);
  this->DispatchEvent(Events::ProjectLoaded, &event);
  Z::gEngine->DispatchEvent(Events::ProjectLoaded, &event);

  DoNotify("Project Loaded.", message, "Disk");

  LoadDefaultLevel();

  // Run any command-line arguments. This has to happen here because otherwise we have no
  // guarantee that their project file and their scripts have been loaded otherwise. There 
  // is a chance that if they open a new project these commands could re-run, but this is 
  // something left for a later refactor.
  CommandManager* commandManager = CommandManager::GetInstance();
  commandManager->RunParsedCommands();
}

void Editor::OnSelectionFinal(SelectionChangedEvent* event)
{
  MetaSelection* selection = event->Selection;
  Handle primary = Handle(selection->GetPrimary());

  // Clear all selection gizmos
  forRange(Cog* gizmo, mSelectionGizmos.All())
  {
    if(gizmo)
      gizmo->Destroy();
  }

  mSelectionGizmos.Clear();

  if(Cog* primaryCog = primary.Get<Cog*>())
  {
    //EditorViewport* editorViewport = mActiveViewport;

    //if(editorViewport)
      //TryOpenPreview(primaryCog, editorViewport, selection);

    // Gizmo creation
    if(selection->Count() == 1)
    {
      // The position we're going to create the gizmo at
      Vec3 objectPos = Vec3::cZero;
      if(Transform* transform = primaryCog->has(Transform))
        objectPos = transform->GetWorldTranslation();

      // Walk each component and look for valid gizmo archetypes
      forRange(Component* component, primaryCog->GetComponents())
      {
        if(MetaEditorGizmo* editorGizmo = ZilchVirtualTypeId(component)->HasInherited<MetaEditorGizmo>())
        {
          String archetypeName = editorGizmo->mGizmoArchetype;
          Cog* gizmo = primaryCog->GetSpace()->CreateAt(archetypeName, objectPos);
          if(gizmo)
          {
            gizmo->mFlags.SetFlag(CogFlags::Transient | CogFlags::ObjectViewHidden);
            mSelectionGizmos.PushBack(gizmo->GetId());
            if(Gizmo* gizmoComponent = gizmo->has(Gizmo))
            {
              gizmoComponent->mEditingObject = primaryCog;

              // Dispatch an event to let them know the target has been set
              ObjectEvent eventToSend(primaryCog);
              gizmo->DispatchEvent(Events::GizmoTargetSet, &eventToSend);
            }
          }
        }
      }
    }
  }
}

void Editor::OnSaveCheck(SavingEvent* event)
{
  if(!Z::gResources->mModifiedResources.Empty())
    event->NeedSaving = true;
}

Widget* Editor::ShowConsole()
{
  return ShowWindow("Console");
}

Widget* Editor::ToggleConsole()
{
  return mManager->ToggleWidget("Console");
}

Widget* Editor::ShowBrowser()
{
  Widget* widget = ShowWindow("Browser");
  if (widget != nullptr)
    return widget;

  WebBrowserSetup setup;
  WebBrowserWidget* browser = new WebBrowserWidget(this, setup);
  browser->SetName("Browser");
  browser->SetHideOnClose(true);

  this->AddManagedWidget(browser, DockArea::Center, true);
  return browser;
}

Widget* Editor::ShowMarket()
{
  Widget* widget = ShowWindow("Market");
  if (widget != nullptr)
    return widget;

  MarketWidget* browser = new MarketWidget(this);
  browser->SetName("Market");
  browser->SetHideOnClose(true);

  this->AddManagedWidget(browser, DockArea::Center, true);
  return browser;
}

void Editor::SelectTool(StringParam toolName)
{
  Tools->SelectToolName(toolName);
}

Widget* Editor::ShowWindow(StringParam name)
{
  return mManager->ShowWidget(name);
}

Window* Editor::AddManagedWidget(Widget* widget, DockArea::Enum dockArea, bool visible)
{
  return mManager->AddManagedWidget(widget, dockArea, visible);
}

class SpaceViewport : public Composite
{
public:
  SpaceViewport(Composite* parent, CameraViewport* cameraViewport, bool destroySpaceOnClose) :
    Composite(parent), mDestroySpaceOnClose(destroySpaceOnClose)
  {
    // Fill the game widget
    SetLayout(CreateFillLayout());
    GameWidget* gameWidget = new GameWidget(this);

    cameraViewport->SetGameWidgetOverride(gameWidget);
    mSpace = cameraViewport->GetSpace();
  }

  void OnDestroy() override
  {
    Composite::OnDestroy();

    if(mDestroySpaceOnClose)
      mSpace.SafeDestroy();
  }

  bool mDestroySpaceOnClose;
  HandleOf<Space> mSpace;
};

void Editor::CreateDockableWindow(StringParam windowName, CameraViewport* cameraViewport,
  Vec2Param initialSize, bool destroySpaceOnClose, DockArea::Enum dockMode)
{
  Space* space = cameraViewport->GetSpace();
  SpaceViewport* viewport = new SpaceViewport(this, cameraViewport, destroySpaceOnClose);
  viewport->SetSize(initialSize);

  viewport->SetName(windowName);
  AddManagedWidget(viewport, dockMode);
}

void Editor::SetEditMode(EditorMode::Enum mode)
{
  Space* editSpace = GetEditSpace();

  if(editSpace == nullptr)
    return;
  
  // With the camera controller mode tied to the level any changes need to mark the active space as modified
  editSpace->MarkModified();
  Cog* editorCameraCog = editSpace->FindObjectByName(SpecialCogNames::EditorCamera);
  Camera* editorCamera = editorCameraCog->has(Camera);
  EditorCameraController* cameraController = editorCameraCog->has(EditorCameraController);

  if(mode == EditorMode::Mode2D)
  {
    if (!cameraController)
      editorCamera->SetPerspectiveMode(PerspectiveMode::Orthographic);

    Tools->mCreationTool->mPlacementMode = Placement::PlaneXY;
  }
  else
  {
    if (!cameraController)
      editorCamera->SetPerspectiveMode(PerspectiveMode::Perspective);

    Tools->mCreationTool->mPlacementMode = Placement::OnTop;
  }
  if (cameraController)
    cameraController->SetEditMode(mode);
}

EditorMode::Enum Editor::GetEditMode()
{
  Space* editSpace = GetEditSpace();

  if (editSpace == nullptr)
    return EditorMode::Mode3D;

  Cog* editorCameraCog = editSpace->FindObjectByName(SpecialCogNames::EditorCamera);
  Camera* editorCamera = editorCameraCog->has(Camera);
  EditorCameraController* cameraController = editorCameraCog->has(EditorCameraController);

  return cameraController->GetEditMode();
}

void Editor::OnProjectFileModified(FileEditEvent* e)
{
  EditorSettings* settings = Z::gEngine->GetConfigCog()->has(EditorSettings);
  if(!settings->mAutoUpdateContentChanges)
    return;

  if(mProjectLibrary == nullptr)
    return;

  // Walk through each content item in the loaded library and find any
  // files we need to reload
  forRange(ContentItem* contentItem, mProjectLibrary->GetContentItems())
  {
    if(contentItem->Filename == e->FileName)
    {
      String filePath = contentItem->GetFullPath();

      // Don't reload it if we were the one that modified it
      if(!FileModifiedState::HasModifiedSinceTime(filePath, e->TimeStamp))
        ReloadContentItem(contentItem);
    }
  }
}

PropertyView* Editor::GetPropertyView()
{
  return mMainPropertyView->GetPropertyView();
}

Composite* Editor::OpenSearchWindow(Widget* returnFocus, bool noBorder)
{
  //If focus is NULL return focus to who current has it
  if(returnFocus == nullptr)
    returnFocus = this->GetRootWidget()->GetFocusObject();

  Window* newWindow = new Window(this->GetParent());

  if(noBorder)
    newWindow->ChangeStyle(WindowStyle::NoFrame);

  GeneralSearchView* cs = new GeneralSearchView(newWindow, returnFocus);
  newWindow->SetTitle("Zero Search");
  newWindow->SetTranslationAndSize(Pixels(0, 0, 0), Pixels(320, 440));
  CenterToWindow(this->GetParent(), newWindow, false);

  //Update the transform so everything is the right size
  newWindow->UpdateTransformExternal();

  //Now populate the list
  cs->StartSearch();

  return newWindow;
}

void Editor::SelectOnly(HandleParam object)
{
  mSelection->SelectOnly(object);
  mSelection->FinalSelectionChanged();
}

void Editor::SelectPrimary(HandleParam object)
{
  mSelection->SetPrimary(object);
  mSelection->FinalSelectionChanged();
}

Space* Editor::CreateNewSpace(uint flags)
{
  Space* space = nullptr;
  Cog* projectCog = Z::gEditor->mProject;
  
  GameSession* gameSession = GetEditGameSession();

  if(projectCog)
  {
    ProjectSettings* project = projectCog->has(ProjectSettings);
    if(!project->ProjectSpace.Empty())
    {
      Archetype* spaceArchetype = ArchetypeManager::FindOrNull(project->ProjectSpace);
      if(spaceArchetype)
        space = gameSession->CreateSpaceFlags(spaceArchetype, flags);
    }
  }

  if(space == nullptr)
  {
    DoNotifyError("Project Error", "Failed to create space from project");
    Archetype* spaceArchetype = ArchetypeManager::FindOrNull(CoreArchetypes::DefaultSpace);
    if(gameSession)
      return gameSession->CreateSpaceFlags(spaceArchetype, flags);
    else
      return Z::gFactory->CreateSpace(CoreArchetypes::DefaultSpace, flags, nullptr);
  }
  else
    return space;
}

void Editor::Undo()
{
  mQueue->Undo();
}

void Editor::Redo()
{
  mQueue->Redo();
}

void EditorSaveResource(Resource* resource)
{
  // Resource was deleted
  if(resource == nullptr)
    return;


  // Check for resource not Writable
  if(!resource->IsWritable())
  {
    DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig);
    if(devConfig == nullptr || !devConfig->mCanModifyReadOnlyResources)
    {
      DoNotifyWarning("Resource is not writable", "Resource is a protected or built in resource.");
      return;
    }
  }
  
  // Check for runtime resources
  if(resource->IsRuntime())
  {
    DoNotifyError("Can not save", "Runtime resources can not be saved.");
    return;
  }

  // Request normal data save
  resource->mContentItem->SaveContent();
}

Status Editor::SaveAll(bool showNotify)
{
  // Reset the focus to save any changes in progress (setting text, etc)
  GetRootWidget()->FocusReset();

  // Signal that project is being saved.
  // Scripts and other resources will save.
  SavingEvent e;
  DispatchEvent(Events::Save, &e);

  SaveConfig(Z::gEditor->mConfig);

  // Save all resources that have been edited.
  forRange(ResourceId id, Z::gResources->mModifiedResources.All())
  {
    Resource* resource = Z::gResources->ResourceIdMap.FindValue(id, nullptr);
    EditorSaveResource(resource);
  }
  Z::gResources->mModifiedResources.Clear();

  GameSession* editedGame = mEditGame;
  // Auto upload the Game Archetype
  if(editedGame)
  {
    if(editedGame->IsModifiedFromArchetype())
      editedGame->UploadToArchetype();
  }

  // Save the project itself
  Cog* projectCog = Z::gEditor->mProject;
  if(projectCog)
    projectCog->has(ProjectSettings)->Save();

  // Scripts need to be fully compiling before we run
  if(ZilchManager::GetInstance()->Compile() == CompileResult::CompilationFailed)
    return Status(StatusState::Failure, "Failed to compile Zilch Scripts");

  if(showNotify)
    DoNotify("Saved", "Project and all scripts saved.", "Disk");

  Tweakables::Save();

  return Status();
}

bool Editor::TakeProjectScreenshot()
{
  // Take a screen shot if it's enabled
  if(Cog* projectCog = mProject)
  {
    ProjectSettings* project = projectCog->has(ProjectSettings);

    // Until we have Ui for taking a screen shot, we have to handle multiple
    // viewports and / or games sessions running. By default, we're going to
    // give games sessions a priority, then editor viewports

    // Search for active games sessions
    forRange(GameSession* gameSession, mGames.All())
    {
      if(gameSession == nullptr)
        continue;

      if(GameWidget* gameWidget = gameSession->mGameWidget)
      {
        if(gameWidget->mActive)
        {
          // Take the screen shot and save it to the project folder
          CreateDirectoryAndParents(project->EditorContentFolder);
          gameWidget->SaveScreenshot(project->GetScreenshotFile());
          return true;
        }
      }
    }

    if(EditorViewport* viewport = mEditorViewport)
    {
      if(viewport->mActive)
      {
        // Take the screen shot and save it to the project folder
        CreateDirectoryAndParents(project->EditorContentFolder);
        viewport->mGameWidget->SaveScreenshot(project->GetScreenshotFile());
        return true;
      }
    }
  }

  return false;
}

void ReInitializeScriptsOnObject(Cog* cog, OperationQueue& queue,
                                 HashSet<ResourceLibrary*>& modifiedLibraries)
{
  BoundType* zilchComponentType = ZilchTypeId(ZilchComponent);

  // We want to walk the components in reverse so we don't run into issues
  // with dependencies
  Cog::ComponentRange r = cog->GetComponents();
  while(!r.Empty())
  {
    Component* component = r.Back();
    BoundType* componentType = ZilchVirtualTypeId(component);

    // Remove proxies in case they were replaced by the actual script type
    bool isProxy = componentType->HasAttribute(ObjectAttributes::cProxy);
    bool isZilchComponent = componentType->IsA(zilchComponentType);

    // We want to force remove the component as we need all zilch objects
    // to be freed. We need to do this because they could have components
    // in an invalid order caused by adding dependencies after the fact
    bool ignoreDependencies = true;

    if(isProxy || isZilchComponent)
      QueueRemoveComponent(&queue, cog, componentType, ignoreDependencies);

    r.PopBack();
  }
}

void ReInitializeScriptsOnGame(GameSession* game, OperationQueue& queue,
                               HashMap<Space*, bool>& spaceModifiedStates,
                               HashSet<ResourceLibrary*>& modifiedLibraries)
{
  // Game can be null if they failed to open a project (for instance, a project created in a new version)
  if(game == nullptr)
    return;

  ReInitializeScriptsOnObject(game, queue, modifiedLibraries);

  // Reinitialize 
  forRange(Space* space, game->GetAllSpaces())
  {
    // Store the modified state of this space
    bool spaceModified = space->GetModified();
    spaceModifiedStates.Insert(space, spaceModified);

    // The space itself can have script components
    ReInitializeScriptsOnObject(space, queue, modifiedLibraries);

    // All cogs in the space
    forRange(Cog& cog, space->AllObjects())
      ReInitializeScriptsOnObject(&cog, queue, modifiedLibraries);
  }
}

void RevertSpaceModifiedState(GameSession* game,
                              HashMap<Space*, bool>& spaceModifiedStates)
{
  // Game can be null if they failed to open a project (for instance, a project created in a new version)
  if(game == nullptr)
    return;

  // Update the modified state of all spaces
  forRange(Space* space, game->GetAllSpaces())
  {
    bool modified = spaceModifiedStates.FindValue(space, true);
    if(!modified)
      space->MarkNotModified();
  }
}

void Editor::OnScriptsCompiledPrePatch(ZilchCompileEvent* e)
{
  // If this is a first compile, defer to when script compilation is finished
  if(ZilchManager::GetInstance()->mCurrentScriptProjectLibrary == nullptr)
  {
    mFirstCompile = true;
    return;
  }

  TearDownZilchStateOnGames(e->mModifiedLibraries);
}

void Editor::OnScriptsCompiledPostPatch(ZilchCompileEvent* e)
{
  if(mFirstCompile)
  {
    TearDownZilchStateOnGames(e->mModifiedLibraries);
    mFirstCompile = false;
  }

  //ZilchScriptManager* zilchManager = ZilchScriptManager::GetInstance();
  //
  //// Patch anything that was not reinitialized if the compilation was successful
  //if(e->mLibrary != nullptr)
  //  zilchManager->PatchLibraryIfNeeded(e->mLibrary);

  // Re-add all the components
  mReInitializeQueue.Undo();

  // Revert all space modified states on all game sessions
  RevertSpaceModifiedState(GetEditGameSession(), mSpaceModifiedStates);
  forRange(GameSession* game, GetGames())
    RevertSpaceModifiedState(game, mSpaceModifiedStates);

  // Cleanup
  mSpaceModifiedStates.Clear();
}

void Editor::TearDownZilchStateOnGames(HashSet<ResourceLibrary*>& modifiedLibraries)
{
  // Shouldn't be anything in here, but just in case..
  mReInitializeQueue.ClearAll();
  mReInitializeQueue.BeginBatch();
  mReInitializeQueue.SetActiveBatchName("TearDownZilchStateOnGames");

  EditorSettings* settings = Z::gEngine->GetConfigCog()->has(EditorSettings);

  // Editor game session
  if(settings->mScriptReloadEditor == ScriptReloadMethod::ReInitializeObjects)
  {
    // Re-Initialize all components in the editors game session
    GameSession* game = GetEditGameSession();
    ReInitializeScriptsOnGame(game, mReInitializeQueue, mSpaceModifiedStates, modifiedLibraries);
  }

  // Running game sessions
  if(settings->mScriptReloadGame == ScriptReloadMethod::ReInitializeObjects)
  {
    // Re-Initialize all components in all game sessions
    forRange(GameSession* game, GetGames())
      ReInitializeScriptsOnGame(game, mReInitializeQueue, mSpaceModifiedStates, modifiedLibraries);
  }

  mReInitializeQueue.EndBatch();
}

void Editor::Update()
{
  //Debug::DefaultConfig config;

  //if(Space* space = GetEditSpace())
  //  config.SpaceId(GetEditSpace()->GetId().Id);

  uint spaceId = 0;
  if(Space* space = GetEditSpace())
    spaceId = space->GetId().Id;

  Debug::ActiveDrawSpace drawSpace(spaceId);

  //MetaSelection* selection = GetActiveSelection();
  //Cog* primary = selection->GetPrimaryAs<Cog>();

  //if(primary && primary->GetMeta() == MetaTypeOf(Cog))
  //{
  //  // If the object is a cog setup of debug drawing
  //  // to draw in that space
  //  Space* space = primary->GetSpace();
  //  config.SpaceId(space->GetId().Id);
  //}

  // Debug Draw the active tool (rare cases it could've been null (old tile editor),
  // just safe guard in case it ever gets set to null again)
  if (Cog* toolCog = Tools->GetActiveCog())
  {
    Event e;
    toolCog->DispatchEvent(Events::ToolDraw, &e);
  }

  // Always draw the selection tool unless
  // it was active and already drawn
  if(Cog* toolCog = Tools->GetActiveCog())
  {
    Cog* selectToolCog = Tools->mSelectTool->GetOwner();
    if(toolCog != selectToolCog)
    {
      Event e;
      selectToolCog->DispatchEvent(Events::ToolDraw, &e);
    }
  }

  mMainPropertyView->Refresh();

  mRuntimeEditorImpl->VisualizePending();

  // If there's an un-ended operation batch, someone in script forgot to end the batch
  OperationQueue* opQueue = GetOperationQueue();
  if(opQueue->ActiveBatch != nullptr)
  {
    DoNotifyErrorNoAssert("Operation Batch Not Closed", "'BeginBatch' was called on an "
                  "operation queue, but 'EndBatch' was not called.");

    // Close all batches
    while(opQueue->ActiveBatch != nullptr)
      opQueue->EndBatch();
  }
}

void Editor::ExecuteCommand(StringParam commandName)
{
  Command* command = mCommands->NamedCommands.FindValue(commandName, nullptr);
  if(command)
  {
    command->Execute();
  }
  else
  {
    String message = String::Format("%s is not a registered command.", commandName.c_str());
    DoNotify("Command not found", message, "Warning");
  }
}

void Editor::OnCaptureContext(CommandCaptureContextEvent* event)
{
  event->ActiveSet->SetContext(this);
}


//------------------------------------------------------------ Game Running Functions

// Game Commands

GameSession* Editor::GetEditGameSession()
{
  if (mEditGame == nullptr)
    mEditGame = EditorCreateGameSession(CreationFlags::Editing | CreationFlags::ProxyComponentsExpected);

  return mEditGame;
}

void Editor::SelectGame()
{
  GameSession* game = GetEditGameSession();
  mSelection->Clear();
  SelectPrimary(game);
}

void Editor::SelectSpace()
{
  mSelection->Clear();
  SelectPrimary(GetEditSpace());
}

bool Editor::AreGamesRunning()
{
  return !GetGames().Empty();
}

void Editor::ClearInvalidGames()
{
  for(size_t i = 0; i < mGames.Size();)
  {
    GameSession* game = mGames[i];
    if(!game)
      mGames.EraseAt(i);
    else
      ++i;
  }
}

Editor::GameRange Editor::GetGames()
{
  ClearInvalidGames();
  return mGames.All();
}

void Editor::DisplayGameSession(StringParam name, GameSession* gameSession)
{
  if (gameSession->mStarted)
    DoNotifyErrorNoAssert("Game Session Playing", "You must start the game session after calling DisplayGameSession");
  //mGamePending = false;

  // Create a GameWidget that will contain the GameSession
  GameWidget* gameWidget = new GameWidget(this);
  gameWidget->TakeFocus();
  gameWidget->SetName(name);
  
  // Attach the gameWidget as a tab to the main window
  Window* main = this->GetCenterWindow();
  main->AttachAsTab(gameWidget);
  main->UpdateTransformExternal();

  // Update game session info as it is now running in the editor window
  gameSession->mMainWindow = main->GetRootWidget()->GetOsWindow();
  gameSession->SetInEditor(true);
  gameSession->mGameWidget = gameWidget;

  gameWidget->SetGameSession(gameSession);

  // Store the game on the editor
  mGames.PushBack(gameSession);
}

GameSession* Editor::PlayGame(PlayGameOptions::Type options)
{
  // Is there a level to start on?
  Level* playLevel = GetStartingLevel();
  if(playLevel == nullptr)
  {
    DoNotifyError("No level loaded.", "Can not play a game without a loaded level.");
    return nullptr;
  }

  // If we had old games running and we're only launching a single instance...
  GameRange games = GetGames();
  if(!games.Empty() && options == PlayGameOptions::SingleInstance)
  {
    // Check to see if there is a valid old game (if so, clear them out)
    forRange(GameSession* oldGame, games)
    {
      // Shut it down
      oldGame->Quit();
    }

    // Delay Creating the game until next frame
    mGamePending = true;
    return nullptr;
  }

  mGamePending = false;

  // Attempt to save the game, and if it fails do not play
  // Note: This should definitely come down here, before we close out of the first game
  // The reason is that if anything is 'in use' while the game is running (eg the Zilch library)
  // then it cannot be updated by saving
  Status status = SaveAll(false);
  if(status.Failed())
  {
    DoNotifyErrorNoAssert("Play Game", String::Format("Cannot run the game because: %s", status.Message.c_str()));
    return nullptr;
  }

  // Create a GameWidget that will contain the GameSession
  GameWidget* gameWidget = new GameWidget(this);
  gameWidget->TakeFocus();
  gameWidget->SetName("Game");

  // Attach the gameWidget as a tab to the main window
  Window* main = this->GetCenterWindow();
  main->AttachAsTab(gameWidget);
  main->UpdateTransformExternal();

  // Create the game session
  GameSession* game = EditorCreateGameSession(CreationFlags::Default);
  game->mMainWindow = main->GetRootWidget()->GetOsWindow();

  game->SetInEditor(true);
  game->mGameWidget = gameWidget;
  gameWidget->SetGameSession(game);

  // Store the game on the editor
  mGames.PushBack(game);

  // Start the game
  game->Start();

  return game;
}

GameSession* Editor::PlaySingleGame()
{
  return PlayGame(PlayGameOptions::SingleInstance);
}

GameSession* Editor::PlayNewGame()
{
  return PlayGame(PlayGameOptions::MultipleInstances);
}

void Editor::ZoomOnGame(GameSession* gameSession)
{
  if(gameSession == nullptr)
    return;

  GameWidget* widget = gameSession->mGameWidget;
  if(widget == nullptr)
    return;

  Window* window = GetWindowContaining(widget);
  if(window == nullptr || window->mDocker == nullptr)
    return;

  window->mDocker->Zoom(widget);
}

void Editor::EditGameSpaces()
{
  GameRange games = GetGames();
  forRange(GameSession* game, games)
  {
    game->EditSpaces();
  }
}

void Editor::StepGame()
{
  // Get all the current games
  GameRange games = GetGames();

  // If there's no active game to step, start a new game and pause it
  if(games.Empty())
  {
    GameSession* game = PlayGame(PlayGameOptions::SingleInstance);
    if(game) game->Pause();
  }
  else
  {
    // Step all active games
    forRange(GameSession* game, games)
    {
      game->Step();
    }
  }
}

void Editor::DestroyGames()
{
  // Clean up an active game sessions
  if(GameSession* session = mEditGame)
    session->Destroy();
}

void Editor::StopGame()
{
  forRange(GameSession* game, GetGames())
    game->Quit();
}

void Editor::PauseGame()
{
  // We really don't want to just randomly toggle each game
  // take the first game, get it's pause state, then toggle that and apply
  // that to all games that are currently running
  GameRange games = GetGames();
  if(!games.Empty())
  {
    GameSession* firstGame = games.Front();
    bool newPausedState = !firstGame->mPaused;

    forRange(GameSession* game, games)
      game->mPaused = newPausedState;
  }

  // If the game pauses due to a script error, we need to undo trap
  // otherwise the mouse gets caught and you can't fix anything
  Mouse* mouse = Mouse::GetInstance();
  mouse->SetTrapped(false);
}

// Resource Editing

void Editor::AddResource()
{
  AddResourceType(nullptr);
}

void Editor::AddResourceType(BoundType* resourceType)
{
  if(!mProject)
  {
    DoNotifyError("No project to add resources", "Need a project to add resources");\
    return;
  }

  OpenAddWindow(resourceType);
}

void Editor::EditResource(Resource* resource)
{
  if(resource == nullptr)
    return;

  // Is this resource already open?
  // If so use that window
  Widget* widget = mManager->ShowWidgetWith(resource);
  if(widget)
    return;

  ResourceEditors::GetInstance()->FindResourceEditor(resource);
}

bool Editor::RequestQuit(bool isRestart)
{
  // Does anything need saving?
  SavingEvent saveEvent;
  this->GetDispatcher()->Dispatch(Events::SaveCheck, &saveEvent);

  if(saveEvent.NeedSaving)
  {
    // If anything needs saving prompt the user
    MessageBox* box;
    if (isRestart)
    {
      const cstr MBSaveRestartCancel[] = { "Save and Restart", "Restart without Saving", "Cancel", '\0' };
      box = MessageBox::Show("Restart Confirmation", "Save all changes to levels, scripts, and resources?", MBSaveRestartCancel);
      ConnectThisTo(box, Events::MessageBoxResult, OnSaveRestartMessageBox);
    }
    else
    {
      const cstr MBSaveQuitCancel[] = { "Save and Quit", "Quit without Saving", "Cancel", '\0' };
      box = MessageBox::Show("Quit Confirmation", "Save all changes to levels, scripts, and resources?", MBSaveQuitCancel);
      ConnectThisTo(box, Events::MessageBoxResult, OnSaveQuitMessageBox);
    }
    // Block the quit
    return false;
  }
  else
  {
    // Just exit immediately
    Z::gEngine->Terminate();
    return true;
  }
}

void Editor::OnSaveQuitMessageBox(MessageBoxEvent* event)
{
  if(event->ButtonIndex == 0)
  {
    SaveAll(false);
    Z::gEngine->Terminate();
  }

  if(event->ButtonIndex == 1)
  {
    Z::gEngine->Terminate();
  }

  // 2 is cancel
}

void Editor::OnSaveRestartMessageBox(MessageBoxEvent* event)
{
  if (event->ButtonIndex == 0)
  {
    SaveAll(false);
    Z::gEngine->Terminate();
  }

  if (event->ButtonIndex == 1)
  {
    Z::gEngine->Terminate();
  }

  // 2 is cancel
  
  // This was a restart request
  Os::SystemOpenFile(GetApplication().c_str());
}

}//namespace Zero
