// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Zero;

namespace Zero
{

void CreateEditor(OsWindow* mainWindow, StringParam project, StringParam newProjectName);
void CreateGame(OsWindow* mainWindow, Cog* projectCog, StringParam projectFile);

} // namespace Zero

extern "C" int main(int argc, char* argv[])
{
  CommandLineToStringArray(gCommandLineArguments, argv, argc);

  SetupApplication(1, 0, 0, 1, sWelderOrganization, sEditorGuid, sEditorName);

  ZeroStartup startup;
  Engine* engine = startup.Initialize();

  String projectFile = Environment::GetValue<String>("file");
  bool playGame = Environment::GetValue<bool>("play", false);
  String newProject = Environment::GetValue<String>("newProject");

  // Check to see if there was a project file in the same directory.
  static const String cDefaultProjectFile("Project.zeroproj");
  if (FileExists(cDefaultProjectFile))
  {
    projectFile = cDefaultProjectFile;
    playGame = true;
  }

  // If there was no specified project file (or it doesn't exist) and we're not
  // creating a new project, then use a fall-back project that we open from our
  // data directory. This project should be read-only, but is useful for testing
  // platforms before the full launcher pipeline is implemented. Note that if
  // the 'projectFile' does not exist, but is specified, we will not use the
  // fall-back.
  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  EditorConfig* editorConfig = configCog->has(EditorConfig);
  if (mainConfig && projectFile.Empty() && newProject.Empty() &&
      (editorConfig == nullptr || editorConfig->EditingProject.Empty()))
  {
    projectFile = FilePath::Combine(mainConfig->DataDirectory, "Fallback", "Fallback.zeroproj");
  }

  // The options defaults are already tailored to the Editor.
  StartupOptions options;

  // If we're playing the game, we need to load the project Cog.
  // We'll also potentially derive some window settings from the project.
  Cog* projectCog = nullptr;
  if (playGame)
  {
    projectCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), projectFile, 0, nullptr);
    if (projectCog == nullptr)
    {
      FatalEngineError("Failed load project '%s'", projectFile.c_str());
      return 1;
    }

    // Since we don't create a resiziable wigdet/close button, etc.
    // for the game, then we want the typical OS border to appear.
    options.mWindowStyle = (WindowStyleFlags::Enum)(options.mWindowStyle & ~WindowStyleFlags::ClientOnly);
  }

  options.mLoadContent = !playGame;
  options.mUseSplashScreen = playGame;
  options.mWindowSettingsFromProjectCog = projectCog;
  OsWindow* mainWindow = startup.Startup(options);

  if (playGame)
  {
    CreateGame(mainWindow, projectCog, projectFile);
  }
  else
  {
    Array<String> coreLibs;
    coreLibs.PushBack("ZeroCore");
    coreLibs.PushBack("UiWidget");
    coreLibs.PushBack("EditorUi");
    coreLibs.PushBack("Editor");
    if (!LoadCoreContent(coreLibs))
      return 1;

    CreateEditor(mainWindow, projectFile, newProject);
  }

  // Run engine until termination
  engine->Run();

  startup.Shutdown();

  ZPrint("Terminated\n");

  return 0;
}
