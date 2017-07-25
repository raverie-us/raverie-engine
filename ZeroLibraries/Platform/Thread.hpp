///////////////////////////////////////////////////////////////////////////////
///
/// \file Thread.hpp
/// Declaration of the Thread class.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "PrivateImplementation.hpp"
#include "OsHandle.hpp"
#include "Common/Containers/HashMap.hpp"

namespace Zero
{

// Is threading enabled on this platform?
ZeroShared extern const bool ThreadingEnabled;

/// Thread config for setting platform specific options
class ZeroShared ThreadConfig
{
public:
  ThreadConfig();
  ~ThreadConfig();

  void SetParameter(StringParam name, void* value);
  void* GetParameter(StringParam name);

private:
  HashMap<String,void*> mConfigValues;
};

/// Thread class manages Os threads.
class ZeroShared Thread
{
public:
  typedef OsInt (*EntryFunction)(void*);

  //Construct a thread object does not create the thread.
  //Initialize and then resume to run the thread.
  Thread();
  ~Thread();

  //Is this a valid thread or uninitialized.
  bool IsValid();

  //Initializes the thread but does not run it.
  bool Initialize(EntryFunction entryFunction, void* instance, StringParam threadName, ThreadConfig* config = nullptr);

  //Resume the thread.
  void Resume();

  //Suspend the thread.
  void Suspend();

  //Close the thread handle. Thread should have been shut down.
  //before calling this function.
  void Close();

  //Block waiting for the thread to complete.
  //The thread should either complete in a reasonable way or
  //be signaled to be closed.
  OsInt WaitForCompletion();

  //Is the thread completed?
  bool IsCompleted();

  //Removes the thread from this object.
  OsHandle Detach();

  //Get the OsHandle to the thread.
  OsHandle GetThreadHandle();

  // Template Helper for creating Entry Functions
  // From member functions
  template<typename classType, OsInt (classType::*MemberFunction)()>
  static OsInt ObjectEntryCreator(void* objectInstance)
  {
    classType* object = (classType*)objectInstance;
    OsInt returnValue = (object->*MemberFunction)();
    return returnValue;
  }

private:
  String mThreadName;
  ZeroDeclarePrivateData(Thread, 20);
};

}//namespace Zero
