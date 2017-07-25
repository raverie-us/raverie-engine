///////////////////////////////////////////////////////////////////////////////
///
/// \file Regex.hpp
/// Declaration of the Regex.
///
/// Authors: Trevor Sundberg
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "String/String.hpp"
#include "Containers/Array.hpp"
#include "Utility/EnumDeclaration.hpp"

namespace Zero
{
  // Forward declaration
  struct RegexPrivateData;
  struct MatchesPrivateData;
  class StringBuilder;

  // The flavor of regular expressions
  DeclareEnum6(RegexFlavor, EcmaScript, PosixBasic, PosixExtended, Awk, Grep, Egrep);

  // Any flags we want to give to the regex (generally used for searching)
  DeclareBitField1(RegexFlags, MatchNotNull);

  // How we escape a string (normal means escape everything, extended
  // means escape everything except extended characters such as '\r'...)
  DeclareEnum2(EscapeMode, Normal, Extended);

  class Matches
  {
  public:
    friend class Regex;

    Matches();
    Matches(const Matches& source);
    ~Matches();
    
    // Clears the array of matches
    void Clear();

    // How many matches we captured
    size_t Size() const;

    // If there are no matches
    bool Empty() const;

    // Grabs the match by index (0 is always the entire match, 1 and on are sub-strings / captures groups)
    StringRange operator[](size_t index) const;

    // Helpers to get the first and last element
    StringRange Front() const;
    StringRange Back() const;

    // All the text before this entire match
    StringRange Prefix() const;

    // All the text after this entire match
    StringRange Suffix() const;

    // Formats a match with a special $ format string
    String Format(StringRange format) const;

    // Formats a match with a special $ format string and outputs directly into a string builder
    void Format(StringRange format, StringBuilder& builder) const;

  private:
    // Store the private data
    MatchesPrivateData* mPrivate;
  };

  class Regex
  {
  public:
    friend class Matches;

    // Default Constructor
    Regex();

    // Constructor
    Regex(StringRange regex, RegexFlavor::Enum flavor = RegexFlavor::EcmaScript, bool caseSensitive = true, bool optimizeForMatching = true);
    
    // Copy constructor
    Regex(const Regex& source);

    // Destructor
    ~Regex();

    // Assignment operator
    Regex& operator=(const Regex& source);

    // Search a given string and return matches (clears matches that are passed in)
    void Search(StringRange text, Matches& matches, RegexFlags::Type flags = RegexFlags::None) const;

    // Replace all matches in a given string
    String Replace(StringRange source, StringRange replaceWith) const;

    // Escape a string so that it can be used directly in a regex, typically for finding exactly that string
    static String Escape(StringRange input, EscapeMode::Enum mode, RegexFlavor::Enum flavor = RegexFlavor::EcmaScript);

    // Validate the regular expression
    static bool Validate(StringRange regex, RegexFlavor::Enum flavor = RegexFlavor::EcmaScript, bool caseSensitive = true);

    // The original regular expression string that created this regex
    String mRegexString;

  private:

    // Store the private data
    RegexPrivateData* mPrivate;
  };
}
