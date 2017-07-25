///////////////////////////////////////////////////////////////////////////////
///
/// \file IrcClient.hpp
/// Declaration of the IrcClient class.
///
/// Authors: Trevor Sundberg.
/// Copyright 2010-2011, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(ChatMessageOfTheDay);
  DeclareEvent(ChatReady);
  DeclareEvent(ChatChannelEvent);
  DeclareEvent(ChatUserEvent);
  DeclareEvent(ChatMessage);
  DeclareEvent(ChatNotice);
  DeclareEvent(ChatNews);
  DeclareEvent(ChatNameChange);
  DeclareEvent(ChatChannelNames);
}

DeclareEnum2(ChannelInfo, Joined, Parted);

struct ChannelEvent : public Event
{
  ChannelEvent(ChannelInfo::Enum info, StringParam channel) : Info(info), Channel(channel) {}
  ChannelInfo::Enum Info;
  String Channel;
};

struct UserEvent : public Event
{
  UserEvent(ChannelInfo::Enum info, StringParam channel, StringParam name, bool showMessage) : Info(info), Channel(channel), Name(name), ShowMessage(showMessage) {}
  ChannelInfo::Enum Info;
  String Channel;
  String Name;
  bool ShowMessage;
};

struct NameChangeEvent : public Event
{
  NameChangeEvent(StringParam from, StringParam to) : From(from), To(to) {}
  String From;
  String To;
};

struct ChatEvent : public Event
{
  ChatEvent(StringParam from, StringParam to, StringParam text) : From(from), To(to), Text(text) {}
  String From;
  String To;
  String Text;
};

struct ChannelNamesEvent : public Event
{
  String Channel;
  Array<String> Names;
};


// Forward declarations
class ReceivedDataEvent;
class ConnectionEvent;

// Type-defines
typedef void (*IRCSimpleCallback)(void* userData);
typedef void (*IRCDataCallback)(StringParam data, void* userData);

namespace Irc
{
  // Constants
  const String AllChannels = "~<ALL>~";
}

/// Manages all the client/server/peer connections .
class IrcClient : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Constructor
  IrcClient();

  // Destructor
  ~IrcClient();

  // Join a server and a port, and set our name
  void Start(StringParam server, unsigned short port, StringParam name);

  // Disconnect from the server
  void Disconnect();
  void Message(StringParam target, StringParam message);
  void Notice(StringParam target, StringParam message);
  void Join(StringParam channel);
  void Part(StringParam channel);
  void Quit(StringParam message);
  void RequestChannelNames(StringParam channel);

  // Get whether or not we are connected and able to send messages
  bool IsConnected();

  // Get the name of the current user
  String GetName();

private:

  // Occurs when we receive data from the socket
  void ReceivedData(ReceivedDataEvent* event);

  // Occurs when we connect to the remote server
  void ConnectionCompleted(ConnectionEvent* event);
  void ConnectionFailedOrDisconnected(ConnectionEvent* event);
  void SocketError(TextErrorEvent* event);

private:

  struct CommandData
  {
    Array<String> Arguments;
    String Content;
    String All;
  };

  struct CommandInfo
  {
    String From;
    String FromIdent;
    String Command;
    CommandData Data;
  };

  // Send data over the network
  void Send(const char* format, ...);

  // Handle any commands that come in
  void HandleCommand(const CommandInfo& command);

private:

  // Store the socket that we use
  TcpSocket mSocket;

  // Store my nickname
  String mName;

  // Are we ready?
  bool mReady;

  // Store all the name requests we make
  HashMap<String, ChannelNamesEvent*> mNameRequests;
};

} // namespace Zero
