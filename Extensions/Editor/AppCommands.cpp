#include "Precompiled.hpp"

namespace Zero
{

void OpenHelp()
{
  Os::SystemOpenNetworkFile("https://help.zeroengine.io");
}

void OpenZeroHub()
{
  Os::SystemOpenNetworkFile("https://dev.zeroengine.io");
}

void OpenDocumentation()
{
	Os::SystemOpenNetworkFile("https://docs.zeroengine.io");
}

void ExitEditor()
{
  Z::gEditor->RequestQuit(false);
}

void Restart()
{
  bool quitSuccess = Z::gEditor->RequestQuit(true);

  if(quitSuccess)
    Os::SystemOpenFile( GetApplication().c_str() );
}

void FullScreen(Editor* editor)
{
  OsWindow* osWindow = editor->mOsWindow;
  if (osWindow->GetState() != WindowState::Fullscreen)
    osWindow->SetState(WindowState::Fullscreen);
  else
    osWindow->SetState(WindowState::Windowed);
}

void ShowTextWindow(StringParam windowTitle, StringParam windowText)
{
  Window* window = new Window(Z::gEditor);
  window->SetTitle(windowTitle);

  window->SetTranslation(Vec3(0, -500, 0));
  MultiLineText* textBox = new MultiLineText(window);
  textBox->SetText(windowText);

  window->SizeToContents();
  CenterToWindow(Z::gEditor, window, true);
}

void ShowAbout()
{
  String text = 
    "DigiPen Zero Engine\n"
    "Copyright: \n"
    "DigiPen Institute of Technology 2016\n"
    "All rights reserved. \n"
    "Version: \n";
  ShowTextWindow("About", BuildString(text, GetBuildVersionName()));
}

DeclareEnum3(VersionStatus, Unknown, UpToDate, NewVersionAvailable);

VersionStatus::Type GlobalVersionStatus = VersionStatus::Unknown;

void BuildVersion()
{
  String buildVersionString = String::Format("BuildVersion: %s", GetBuildVersionName());
  ZPrintFilter(Filter::DefaultFilter, "%s\n", buildVersionString.c_str());
  OsShell* platform = Z::gEngine->has(OsShell);
  platform->SetClipboardText(buildVersionString);
}

void WriteBuildInfo()
{
  String sourceDir = Z::gEngine->GetConfigCog()->has(MainConfig)->SourceDirectory;
  Environment* environment = Environment::GetInstance();
  String filePath = environment->GetParsedArgument("WriteBuildInfo");
  if(filePath.Empty() || filePath == "true")
    filePath = FilePath::CombineWithExtension(sourceDir, "Meta", ".data");
  
  StringBuilder builder;
  builder.AppendFormat("MajorVersion %d\n", GetMajorVersion());
  builder.AppendFormat("MinorVersion %d\n", GetMinorVersion());
  builder.AppendFormat("PatchVersion %d\n", GetPatchVersion());
  builder.AppendFormat("RevisionId %d\n", GetRevisionNumber());
  builder.AppendFormat("Platform \"%s\"\n", GetPlatformString());
  cstr experimentalBranchName = GetExperimentalBranchName();
  if(experimentalBranchName != nullptr)
    builder.AppendFormat("ExperimentalBranchName \"%s\"\n", experimentalBranchName);
  builder.AppendFormat("ShortChangeSet \"%s\"\n", GetShortChangeSetString());
  builder.AppendFormat("ChangeSet \"%s\"\n", GetChangeSetString());
  builder.AppendFormat("ChangeSetDate \"%s\"\n", GetChangeSetDateString());
  builder.AppendFormat("BuildId \"%s\"\n", GetBuildIdString());
  builder.AppendFormat("LauncherMajorVersion %d\n", GetLauncherMajorVersion());

  String result = builder.ToString();
  WriteStringRangeToFile(filePath, result);

  ZPrint("Writing build info to '%s'\n", filePath.c_str());
  ZPrint("File Contents: %s\n", result.c_str());

  Z::gEngine->Terminate();
}

void OpenTestWidgetsCommand()
{
  OpenTestWidgets(Z::gEditor);
}

// Used to test the crash handler
void CrashEngine()
{
  Z::gEngine->CrashEngine();
}

void SortAndPrintMetaTypeList(StringBuilder& builder, Array<String>& names, cstr category)
{
  if (names.Empty())
    return;

  Sort(names.All());

  builder.Append(category);
  builder.Append(":\n");
  forRange(String& name, names.All())
  {
    builder.Append("  ");
    builder.Append(name);
    builder.Append("\n");
  }
  builder.Append("\n");
}

// Special class to delay dispatching the unit test command until scripts have been compiled
class UnitTestDelayRunner : public Composite
{
public:
  typedef UnitTestDelayRunner ZilchSelf;

  void OnScriptsCompiled(Event* e)
  {
    ActionSequence* seq = new ActionSequence(Z::gEditor, ActionExecuteMode::FrameUpdate);
    //wait a little bit so we the scripts will be finished compiling (and hooked up)
    seq->Add(new ActionDelay(0.1f));
    seq->Add(new ActionEvent(Z::gEngine, "RunUnitTests", new Event()));

    //when the game plays the scripts will be compiled again so don't dispatch this event
    DisconnectAll(Z::gEngine, this);
    this->Destroy();
  }

  UnitTestDelayRunner(Composite* parent) : Composite(parent)
  {
    OnScriptsCompiled(nullptr);
  }
};

void RunUnitTests()
{
  Zilch::Sha1Builder::RunUnitTests();
  new UnitTestDelayRunner(Z::gEditor);
}

void RecordUnitTestFile()
{
  UnitTestSystem* unitTestSystem = Z::gEngine->has(UnitTestSystem);
  unitTestSystem->RecordToZeroTestFile();
}

void PlayUnitTestFile()
{
  UnitTestSystem* unitTestSystem = Z::gEngine->has(UnitTestSystem);
  unitTestSystem->PlayFromZeroTestFile();
}

void HostZilchDebugger()
{
  ZilchManager* manager = ZilchManager::GetInstance();
  manager->mDebugger.Host(8000);
}

void RunZilchDebugger()
{
  HostZilchDebugger();
  String debuggerPath = FilePath::Combine(Z::gContentSystem->ToolPath, "ZilchDebugger.html");
  Os::SystemOpenFile(debuggerPath.c_str());
}


void BindAppCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("About", BindCommandFunction(ShowAbout));

  commands->AddCommand("Exit", BindCommandFunction(ExitEditor));
  commands->AddCommand("ToggleFullScreen", BindCommandFunction(FullScreen));
  commands->AddCommand("Restart", BindCommandFunction(Restart));

  commands->AddCommand("BuildVersion", BindCommandFunction(BuildVersion));
  commands->AddCommand("WriteBuildInfo", BindCommandFunction(WriteBuildInfo));
  commands->AddCommand("RunUnitTests", BindCommandFunction(RunUnitTests));
  commands->AddCommand("RecordUnitTestFile", BindCommandFunction(RecordUnitTestFile));
  commands->AddCommand("PlayUnitTestFile", BindCommandFunction(PlayUnitTestFile));

  if(DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig))
  {
    commands->AddCommand("OpenTestWidgets", BindCommandFunction(OpenTestWidgetsCommand));
    commands->AddCommand("CrashEngine", BindCommandFunction(CrashEngine));

    commands->AddCommand("HostZilchDebugger", BindCommandFunction(HostZilchDebugger));
    commands->AddCommand("RunZilchDebugger", BindCommandFunction(RunZilchDebugger));
  }
  commands->AddCommand("Help", BindCommandFunction(OpenHelp));
  commands->AddCommand("ZeroHub", BindCommandFunction(OpenZeroHub));
  commands->AddCommand("Documentation", BindCommandFunction(OpenDocumentation));
}

}
