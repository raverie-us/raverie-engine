///////////////////////////////////////////////////////////////////////////////
///
/// \file CommandBinding.cpp
/// Definition of the HotKey Editor class.
///
/// Authors: Ryan Edgemon
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------- MetaScriptTagAttribute ---
//******************************************************************************
ZilchDefineType(MetaScriptTagAttribute, builder, type)
{
  ZilchBindField(mTags);
}

//******************************************************************************
MetaScriptTagAttribute::MetaScriptTagAttribute()
{
}

//******************************************************************************
void MetaScriptTagAttribute::PostProcess(Status& status, ReflectionObject* owner)
{
  if(mTags.Empty())
  {
    String message = "Tags are ' ' (space) delimited. Additionally: No tags are specified.";
    status.SetFailed(message);
    return;
  }

  mTagSet.Clear();

  StringTokenRange tokens(mTags.c_str(), ' ');
  for(; !tokens.Empty(); tokens.PopFront())
  {
    String tag = tokens.Front();
    if(tag.Empty())
      continue;

    if(!IsValidFilename(tag, status))
    {
      status.Message = BuildString("Tags are ' ' (space) delimited. Additionally: '", tag, "' ", status.Message);
      return;
    }

    mTagSet.Insert(tag);
  }

  // If the only token(s) found consist(s) of ' ' characters, it'll be caught here.
  if(mTagSet.Empty())
  {
    String message = "Tags are ' ' (space) delimited. Additionally: No tags are specified.";
    status.SetFailed(message);
    return;
  }

  // Now that tags have been verified, cleaned up, and duplicates removed -
  // rebuild the whole tag string.
  mTags.Clear();
  mTags = String::JoinRange(" ", mTagSet.All());
}

}  // namespace Zero
