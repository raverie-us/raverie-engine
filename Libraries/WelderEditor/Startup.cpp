// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

OsShell* CreateOsShellSystem();
System* CreateSoundSystem();
System* CreateGraphicsSystem();
System* CreatePhysicsSystem();
System* CreateTimeSystem();

void CreateEditor(Cog* config, StringParam project, StringParam newProjectName);
void CreateGame(Cog* config, Cog* projectCog, StringParam projectFile);

bool ZeroEditorStartup()
{
  String projectFile = Environment::GetValue<String>("file");
  bool playGame = Environment::GetValue<bool>("play", false);

  String newProject =
      Environment::GetValue<String>("newProject");

  // Check to see if there was a project file in the same directory.
  static const String cDefaultProjectFile("Project.zeroproj");
  if (FileExists(cDefaultProjectFile))
  {
    projectFile = cDefaultProjectFile;
    playGame = true;
  }

  // Load config object
  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  EditorConfig* editorConfig = configCog->has(EditorConfig);

  // If there was no specified project file (or it doesn't exist) and we're not
  // creating a new project, then use a fall-back project that we open from our
  // data directory. This project should be read-only, but is useful for testing
  // platforms before the full launcher pipeline is implemented. Note that if
  // the 'projectFile' does not exist, but is specified, we will not use the
  // fall-back.
  if (mainConfig && projectFile.Empty() && newProject.Empty() &&
      (editorConfig == nullptr || editorConfig->EditingProject.Empty()))
    projectFile = FilePath::Combine(
        mainConfig->DataDirectory, "Fallback", "Fallback.zeroproj");

  // Initialize Extensions
  ZPrint("Loading Extensions\n");

  if (playGame)
  {
    // Creating project cog outside of CreateGame() so that eula window can make
    // changes if needed. Note: We don't currently show a eula window so this
    // may need to be refactored.
    Cog* projectCog = Z::gFactory->Create(
        Z::gEngine->GetEngineSpace(), projectFile, 0, nullptr);
    if (projectCog == nullptr)
    {
      FatalEngineError("Failed load project '%s'", projectFile.c_str());
      return false;
    }

    CreateGame(configCog, projectCog, projectFile);
  }
  else
  {
    CreateEditor(configCog, projectFile, newProject);
  }


  return true;
}

} // namespace Zero
