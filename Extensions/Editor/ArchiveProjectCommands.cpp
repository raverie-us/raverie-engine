///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
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
}

void UploadToDevelopers(StringParam projectName, StringParam filename)
{
  BlockingWebRequest request;
  request.AddFile("userfile", filename);
  request.AddField("project_name", projectName);
  request.mUrl = "https://uploadproject.zeroengine.io";
  String response = request.Run();

  if(response == "success")
    DoNotify("Success", "Uploaded Project", "Disk");
  else
    DoNotifyError("Failed to upload", "Failed to upload project. Project is too large or no network access");

}

class ArchiveProjectJob : public Job
{
public:
  bool mUpload;
  String mFileName;
  Cog* mProject;

  int Execute()
  {
    SendBlockingTaskStart("Archiving");

    ProjectSettings* project = mProject->has(ProjectSettings);

    ArchiveProjectFile(mProject, mFileName);

    if(mUpload)
      UploadToDevelopers(project->ProjectName, mFileName);

    SendBlockingTaskFinish();

    return true;
  }
};

void StartArchiveJob(StringParam filename, bool upload = false)
{
  Cog* projectCog = Z::gEditor->mProject;
  Z::gEditor->SaveAll(true);

  ArchiveProjectJob* job = new ArchiveProjectJob();
  job->mProject = projectCog;
  job->mUpload = upload;
  job->mFileName = filename;
  Z::gJobs->AddJob(job);
}

void BackupProject(ProjectSettings* project)
{
  String backupDirectory = FilePath::Combine(GetUserDocumentsDirectory(), "ZeroProjects", "Backups");
  CreateDirectoryAndParents(backupDirectory);
  String timeStamp = GetTimeAndDateStamp();
  String fileName = BuildString(project->ProjectName, timeStamp, ".zip");
  String fullPath = FilePath::Combine(backupDirectory, fileName);
  StartArchiveJob(fullPath, false);
}

struct ArchiveProjectCallback : public EventObject
{
  typedef ArchiveProjectCallback ZilchSelf;

  ArchiveProjectCallback()
  {
    const String CallBackEvent = "ArchiveCallback"; 
    ProjectSettings* project = Z::gEditor->mProject.has(ProjectSettings);
    
    FileDialogConfig config;
    config.EventName = CallBackEvent;
    config.CallbackObject = this;
    config.Title = "Archive a project";
    config.AddFilter("Zip", "*.zip");
    config.DefaultFileName = BuildString(project->ProjectName, ".zip");
    config.mDefaultSaveExtension = "zip";

    ConnectThisTo(this, CallBackEvent, OnArchiveProjectFile);
    Z::gEngine->has(OsShell)->SaveFile(config);
  }

  void OnArchiveProjectFile(OsFileSelection* event)
  {
    if(event->Success)
      StartArchiveJob(event->Files[0]);
    delete this;
  }
};

void ArchiveProject(ProjectSettings* project)
{
  new ArchiveProjectCallback();
}

void BuildContent(ProjectSettings* project)
{
  //Build content for this project to make sure all files are up to date.
  Status status;
  ResourcePackage package;
  Z::gContentSystem->BuildLibrary(status, project->ProjectContentLibrary, package);
  DoNotifyStatus(status);
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
  BuildContent(project);

  Exporter* exporter = Exporter::GetInstance();
  exporter->mProjectCog = project->GetOwner();
  exporter->ExportContent(exporter->mDefaultTargets);
}

void ShowProjectFolder(ProjectSettings* project)
{
  Os::SystemOpenFile(project->ProjectFolder.c_str());
}

void ShowContentOutput(ProjectSettings* project)
{
  String outputPath = project->ProjectContentLibrary->GetOutputPath();
  Os::SystemOpenFile(outputPath.c_str());
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

  commands->AddCommand("ShowProjectFolder", BindCommandFunction(ShowProjectFolder));
  commands->AddCommand("ShowContentOutput", BindCommandFunction(ShowContentOutput));
}

}
