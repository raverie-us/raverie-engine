///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(LauncherDebuggerCommunicationCompleted);
DefineEvent(LauncherDebuggerCommunicationFailed);
}// namespace Events

void LoadProject(Editor* editor, Cog* projectCog, StringParam path, StringParam projectFile)
{
  ZPrint("Loading project '%s'\n", projectFile.c_str());

  if(editor->mProject)
    UnloadProject(editor, editor->mProject);

  Z::gEditor->mProject = projectCog;

  HasOrAdd<EditorConfig>(Z::gEditor->mConfig)->EditingProject = projectFile;

  RecentProjects* recent = HasOrAdd<RecentProjects>(Z::gEditor->mConfig);
  recent->AddRecentProject(projectFile);

  // Save what project was opened
  SaveConfig(Z::gEditor->mConfig);

  ProjectSettings* project = projectCog->has(ProjectSettings);
  project->ContentFolder = FilePath::Combine(path, "Content");
  project->EditorContentFolder = FilePath::Combine(path, "EditorContent");
  project->ProjectFolder = path;
  project->ProjectFile = projectFile;

  // Set the Platform Title Bar.
  editor->mMainWindow->SetTitle(BuildString("Zero Editor - ", project->ProjectName));

  // Set the store name based on the project name.
  ObjectStore::GetInstance()->SetStoreName(project->ProjectName);

  String projectFolder = project->ProjectFolder;

  // Load shared content libraries if present
  if(SharedContent* sharedConent = projectCog->has(SharedContent))
  {
    forRange(ContentLibraryReference libraryRef, sharedConent->ExtraContentLibraries.All())
    {
      String libraryName = libraryRef.mContentLibraryName;
      String contentFolder = FilePath::Combine(projectFolder, libraryName);
      Status loadContentLibrary;
      ContentLibrary* projectLibrary = Z::gContentSystem->LibraryFromDirectory(loadContentLibrary, libraryName, contentFolder);
      if(projectLibrary)
        Z::gContentSystem->BuildLibraryIntoPackageJob(projectLibrary);
      else
        DoNotifyWarning("Missing Library", String::Format("Failed to find shared content library %s", libraryName.c_str()));
    }
  }

  EditorSettings* engineEditorSettings = HasOrAdd<EditorSettings>(Z::gEditor->mConfig);

  // Load content package of project
  Status loadContentLibrary;
  ContentLibrary* projectLibrary = Z::gContentSystem->LibraryFromDirectory(loadContentLibrary,
                                                                           project->ProjectName, 
                                                                           project->ContentFolder);

  /// Store the library on the project
  project->ProjectContentLibrary = projectLibrary;

  Z::gContentSystem->BuildLibraryIntoPackageJob(projectLibrary);

  // Always select the first tool
  if (editor->Tools)
    editor->Tools->SelectToolIndex(0);
}

void UnloadProject(Editor* editor, Cog* projectCog)
{
  ProjectSettings* project = projectCog->has(ProjectSettings);

  ZPrint("Unloading project %s\n", project->ProjectName.c_str());

  ResourceSystem* resourceSystem = Z::gResources;

  // Send Event so other controls can clean up state
  Event event;
  editor->DispatchEvent(Events::UnloadProject, &event);

  // Clear Library View
  editor->mLibrary->View(nullptr, nullptr);

  // Clear Active Selection
  editor->mSelection->Clear();

  // Clear Undo history
  editor->mQueue->ClearAll();

  // Clear Property View
  editor->mMainPropertyView->HardReset();

  // Unload resources
  if(ResourceLibrary* projectLibrary = project->ProjectResourceLibrary)
    projectLibrary->Unload();

  forRange(ResourceLibrary* library, project->SharedResourceLibraries.All())
    library->Unload();

  // Send Out Editor Close

  // Destroy the edit game session
  editor->mEditGame.SafeDestroy();
  editor->mProject = nullptr;
  editor->mProjectLibrary = nullptr;

  projectCog->Destroy();

  // Always select the first tool
  if (editor->Tools)
    editor->Tools->SelectToolIndex(0);
}

void OpenProjectFile(StringParam filename)
{
  // File check
  if(!FileExists(filename))
  {
    DoNotifyError("Failed to load project.", "Project file not found.");
    return;
  }

  // Load the project object
  Cog* projectCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), filename, CreationFlags::ProxyComponentsExpected, nullptr);
  if(projectCog == nullptr)
  {
    DoNotifyError("Failed to load project.", "Project file invalid.");
    return;
  }

  ProjectSettings* project = projectCog->has(ProjectSettings);
  if(project == nullptr) return;

  // Begin the loading project
  String projectFolder = FilePath::GetDirectoryPath(filename);
  LoadProject(Z::gEditor, projectCog, projectFolder, filename);
}

// Project Commands


void RunLauncherOpenProject(StringParam eventName)
{
  //See if the composite is already open
  Widget* widget = Z::gEditor->mManager->FindWidget(ZilchTypeId(LauncherOpenProjectComposite)->Name);
  LauncherOpenProjectComposite* launcherComposite = (LauncherOpenProjectComposite*)widget;
  //if not create it
  if(launcherComposite == nullptr)
  {
    launcherComposite = new LauncherOpenProjectComposite(Z::gEditor);
    Z::gEditor->AddManagedWidget(launcherComposite, DockArea::Floating, false);
  }
  //run the given event name (to communicate with the launcher)
  launcherComposite->SendEvent(eventName);
}

void OpenProject()
{
  RunLauncherOpenProject(Events::LauncherOpenRecentProjects);
}

void ReloadProject()
{
  Cog* projectCog = Z::gEditor->mProject;
  if(projectCog == nullptr)
    return;

  ProjectSettings* project = projectCog->has(ProjectSettings);
  OpenProjectFile(project->ProjectFile);
}

void NewProject()
{
  RunLauncherOpenProject(Events::LauncherNewProject);
}

void SaveProject()
{
  Editor* editor = Z::gEditor;
  editor->SaveAll(true);

  // Take a screen shot if it's enabled
  if(Cog* projectCog = editor->mProject)
  {
    ProjectSettings* project = projectCog->has(ProjectSettings);
    if(project->AutoTakeProjectScreenshot)
      editor->TakeProjectScreenshot();
  }
}

void BindProjectCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("OpenProject", BindCommandFunction(OpenProject));
  commands->AddCommand("ReloadProject", BindCommandFunction(ReloadProject));
  commands->AddCommand("NewProject", BindCommandFunction(NewProject));
  commands->AddCommand("SaveProject", BindCommandFunction(SaveProject));
}

ZilchDefineType(LauncherOpenProjectComposite, builder, type)
{

}

LauncherOpenProjectComposite::LauncherOpenProjectComposite(Composite* parent) : Composite(parent)
{
  mSocket = nullptr;
  mName = ZilchVirtualTypeId(this)->Name;
}

LauncherOpenProjectComposite::~LauncherOpenProjectComposite()
{
  delete mSocket;
}

void LauncherOpenProjectComposite::SendEvent(StringParam eventType)
{
  mEventType = eventType;
  // delete the old socket
  if(mSocket != nullptr)
    delete mSocket;
  mSocket = nullptr;

  //load the launcher config, if we can load it and get the launcher config
  //then try to communicate with the launcher if it's open or launch it if it's closed
  mLauncherConfig = LoadLauncherConfig(nullptr, StringMap());
  if(mLauncherConfig != nullptr)
  {
    mVersionConfig = mLauncherConfig->has(LauncherConfig);
    if(mVersionConfig != nullptr && FileExists(mVersionConfig->mLauncherLocation))
    {
      CommunicateWithLauncher();
      return;
    }
  }

  // Try to run from the installed location, if not notify the user
  if(!RunFromInstalledPath())
    FailedToOpenLauncher();
}

void LauncherOpenProjectComposite::FailedToOpenLauncher()
{
  DoNotifyError("Launcher not found", "Couldn't find 'ZeroLauncher.exe'. "
    "Please download the installer from https://www.zeroengine.io.");
}

bool LauncherOpenProjectComposite::RunLauncherExe(StringParam exePath)
{
  if(FileExists(exePath))
  {
    String cmd;
    if(mEventType == Events::LauncherOpenRecentProjects)
      cmd = BuildString("-", LauncherStartupArguments::Names[LauncherStartupArguments::Projects]);
    else if(mEventType == Events::LauncherNewProject)
      cmd = BuildString("-", LauncherStartupArguments::Names[LauncherStartupArguments::New]);
    
    // If the debugger is attached tell the launcher to run in debugger mode
    if(Os::IsDebuggerAttached())
      cmd = BuildString(cmd, " -", LauncherStartupArguments::Names[LauncherStartupArguments::DebuggerMode]);

    Os::SystemOpenFile(exePath.c_str(), Os::Verb::Default, cmd.c_str());
    return true;
  }
  return false;
}

bool LauncherOpenProjectComposite::RunFromInstalledPath()
{
  String installPath;
  bool result = GetRegistryValueFromCommonInstallPaths(GetLauncherGuidString(), "InstallLocation", installPath);
  if(result == false)
    return false;

  String exePath = FilePath::Combine(installPath, "ZeroLauncher.exe");
  return RunLauncherExe(exePath);
}

void LauncherOpenProjectComposite::CommunicateWithLauncher()
{
  mSocket = new TcpSocket(Protocol::Events | Protocol::Chunks, "Launcher");

  ConnectThisTo(mSocket, Events::ConnectionCompleted, OnConnectionCompleted);
  ConnectThisTo(mSocket, Events::ConnectionFailed, OnConnectionFailed);
  mSocket->Connect("localhost", LauncherCommunicationEvent::DesiredPort);
}

void LauncherOpenProjectComposite::OnConnectionCompleted(Event* e)
{
  //send a message to the launcher telling it to show the new or recent projects ui
  LauncherCommunicationEvent toSend;
  if(Os::IsDebuggerAttached())
    toSend.mExtraData = LauncherStartupArguments::Names[LauncherStartupArguments::DebuggerMode];
  mSocket->SendToAll(mEventType, &toSend);

  mSocket->Close();
}

void LauncherOpenProjectComposite::OnConnectionFailed(Event* e)
{
  // If the version selector exists then run the project through it
  if(mVersionConfig != nullptr && !mVersionConfig->mLauncherLocation.Empty())
  {
    String launcherPath = mVersionConfig->mLauncherLocation;
    if(RunLauncherExe(launcherPath))
      return;
  }
  // Try to run from the installed location, if not open the old dialog
  if(!RunFromInstalledPath())
  {
    //otherwise the launcher doesn't seem to exist so run the old open/new dialog
    FailedToOpenLauncher();
  }

  mSocket->Close();
}

//---------------------------------------------------------------------------------------
ZilchDefineType(LauncherSingletonCommunication, builder, type)
{

}

LauncherSingletonCommunication::LauncherSingletonCommunication(const StringMap& arguments)
{
  mArguments = arguments;
  mTimesTryingToConnect = 0;

  mSocket = new TcpSocket(Protocol::Events | Protocol::Chunks, "Launcher");
  ConnectThisTo(mSocket, Events::ConnectionCompleted, OnConnectionCompleted);
  ConnectThisTo(mSocket, Events::ConnectionFailed, OnConnectionFailed);
  mSocket->Connect("localhost", LauncherCommunicationEvent::DesiredPort);

  //this happens before the engine even exists and the tcp socket only updates during EngineUpdate
  //so manually pump the engine a set number of times until we succeed or fail to connect
  while(mTimesTryingToConnect < 100)
    Z::gEngine->Update();
}

LauncherSingletonCommunication::~LauncherSingletonCommunication()
{
  delete mSocket;
}

void LauncherSingletonCommunication::OnConnectionCompleted(Event* e)
{
  //stop trying to connect now
  mTimesTryingToConnect = 100;
  mStatus.Succeeded();

  LauncherCommunicationEvent myEvent;
  myEvent.LoadFromCommandArguments(mArguments);

  // If no arguments were provided then just tell the open launcher to display recent projects
  if(myEvent.EventId.Empty())
    myEvent.EventId = Events::LauncherOpenRecentProjects;
  mSocket->SendToAll(myEvent.EventId, &myEvent);
}

void LauncherSingletonCommunication::OnConnectionFailed(Event* e)
{
  //stop trying to connect now
  mTimesTryingToConnect = 100;
  mStatus.SetFailed("Failed to connect");
}

//---------------------------------------------------------------------------------------
ZilchDefineType(LauncherDebuggerCommunication, builder, type)
{

}

LauncherDebuggerCommunication::LauncherDebuggerCommunication()
{
  mSocket = nullptr;
}

LauncherDebuggerCommunication::~LauncherDebuggerCommunication()
{

}

void LauncherDebuggerCommunication::SendOpenProject(StringParam projectFile)
{
  SendOpenProject(projectFile, LauncherCommunicationEvent::DebuggerDesiredPort);
}

void LauncherDebuggerCommunication::SendOpenProject(StringParam projectFile, int port)
{
  mProjectFile = projectFile;
  mSocket = new TcpSocket(Protocol::Events | Protocol::Chunks, "Launcher");
  mSocket->mDataVersion = DataVersion::Legacy;
  
  ConnectThisTo(mSocket, Events::ConnectionCompleted, OnConnectionCompleted);
  ConnectThisTo(mSocket, Events::ConnectionFailed, OnConnectionFailed);
  mSocket->Connect("localhost", port);
}

void LauncherDebuggerCommunication::OnConnectionCompleted(Event* e)
{
  LauncherCommunicationEvent toSend;
  toSend.mProjectFile = mProjectFile;
  mSocket->SendToAll(Events::LauncherOpenProject, &toSend);
  DispatchEvent(Events::LauncherDebuggerCommunicationCompleted, &toSend);

  mSocket->Close();
}

void LauncherDebuggerCommunication::OnConnectionFailed(Event* e)
{
  LauncherCommunicationEvent toSend;
  toSend.mProjectFile = mProjectFile;
  DispatchEvent(Events::LauncherDebuggerCommunicationFailed, &toSend);
}

//---------------------------------------------------------------------------------------
ZilchDefineType(SimpleDebuggerListener, builder, type)
{

}

SimpleDebuggerListener::SimpleDebuggerListener()
{
  mListener = new TcpSocket(Protocol::Events | Protocol::Chunks, "IPC-Listener");
  // Listen for the launcher telling us to open a project
  ConnectThisTo(mListener, Events::LauncherOpenProject, OnLauncherOpenProject);
  mListener->Listen(LauncherCommunicationEvent::DebuggerDesiredPort, TcpSocket::MaxPossibleConnections, TcpSocketBind::Loopback);
}

SimpleDebuggerListener::~SimpleDebuggerListener()
{
  delete mListener;
}

void SimpleDebuggerListener::OnLauncherOpenProject(LauncherCommunicationEvent* e)
{
  OpenProjectFile(e->mProjectFile);
}

}//namespace Zero
