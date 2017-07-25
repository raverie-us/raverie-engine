/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  CodeLocation::CodeLocation() :
    StartLine(1),
    PrimaryLine(1),
    EndLine(1),
    StartCharacter(1),
    PrimaryCharacter(1),
    EndCharacter(1),
    StartPosition(0),
    PrimaryPosition(0),
    EndPosition(0),
    Origin(UnknownOrigin),
    CodeUserData(nullptr),
    IsNative(true)
  {
  }

  //***************************************************************************
  CodeLocation CodeLocation::GetStartOnlyLocation()
  {
    // First copy the current code location, then just change the end and primary to point at the start
    CodeLocation result;
    result = *this;
    result.EndCharacter     = result.StartCharacter;
    result.EndLine          = result.StartLine;
    result.PrimaryCharacter = result.StartCharacter;
    result.PrimaryLine      = result.StartLine;
    return result;
  }
  
  //***************************************************************************
  CodeLocation CodeLocation::GetEndOnlyLocation()
  {
    // First copy the current code location, then just change the start and primary to point at the end
    CodeLocation result;
    result = *this;
    result.StartCharacter   = result.EndCharacter;
    result.StartLine        = result.EndLine;
    result.PrimaryCharacter = result.EndCharacter;
    result.PrimaryLine      = result.EndLine;
    return result;
  }
  
  //***************************************************************************
  size_t CodeLocation::GetHash()
  {
    return this->Code.Hash() ^ this->Origin.Hash() * 5689;
  }

  //***************************************************************************
  bool CodeLocation::IsValid()
  {
    return this->Origin != UnknownOrigin;
  }

  //***************************************************************************
  String CodeLocation::GetFormattedString(MessageFormat::Enum format) const
  {
    // Based on the format (typically we support many different language formats)
    switch (format)
    {
      case MessageFormat::Zilch:
      {
        // If we have no function name...
        if (this->Function.Empty())
        {
          return String::Format("  In %s at line %d, character %d",
            this->Origin.c_str(),
            this->PrimaryLine,
            this->PrimaryCharacter);
        }
        // Otherwise, a function name was provided
        else
        {
          return String::Format("  In %s at line %d, character %d (within function %s)",
            this->Origin.c_str(),
            this->PrimaryLine,
            this->PrimaryCharacter,
            this->Function.c_str());
        }
      }

      case MessageFormat::Python:
      {
        // If we have no function name...
        if (this->Function.Empty())
        {
          return String::Format("  File \"%s\", line %d",
            this->Origin.c_str(),
            this->PrimaryLine);
        }
        // Otherwise, a function name was provided
        else
        {
          return String::Format("  File \"%s\", line %d, in %s",
            this->Origin.c_str(),
            this->PrimaryLine,
            this->Function.c_str());
        }
      }

      case MessageFormat::MsvcCpp:
      {
        // If we have no function name...
        if (this->Function.Empty())
        {
          return String::Format("  %s(%d)",
            this->Origin.c_str(),
            this->PrimaryLine);
        }
        // Otherwise, a function name was provided
        else
        {
          return String::Format("  %s(%d) in %s",
            this->Origin.c_str(),
            this->PrimaryLine,
            this->Function.c_str());
        }
      }
    }

    // We should never end up here!
    Error("Unhandled format type or the user passed in a garbage value");
    return String();
  }

  //***************************************************************************
  //bool CodeLocation::IsValid()
  //{
  //  return this->CodeEntryId != 0;
  //}

  //***************************************************************************
  String CodeLocation::GetFormattedStringWithMessage(MessageFormat::Enum format, StringParam message) const
  {
    // First, just get the formatted code location
    String formattedLocation = this->GetFormattedString(format);

    // This will be used as the format string
    cstr strFormat = nullptr;
    size_t tabbing = 0;
    bool tabFirstLine = true;

    // Based on the format (typically we support many different language formats)
    switch (format)
    {
      case MessageFormat::Zilch:
      {
        strFormat = "%s:\n%s\n";
        tabbing = 4;
        break;
      }

      case MessageFormat::Python:
      {
        strFormat = "%s\n%s\n";
        tabbing = 4;
        break;
      }

      case MessageFormat::MsvcCpp:
      {
        strFormat = "%s: %s\n";
        tabbing = 2;
        tabFirstLine = false;
        break;
      }
    }

    // Tab in every line in the message, if requested
    StringBuilder builder;
    for (Zero::StringSplitRange range = message.Split("\n"); range.Empty() == false;)
    {
      // Grab the current line and immediately pop (this lets us know if we're at the end so we don't Append an extra "\n")
      StringRange line = range.Front();
      range.PopFront();

      // If this isn't the first line, or we specify that we want to tab the first line...
      if (builder.GetSize() > 0 || tabFirstLine)
        builder.Repeat(tabbing, " ");

      builder.Append(line);
      if (range.Empty() == false)
        builder.Append("\n");
    }
    String tabbedMessage = builder.ToString();

    // Make sure we got a string format from above
    ErrorIf(strFormat == nullptr, "Unhandled format type or the user passed in a garbage value");

    // Return the formatted location with the message attached
    return String::Format(strFormat,
            formattedLocation.c_str(),
            tabbedMessage.c_str());
  }
}
