///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorMain.cpp
/// Implementation of Editor Ui. (Rough drafting area for the editor) This
/// file is messy and will be replaced with scripts.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(EditorMain, builder, type)
{
}

//------------------------------------------------------------------ Editor Main
EditorMain::EditorMain(Composite* parent, OsWindow* window)
  : Editor(parent)
{
  this->SetName("EditorMain");
  mTimeSinceEscape = 100.0f;
  mDisableInput = false;
  mGamePending = false;
  ConnectThisTo(parent, Events::Closing, OnClosing);
  ConnectThisTo(this, Events::MouseDown, OnMouseDown);
}

EditorMain::~EditorMain()
{
  SafeDelete(mManager);
}

void EditorMain::OnEngineUpdate(UpdateEvent* event)
{
  mTimeSinceEscape += event->RealDt;
  Update();

  // We should only ever set the pending flag if we previously tried to create a single instance
  if(mGamePending)
    PlayGame(PlayGameOptions::SingleInstance);
}

void EditorMain::OnClosing(HandleableEvent* event)
{
  // Prevent the window from closing
  // the main window automatically
  event->Handled = true;

  // Run editor quit logic
  RequestQuit(false);
}

void EditorMain::OnMouseDown(MouseEvent* mouseEvent)
{
  if(mouseEvent->Handled)
    return;

  if(mouseEvent->Button == MouseButtons::XOneBack)
  {
    mMainPropertyView->GetHistory()->Previous();
    if(mouseEvent->CtrlPressed)
      FocusOnSelectedObjects();
  }

  if(mouseEvent->Button == MouseButtons::XTwoForward)
  {
    mMainPropertyView->GetHistory()->Next();
    if(mouseEvent->CtrlPressed)
      FocusOnSelectedObjects();
  }
}

void EditorMain::OnKeyDown(KeyboardEvent* keyEvent)
{
  if(keyEvent->Handled || keyEvent->HandledEventScript || mDisableInput)
    return;

  // Tools
  uint keyPressed = keyEvent->Key;
  if(keyPressed >= '0' && keyPressed <= '9')
  {
    if(keyPressed == '0') keyPressed += 10;
    uint toolIndex = keyPressed - '0' - 1;
    Z::gEditor->Tools->SelectToolIndex(toolIndex, ShowToolProperties::Auto);
  }

  CommandManager* commands = CommandManager::GetInstance();
  if(commands->TestCommandKeyboardShortcuts(keyEvent))
    return;

  if(keyEvent->CtrlPressed && keyEvent->AltPressed && mConfig->has(Zero::DeveloperConfig) && keyEvent->Key == 'F')
    this->ShowWindow("Find/Replace Objects");

  // Global special hot keys
  if (keyEvent->ShiftPressed && keyEvent->CtrlPressed)
  {
    switch (keyEvent->Key)
    {
      case Keys::Tilde:
      {
        Editor::ToggleConsole();
        break;
      }

      case Keys::Space:
      {
        // Moved this to using Ctrl+Shift because shift represents anything to do with selection,
        // Ctrl + Space should bring up auto-complete, and Shift + Space is really easy
        // to accidentally type.
        OpenSearchWindow(nullptr);
        break;
      }

      case Keys::G:
      {
        SelectGame();
        break;
      }

      case Keys::S:
      {
        SelectSpace();
        break;
      }
    }
  }

  // Special keys
  Mouse* mouse = Mouse::GetInstance();

  switch (keyEvent->Key)
  {
    case Keys::Escape:
    {
      mouse->SetTrapped(false);

      if(DeveloperConfig* devConfig = mConfig->has(DeveloperConfig))
      {
        if(devConfig->mDoubleEscapeQuit && mTimeSinceEscape < 0.4f)
          Z::gEngine->Terminate();
      }
      mTimeSinceEscape = 0.0f;
      break;
    }

    case Keys::F10:
    {
      mouse->ToggleTrapped();
      break;
    }
  }
}

void EditorMain::ShowTools(CommandEvent* event)
{
  this->ShowWindow("Tools");
}

void EditorMain::ShowLibrary(CommandEvent* event)
{
  // If the library window is hidden, show it
  if (mManager->InactiveWidgets.ContainsKey("Library"))
  {
    this->ShowWindow("Library");
    return;
  }

  this->CreateLibraryView(Z::gEditor->mConfig->has(DeveloperConfig));
}

void EditorMain::ShowCoreLibrary(CommandEvent* event)
{
  this->CreateLibraryView(true);
}

LibraryView* EditorMain::CreateLibraryView(bool showCore, bool autoDock)
{
  Vec2 rootSize = this->GetRootWidget()->GetSize();
  float dockWidth = Math::Min(rootSize.x * 0.15f, 280.0f);

  // Create a new one library view
  LibraryView* library = new LibraryView(this);
  library->SetName("Library");
  library->SetHideOnClose(false);
  library->SetSize(Pixels(dockWidth, 280));

  // Hide core libraries
  if (!showCore)
  {
    library->AddHiddenLibrary("Loading");
    library->AddHiddenLibrary("ZeroCore");
    library->AddHiddenLibrary("Editor");
    library->AddHiddenLibrary("EditorUi");
    library->AddHiddenLibrary("EditorScripts");
    library->AddHiddenLibrary("FragmentCore");
  }
  else
  {
    library->SetName("Library");
  }

  // Initially select the loaded project's library
  Editor* editor = Z::gEditor;
  Cog* projectCog = editor->mProject;
    
  // If no project is loaded, select the 0th element in the dropdown
  if (projectCog == nullptr)
  {
    library->View();
  }
  else
  {
    ProjectSettings* project = projectCog->has(ProjectSettings);
    library->View(editor->mProjectLibrary, project->ProjectResourceLibrary);
  }

  if (autoDock)
  {
    // Dock it to the layout
    LayoutInfo info;
    info.Area = DockArea::Right;
    info.Size = library->GetSize();
    info.Name = library->GetName();
    info.Visible = true;
    info.ActiveWidget = library;

    this->AddWidget(library, info);
  }

  return library;
}

void EditorMain::ToggleConsole(CommandEvent* event)
{
  Editor::ToggleConsole();
}

void EditorMain::ShowBrowser(CommandEvent* event)
{
  Editor::ShowBrowser();
}

void EditorMain::ShowMarket(CommandEvent* event)
{
  Editor::ShowMarket();
}

void EditorMain::ShowObjects(CommandEvent* event)
{
  this->ShowWindow("Objects");
}

void EditorMain::ShowAnimator(CommandEvent* event)
{
  Widget* widget = ShowWindow("Animator");
  widget->TakeFocus();
}

void EditorMain::ShowHotKeyEditor(CommandEvent* event)
{
  //Widget* widget = ShowWindow("CommandListViewer");
  ////widget->SetSize(Pixels(850, 500));
  //widget->TakeFocus();

  Z::gEditor->EditResource(HotKeyManager::GetDefault());
  //((HotKeyEditor*)widget)->EditResource
}

void EditorMain::ShowOperationHistroy(CommandEvent* event)
{
  if(mManager->FindWidget("HistoryWindow"))
      return;

    GameSession* editorGameSession = mEditGame;
    Space* newSpace = editorGameSession->CreateNamedSpace("HistoryWindow", ArchetypeManager::FindOrNull(CoreArchetypes::Space));

    Level* level = LevelManager::FindOrNull("HistoryWindowLevel");
    if(level != nullptr)
      newSpace->LoadLevel(level);
    else
      Error("HistoryWindowLevel not found.");

    Cog* cameraCog = newSpace->FindObjectByName("Camera");
    CameraViewport* cameraViewport = cameraCog->has(CameraViewport);

    CreateDockableWindow("HistoryWindow", cameraViewport, Vec2(300, 450), true);
}

void EditorMain::ShowBroadPhaseTracker(CommandEvent* event)
{
  this->ShowWindow("BroadPhaseTracker");
}

void EditorMain::ShowProperties(CommandEvent* event)
{
  this->ShowWindow("Properties");
}

void EditorMain::ShowConfig(CommandEvent* event)
{
  ShowProperties(event);
  SelectOnly(mConfig);
}

void EditorMain::ShowProject(CommandEvent* event)
{
  ShowProperties(event);
  SelectOnly(mProject);
}

void EditorMain::SelectTweakables(CommandEvent* event)
{
  ShowProperties(event);
  if(Z::gEngine->GetConfigCog()->has(DeveloperConfig))
    mMainPropertyView->EditObject(Z::gTweakables, true);
}

void EditorMain::ShowFindNext(CommandEvent* event)
{
  this->ShowWindow("Find/Replace Text");
  mFindTextDialog->DefaultFindNextSettings();
}

void EditorMain::ShowFindAll(CommandEvent* event)
{
  this->ShowWindow("Find/Replace Text");
  mFindTextDialog->DefaultFindAllSettings();
}

void EditorMain::ShowReplaceNext(CommandEvent* event)
{
  this->ShowWindow("Find/Replace Text");
  mFindTextDialog->DefaultReplaceNextSettings();
}

void EditorMain::ShowReplaceAll(CommandEvent* event)
{
  this->ShowWindow("Find/Replace Text");
  mFindTextDialog->DefaultReplaceAllSettings();
}

void EditorMain::ShowBugReporter(CommandEvent* event)
{
  this->ShowWindow("Bug Reporter");
  mBugReporter->Reset();
}

void EditorMain::EditColorScheme(CommandEvent* event)
{
  this->ShowWindow("Properties");
  mMainPropertyView->EditObject( GetColorScheme(), true);
}

void EditorMain::OnNameActivated(TypeEvent* event)
{
  BoundType* boundType = event->mType;

  CodeDefinition definition;
  // If this is a native location, we need to generate stub code
  if(boundType->NameLocation.IsNative)
  {
    // Generate stub code for the library (if its already generated, this will do nothing)
    Library* library = boundType->GetOwningLibrary();
    library->GenerateDefinitionStubCode();
  }

  // Copy out the needed information to the code definition (this looks like all that's needed)
  definition.NameLocation = boundType->NameLocation;
  definition.Name = boundType->Name;
  definition.ElementLocation = boundType->Location;

  DisplayCodeDefinition(definition);
}

void EditorMain::ShowLibrary(StringParam libraryName)
{
  ContentLibrary* coreLibrary = Z::gContentSystem->Libraries.FindValue(libraryName, nullptr);
  ResourceLibrary* coreResourcSet = Z::gResources->LoadedResourceLibraries.FindValue(libraryName, nullptr);

  forRange(ContentItem* contentItem, coreLibrary->GetContentItems())
    contentItem->ShowInEditor = true;

  LibraryView* libraryView = new LibraryView(this);
  libraryView->View(coreLibrary, coreResourcSet);
  libraryView->SetName(libraryName);
  libraryView->SetHideOnClose(true);
  libraryView->SetSize(Pixels(280, 280));
  this->AddManagedWidget(libraryView, DockArea::Floating, true);
}

void EditorMain::ShowVolumeMeter(CommandEvent* event)
{
  if (mManager->FindWidget("VolumeMeterWindow"))
    return;

  GameSession* editorGameSession = mEditGame;
  Space* newSpace = editorGameSession->CreateNamedSpace("VolumeMeterWindow", ArchetypeManager::FindOrNull(CoreArchetypes::Space));

  Level* level = LevelManager::FindOrNull("VolumeMeterLevel");
  if (level != nullptr)
    newSpace->LoadLevel(level);
  else
    Error("VolumeMeterLevel not found.");

  Cog* cameraCog = newSpace->FindObjectByName("GameCamera");
  CameraViewport* cameraViewport = cameraCog->has(CameraViewport);

  CreateDockableWindow("VolumeMeterWindow", cameraViewport, Vec2(300, 330), true);
}

void EditorMain::ShowSoundNodeGraph(CommandEvent* event)
{
  if (mManager->FindWidget("SoundNodeGraphWindow"))
    return;

  GameSession* editorGameSession = mEditGame;
  Space* newSpace = editorGameSession->CreateNamedSpace("SoundNodeGraphWindow", ArchetypeManager::FindOrNull(CoreArchetypes::Space));

  Level* level = LevelManager::FindOrNull("SoundNodeGraphLevel");
  if (level != nullptr)
    newSpace->LoadLevel(level);
  else
    Error("SoundNodeGraphLevel not found.");

  Cog* cameraCog = newSpace->FindObjectByName("GameCamera");
  CameraViewport* cameraViewport = cameraCog->has(CameraViewport);

  CreateDockableWindow("SoundNodeGraphWindow", cameraViewport, Vec2(700, 500), true);
}

void EditorMain::AttachDocumentEditor(StringParam name, DocumentEditor* docEditor)
{
  docEditor->SetName(name);
  Z::gEditor->AddManagedWidget(docEditor, DockArea::Center);

  Connect(this, Events::Save, docEditor, &DocumentEditor::OnSave);
  Connect(this, Events::SaveCheck, docEditor, &DocumentEditor::OnSaveCheck);
}

DocumentEditor* EditorMain::OpenTextString(StringParam name, StringParam text, StringParam extension)
{
  StringDocument* document = new StringDocument();
  String documentName = name;
  // Generate a name
  if(documentName.Empty())
    documentName = BuildString("Text",ToString(text.Hash()));

  Widget* widget = mManager->ShowWidget(documentName);
  if(widget)
    return Type::DynamicCast<DocumentEditor*>(widget);

  document->mName = documentName;
  document->mData = text;
  DocumentEditor* editor = CreateDocumentEditor(this, document);
  TypeExtensionEntry* zilchEntry = FileExtensionManager::GetZilchScriptTypeEntry();

  if(zilchEntry->IsValidExtensionNoDot(extension))
    editor->SetLexer(Lexer::Zilch);

  AttachDocumentEditor( document->mName, editor);
  return editor;
}

DocumentEditor* EditorMain::OpenTextFile(StringParam filename)
{
  String smallFileName = FilePath::GetFileName(filename);
  String name = BuildString("File: ", smallFileName);
  Widget* widget = mManager->ShowWidget(name);
  if(widget)
  {
    return Type::DynamicCast<DocumentEditor*>(widget);
  }
  else
  {
    if (FileExists(filename))
    {
      FileDocument* document = new FileDocument(smallFileName, filename);
      DocumentEditor* editor = CreateDocumentEditor(this, document);

      String extension = FilePath::GetExtension(filename);
      TypeExtensionEntry* zilchEntry = FileExtensionManager::GetZilchScriptTypeEntry();

      if(zilchEntry->IsValidExtensionNoDot(extension))
        editor->SetLexer(Lexer::Zilch);

      AttachDocumentEditor(name, editor);
      return editor;
    }
  }
  return nullptr;
}

DocumentEditor* EditorMain::OpenDocumentResource(DocumentResource* docResource)
{
  //Is the window already open?
  Widget* widget = mManager->ShowWidgetWith(docResource);

  if(widget)
  {
    return Type::DynamicCast<DocumentEditor*>(widget);
  }
  else
  {
    DocumentManager* docManager = DocumentManager::GetInstance();
    ResourceDocument* document = (ResourceDocument*)docManager->Documents.FindValue((u64)docResource->mResourceId, nullptr);
    if(document == nullptr)
      document = new ResourceDocument(docResource);

    Window* mainWindow = this->GetCenterWindow();

    DocumentEditor* editor = nullptr;

    String format = docResource->GetFormat();

    if(format == "Text")
      editor = CreateDocumentEditor(this, document);
    else
      editor = CreateScriptEditor(this, document);

    AttachDocumentEditor(document->GetDisplayName(), editor);

    return editor;
  }
}

DocumentEditor* EditorMain::OpenTextFileAuto(StringParam file)
{
  // Attempt to get the resource via loaded file lookup
  ResourceId resourceId = Z::gResources->TextResources.FindValue(file, 0);
  DocumentResource* resource = (DocumentResource*)Z::gResources->GetResource(resourceId);

  if(resource)
    return OpenDocumentResource(resource);

  //Generic file
  return OpenTextFile(file);
}

void EditorMain::OnScriptError(DebugEngineEvent* event)
{
  if(event->Handled == false)
  {
    // At the moment we always pause due to a syntax error or exception
    // If we are live editing, we really want to continue (live edit may need to be a mode)
    PauseGame();

    if(event->Script)
    {
      //debug exception needs the full file path, so set the filename now to the full path
      event->Location.Origin = event->Script->LoadPath;
      event->Handled = true;
      ScriptEditor* editor = (ScriptEditor*)OpenDocumentResource(event->Script);
      editor->OnDebugException(event);
    }
    // If there was no valid script to display an error message on then just do-notify the warning message.
    else
    {
      DoNotifyWarning("Script Error", event->Message);
    }
  }
}

void EditorMain::OnBlockingTaskStart(BlockingTaskEvent* event)
{
  mDisableInput = true;
  mLoading->Activate(event->mTaskName);
}

void EditorMain::OnBlockingTaskFinish(Event* event)
{
  mDisableInput = false;
  mLoading->Deactivate();
}

void EditorMain::OnNotifyEvent(NotifyEvent* event)
{
  if(event->Type == NotifyType::Error)
    Editor::ShowConsole();

  DoNotifyPopup(this->GetRootWidget()->GetPopUp(), event);
}

void EditorMain::StressTest(CommandEvent* event)
{
  Widget* widget = ShowWindow("Stress Test");
  if(widget != nullptr)
  {
    StressTestDialog* dialog = Type::DynamicCast<StressTestDialog*>(widget);
    if(dialog != nullptr)
      dialog->Refresh();
  }
}

void EditorMain::OnMainClick(MouseEvent* event)
{
  Composite* searchWindow = Z::gEditor->OpenSearchWindow(nullptr, true);
  searchWindow->SetTranslation(Vec3(0,0,0));
}

void OnExportTypeList(Editor* editor)
{
  MetaDatabase* instance = MetaDatabase::GetInstance();
  // Append all libraries into one list to make searching easier
  Zilch::LibraryArray allLibraries;
  allLibraries.Append(instance->mLibraries.All());
  allLibraries.Append(instance->mNativeLibraries.All());

  Array<BoundType*> baseTypesToFind;
  baseTypesToFind.PushBack(ZilchTypeId(Collider));
  baseTypesToFind.PushBack(ZilchTypeId(Joint));
  baseTypesToFind.PushBack(ZilchTypeId(PhysicsEffect));
  baseTypesToFind.PushBack(ZilchTypeId(Graphical));
  baseTypesToFind.PushBack(ZilchTypeId(ParticleAnimator));
  baseTypesToFind.PushBack(ZilchTypeId(ParticleEmitter));
  baseTypesToFind.PushBack(ZilchTypeId(Component));
  baseTypesToFind.PushBack(ZilchTypeId(Resource));
  baseTypesToFind.PushBack(ZilchTypeId(Event));
  baseTypesToFind.PushBack(ZilchTypeId(Tool));
  baseTypesToFind.PushBack(ZilchTypeId(Enum));
  baseTypesToFind.PushBack(ZilchTypeId(MetaComposition));
  baseTypesToFind.PushBack(ZilchTypeId(Widget));
  baseTypesToFind.PushBack(ZilchTypeId(ContentComponent));

  typedef OrderedHashMap<BoundType*, HashSet<String> > MapType;
  // Build a map of base type we're searching for to a set of all names that derive from that type
  MapType namesPerBaseType;
  for(size_t i = 0; i < baseTypesToFind.Size(); ++i)
  {
    namesPerBaseType[baseTypesToFind[i]];
  }
  // Add one extra set for the null type (one we didn't search for)
  namesPerBaseType[nullptr] = HashSet<String>();

  // Check all libraries
  for(size_t libraryIndex = 0; libraryIndex < allLibraries.Size(); ++libraryIndex)
  {
    // Check all types in that library
    LibraryRef& library = allLibraries[libraryIndex];
    forRange(BoundType* boundType, library->BoundTypes.Values())
    {
      BoundType* foundBaseType = nullptr;
      // Check all of our potential base types. If we don't find one then resort to null (backup)
      for(size_t typeIndex = 0; typeIndex < baseTypesToFind.Size(); ++typeIndex)
      {
        BoundType* baseType = baseTypesToFind[typeIndex];
        // If this isn't the current base type skip this
        if(!boundType->IsA(baseType))
          continue;

        foundBaseType = baseType;
        break;
      }

      namesPerBaseType[foundBaseType].Insert(boundType->Name);
    }
  }

  // Output all of the types into one file
  StringBuilder builder;
  forRange(MapType::PairType& pair, namesPerBaseType.All())
  {
    // Sort by name all of the types
    Array<String> sortedNames;
    sortedNames.Append(pair.second.All());
    Sort(sortedNames.All());

    // Figure out the category name. If the type is null then we
    // didn't categorize this type so mark it as unknown
    String name = "Unknown";
    if(pair.first != nullptr)
      name = pair.first->Name;

    // Write the category name
    builder.AppendFormat("%s:\n", name.c_str());
    // Write out the type name in phabricator's check-box format
    for(size_t i = 0; i < sortedNames.Size(); ++i)
      builder.AppendFormat("  [ ] %s\n", sortedNames[i].c_str());
    builder.Append("\n");
  }

  // Write out the results to the project directory
  ProjectSettings* projectSettings = editor->mProject->has(ProjectSettings);
  String outDir = projectSettings->ProjectFolder;

  String outFilePath = FilePath::Combine(outDir, "TypeList.txt");
  WriteStringRangeToFile(outFilePath, builder.ToString());
}

void OnExportCommandsList(Editor* editor)
{
  StringBuilder builder;
  CommandManager* instance = CommandManager::GetInstance();

  // Sort all commands by name
  Array<String> sortedCommandNames;
  forRange(Command* command, instance->mCommands)
    sortedCommandNames.PushBack(command->Name);
  Sort(sortedCommandNames.All());

  // Print out the commands
  builder.AppendFormat("Commands:\n");
  for(size_t i = 0; i < sortedCommandNames.Size(); ++i)
    builder.AppendFormat("  [ ] %s\n", sortedCommandNames[i].c_str());

  // Write out the results to the project directory
  ProjectSettings* projectSettings = editor->mProject->has(ProjectSettings);
  String outDir = projectSettings->ProjectFolder;

  String outFilePath = FilePath::Combine(outDir, "CommandsList.txt");
  WriteStringRangeToFile(outFilePath, builder.ToString());
}

void OnResaveAllResources(Editor* editor)
{
  forRange(ContentLibrary* library, Z::gContentSystem->Libraries.Values())
  {
    library->SaveAllContentItemMeta();
    library->Save();

    forRange(ContentItem* contentItem, library->GetContentItems())
    {
      contentItem->SaveContent();
    }
  }
}

void EditorRescueCall(void* userData)
{
  // Get the error context printed
  //DoNotifyErrorWithContext("Crashing");

  // Make sure the editor is valid
  if(Z::gEditor != nullptr)
  {
    // Get the target space from the editor
    Space* space = Z::gEditor->GetEditSpace();

    //// Make sure the space is valid
    if(space != nullptr && space->GetModified())
    {
      if(Level* level = space->mLevelLoaded)
      {
        ContentLibrary* library = Z::gEditor->mProjectLibrary;
        if(library)
        {
          String path = Z::gContentSystem->GetHistoryPath(library);
          // Build a string for the crashed level that places it in the library.
          String fileName = BuildString("Recovered", GetTimeAndDateStamp(), ".data");
          String backupFile = FilePath::Combine(path, fileName);
          // Attempt to save the space
          space->SaveLevelFile(backupFile);
        }
      }
    }
  }
}

void OnTweakablesModified()
{
  Z::gEditor->MarkAsNeedsUpdate();

  if(Z::gEditor->GetEditSpace())
    Z::gEditor->GetEditSpace()->MarkModified();
}

void AutoVersionCheck();
void SetupTools(Editor* editor);

#define BindCommand(commandName, memberFunction) \
  Connect(commands->GetCommand(commandName), Events::CommandExecute, editorMain, &EditorMain::memberFunction);

void CreateEditor(Cog* config, StringParam fileToOpen, StringParam newProjectName)
{
  ZPrint("Loading in Editor Mode.\n");

  Z::EditorDebugFeatures = true;

  OsShell* osShell = Z::gEngine->has(OsShell);

  LoadContentConfig(config);

  IntVec2 size = IntVec2(1280, 720);
  IntVec2 position = IntVec2(0, 0);

  WindowStyleFlags::Enum mainStyle = (WindowStyleFlags::Enum)(WindowStyleFlags::MainWindow | WindowStyleFlags::OnTaskBar | WindowStyleFlags::ClientOnly);
  OsWindow* mainWindow = osShell->CreateOsWindow("MainWindow", size, position, nullptr, mainStyle);
  mainWindow->SetMinSize(IntVec2(800, 600));
  mainWindow->SetState(WindowState::Maximized);

  // Pass window handle to initialize the graphics api
  Z::gEngine->has(GraphicsEngine)->CreateRenderer(mainWindow->GetWindowHandle());

  // This is after CreateRenderer so that the graphics api is initialized for graphics resources
  if (!LoadEditorContent(config))
    return;

  DeveloperConfig* devConfig = config->has(DeveloperConfig);

  //Setup the crash handler
  CrashHandler::SetupRescueCallback(EditorRescueCall, nullptr);

  // Set the tweakables modified callback so that we can update the Ui
  Tweakables::sModifiedCallback = &OnTweakablesModified;

  TimerBlock block("Creating Editor");

  MainWindow* rootWidget = new MainWindow(mainWindow);
  EditorMain* editorMain = new  EditorMain(rootWidget, mainWindow);
  MainConfig* mainConfig = config->has(MainConfig);
  editorMain->mConfig = config;
  editorMain->mOsWindow = mainWindow;
  editorMain->mMainWindow = rootWidget;

  String dataDirectory = mainConfig->DataDirectory;
  CommandManager* commands = CommandManager::GetInstance();
  editorMain->mCommands = commands;
  commands->LoadCommands(FilePath::Combine(dataDirectory, "Commands.data"));
  commands->LoadMenu(FilePath::Combine(dataDirectory, "Menus.data"));
  commands->LoadMenu(FilePath::Combine(dataDirectory, "Toolbars.data"));

  SetupTools(editorMain);

  commands->SetContext(editorMain, ZilchTypeId(Editor));
  rootWidget->LoadMenu("Main");

  Connect(Z::gEngine, Events::Notify, editorMain, &EditorMain::OnNotifyEvent);

  BindEditorCommands(config, commands);
  BindAppCommands(config, commands);
  BindCodeTranslatorCommands(config, commands);
  SetupGraphCommands(config, commands);
  BindArchiveCommands(config, commands);
  BindGraphicsCommands(config, commands);
  //BindGeometryCommands(config, commands);
  BindCreationCommands(config, commands);
  BindDocumentationCommands(config, commands);
  BindProjectCommands(config, commands);
  BindContentCommands(config, commands);

  //HotKeyManager *hkManager = HotKeyManager::Instance;// GetSystemObject(HotKeyManager);
  //hkManager->RegisterSystemCommands(commands->mCommands);

  // Listen to the resource system if any unhandled exception or syntax error occurs
  Connect(Z::gResources, Events::UnhandledException, editorMain, &EditorMain::OnScriptError);
  Connect(Z::gResources, Events::SyntaxError, editorMain, &EditorMain::OnScriptError);

  // For setting the default docked windows' width to a percentage
  // to make a better initial layout on smaller resolutions
  Vec2 rootSize = editorMain->GetRootWidget()->GetSize();
  float dockWidth = Math::Min(rootSize.x * 0.15f, 280.0f);

  MultiManager* manager = new MultiManager(rootWidget, editorMain);
  editorMain->mManager = manager;

  Connect(manager, Events::OsKeyDown, editorMain, &EditorMain::OnKeyDown);

  //----------------------------------------------------------------------------
  {
    // Create a persistent Library instance so that the rest of the engine can manipulate it
    // regardless of how many instances there are existing (See ContentPackageImporter)
    editorMain->mLibrary = editorMain->CreateLibraryView(true, false);
    editorMain->mLibrary->SetHideOnClose(true);
    editorMain->AddManagedWidget(editorMain->mLibrary, DockArea::Right, true);
  }

  //----------------------------------------------------------------------------
  {
    ObjectView* objects = new ObjectView(editorMain);
    objects->SetName("Objects");
    objects->SetHideOnClose(true);
    objects->SetSize(Pixels(dockWidth, 280));
    editorMain->AddManagedWidget(objects, DockArea::Right, true);
    editorMain->mObjectView = objects;
  }

  //----------------------------------------------------------------------------
  {
    HotKeyEditor* hotkeyEditor = new HotKeyEditor(editorMain);
    hotkeyEditor->SetName("CommandListViewer");
    hotkeyEditor->SetHideOnClose(true);
    hotkeyEditor->SetSize(Pixels(850, 500));
    editorMain->AddManagedWidget(hotkeyEditor, DockArea::Floating, false);
  }

  //----------------------------------------------------------------------------
  {
    BindCommand("CommandListViewer", ShowHotKeyEditor);
    BindCommand("OperationHistory", ShowOperationHistroy);
    BindCommand("Animator", ShowAnimator);
    BindCommand("FindNext", ShowFindNext);
    BindCommand("FindAll", ShowFindAll);
    BindCommand("ReplaceNext", ShowReplaceNext);
    BindCommand("ReplaceAll", ShowReplaceAll);
    BindCommand("ReportBug", ShowBugReporter);
    BindCommand("Tools", ShowTools);
    BindCommand("Properties", ShowProperties);
    BindCommand("SelectEditorConfig", ShowConfig);
    BindCommand("SelectProject", ShowProject);
    BindCommand("Tweakables", SelectTweakables);
    BindCommand("Library", ShowLibrary);
    BindCommand("Console", ToggleConsole);
    BindCommand("Browser", ShowBrowser);
    BindCommand("Market", ShowMarket);
    BindCommand("Objects", ShowObjects);
    BindCommand("BroadPhaseTracker", ShowBroadPhaseTracker);
    BindCommand("VolumeMeter", ShowVolumeMeter);
    BindCommand("SoundNodeGraph", ShowSoundNodeGraph);

    BindCommand("EditColorScheme", EditColorScheme);
    BindCommand("ShowCoreLibrary", ShowCoreLibrary);

    Connect(Z::gEngine, Events::BlockingTaskStart, editorMain, &EditorMain::OnBlockingTaskStart);
    Connect(Z::gEngine, Events::BlockingTaskFinish, editorMain, &EditorMain::OnBlockingTaskFinish);

    BindCommand("StressTest", StressTest);
    // Add a command to write out all bound types in the engine
    if(devConfig != nullptr)
    {
      commands->AddCommand("ExportTypeList", BindCommandFunction(OnExportTypeList));
      commands->AddCommand("ResaveAllResources", BindCommandFunction(OnResaveAllResources));
      commands->AddCommand("OnExportCommandsList", BindCommandFunction(OnExportCommandsList));
    }

    //-------------------------------------------------------- Tool Bar Creation
    {
      // All tools exist under this toolbar area
      ToolBarArea* toolBarArea = new ToolBarArea(editorMain);
      toolBarArea->SetTranslation(Vec3(0,Pixels(-40),0));
      toolBarArea->SetSize(Pixels(1,38));
      toolBarArea->SetDockArea(DockArea::TopTool);

      Spacer* spacer = new Spacer(toolBarArea);
      spacer->SetSize(Pixels(3,0));
      spacer->SetDockMode(DockMode::DockLeft);

      rootWidget->mMainMenu->SetActive(true);
      Connect(rootWidget->mMainMenu, Events::LeftClick,
              editorMain, &EditorMain::OnMainClick);

      // Save, copy/cut/paste, undo/redo
      ToolBar* primaryActions = new ToolBar(toolBarArea);
      primaryActions->LoadMenu("PrimaryToolbar");
      primaryActions->SetDockMode(DockMode::DockLeft);

      // Primary Tools
      ToolBar* primaryTools = new ToolBar(toolBarArea);
      primaryTools->LoadMenu("PrimaryToolsToolbar");
      primaryTools->SetDockMode(DockMode::DockLeft);

      // Edit Tools
      ToolBar* editTools = new ToolBar(toolBarArea);
      editTools->LoadMenu("EditToolsToolbar");
      editTools->SetDockMode(DockMode::DockLeft);

      ToolBar* addBar = new ToolBar(toolBarArea);
      addBar->LoadMenu("ResourceToolbar");
      addBar->SetDockMode(DockMode::DockLeft);

      // Extra windows
      ToolBar* windowsGroup = new ToolBar(toolBarArea);
      editTools->LoadMenu("WindowsToolbar");
      windowsGroup->SetDockMode(DockMode::DockLeft);

      ToolBar* gameToolbar = new ToolBar(toolBarArea);
      gameToolbar->SetTranslation(Pixels(825, 0, 0));
      gameToolbar->SetDockMode(DockMode::Enum(DockMode::DockLeft | DockMode::DockRight));
      gameToolbar->LoadMenu("GameToolbar");

      ToolBar* helpToolbar = new ToolBar(toolBarArea);
      helpToolbar->SetTranslation(Pixels(1856, 0, 0));
      helpToolbar->SetDockMode(DockMode::DockRight);
      new BackgroundTaskButton(helpToolbar);
      helpToolbar->LoadMenu("HelpToolbar");
    }
  }

  MetaSelection* selection = editorMain->GetSelection();

  //----------------------------------------------------------------------------
  // Tool Area
  editorMain->Tools->SetSize(Pixels(dockWidth, 280));
  Window* sideBar = editorMain->AddManagedWidget(editorMain->Tools, DockArea::Left, true);

  MainPropertyView* propertyViewArea = new MainPropertyView(editorMain, selection,
                                                            editorMain->mQueue);

  propertyViewArea->SetHideOnClose(true);
  editorMain->mMainPropertyView = propertyViewArea;

  sideBar->AttachAsTab(propertyViewArea, false);
  
  editorMain->Tools->SelectToolIndex(0);

  Connect(propertyViewArea->GetPropertyView(), Events::NameActivated, editorMain, &EditorMain::OnNameActivated);

  //----------------------------------------------------------------------------
  {
    // Create the console window
    ConsoleUi* console = new ConsoleUi(editorMain);
    console->SetName("Console");
    console->SetHideOnClose(true);
    console->SetSize(Pixels(800, 220));

    editorMain->AddManagedWidget(console, DockArea::Bottom, false);
    editorMain->mConsole = console;
  }

  //----------------------------------------------------------------------------
  {
    // Create the find text dialog
    FindTextDialog* findText = new FindTextDialog(editorMain);
    findText->SetName("Find/Replace Text");
    findText->SetHideOnClose(true);
    editorMain->AddManagedWidget(findText, DockArea::Floating, false);
    editorMain->mFindTextDialog = findText;
  }

  //----------------------------------------------------------------------------
  {
    // Create the bug report dialog
    BugReporter* dialog = new BugReporter(editorMain);
    dialog->SetName("Bug Reporter");
    dialog->SetHideOnClose(true);
    editorMain->AddManagedWidget(dialog, DockArea::Floating, false);
    editorMain->mBugReporter = dialog;
  }

  //----------------------------------------------------------------------------
  {
    // Create the stress test dialog
    StressTestDialog* stress = new StressTestDialog(editorMain);
    stress->SetName("Stress Test");
    stress->SetHideOnClose(true);
    editorMain->AddManagedWidget(stress, DockArea::Floating, false);
    editorMain->mStressTestDialog = stress;
  }

  //----------------------------------------------------------------------------
  {
    // Create the Broad Phase Editor window
    BroadPhaseEditor* bpEditor = new BroadPhaseEditor(editorMain);
    bpEditor->SetName("BroadPhaseTracker");
    bpEditor->SetHideOnClose(true);
    bpEditor->SetSize(Pixels(320, 360));
    editorMain->AddManagedWidget(bpEditor, DockArea::Floating, false);
  }

  //----------------------------------------------------------------------------
  {
    // Create the desync window
    Window* desyncWindow = new Window(editorMain);
    desyncWindow->SetTitle("Desync");
    desyncWindow->SetTranslationAndSize(Pixels(0, 0, 0), Pixels(800, 800));
    desyncWindow->SetHideOnClose(true);
    desyncWindow->SetActive(false);
    editorMain->mDesyncWindow = desyncWindow;
  }

  if(DeveloperConfig* devConfig = config->has(DeveloperConfig))
  {
    //editorMain->Tools->AddTool(new ClothAnchorTool());
    //editorMain->Tools->AddTool(new ClothCutterTool());
  }

  //----------------------------------------------------------------------------

  {
    AnimationEditor* animator = new AnimationEditor(editorMain);
    animator->SetPropertyView(editorMain->mMainPropertyView);
    animator->SetName("Animator");
    animator->SetHideOnClose(true);
    animator->SetSize(Pixels(500, 95));
    animator->SetMinSize(Pixels(300, 200));
    editorMain->AddManagedWidget(animator, DockArea::Bottom, false);
  }

  // This sets the size of the editor to the correct full size of the screen
  rootWidget->Refresh();

  // Moves everything off the screen
  editorMain->SetExploded(true, false);

  ZPrint("Welcome to the Zero Editor.\n");

  editorMain->mLoading = new LoadingWindow(rootWidget);
  editorMain->mLoading->SetActive(false);

  // Compile once before trying to load a project so that the engine can render
  ZilchManager::GetInstance()->Compile();

  // If we have a file to be loaded
  if(!fileToOpen.Empty())
  {
    // Project extension
    String extension = FilePath::GetExtension(fileToOpen);

    // If the file passed in is a project file...
    if(extension == "zeroproj")
    {
      // Open the project
      OpenProjectFile(fileToOpen);
    }
    else
    {
      Event event;
      Z::gEngine->DispatchEvent(Events::NoProjectLoaded, &event);
      DoNotifyError("Unknown file type", "Unknown file type must be a valid zero project");
    }
  }
  else
  {
    // Open cached project in user config
    String startingProject = HasOrAdd<EditorConfig>(Z::gEditor->mConfig)->EditingProject;
    //if the user has requested to create a new project then don't open the last edited project
    if(newProjectName.Empty() && FileExists(startingProject))
    {
      OpenProjectFile(startingProject);
    }
    else
    {
      Event event;
      Z::gEngine->DispatchEvent(Events::NoProjectLoaded, &event);
      DoNotifyWarning("No project found", "No project file found. Opening launcher");
      NewProject();
    }
  }

  HasOrAdd<TextEditorConfig>(Z::gEditor->mConfig);

  editorMain->ShowWindow("Tools");

  // If the debugger is attached add a simple listener component to listen on
  // a tcp socket for the launcher telling us to open a project file
  if(Os::IsDebuggerAttached())
  {
    Z::gEditor->mSimpleDebuggerListener = new SimpleDebuggerListener();
  }

  CommandManager::GetInstance()->ValidateCommands();
}

}//namespace Zero
