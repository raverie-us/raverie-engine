///////////////////////////////////////////////////////////////////////////////
///
/// \file Thread.cpp
/// Declaration of the Thread class.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/Thread.hpp"

namespace Zero
{

const bool ThreadingEnabled = false;

Thread::Thread()
{
}

Thread::~Thread()
{
}


bool Thread::IsValid()
{
return true;  
}

bool Thread::Initialize(EntryFunction entry, void* instance, cstr threadName)
{
return true;  
}

void Thread::Resume()
{
}


void Thread::Suspend()
{
}


void Thread::Close()
{

}


OsInt Thread::WaitForCompletion()
{
  return 0;
}

}//namespace Zero
