// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Platform/PlatformCommunication.hpp"

namespace Raverie
{

void LoadProject(Editor* editor, Cog* projectCog, StringParam path, StringParam projectFile)
{
  ZPrint("Loading project '%s'\n", projectFile.c_str());

  if (editor->mProject)
    UnloadProject(editor, editor->mProject);

  Z::gEditor->mProject = projectCog;

  HasOrAdd<EditorConfig>(Z::gEditor->mConfig)->EditingProject = projectFile;

  // Save what project was opened
  SaveConfig();

  ProjectSettings* project = projectCog->has(ProjectSettings);
  project->ContentFolder = FilePath::Combine(path, "Content");
  project->EditorContentFolder = FilePath::Combine(path, "EditorContent");
  project->ProjectFolder = path;
  project->ProjectFile = projectFile;

  // Set the Platform Title Bar.
  editor->mMainWindow->SetTitle(BuildString(GetOrganization(), " ", GetApplicationName(), " - ", project->ProjectName));

  // Set the store name based on the project name.
  ObjectStore::GetInstance()->SetStoreName(project->ProjectName);

  String projectFolder = project->ProjectFolder;

  // Load shared content libraries if present
  if (SharedContent* sharedConent = projectCog->has(SharedContent))
  {
    forRange (ContentLibraryReference libraryRef, sharedConent->ExtraContentLibraries.All())
    {
      String libraryName = libraryRef.mContentLibraryName;
      String contentFolder = FilePath::Combine(projectFolder, libraryName);
      Status loadContentLibrary;
      ContentLibrary* contentLibrary = Z::gContentSystem->LibraryFromDirectory(loadContentLibrary, libraryName, contentFolder);
      if (contentLibrary)
      {
        Status status;
        Z::gContentSystem->BuildLibrary(status, contentLibrary, true);
      }
      else
      {
        DoNotifyWarning("Missing Library", String::Format("Failed to find shared content library %s", libraryName.c_str()));
      }
    }
  }

  EditorSettings* engineEditorSettings = HasOrAdd<EditorSettings>(Z::gEditor->mConfig);

  // Load content package of project
  Status loadContentLibrary;
  ContentLibrary* projectLibrary = Z::gContentSystem->LibraryFromDirectory(loadContentLibrary, project->ProjectName, project->ContentFolder);

  /// Store the library on the project
  project->ProjectContentLibrary = projectLibrary;

  Status status;
  Z::gContentSystem->BuildLibrary(status, projectLibrary, true);

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
  if (ResourceLibrary* projectLibrary = project->ProjectResourceLibrary)
    projectLibrary->Unload();

  forRange (ResourceLibrary* library, project->SharedResourceLibraries.All())
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

bool OpenProjectFile(StringParam filename)
{
  // File check
  if (!FileExists(filename))
  {
    DoNotifyError("Failed to load project.", String::Format("Project file not found '%s'", filename.c_str()));
    return false;
  }

  // Load the project object
  Cog* projectCog = Z::gFactory->Create(Z::gEngine->GetEngineSpace(), filename, CreationFlags::ProxyComponentsExpected, nullptr);
  if (projectCog == nullptr)
  {
    DoNotifyError("Failed to load project.", "Project file invalid.");
    return false;
  }
  // Prevent components from being added or removed from the project cog
  projectCog->mFlags.SetFlag(CogFlags::ScriptComponentsLocked);

  ProjectSettings* project = projectCog->has(ProjectSettings);
  if (project == nullptr)
    return false;

  // Begin the loading project
  String projectFolder = FilePath::GetDirectoryPath(filename);
  LoadProject(Z::gEditor, projectCog, projectFolder, filename);
  return true;
}

// Project Commands

void OpenProject()
{
  ImportOpenUrl(Urls::cOpenProject);
}

void ReloadProject()
{
  Cog* projectCog = Z::gEditor->mProject;
  if (projectCog == nullptr)
    return;

  ProjectSettings* project = projectCog->has(ProjectSettings);
  OpenProjectFile(project->ProjectFile);
}

void NewProject()
{
  ImportOpenUrl(Urls::cNewProject);
}

void SaveProject()
{
  Editor* editor = Z::gEditor;
  // Take a screen shot if it's enabled
  if (Cog* projectCog = editor->mProject)
  {
    ProjectSettings* project = projectCog->has(ProjectSettings);
    if (project->AutoTakeProjectScreenshot)
      editor->TakeProjectScreenshot();
  }

  editor->SaveAll(true, true);
}

void BindProjectCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("OpenProject", BindCommandFunction(OpenProject));
  commands->AddCommand("ReloadProject", BindCommandFunction(ReloadProject));
  commands->AddCommand("NewProject", BindCommandFunction(NewProject));
  commands->AddCommand("SaveProject", BindCommandFunction(SaveProject));
}

} // namespace Raverie
