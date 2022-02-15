// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void OpenHelp()
{
  Os::OpenUrl(Urls::cUserHelp);
}

void OpenZeroHub()
{
  Os::OpenUrl(Urls::cUserZeroHub);
}

void OpenDocumentation()
{
  Os::OpenUrl(Urls::cUserOnlineDocs);
}

void ExitEditor()
{
  if (Z::gWidgetManager->RootWidgets.Empty())
    return;

  OsWindow* mainWindow = Z::gWidgetManager->RootWidgets.Begin()->GetOsWindow();
  OsWindowEvent event;
  mainWindow->DispatchEvent(Events::OsClose, &event);
}

void Restart()
{
  bool quitSuccess = Z::gEditor->RequestQuit(true);

  if (quitSuccess)
    Os::ShellOpenApplication(GetApplication());
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
  String text = GetApplicationName() + "\n" + GetBuildVersionName() + "\n";
  ShowTextWindow("About", text);
}

DeclareEnum3(VersionStatus, Unknown, UpToDate, NewVersionAvailable);

VersionStatus::Type GlobalVersionStatus = VersionStatus::Unknown;

void BuildVersion()
{
  String buildVersionString = String::Format("BuildVersion: %s", GetBuildVersionName().c_str());
  ZPrintFilter(Filter::DefaultFilter, "%s\n", buildVersionString.c_str());
  OsShell* platform = Z::gEngine->has(OsShell);
}

void WriteBuildInfo()
{
  String sourceDir = Z::gEngine->GetConfigCog()->has(MainConfig)->SourceDirectory;
  Environment* environment = Environment::GetInstance();
  String filePath = environment->GetParsedArgument("WriteBuildInfo");
  if (filePath.Empty() || filePath == "true")
    filePath = FilePath::CombineWithExtension(sourceDir, "Meta", ".data");

  StringBuilder builder;
  builder.AppendFormat("MajorVersion %d\n", GetMajorVersion());
  builder.AppendFormat("MinorVersion %d\n", GetMinorVersion());
  builder.AppendFormat("PatchVersion %d\n", GetPatchVersion());
  builder.AppendFormat("RevisionId %d\n", GetRevisionNumber());
  builder.AppendFormat("Platform \"%s\"\n", GetPlatformString());
  builder.AppendFormat("ShortChangeSet \"%s\"\n", GetShortChangeSetString());
  builder.AppendFormat("ChangeSet \"%s\"\n", GetChangeSetString());
  builder.AppendFormat("ChangeSetDate \"%s\"\n", GetChangeSetDateString());
  builder.AppendFormat("BuildId \"%s\"\n", GetBuildIdString().c_str());

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
  forRange (String& name, names.All())
  {
    builder.Append("  ");
    builder.Append(name);
    builder.Append("\n");
  }
  builder.Append("\n");
}

// Special class to delay dispatching the unit test command until scripts have
// been compiled
class UnitTestDelayRunner : public Composite
{
public:
  typedef UnitTestDelayRunner ZilchSelf;

  void OnScriptsCompiled(Event* e)
  {
    ActionSequence* seq = new ActionSequence(Z::gEditor, ActionExecuteMode::FrameUpdate);
    // wait a little bit so we the scripts will be finished compiling (and
    // hooked up)
    seq->Add(new ActionDelay(0.1f));
    seq->Add(new ActionEvent(Z::gEngine, "RunUnitTests", new Event()));

    // when the game plays the scripts will be compiled again so don't dispatch
    // this event
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

void CopyPrebuiltContent()
{
  ZPrint("Copying prebuilt content...\n");
  // Save all resources and build them so the
  // output directory is up to date
  if (Z::gEditor)
    Z::gEditor->SaveAll(true);
  // We copy all libraries (including Fallback) because we only expect this to be run by the install steps
  const String outputDirectory = Z::gContentSystem->PrebuiltContentPath;
  forRange (ContentLibrary* library, Z::gContentSystem->Libraries.Values())
  {
    ZPrint("  Copying %s\n", library->Name.c_str());
    ExportUtility::CopyLibraryOut(outputDirectory, library, false);
  }
  Download(outputDirectory);
}

void BindAppCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("About", BindCommandFunction(ShowAbout), true);

  commands->AddCommand("Exit", BindCommandFunction(ExitEditor));
  commands->AddCommand("ToggleFullScreen", BindCommandFunction(FullScreen), true);
  commands->AddCommand("Restart", BindCommandFunction(Restart));

  commands->AddCommand("BuildVersion", BindCommandFunction(BuildVersion), true);
  commands->AddCommand("WriteBuildInfo", BindCommandFunction(WriteBuildInfo));
  commands->AddCommand("RunUnitTests", BindCommandFunction(RunUnitTests));
  commands->AddCommand("RecordUnitTestFile", BindCommandFunction(RecordUnitTestFile));
  commands->AddCommand("PlayUnitTestFile", BindCommandFunction(PlayUnitTestFile));

  if (DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig))
  {
    commands->AddCommand("OpenTestWidgets", BindCommandFunction(OpenTestWidgetsCommand));
    commands->AddCommand("CrashEngine", BindCommandFunction(CrashEngine));
  }
  commands->AddCommand("Help", BindCommandFunction(OpenHelp), true);
  commands->AddCommand("ZeroHub", BindCommandFunction(OpenZeroHub), true);
  commands->AddCommand("Documentation", BindCommandFunction(OpenDocumentation), true);

  commands->AddCommand("CopyPrebuiltContent", BindCommandFunction(CopyPrebuiltContent));
}

} // namespace Zero
