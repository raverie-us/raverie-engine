// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void CreateEditor(OsWindow* mainWindow, StringParam projectFile, StringParam newProjectName);
void CreateGame(OsWindow* mainWindow, StringParam projectFile, Cog* projectCog);
void LoadGamePackages(StringParam projectFile, Cog* projectCog);

void GameOrEditorStartup::UserInitializeConfig(Cog* configCog)
{
  HasOrAdd<EditorSettings>(configCog);
  HasOrAdd<ContentConfig>(configCog);
  HasOrAdd<TextEditorConfig>(configCog);
}

void GameOrEditorStartup::UserInitialize()
{
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
  // platforms before the full pipeline is implemented. Note that if the
  // 'projectFile' does not exist, but is specified, we will not use the fall-back.
  Cog* configCog = Z::gEngine->GetConfigCog();
  MainConfig* mainConfig = configCog->has(MainConfig);
  EditorConfig* editorConfig = configCog->has(EditorConfig);
  if (mainConfig && projectFile.Empty() && newProject.Empty() &&
      (editorConfig == nullptr || editorConfig->EditingProject.Empty()))
  {
    projectFile = FilePath::Combine(mainConfig->DataDirectory, "Fallback", "Fallback.zeroproj");
  }

  // The options defaults are already tailored to the Editor.
  // If we're playing the game, we need to load the project Cog.
  // We'll also potentially derive some window settings from the project.
  Cog* projectCog = nullptr;
  if (playGame)
  {
    projectCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), projectFile, 0, nullptr);
    if (projectCog == nullptr)
    {
      FatalEngineError("Failed load project '%s'", projectFile.c_str());
      return Exit(1);
    }

    // Since we don't create a resiziable wigdet/close button, etc.
    // for the game, then we want the typical OS border to appear.
    mWindowStyle = (WindowStyleFlags::Enum)(mWindowStyle & ~WindowStyleFlags::ClientOnly);
  }

  mLoadContent = !playGame;
  mUseSplashScreen = playGame;
  mWindowSettingsFromProjectCog = projectCog;

  mPlayGame = playGame;
  mProjectCog = projectCog;
  mProjectFile = projectFile;
  mNewProject = newProject;
}

void GameOrEditorStartup::UserStartup()
{
  if (mPlayGame)
  {
    LoadGamePackages(mProjectFile, mProjectCog);
  }
  else
  {
    Array<String> coreLibs;
    coreLibs.PushBack("ZeroCore");
    coreLibs.PushBack("UiWidget");
    coreLibs.PushBack("EditorUi");
    coreLibs.PushBack("Editor");
    LoadCoreContent(coreLibs);
  }

  String cloneUrl = Environment::GetValue<String>("cloneUrl");
  if (!cloneUrl.Empty() && !FileExists(mProjectFile))
  {
    GitCloneJob* job = new GitCloneJob();
    job->mUrl = cloneUrl;
    job->mDirectory = FilePath::GetDirectoryPath(mProjectFile);
    ZPrint("Cloning url '%s' to directory '%s'\n", job->mUrl.c_str(), job->mDirectory.c_str());
    Z::gJobs->AddJob(job);
  }
}

void GameOrEditorStartup::UserCreation()
{
  if (mPlayGame)
  {
    CreateGame(mMainWindow, mProjectFile, mProjectCog);
  }
  else
  {
    CreateEditor(mMainWindow, mProjectFile, mNewProject);
  }
}

} // namespace Zero
