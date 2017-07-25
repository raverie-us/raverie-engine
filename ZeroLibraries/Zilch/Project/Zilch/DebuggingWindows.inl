/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

// Undef windows defines that overlap with core functions
#undef CopyFile
#undef MoveFile
#undef DeleteFile
#undef CreateDirectory

namespace Zilch
{
  //***************************************************************************
  String StripQuotes(cstr text)
  {
    // Use this to build a quote removed message
    StringBuilder builder;

    // Remove quotes from message
    while ((*text) != '\0')
    {
      // Get the current character
      char c = *text;

      // If the character is a quote...
      if (c == '"')
      {
        // Append a single quote
        builder.Append('\'');
      }
      else
      {
        // Otherwise, Append the character
        builder.Append(*text);
      }

      // Increment our text iterator
      ++text;
    }

    // Output the message
    return builder.ToString();
  }

  //***************************************************************************
  bool DebugErrorHandler(ErrorSignaler::ErrorData& errorData)
  {
    // Stores the resulting quote removed message from below
    String message;
    String expression = StripQuotes(errorData.Expression);

    // Check if no message was provided
    if (errorData.Message != nullptr)
    {
      message = StripQuotes(errorData.Message);
    }
    else
    {
      message = "No message";
    }

    // Output the command line
    String commandLine = String::Format("ErrorDialog.exe \"%s\" \"%s\" \"%s:%d\" %s",
      message.c_str(), expression.c_str(), errorData.File, errorData.Line, "Default");

    // Create a structure to facilitating starting of a process
    STARTUPINFO startUpInfo;
    memset(&startUpInfo, 0, sizeof(startUpInfo));

    // Create another structure to store process information
    PROCESS_INFORMATION processInfo;
    memset(&processInfo, 0, sizeof(processInfo));

    // Start the child process.
    BOOL result = CreateProcess
    (
      NULL,                         // No module name (use command line)
      (LPTSTR)commandLine.c_str(),  // Command line
      NULL,                         // Process handle not inheritable
      NULL,                         // Thread handle not inheritable
      FALSE,                        // Set handle inheritance to FALSE
      CREATE_NO_WINDOW,             // Creation flags
      NULL,                         // Use parent's environment block
      NULL,                         // Use parent's starting directory
      &startUpInfo,                 // Pointer to STARTUPINFO structure
      &processInfo
    );

    // If we failed to start the process...
    if (!result)
    {
      // Show a message box instead
      message = BuildString(message, "\nWould you like to continue?");
      int result = MessageBoxA(NULL, message.c_str(), "Error", MB_YESNO | MB_ICONEXCLAMATION);
      
      // Trigger a break point
      return result == IDNO;
    }

    // Now wait forever for the process to finish
    WaitForSingleObject(processInfo.hProcess, INFINITE);

    // Get the exit code of the process since it should have finished by now
    DWORD exitCode = 0;
    BOOL success = GetExitCodeProcess(processInfo.hProcess, &exitCode);

    // Close unused thread handle
    CloseHandle(processInfo.hThread);

    // If we somehow failed to get the exit code, trigger a break point
    if (!success)
    {
      return true;
    }

    // Special exit codes
    const DWORD Continue = 0;
    const DWORD DebugBreak = 1;
    const DWORD ForceShutdown = 2;

    // Based on the exit code...
    switch (exitCode)
    {
    case Continue:
      // No debug break, just continue
      return false;

    case DebugBreak:
      // Returning true will cause a break point
      return true;

    case ForceShutdown:
      // Immediately kill the application
      exit(0);
      break;

    default:
      // Force a break point, we have no idea what we got back
      return true;
    }
  }
}
