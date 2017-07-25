///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(LauncherNewProject);
  DefineEvent(LauncherOpenProject);
  DefineEvent(LauncherRunProject);
  DefineEvent(LauncherOpenRecentProjects);
  DefineEvent(LauncherRunCommand);
  DefineEvent(LauncherUpdateTags);
  DefineEvent(LauncherOpenTemplate);
  DefineEvent(LauncherInstallProject);
  DefineEvent(LauncherOpenBuild);
}

ZilchDefineType(LauncherCommunicationEvent, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = false;
}

void LauncherCommunicationEvent::Serialize(Serializer& stream)
{
  SendableEvent::Serialize(stream);
  SerializeNameDefault(mExtraData, String());
  SerializeNameDefault(mProjectFile, String());
  SerializeNameDefault(mTags, String());
}

void LauncherCommunicationEvent::LoadFromCommandArguments(StringMap& arguments)
{
  String eventName;

  arguments.TryGetValue("file", mProjectFile);
  // We can have just a project name and no other command-line args and
  // that implies we are performing the open command
  if(!mProjectFile.Empty())
  {
    String extension = FilePath::GetExtension(mProjectFile);
    if(extension == "zeroproj")
      eventName = Events::LauncherOpenProject;
    else if(extension == "zerotemplate")
      eventName = Events::LauncherOpenTemplate;
    else if(extension == "zerobuild")
      eventName = Events::LauncherOpenBuild;
    else if(extension == "zeroprojpack")
      eventName = Events::LauncherInstallProject;
  }

  // This path needs to be turned into the full file path so since the project file
  // could be specified relative to the current working directory (it's just the command line arguments).
  // To do this, if the path is relative then we then we need to pre-pend the working directory.
  if(!mProjectFile.Empty() && !PathIsRooted(mProjectFile))
    mProjectFile = FilePath::Combine(GetWorkingDirectory(), mProjectFile);

  String tagsArg = LauncherStartupArguments::Names[LauncherStartupArguments::Tags];
  mTags = arguments.FindValue(tagsArg, String());
  if(eventName.Empty() && !mTags.Empty())
    eventName = Events::LauncherUpdateTags;

  //grab the command strings
  String newCommand = LauncherStartupArguments::Names[LauncherStartupArguments::New];
  String openCommand = LauncherStartupArguments::Names[LauncherStartupArguments::Open];
  String runCommand = LauncherStartupArguments::Names[LauncherStartupArguments::Run];
  String installAndRunCommand = LauncherStartupArguments::Names[LauncherStartupArguments::InstallAndRun];
  String projectsCommand = LauncherStartupArguments::Names[LauncherStartupArguments::Projects];
  String debuggerModeCommand = LauncherStartupArguments::Names[LauncherStartupArguments::DebuggerMode];

  if(arguments.FindPointer(newCommand) != nullptr)
    eventName = Events::LauncherNewProject;
  if(arguments.FindPointer(openCommand) != nullptr)
    eventName = Events::LauncherOpenProject;
  if(arguments.FindPointer(runCommand) != nullptr)
    eventName = Events::LauncherRunProject;
  if(arguments.FindPointer(projectsCommand) != nullptr)
    eventName = Events::LauncherOpenRecentProjects;

  mExtraData = arguments.FindValue(debuggerModeCommand, String());
  
  EventId = eventName;
}

}//namespace Zero
