///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

//#include "Networking/SendableEvent.hpp"

namespace Zero
{

namespace Events
{
  DeclareEvent(LauncherNewProject);
  DeclareEvent(LauncherOpenProject);
  DeclareEvent(LauncherRunProject);
  DeclareEvent(LauncherOpenRecentProjects);
  DeclareEvent(LauncherRunCommand);
  DeclareEvent(LauncherUpdateTags);
  DeclareEvent(LauncherOpenTemplate);
  DeclareEvent(LauncherInstallProject);
  DeclareEvent(LauncherOpenBuild);
}

/// Event used for the engine to communicate with the launcher (to tell it to open
/// recent projects and so on). Also used for the launcher to communicate with itself
/// as the launcher is a singleton application but needs to send any command-line arguments
/// to the current running instance.
class LauncherCommunicationEvent : public SendableEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream);
  void LoadFromCommandArguments(StringMap& arguments);

  /// The port for the launcher to listen on
  const static int DesiredPort = 25033;
  const static int DebuggerDesiredPort = 25034;

  /// Any extra data we want to send
  String mExtraData;
  /// The project file to operate some commands on (such as open or run)
  String mProjectFile;
  String mTags;
};

}//namespace Zero
