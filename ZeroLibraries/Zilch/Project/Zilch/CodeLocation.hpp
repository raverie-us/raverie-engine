/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_CODE_LOCATION_HPP
#define ZILCH_CODE_LOCATION_HPP

namespace Zilch
{
  // We can print Zilch messages in many different formats
  // This is used for printing errors, exceptions, and general code location information
  namespace MessageFormat
  {
    enum Enum
    {
      // This is the standard format we use to print Zilch error messages
      // We try to be very descriptive about where our errors occur
      // Style: 'In <origin> at line <line>, character <character> (within function <function>)'
      //        '  <message>'
      Zilch,
      
      // Style: '  File "<origin>", line <line>, in <function>'
      //        '    <message>'
      Python,
      
      // Msvc errors are useful when editing Zilch inside of Visual Studio,
      // because you can double click the error and go directly to the file
      // Style: '<origin>(<line>): <message>'
      MsvcCpp
    };
  }

  // A code location provides us with a context of where something occurred
  class ZeroShared CodeLocation
  {
  public:
    // Default constructor
    CodeLocation();

    // Checks if this location was ever set to a valid value
    // This is true if the origin is set
    bool IsValid();

    // Get a formatted message that includes this location (may include newlines depending on the format)
    String GetFormattedStringWithMessage(MessageFormat::Enum format, StringParam message) const;

    // Get this location formatted in different styles (does not include newlines)
    String GetFormattedString(MessageFormat::Enum format) const;

    // Creates a code location that is strictly at the start of this location
    CodeLocation GetStartOnlyLocation();

    // Creates a code location that is strictly at the end of this location
    CodeLocation GetEndOnlyLocation();

    // This hash matches the hash used in the CodeEntry, and can generally be used to map back to files
    size_t GetHash();

    // Every file and code string compiled gets a unique id
    String Code;
    
    // Note: Primary is a location that we generally use when displaying errors or other visualizations
    // For example, in a binary operator the location Start/End encompasses both the Left/Right operands
    // however the primary location is the operator itself
    // Primary should always be between start and end

    // The line range that the node originated from
    // Lines start at a value of 1 (a value of 0 is invalid)
    size_t StartLine;
    size_t PrimaryLine;
    size_t EndLine;

    // The character range that the node originated from (relative to the start of the line)
    // Characters start at a value of 1 (a value of 0 is invalid)
    size_t StartCharacter;
    size_t PrimaryCharacter;
    size_t EndCharacter;

    // The zero-based character position from the associated code
    size_t StartPosition;
    size_t PrimaryPosition;
    size_t EndPosition;

    // The file/script this node originated from
    String Origin;

    // This is an optional library that we're from (typically filled out in the syntaxer phase)
    String Library;

    // This is an optional class that we're from (typically filled out in the syntaxer phase)
    String Class;

    // This is an optional function that we're from (typically filled out in the syntaxer phase)
    String Function;

    // Tells us if the location is within C++, which
    // means that it cannot be debugged or visualized
    bool IsNative;

    // When the user provides a code block to the project to be compiled
    // they have the option of providing user-data. This user-data is the
    // same data that they passed in
    mutable const void* CodeUserData;
  };

  // Every time we add code to the project we do it through this
  // This is also stored on the library that gets generated out of the project
  class ZeroShared CodeEntry
  {
  public:
    // Constructor
    CodeEntry();

    String Code;
    String Origin;
    void* CodeUserData;

    // Gets a hash that we can use to uniquely identify this code
    // This includes the code and its origin
    // If a file changes names, this will no longer map up to that same file
    size_t GetHash();
  };
}

// Explicit specializations
namespace Zero
{
  // Code locations should be directly memory movable
  // String would technically just increment a reference and then decrement, so skip it!
  // WARNING: If this ever becomes non-movable, then be sure to update UserToken!
  template <>
  struct ZeroShared MoveWithoutDestructionOperator<Zilch::CodeLocation>
  {
    static inline void MoveWithoutDestruction(Zilch::CodeLocation* dest, Zilch::CodeLocation* source)
    {
      memcpy(dest, source, sizeof(*source));
    }
  };
}

#endif
