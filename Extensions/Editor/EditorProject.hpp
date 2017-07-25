///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(LauncherDebuggerCommunicationCompleted);
DeclareEvent(LauncherDebuggerCommunicationFailed);
}// namespace Events


class Cog;
class CommandManager;
class Editor;
class LauncherConfig;
class LauncherCommunicationEvent;
class TcpSocket;

void BindProjectCommands(Cog* config, CommandManager* commands);
void OpenProjectFile(StringParam filename);

void NewProject();
void OpenProject();

void UnloadProject(Editor* editor, Cog* projectCog);
void LoadProject(Editor* editor, Cog* projectCog, StringParam path, StringParam projectFile);

/// This composite takes care of communicating with the launcher for the purposes of the
/// open/new projects dialog. Since this takes place via a tcp connection this can take
/// some time for a response which this composite helps to deal with.
class LauncherOpenProjectComposite : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  LauncherOpenProjectComposite(Composite* parent);
  ~LauncherOpenProjectComposite();

  /// Sends the given event name to the launcher
  void SendEvent(StringParam eventType);

  void FailedToOpenLauncher();
  bool RunLauncherExe(StringParam exePath);
  bool RunFromInstalledPath();
  void CommunicateWithLauncher();
  
  void OnConnectionCompleted(Event* e);
  void OnConnectionFailed(Event* e);

  TcpSocket* mSocket;
  Cog* mLauncherConfig;
  LauncherConfig* mVersionConfig;
  String mEventType;
};

/// Class that takes care of communicate from one launcher to another during startup.
/// As the launcher behaves as a singleton, command-line arguments need to be passed
/// from the newly opened launcher to the currently running launcher.
class LauncherSingletonCommunication : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  LauncherSingletonCommunication(const StringMap& arguments);
  ~LauncherSingletonCommunication();
  
  void OnConnectionCompleted(Event* e);
  void OnConnectionFailed(Event* e);

  size_t mTimesTryingToConnect;
  TcpSocket* mSocket;
  StringMap mArguments;
  /// The resulting status of the connection attempt
  Status mStatus;
};

// A simple class to send an event from the launcher to the editor to tell it to open a project.
// The tcp socket needs to exist for a bit (and we need callback functions) so this is
// created to send an event after a connection is established.
class LauncherDebuggerCommunication : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  LauncherDebuggerCommunication();
  ~LauncherDebuggerCommunication();

  void SendOpenProject(StringParam projectFile);
  void SendOpenProject(StringParam projectFile, int port);

  void OnConnectionCompleted(Event* e);
  void OnConnectionFailed(Event* e);

  TcpSocket* mSocket;
  String mProjectFile;
};

// A simple class to open a tcp socket and listen for any communication from the launcher
// (so we can have the launcher tell us to open a file while we have the debugger attached)
class SimpleDebuggerListener : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SimpleDebuggerListener();
  ~SimpleDebuggerListener();

  void OnLauncherOpenProject(LauncherCommunicationEvent* e);
  
  TcpSocket* mListener;
};

}//namespace Zero
