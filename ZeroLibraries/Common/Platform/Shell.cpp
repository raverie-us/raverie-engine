///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------FileDialogFilter
FileDialogFilter::FileDialogFilter()
{

}

FileDialogFilter::FileDialogFilter(StringParam filter) 
  : mDescription(filter)
  , mFilter(filter)
{

}

FileDialogFilter::FileDialogFilter(StringParam description, StringParam filter)
  : mDescription(description)
  , mFilter(filter)
{

}

//-------------------------------------------------------------------FileDialogConfig
FileDialogSetup::FileDialogSetup() :
  mCallback(nullptr),
  mUserData(nullptr),
  Flags(0)
{
}

void FileDialogSetup::AddFilter(StringParam description, StringParam filter)
{
  mSearchFilters.PushBack(FileDialogFilter(description, filter));
}
}//namespace Zero
