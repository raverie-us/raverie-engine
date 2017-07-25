///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

template <typename Function>
bool TryFileOperation(StringParam dest, StringParam source, Function fileOp)
{
  bool result = false;
  // Keep trying to copy the file until we succeed up to some max number of times.
  // This is attempting to deal with random file locks (from anti-virus or something).
  for(size_t i = 0; i < 10; ++i)
  {
    result = fileOp(dest, source);
    if(result == true)
      break;

    Os::Sleep(1);
  }

  // Maybe log some extra error messages here?
  if(result == false)
  {

  }

  return result;
}

bool CopyFile(StringParam dest, StringParam source)
{
  FileModifiedState::BeginFileModified(dest);
  bool result = TryFileOperation(dest, source, CopyFileInternal);
  FileModifiedState::EndFileModified(dest);
  return result;
}

bool MoveFile(StringParam dest, StringParam source)
{
  FileModifiedState::BeginFileModified(dest);
  bool result = TryFileOperation(dest, source, MoveFileInternal);
  FileModifiedState::EndFileModified(dest);
  return result;
}

bool DeleteFile(StringParam dest)
{
  bool result = false;

  FileModifiedState::BeginFileModified(dest);

  // Keep trying to copy the file until we succeed up to some max number of times.
  // This is attempting to deal with random file locks (from anti-virus or something).
  for(size_t i = 0; i < 10; ++i)
  {
    result = DeleteFileInternal(dest);
    if(result == true)
      break;

    Os::Sleep(1);
  }

  FileModifiedState::EndFileModified(dest);

  // Maybe log some extra error messages here?
  if(result == false)
  {

  }

  return result;
}

}//namespace Zero
