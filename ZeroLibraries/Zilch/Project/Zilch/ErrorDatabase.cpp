/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ErrorDatabase::ErrorDatabase()
  {
    // Setup all the errors using generated code
    #include "ErrorDatabaseSetup.inl"
  }

  //***************************************************************************
  ErrorDatabase& ErrorDatabase::GetInstance()
  {
    // Singleton pattern
    static ErrorDatabase instance;
    return instance;
  }

  //***************************************************************************
  const ErrorInfo& ErrorDatabase::GetErrorInfo(ErrorCode::Enum errorCode) const
  {
    return this->Errors[errorCode];
  }

  //***************************************************************************
  ZilchDefineType(ErrorEvent, builder, type)
  {
  }

  //***************************************************************************
  ErrorEvent::ErrorEvent() :
    ErrorCode(ErrorCode::Invalid)
  {
  }

  //***************************************************************************
  ErrorEvent::ErrorEvent(const ErrorInfo& info, const CodeLocation& location, ErrorCode::Enum code, va_list args) :
    Reason(info.Reason),
    Examples(info.Examples),
    Location(location),
    ExactError(String::FormatArgs(info.Error.c_str(), args)),
    ErrorCode(code)
  {
  }

  //***************************************************************************
  String ErrorEvent::GetFormattedMessage(MessageFormat::Enum format) const
  {
    // Print the error message out with the location (in a standard format)
    String message = this->Location.GetFormattedStringWithMessage(format, this->ExactError);

    // The message may have more things appended to it, so create a string builder
    StringBuilder builder;
    builder.Append(message);
    
    // Walk through all locations associated with this error (many errors have no associated locations, just the first 'Location')
    // Associated locations are things like duplicate class definitions, where it shows the other location the class was defined
    for (size_t i = 0; i < this->AssociatedOtherLocations.Size(); ++i)
    {
      // Grab the current location and Append it to the error message
      const CodeLocation& associatedLocation = this->AssociatedOtherLocations[i];
      String seeLocation = associatedLocation.GetFormattedString(format);
      builder.Append("      See");
      builder.Append(seeLocation);
    }

    // Compact the builder into one final string containing everything about the error
    return builder.ToString();
  }
}
