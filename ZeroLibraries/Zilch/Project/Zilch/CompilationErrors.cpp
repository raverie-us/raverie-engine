/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  namespace Events
  {
    ZilchDefineEvent(CompilationError);
  }

  //***************************************************************************
  void DefaultErrorCallback(ErrorEvent* e)
  {
    // Print out the standard formatted error message to the console
    printf("%s", e->GetFormattedMessage(MessageFormat::Zilch).c_str());
  }

  //***************************************************************************
  void OutputErrorStringCallback(ErrorEvent* e, void* stringPointer)
  {
    String& outputString = *(String*)stringPointer;
    outputString = e->GetFormattedMessage(MessageFormat::Zilch);
  }

  //***************************************************************************
  CompilationErrors::CompilationErrors() :
    WasError(false),
    IgnoreMultipleErrors(true),
    UserData(nullptr),
    TolerantMode(false)
  {
  }

  //***************************************************************************
  void CompilationErrors::RaiseArgs(const CodeLocation& location, ErrorCode::Enum errorCode, va_list args)
  {
    return RaiseArgs(location, String(), LocationArray(), errorCode, args);
  }

  //***************************************************************************
  void CompilationErrors::RaiseArgs(const CodeLocation& location, StringParam extra, const CodeLocation& associatedLocation, ErrorCode::Enum errorCode, va_list args)
  {
    LocationArray associatedLocations;
    associatedLocations.PushBack(&associatedLocation);
    return RaiseArgs(location, String(), associatedLocations, errorCode, args);
  }

  //***************************************************************************
  void CompilationErrors::RaiseArgs(const CodeLocation& location, StringParam extra, const LocationArray& associatedLocations, ErrorCode::Enum errorCode, va_list args)
  {
    // If there already was an error and we're set to ignore multiple errors, exit out early
    if (this->WasError && this->IgnoreMultipleErrors)
      return;

    // If we're in tolerant mode, skip raising any errors
    if (this->TolerantMode)
      return;

    // An error occurred, set the flag
    this->WasError = true;

    // Get the error information from the database
    const ErrorInfo& errorInfo = ErrorDatabase::GetInstance().GetErrorInfo(errorCode);

    // Create an error details object that encompasses the error (including the context of the error)
    ErrorEvent errorDetails(errorInfo, location, errorCode, args);

    // Copy over any associated locations
    // For example, duplicate class definitions, where is the duplicate class?
    ZilchForEach(const CodeLocation* location, associatedLocations)
      errorDetails.AssociatedOtherLocations.PushBack(*location);

    // Append any extra context to the error
    // Eg. when the parser expects something, it will say what it got and what it expected
    errorDetails.ExactError = BuildString(errorDetails.ExactError, extra);

    // Send the event and let everyone receive it
    EventSend(this, Events::CompilationError, &errorDetails);

    // If the error was an internal error, then break here
    if (errorCode == ErrorCode::InternalError)
    {
      // We have to break and solve this issue
      Error("Internal error: %s", errorDetails.ExactError.c_str());
    }
  }
  
  //***************************************************************************
  void CompilationErrors::Raise(const CodeLocation& location, ErrorCode::Enum errorCode, ...)
  {
    // Create a variable argument list
    va_list argList;
    va_start(argList, errorCode);

    // Raise the error with the argument list
    RaiseArgs(location, String(), LocationArray(), errorCode, argList);

    // Finish reading variable arguments
    va_end(argList);
  }

  //***************************************************************************
  void CompilationErrors::Raise(const CodeLocation& location, StringParam extra, const CodeLocation& associatedLocation, ErrorCode::Enum errorCode, ...)
  {
    // Create a variable argument list
    va_list argList;
    va_start(argList, errorCode);

    // Raise the error with the argument list
    LocationArray associatedLocations;
    associatedLocations.PushBack(&associatedLocation);
    RaiseArgs(location, String(), associatedLocations, errorCode, argList);

    // Finish reading variable arguments
    va_end(argList);
  }

  //***************************************************************************
  void CompilationErrors::Raise(const CodeLocation& location, StringParam extra, const LocationArray& associatedLocations, ErrorCode::Enum errorCode, ...)
  {
    // Create a variable argument list
    va_list argList;
    va_start(argList, errorCode);

    // Raise the error with the argument list
    RaiseArgs(location, errorCode, argList);

    // Finish reading variable arguments
    va_end(argList);
  }
}