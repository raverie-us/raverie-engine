// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void ArchiveProjectFile(Cog* projectCog, StringParam filename)
{
  ProjectSettings* project = projectCog->has(ProjectSettings);
  String projectDirectory = project->ProjectFolder;

  Status status;
  Archive projectArchive(ArchiveMode::Compressing);
  projectArchive.ArchiveDirectory(status, projectDirectory);
  projectArchive.WriteZipFile(filename);
  Download(filename);
}

class ArchiveProjectJob : public Job
{
public:
  String mFileName;
  Cog* mProject;

  void Execute()
  {
    SendBlockingTaskStart("Archiving");

    ProjectSettings* project = mProject->has(ProjectSettings);

    ArchiveProjectFile(mProject, mFileName);

    SendBlockingTaskFinish();
  }
};

void StartArchiveJob(StringParam filename)
{
  Cog* projectCog = Z::gEditor->mProject;
  Z::gEditor->SaveAll(true);

  ArchiveProjectJob* job = new ArchiveProjectJob();
  job->mProject = projectCog;
  job->mFileName = filename;
  Z::gJobs->AddJob(job);
}

void BackupProject(ProjectSettings* project)
{
  String backupDirectory = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "Backups");
  CreateDirectoryAndParents(backupDirectory);
  String timeStamp = GetTimeAndDateStamp();
  String fileName = BuildString(project->ProjectName, timeStamp, ".zip");
  String fullPath = FilePath::Combine(backupDirectory, fileName);
  StartArchiveJob(fullPath);
}

struct ArchiveProjectCallback : public SafeId32EventObject
{
  typedef ArchiveProjectCallback ZilchSelf;

  ArchiveProjectCallback()
  {
    const String CallBackEvent = "ArchiveCallback";
    ProjectSettings* project = Z::gEditor->mProject.has(ProjectSettings);

    FileDialogConfig* config = FileDialogConfig::Create();
    config->EventName = CallBackEvent;
    config->CallbackObject = this;
    config->Title = "Archive a project";
    config->AddFilter("Zip", "*.zip");
    config->DefaultFileName = BuildString(project->ProjectName, ".zip");
    config->mDefaultSaveExtension = "zip";

    ConnectThisTo(this, CallBackEvent, OnArchiveProjectFile);
    Z::gEngine->has(OsShell)->SaveFile(config);
  }

  void OnArchiveProjectFile(OsFileSelection* event)
  {
    if (event->Success)
      StartArchiveJob(event->Files[0]);
    delete this;
  }
};

void ArchiveProject(ProjectSettings* project)
{
  new ArchiveProjectCallback();
}

void ExportGame(ProjectSettings* project)
{
  Exporter* exporter = Exporter::GetInstance();
  exporter->ExportGameProject(project->mOwner);
}

void ExportContent(ProjectSettings* project)
{
  // Save all resources and build them so the
  // output directory is up to date
  Editor* editor = Z::gEditor;
  editor->SaveAll(true);

  Exporter* exporter = Exporter::GetInstance();
  exporter->mProjectCog = project->GetOwner();
  exporter->ExportContent(exporter->mDefaultTargets);
}

void ShowProjectFolder(ProjectSettings* project)
{
  Os::ShellOpenDirectory(project->ProjectFolder);
}

void ShowContentOutput(ProjectSettings* project)
{
  String outputPath = project->ProjectContentLibrary->GetOutputPath();
  Os::ShellOpenDirectory(outputPath);
}

void ExportAndPlayGame(ProjectSettings* project)
{
  Exporter* exporter = Exporter::GetInstance();
  exporter->ExportAndPlay(project->mOwner);
}

void BindArchiveCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("ArchiveProject", BindCommandFunction(ArchiveProject));
  commands->AddCommand("BackupProject", BindCommandFunction(BackupProject));

  commands->AddCommand("ExportGame", BindCommandFunction(ExportGame));
  commands->AddCommand("ExportAndPlayGame", BindCommandFunction(ExportAndPlayGame));
  commands->AddCommand("ExportContent", BindCommandFunction(ExportContent));

  commands->AddCommand("ShowProjectFolder", BindCommandFunction(ShowProjectFolder), true);
  commands->AddCommand("ShowContentOutput", BindCommandFunction(ShowContentOutput), true);
}

} // namespace Zero
