#include "Precompiled.hpp"

namespace Zero
{

void OpenHelp()
{
  Os::SystemOpenNetworkFile("https://help.zeroengine.io");
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
    osWindow->SetState(WindowState::Maximized);
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

void ShowControls(Editor* editor)
{
  String controlsText = 
    "Movement\n"
    "  WASD - Dolly camera on x/z plane\n" 
    "  Q / Alt - Orbit camera with left mouse\n"
    "  E - Pan camera with left mouse\n"
    "  Mouse Wheel - Dolly camera on view\n"
    "  F - Focus on selected objects\n"
    "  FF - Focus and Zoom selected Objects\n"
    "\n"
    "Commands\n"
    "  ~ - Toggle Console\n"
    "  Ctrl+X - Cut selected objects\n"
    "  Ctrl+C - Copy selected objects\n"
    "  Ctrl+V - Paste objects\n"
    "  F5 - Play game\n"
    "  F6 - Toggle pause game\n"
    "  F7 - Step game forward one frame\n"
    "  F8 - Stop game\n"
    "  F9 - Edit running game\n"
    "  F10 - Lock mouse to window\n"
    "  F11 - Zoom\n"
    "\n"
    "Selection\n"
    "  Shift+A - Select all\n"
    "  Shift+W - Select Parent\n"
    "  Shift+S - Select Space\n"
    "  Shift+C - Select Camera\n"
    "  Shift+D - Deselect\n"
    "  Left Mouse - Select objects\n"
    "  Left Mouse & Drag - Box select\n" 
    "  Left Mouse & Drag+Ctrl - Smart Group Select\n"
    "  Ctrl+L - Enable lighting\n";

  ShowTextWindow("Controls", controlsText);
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
  builder.AppendFormat("ExperimentalBranchName \"%s\"\n", GetExperimentalBranchName());
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

void HostZilchDebugger()
{
  // METAREFACTOR - Removed until ZilchScript compiles
  //ZilchScriptManager* manager = ZilchScriptManager::GetInstance();
  //manager->mDebugger.Host(8000);
}

void RunZilchDebugger()
{
  HostZilchDebugger();
  String debuggerPath = FilePath::Combine(Z::gContentSystem->ToolPath, "ZilchDebugger.html");
  Os::SystemOpenFile(debuggerPath.c_str());
}


void BindAppCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("Controls", BindCommandFunction(ShowControls));
  commands->AddCommand("About", BindCommandFunction(ShowAbout));

  commands->AddCommand("Exit", BindCommandFunction(ExitEditor));
  commands->AddCommand("ToggleFullScreen", BindCommandFunction(FullScreen));
  commands->AddCommand("Restart", BindCommandFunction(Restart));

  commands->AddCommand("BuildVersion", BindCommandFunction(BuildVersion));
  commands->AddCommand("WriteBuildInfo", BindCommandFunction(WriteBuildInfo));
  commands->AddCommand("RunUnitTests", BindCommandFunction(RunUnitTests));

  if(DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig))
  {
    commands->AddCommand("OpenTestWidgets", BindCommandFunction(OpenTestWidgetsCommand));
    commands->AddCommand("CrashEngine", BindCommandFunction(CrashEngine));
  }
  // These commands functionality does not work at the moment and can result in an engine hang
  //commands->AddCommand("HostZilchDebugger", BindCommandFunction(HostZilchDebugger));
  //commands->AddCommand("RunZilchDebugger", BindCommandFunction(RunZilchDebugger));
  commands->AddCommand("Help", BindCommandFunction(OpenHelp));
}

}
