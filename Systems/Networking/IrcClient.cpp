///////////////////////////////////////////////////////////////////////////////
///
/// \file IrcClient.cpp
/// Implementation of the IrcClient class.
///
/// Authors: Trevor Sundberg.
/// Copyright 2010-2011, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace
{
  Math::Random gRandom;
}

namespace Events
{
  DefineEvent(ChatMessageOfTheDay);
  DefineEvent(ChatReady);
  DefineEvent(ChatChannelEvent);
  DefineEvent(ChatUserEvent);
  DefineEvent(ChatMessage);
  DefineEvent(ChatNotice);
  DefineEvent(ChatNews);
  DefineEvent(ChatNameChange);
  DefineEvent(ChatChannelNames);
}

ZilchDefineType(IrcClient, builder, type)
{
}

// Constructor
IrcClient::IrcClient() :
  mSocket("Irc")
{
  // We aren't ready until we connect and get the MOTD
  mReady = false;

  // Connect up events
  ConnectThisTo(&mSocket, Events::ReceivedData, ReceivedData);
  ConnectThisTo(&mSocket, Events::ConnectionCompleted, ConnectionCompleted);

  ConnectThisTo(&mSocket, Events::ConnectionFailed, ConnectionFailedOrDisconnected);
  ConnectThisTo(&mSocket, Events::Disconnected, ConnectionFailedOrDisconnected);
  ConnectThisTo(&mSocket, Events::SocketError, SocketError);
}

// Destructor
IrcClient::~IrcClient()
{
  // If we were ready...
  if (mReady == true)
  {
    // Disconnect from the chat gracefully
    Disconnect();
  }

  auto requests = mNameRequests.Values();

  while (!requests.Empty())
  {
    auto request = requests.Front();
    requests.PopFront();

    delete request;
  }
}

// Send data over the network
void IrcClient::Send(const char* format, ...)
{
  // If we're not connected, then don't do anything...
  if (mSocket.IsConnected() == false)
    return;

  // Start a variable argument list
  va_list va;
  va_start(va, format);

  // Create a string from the given format 
  String data;
  data = String::FormatArgs(format, va);

  // Send it over the socket
  mSocket.SendBufferToAll((const byte*)data.c_str(), data.SizeInBytes());

  // End the world
  va_end(va);
}

// Handle any commands that come in
void IrcClient::HandleCommand(const CommandInfo& info)
{
  // Taken nickname
  if (info.Command == "433")
  {
    mName = BuildString(mName, ToString(gRandom.Next()));
    Send("NICK %s\r\n", mName.c_str());
  }
  // MOTD
  else if (info.Command == "372")
  {
    // Send out an event that the chat client is ready
    TextEvent e(info.Data.Content);
    mDispatcher.Dispatch(Events::ChatMessageOfTheDay, &e);
  }
  // End of the MOTD
  else if (info.Command == "376")
  {
    // We should be ready to send now!
    mReady = true;

    // Send out an event that the chat client is ready
    Event event;
    mDispatcher.Dispatch(Events::ChatReady, &event);
  }
  // News flash?
  else if (info.Command == "343")
  {
    // Generally there's an extra character we should pop off in the beginning
    StringRange range = info.Data.Content.All();
    range.PopFront();

    // Dispatch a message
    TextEvent e(range);
    mDispatcher.Dispatch(Events::ChatNews, &e);
  }
  // On a message
  else if (info.Command == "PRIVMSG")
  {
    // Dispatch a message
    ChatEvent e(info.FromIdent, info.Data.Arguments[0], info.Data.Content);
    mDispatcher.Dispatch(Events::ChatMessage, &e);
  }
  // On a notice
  else if (info.Command == "NOTICE")
  {
    // Dispatch a message
    ChatEvent e(info.FromIdent, info.Data.Arguments[0], info.Data.Content);
    mDispatcher.Dispatch(Events::ChatNotice, &e);
  }
  // On a nick change
  else if (info.Command == "NICK")
  {
    // Dispatch a message
    NameChangeEvent e(info.FromIdent, info.Data.Content);
    mDispatcher.Dispatch(Events::ChatNameChange, &e);
  }
  // On a join
  else if (info.Command == "JOIN")
  {
    // If the user that's joining is us...
    if (info.FromIdent == mName)
    {
      // Dispatch a message
      ChannelEvent e(ChannelInfo::Joined, info.Data.Content);
      mDispatcher.Dispatch(Events::ChatChannelEvent, &e);
    }
    else
    {
      // Dispatch a message
      UserEvent e(ChannelInfo::Joined, info.Data.Content, info.FromIdent, true);
      mDispatcher.Dispatch(Events::ChatUserEvent, &e);
    }
  }
  // On a part
  else if (info.Command == "PART")
  {
    // If the user that's parting is us...
    if (info.FromIdent == mName)
    {
      // Dispatch a message
      ChannelEvent e(ChannelInfo::Parted, info.Data.Content);
      mDispatcher.Dispatch(Events::ChatChannelEvent, &e);
    }
    else
    {
      // Dispatch a message
      UserEvent e(ChannelInfo::Parted, info.Data.Content, info.FromIdent, true);
      mDispatcher.Dispatch(Events::ChatUserEvent, &e);
    }
  }
  // On a quit
  else if (info.Command == "QUIT")
  {
    // Dispatch a message
    UserEvent e(ChannelInfo::Parted, Irc::AllChannels, info.FromIdent, true);
    mDispatcher.Dispatch(Events::ChatUserEvent, &e);
  }
  // On a join (NAMES)
  else if (info.Command == "353")
  {
    // Create a tokenizer
    StringTokenRange tokens(info.Data.Content.c_str(), ' ');

    auto channel = info.Data.Arguments[2];

    auto request = mNameRequests.FindValue(channel, nullptr);

    // Loop through all the users in the list
    while (tokens.Empty() == false)
    {
      // Get the user's name
      String name(tokens.Front());

      // If we have a pending name request...
      if (request != nullptr)
      {
        request->Names.PushBack(name);
      }

      // Dispatch a message
      UserEvent e(ChannelInfo::Joined, channel, name, false);
      mDispatcher.Dispatch(Events::ChatUserEvent, &e);

      // Pop the token and move to the next one
      tokens.PopFront();
    }
  }
  // End of NAMES
  else if (info.Command == "366")
  {
    auto channel = info.Data.Arguments[1];

    auto request = mNameRequests.FindValue(channel, nullptr);
    
    // If we have a pending name request...
    if (request != nullptr)
    {
      // Send out an event that the chat client is ready
      mDispatcher.Dispatch(Events::ChatChannelNames, request);
      mNameRequests.Erase(channel);
      delete request;
    }
  }
}

void IrcClient::Start(StringParam server, unsigned short port, StringParam name)
{
  // Store our name
  mName = name;

  // Connect to the server
  mSocket.Connect(server, port);
}

void IrcClient::Disconnect()
{
  // Send a quit message and stop the socket
  Quit("User disconnected");
  mSocket.Close();

  // We're no longer ready
  mReady = false;
}

String FilterNewLines(StringRange text)
{
  StringBuilder builder;
  for (;!text.Empty(); text.PopFront())
  {
    Rune r = text.Front();

    if (r == '\r' || r == '\n')
    {
      builder.Append(' ');
    }
    else
    {
      builder.Append(r);
    }
  }

  return builder.ToString();
}

void IrcClient::Message(StringParam target, StringParam message)
{
  String filteredMessage = FilterNewLines(message);
  Send("PRIVMSG %s :%s\r\n", target.c_str(), filteredMessage.c_str());
}

void IrcClient::Notice(StringParam target, StringParam message)
{
  String filteredMessage = FilterNewLines(message);
  Send("NOTICE %s :%s\r\n", target.c_str(), filteredMessage.c_str());
}

void IrcClient::Join(StringParam channel)
{
  Send("JOIN %s\r\n", channel.c_str());
}

void IrcClient::Part(StringParam channel)
{
  Send("PART %s\r\n", channel.c_str());
}

void IrcClient::Quit(StringParam message)
{
  String filteredMessage = FilterNewLines(message);
  Send("QUIT %s\r\n", filteredMessage.c_str());
}

void IrcClient::RequestChannelNames(StringParam channel)
{
  auto event = new ChannelNamesEvent();
  event->Channel = channel;
  mNameRequests.Insert(channel, event);
  Send("NAMES %s\r\n", channel.c_str());
}

bool IrcClient::IsConnected()
{
  return mReady;
}

// Get the name of the current user
String IrcClient::GetName()
{
  return mName;
}

void IrcClient::ConnectionFailedOrDisconnected(ConnectionEvent* event)
{
  // Forward the event
  mReady = false;
  this->DispatchEvent(event->EventId, event);
  DoNotifyWarning("Irc Client", "The connection failed or you were disconnected from the IRC server. Use the 'Chat' command to reconnect");
}

void IrcClient::SocketError(TextErrorEvent* event)
{
  // Forward the event
  this->DispatchEvent(event->EventId, event);
  DoNotifyWarning("Irc Client", BuildString("An error occurred: ", event->Text, ". If you were disconnected, use the 'Chat' command to reconnect"));
}

// Occurs when we connect to the remote server
void IrcClient::ConnectionCompleted(ConnectionEvent* event)
{
  // Forward the event
  this->DispatchEvent(event->EventId, event);
  // Send identification information
  Send("PASS ZeroEditor\r\n");
  Send("NICK %s\r\n", mName.c_str());
  Send("USER %s 0 * :%s\r\n", mName.c_str(), mName.c_str());
}

// Occurs when we receive data from the socket
void IrcClient::ReceivedData(ReceivedDataEvent* event)
{
  // A temporary object that represents a parsed incoming command
  CommandInfo command;

  //Null terminated the buffer
  event->Data.PushBack('\0');

  // Get the data stream
  const char* stream = (const char*)event->Data.Data();

  // Loop until we hit the NULL terminator
  while (stream != nullptr && (stream - (const char*)event->Data.Data()) < (long)event->Data.Size() && *stream != '\0')
  {
    // Read the first character
    if (*stream == ':')
    {
      // Go past the ':'
      ++stream;

      // Store a temporary pointer at the start of the host name
      const char* fromStart = stream;

      // Search for a space (which designates the end of the host name)
      stream = strchr(stream, ' ');

      // Exit out if the stream is NULL
      if (stream == nullptr)
      {
        Error("The stream operations failed in IRC packet parsing");
        return;
      }

      // Create a string that represents the host name
      command.From = String(fromStart, stream);

      // Now try and find a '!' inside the host name which designates that one part of the host is a nickname / identity
      const char* identEnd = strchr(fromStart, '!');

      // If we didn't find anything, just use the whole host as the identity
      if (identEnd == nullptr)
        identEnd = stream;

      // Create an identity string
      command.FromIdent = String(fromStart, identEnd);

      // Increment the stream past the space
      ++stream;

      // Store a temporary pointer from the start of the command
      const char* commandStart = stream;

      // Look for a space again which designates the end of a command
      stream = strchr(stream, ' ');

      // Exit out if the stream is NULL
      if (stream == nullptr)
      {
        Error("The stream operations failed in IRC packet parsing");
        return;
      }

      // Create a string to represent the command
      command.Command = String(commandStart, stream);
        
      // Increment the stream past the space
      ++stream;

      // Store a temporary pointer from the start of the command data
      const char* commandDataStart = stream;

      // Now read until we hit a newline!
      stream = strstr(stream, "\r\n");

      // Exit out if the stream is NULL
      if (stream == nullptr)
      {
        Error("The stream operations failed in IRC packet parsing");
        return;
      }

      // Create a string to represent the command data
      command.Data.All = String(commandDataStart, stream);

      // Move to the next packet
      stream += strlen("\r\n");

      // Now handle the command
      const char* dataStream = command.Data.All.c_str();

      // Get a pointer to the start of the content (past the ':')
      const char* content = strchr(dataStream, ':');

      // Clear out the arguments
      command.Data.Arguments.Clear();

      // If we didn't find a ':' for content...
      if (content == nullptr)
      {
        // The content is all the data!
        command.Data.Content = command.Data.All;
      }
      else
      {
        // Otherwise, create a string that only extends to just before the content
        String arguments = String(dataStream, content);

        // Increment the content pointer to go past the ':'
        ++content;

        // Create the content string (everything past the arguments)
        command.Data.Content = String(content, dataStream + command.Data.All.SizeInBytes());

        // Get a pointer to the first argument
        const char* currentArg = arguments.c_str();

        // While we didn't hit a NULL pointer
        while (*currentArg != '\0')
        {
          // Read to the end of the argument (there should always be a ' ' at the end of all the arguments)
          const char* currentArgEnd = strchr(currentArg, ' ');

          // If for some reason we didn't find a space at the end...
          if (currentArgEnd == nullptr)
          {
            // Break out since we failed to parse the arguments properly
            break;
          }

          // Push back a string argument
          command.Data.Arguments.PushBack(String(currentArg, currentArgEnd));

          // Move past the space and go to the next argument
          currentArg = currentArgEnd + 1;
        }
      }

      // Now handle the command
      HandleCommand(command);
    }
    // If we got a ping
    else if (stream[0] == 'P' && stream[1] == 'I' && stream[2] == 'N' && stream[3] == 'G')
    {
      // Create the pong reply and send it out (we just reply with the data we were given)
      String reply(strchr(stream, ':'), strchr(stream, '\n') + 1);
      Send("PONG %s", reply.c_str());

      // Now read until we hit a newline!
      stream = strstr(stream, "\r\n");

      // Exit out if the stream is NULL
      if (stream == nullptr)
      {
        Error("The stream operations failed in IRC packet parsing");
        return;
      }

      // Move to the next packet
      stream += strlen("\r\n");
    }
    // We didn't start with a ':'...
    else
    {
      // Display a warning that this was unhandled
      Warn("Unhandled packet");

      // Now read until we hit a newline!
      stream = strstr(stream, "\r\n");

      // If we did actually move to the end...
      if (stream != nullptr)
      {
        // Move to the next packet
        stream = strchr(stream, ':');
      }
    }
  }
}

}
