///////////////////////////////////////////////////////////////////////////////
///
/// \file Regex.cpp
/// Implementation of the Regex class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include <regex>

// Using directives
using namespace std;

// Bring in the namespaces (cross platform)
#ifdef _MSC_VER
  using namespace std::tr1;
  namespace reg = std::tr1::regex_constants;
#else
  namespace reg = std::regex_constants;
#endif

namespace Zero
{
// We use the pimpl pattern to avoid including stl in the header
struct RegexPrivateData
{
  // The actual processed regular expression
  regex InternalRegex;
};

struct MatchesPrivateData
{
  cmatch InternalMatch;
};

Matches::Matches()
{
  mPrivate = new MatchesPrivateData();
}

Matches::Matches(const Matches& source)
{
  mPrivate = new MatchesPrivateData(*source.mPrivate);
}

Matches::~Matches()
{
  delete mPrivate;
}


void Matches::Clear()
{
  mPrivate->InternalMatch = cmatch();
}

size_t Matches::Size() const
{
  return mPrivate->InternalMatch.size();
}

bool Matches::Empty() const
{
  return mPrivate->InternalMatch.empty();
}

StringRange Matches::operator[](size_t index) const
{
  ErrorIf(index >= this->Size(), "Attempting to access an array element out of bounds");
  const std::csub_match& subMatch = mPrivate->InternalMatch[index];
  return StringRange(subMatch.first, subMatch.first, subMatch.second);
}

StringRange Matches::Front() const
{
  ErrorIf(this->Empty(), "Attempting to access an array element out of bounds");
  const std::csub_match& subMatch = mPrivate->InternalMatch[0];
  return StringRange(subMatch.first, subMatch.first, subMatch.second);
}

StringRange Matches::Back() const
{
  ErrorIf(this->Empty(), "Attempting to access an array element out of bounds");
  const std::csub_match& subMatch = mPrivate->InternalMatch[mPrivate->InternalMatch.size() - 1];
  return StringRange(subMatch.first, subMatch.first, subMatch.second);
}


StringRange Matches::Prefix() const
{
  const std::csub_match& subMatch = mPrivate->InternalMatch.prefix();
  return StringRange(subMatch.first, subMatch.first, subMatch.second);
}

StringRange Matches::Suffix() const
{
  const std::csub_match& subMatch = mPrivate->InternalMatch.suffix();
  return StringRange(subMatch.first, subMatch.first, subMatch.second);
}

String Matches::Format(StringRange format) const
{
  // Unfortunately we pretty much have to construct an std::string here for the format, and the result is an std::string
  // I would love to at least make the result just output to a buffer, but I can't figure out how to measure the length of the output
  // We could possibly create an 'OutputIterator' for StringBuilder that mimics a char* (writes a character on assignment, advances on ++)
  // It also appears that VS2010 doesn't implement all of the overloads of format
  std::string stdFormat(format.mBegin, format.mEnd);
  std::string result = mPrivate->InternalMatch.format(stdFormat);
  return String(result.c_str(), result.size());
}

void Matches::Format(StringRange format, StringBuilder& builder) const
{
  // See notes above for optimization ideas
  std::string stdFormat(format.mBegin, format.mEnd);
  std::string result = mPrivate->InternalMatch.format(stdFormat);
  StringRange range(result.c_str(), result.c_str(), result.c_str() + result.size());
  builder.Append(range);
}

// Create a regular expression...
regex CreateRegex(StringRange regexStr, RegexFlavor::Enum flavor, bool caseSensitive, bool optimizeForMatching)
{
  // Default flags
  regex::flag_type flags = reg::basic;

  // Based on the flavor...
  switch (flavor)
  {
  case RegexFlavor::EcmaScript:
    flags = reg::ECMAScript;
    break;

  case RegexFlavor::PosixBasic:
    flags = reg::basic;
    break;

  case RegexFlavor::PosixExtended:
    flags = reg::extended;
    break;

  case RegexFlavor::Awk:
    flags = reg::awk;
    break;

  case RegexFlavor::Grep:
    flags = reg::grep;
    break;

  case RegexFlavor::Egrep:
    flags = reg::egrep;
    break;
  }

  // If we're not case sensitive...
  if(caseSensitive == false)
    flags |= reg::icase;
  if(optimizeForMatching)
    flags |= reg::optimize;

  return regex(regexStr.mBegin, regexStr.mEnd, flags);
}

Regex::Regex()
{
  // Create the private data
  mPrivate = new RegexPrivateData();
}

Regex::Regex(StringRange regexStr, RegexFlavor::Enum flavor, bool caseSensitive, bool optimizeForMatching)
{
  // Create the private data
  mPrivate = new RegexPrivateData();

  // Store the regular expression string so it won't invalidate
  mRegexString = regexStr;

  try
  {
    // Create the regular expression from the given text
    mPrivate->InternalRegex = CreateRegex(regexStr, flavor, caseSensitive, optimizeForMatching);
  }
  catch (...)
  {
    // Remove the private data...
    Error("Invalid regular expression");
    delete mPrivate;
    mPrivate = nullptr;
  }
}

Regex::Regex(const Regex& source)
{
  // Create the private data
  mPrivate = new RegexPrivateData();

  // Copy the regular expression string so it won't invalidate
  mRegexString = source.mRegexString;

  // Copy the regular expression
  mPrivate->InternalRegex = source.mPrivate->InternalRegex;
}

Regex::~Regex()
{
  // Delete the private data
  delete mPrivate;
}

Regex& Regex::operator=(const Regex& source)
{
  // Copy the regular expression string so it won't invalidate
  mRegexString = source.mRegexString;

  // Copy the regular expression
  mPrivate->InternalRegex = source.mPrivate->InternalRegex;

  // Return a reference to itself
  return *this;
}

bool Regex::Validate(StringRange regexStr, RegexFlavor::Enum flavor, bool caseSensitive)
{
  try
  {
    // Create the regular expression from the given text
    CreateRegex(regexStr, flavor, caseSensitive, false);
    return true;
  }
  catch (...)
  {
    return false;
  }
}

void Regex::Search(StringRange text, Matches& matches, RegexFlags::Type flags) const
{
  matches.Clear();

  reg::match_flag_type translatedFlags = reg::match_default;
  if (flags & RegexFlags::MatchNotNull)
    translatedFlags |= reg::match_not_null;
  regex_search(text.mBegin, text.mEnd, matches.mPrivate->InternalMatch, mPrivate->InternalRegex, translatedFlags);
}

String Regex::Replace(StringRange source, StringRange replaceWith) const
{
  // Perform the replacement and get the result back
  string result = regex_replace(string(source.mBegin, source.mEnd), mPrivate->InternalRegex, string(replaceWith.mBegin, replaceWith.mEnd));
    
  // Return that result as our string type
  return String(result.c_str(), result.size());
}

String Regex::Escape(StringRange input, EscapeMode::Enum mode, RegexFlavor::Enum flavor)
{
  // Error checking
  ErrorIf(flavor != RegexFlavor::EcmaScript, "No other regex flavors have been implemented yet!");

  // A builder so we can put in escape sequences
  StringBuilder builder;

  // While we didn't reach the end of the input
  while (input.Empty() == false)
  {
    // Get the current character
    Rune current  = input.Front();

    // If the current character is a wild card
    if (mode == EscapeMode::Extended && current == '*')
    {
      // Add the typical regex wild-card notation
      builder.Append("(.*)");
    }
    else 
    {
      // If the current character is a symbol...
      if (ispunct(current.value) && current != '_')
      {
        // If the current character is itself an escape, and we allow input escapes...
        if (current == '\\' && mode == EscapeMode::Extended && input.SizeInBytes() > 1)
        {
          // Get the next character
          Rune next = *(input.Begin() + 1);

          // Based on the next character...
          switch (next.value)
          {
          // If it's an extended character
          case '\\':
          case '0':
          case 'a':
          case 'b':
          case 'f':
          case 'n':
          case 'r':
          case 't':
          case 'v':
            break;

          default:
            // Append an escape character
            builder.Append('\\');
            break;
          }
        }
        else
        {
          // Append an escape character
          builder.Append('\\');
        }
      }

      // Append the current character to the string builder
      builder.Append(current);
    }

    // Iterate to the next character
    input.PopFront();
  }

  // Return the final escaped string
  return builder.ToString();
}
}