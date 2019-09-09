// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
typedef void* OsHandle;
typedef unsigned long OsInt;
const OsHandle cInvalidHandle = (OsHandle)(uintptr_t)-1;
const uint cDebugNameMax = 32;

class ZeroShared StackHandle
{
public:
  StackHandle()
  {
    mHandle = cInvalidHandle;
  }

  ~StackHandle()
  {
    Close();
  }

  OsHandle Transfer()
  {
    OsHandle handle = mHandle;
    mHandle = cInvalidHandle;
    return handle;
  }

  void operator=(OsHandle handle)
  {
    mHandle = handle;
  }

  operator OsHandle()
  {
    return mHandle;
  }

  void Close();

  OsHandle mHandle;
};
} // namespace Zero
