/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_ERROR_DATABASE_HPP
#define ZILCH_ERROR_DATABASE_HPP

namespace Zilch
{
  // All the possible errors in the language
  namespace ErrorCode
  {
    enum Enum
    {
      Invalid = -1,

      // Include the generated error enumeration values
      #include "ErrorDatabaseEnum.inl"

      Count
    };
  }

  // Store example error information and how it was fixed
  class ZeroShared ErrorExample
  {
  public:
    // The lines of code where the error can be seen
    String ErrorCode;

    // The same lines of code as above, but with the error fixed
    String FixedCode;

    // A brief explanation of the fix
    String ExplanationOfFix;
  };

  // Store information about a particular error
  class ZeroShared ErrorInfo
  {
  public:
    // The error itself (possibly a context sensitive string)
    String Error;

    // The name of the error
    String Name;

    // A reason given for why the error occurs that generally explains to the user why it exists and common pitfalls (human friendly!)
    String Reason;

    // A series of examples as to where the error occurs and examples of fixes (as well as a brief explanation)
    Array<ErrorExample> Examples;
  };

  // A created database that stores all the errors
  class ZeroShared ErrorDatabase
  {
  public:

    // Get the singleton instance of the error database (which also initializes it)
    static ErrorDatabase& GetInstance();

    // Get the error info for a given error code
    const ErrorInfo& GetErrorInfo(ErrorCode::Enum errorCode) const;

  private:

    // Constructor (initializes the errors)
    ErrorDatabase();

    // Store an array of all the errors and their information
    Array<ErrorInfo> Errors;
  };

  // This structure is what gets reported to the user
  class ZeroShared ErrorEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);
    
    ErrorEvent();
    ErrorEvent(const ErrorInfo& info, const CodeLocation& location, ErrorCode::Enum code, va_list args);

    // The specific error code for the error
    ErrorCode::Enum ErrorCode;

    // The location that the error occurred (file/place, line, etc)
    CodeLocation Location;

    // Other locations associated with the error
    // For example, duplicate class names would include the other place where the class was defined
    // Never rely upon this being set in certain errors, as sometimes the locations are unknown and do not get populated
    Array<CodeLocation> AssociatedOtherLocations;

    // A reason given for why the error occurs that generally explains to the user why it exists and common pitfalls (human friendly!)
    String Reason;

    // The exact error message, including context
    String ExactError;

    // A series of examples as to where the error occurs and examples of fixes (as well as a brief explanation)
    Array<ErrorExample> Examples;

    // Get the standard formatting for error messages
    String GetFormattedMessage(MessageFormat::Enum format) const;
  };
}

#endif
