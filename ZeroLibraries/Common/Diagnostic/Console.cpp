///////////////////////////////////////////////////////////////////////////////
///
/// \file Console.cpp
/// Implementation of the Console
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ConsoleListener::~ConsoleListener()
{
  Console::Remove(this);
};

Array<ConsoleListener*> ConsoleListeners;

void Console::PrintVa(Filter::Enum filter, cstr format, va_list args)
{
  //Get the number of characters needed
  int bufferSize;
  ZeroVSPrintfCount(format, args, 1, bufferSize);

  if(bufferSize > 0)
  {
    char* messageBuffer = (char*)alloca((bufferSize + 1) * sizeof(char));
    ZeroVSPrintf(messageBuffer, bufferSize, format, args);

    PrintRaw(filter, messageBuffer);
  }
}

void Console::Print(Filter::Enum filter, cstr format, ...)
{
  va_list args;
  va_start(args, format);
  PrintVa(filter, format, args);
  va_end(args);
}

void Console::PrintRaw(Filter::Enum filter, cstr messageBuffer)
{
  forRange(ConsoleListener* listener, ConsoleListeners.All())
    listener->Print(filter, messageBuffer);
}

void Console::FlushAll()
{
  forRange(ConsoleListener* listener, ConsoleListeners.All())
    listener->Flush();
}

void Console::Add(ConsoleListener* listener)
{
  ConsoleListeners.PushBack(listener);
}

void Console::Remove(ConsoleListener* listener)
{
  size_t index = ConsoleListeners.FindIndex(listener);
  if(index < ConsoleListeners.Size())
    ConsoleListeners.EraseAt(index);
}

}//namespace Zero
