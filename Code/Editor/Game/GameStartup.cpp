// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
void CreateGame(OsWindow* mainWindow, StringParam projectFile, Cog* projectCog);
void LoadGamePackages(StringParam projectFile, Cog* projectCog);

void GameStartup::UserInitializeConfig(Cog* configCog)
{
  HasOrAdd<EditorSettings>(configCog);
  HasOrAdd<ContentConfig>(configCog);
  HasOrAdd<TextEditorConfig>(configCog);
}

void GameStartup::UserInitialize()
{
  String projectFile = Environment::GetValue<String>("file");

  // Check to see if there was a project file in the same directory.
  static const String cDefaultProjectFile("Project.zeroproj");
  if (FileExists(cDefaultProjectFile))
  {
    projectFile = cDefaultProjectFile;
  }

  // The options defaults are already tailored to the Editor.
  // If we're playing the game, we need to load the project Cog.
  // We'll also potentially derive some window settings from the project.
  Cog* projectCog = nullptr;
  projectCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), projectFile, 0, nullptr);
  if (projectCog == nullptr)
  {
    FatalEngineError("Failed load project '%s'", projectFile.c_str());
    return Exit(1);
  }

  // Since we don't create a resiziable wigdet/close button, etc.
  // for the game, then we want the typical OS border to appear.
  mWindowStyle = (WindowStyleFlags::Enum)(mWindowStyle & ~WindowStyleFlags::ClientOnly);

  mLoadContent = false;
  mUseSplashScreen = true;
  mWindowSettingsFromProjectCog = projectCog;

  mProjectCog = projectCog;
  mProjectFile = projectFile;
}

void GameStartup::UserStartup()
{
  LoadGamePackages(mProjectFile, mProjectCog);
}

void GameStartup::UserCreation()
{
  CreateGame(mMainWindow, mProjectFile, mProjectCog);
}

} // namespace Zero
