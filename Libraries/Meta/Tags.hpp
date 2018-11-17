///////////////////////////////////////////////////////////////////////////////
///
/// \file Tags.hpp
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
  
typedef HashSet<String> TagList;

#define DeclareTag(name) extern const String name
#define DefineTag(name) const String name = #name

inline bool ContainsRequiredTags(const TagList& requiredTags, const TagList& tags)
{
  for(TagList::range range = requiredTags.All(); !range.Empty(); range.PopFront())
  {
    StringParam requiredTag = range.Front();
    if(!tags.Contains(requiredTag))
      return false;
  }

  return true;
}

inline bool ContainsRequiredTagsCaseInsensitive(const TagList& requiredTags, const TagList& tags)
{
  for(TagList::range range = requiredTags.All(); !range.Empty(); range.PopFront())
  {
    String requiredTag = range.Front().ToLower();
    bool found = false;
    for(TagList::range tagRange = tags.All(); !tagRange.Empty(); tagRange.PopFront())
    {
      String tag = tagRange.Front().ToLower();
      if(tag == requiredTag)
      {
        found = true;
        break;
      }
    }
    if(!found)
      return false;
  }

  return true;
}

}//namespace Zero
