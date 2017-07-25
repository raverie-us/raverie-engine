/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_ERROR_HANDLER_HPP
#define ZILCH_ERROR_HANDLER_HPP

namespace Zilch
{
  namespace Events
  {
    // Sent on the CompilationErrors object every time an error occurs
    ZilchDeclareEvent(CompilationError, ErrorEvent);
  }

  // Type-defines
  typedef Array<const CodeLocation*> LocationArray;

  // The default error callback prints compiler errors to stderr (pass null for userData)
  ZeroShared void DefaultErrorCallback(ErrorEvent* e);

  // A special callback that assumes the user-data is a pointer to a String class
  ZeroShared void OutputErrorStringCallback(ErrorEvent* e, void* stringPointer);

  // This class provides a general output handler that we can use in all modules (for outputting messages, warnings, and errors)
  class ZeroShared CompilationErrors : public EventHandler
  {
  public:

    // Constructor
    CompilationErrors();

    // Print out an error message with extra context (one extra location, given a va_list)
    void RaiseArgs(const CodeLocation& location, StringParam extra, const CodeLocation& associatedLocation, ErrorCode::Enum errorCode, va_list args);

    // Print out an error message (given a va_list)
    void RaiseArgs(const CodeLocation& location, StringParam extra, const LocationArray& associatedLocations, ErrorCode::Enum errorCode, va_list args);

    // Print out an error message with extra context (multiple locations, given a va_list)
    void RaiseArgs(const CodeLocation& location, ErrorCode::Enum errorCode, va_list args);

    // Print out an error message with extra context (one extra location)
    void Raise(const CodeLocation& location, StringParam extra, const CodeLocation& associatedLocation, ErrorCode::Enum errorCode, ...);

    // Print out an error message with extra context (multiple locations)
    void Raise(const CodeLocation& location, StringParam extra, const LocationArray& associatedLocations, ErrorCode::Enum errorCode, ...);

    // Print out an error message
    void Raise(const CodeLocation& location, ErrorCode::Enum errorCode, ...);

    // A pointer to any data the user wants to attach
    mutable const void* UserData;

    // Any user data that cant simply be represented by a pointer
    // Data can be written to the buffer and will be properly destructed
    // when this object is destroyed (must be read in the order it's written)
    mutable DestructibleBuffer ComplexUserData;

  public:

    // If set to true, all operations will cease and the stack will be unwound
    // This is used internally for error handling
    bool WasError;

    // If set to true, then any errors we get after the 'WasError' flag is
    // set will be ignored. In general this is only set in a few specific
    // cases and should not be set by the user as it can lead to missed errors
    bool IgnoreMultipleErrors;

    // If this is set, errors will be reported but ignored (which allows parsing and syntaxing to continue)
    bool TolerantMode;
  };
}

#endif
