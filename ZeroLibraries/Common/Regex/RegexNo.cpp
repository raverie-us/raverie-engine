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

#include "Regex.hpp"
#include "String/StringBuilder.hpp"

namespace Zero
{
Matches::Matches()
{
  mPrivate = nullptr;
}

Matches::Matches(const Matches& source)
{
  mPrivate = nullptr;
}

Matches::~Matches()
{
}

void Matches::Clear()
{
}

size_t Matches::Size() const
{
  return 0;
}

bool Matches::Empty() const
{
  return true;
}

StringRange Matches::operator[](size_t index) const
{
  return StringRange();
}

StringRange Matches::Front() const
{
  return StringRange();
}

StringRange Matches::Back() const
{
  return StringRange();
}

StringRange Matches::Prefix() const
{
  return StringRange();
}

StringRange Matches::Suffix() const
{
  return StringRange();
}

String Matches::Format(StringRange format) const
{
  return String();
}

void Matches::Format(StringRange format, StringBuilder& builder) const
{
}

Regex::Regex()
{
  mPrivate = nullptr;
}

Regex::Regex(StringRange regexStr, RegexFlavor::Enum flavor, bool caseSensitive, bool optimizeForMatching)
{
  mPrivate = nullptr;
}

Regex::Regex(const Regex& source)
{
  mPrivate = nullptr;
}

Regex::~Regex()
{
}

Regex& Regex::operator=(const Regex& source)
{
  return *this;
}

bool Regex::Validate(StringRange regexStr, RegexFlavor::Enum flavor, bool caseSensitive)
{
  return false;
}

void Regex::Search(StringRange text, Matches& matches, RegexFlags::Type flags) const
{
}

String Regex::Replace(StringRange source, StringRange replaceWith) const
{
  return source;
}

String Regex::Escape(StringRange input, EscapeMode::Enum mode, RegexFlavor::Enum flavor)
{
  return input;
}
}