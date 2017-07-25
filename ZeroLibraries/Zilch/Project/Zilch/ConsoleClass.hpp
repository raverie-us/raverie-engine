/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_CONSOLE_HPP
#define ZILCH_CONSOLE_HPP

namespace Zilch
{
  namespace Events
  {
    // Sent when anyone prints to the console
    ZilchDeclareEvent(ConsoleWrite, ConsoleEvent);

    // Sent when anyone attempts to read from the console
    ZilchDeclareEvent(ConsoleRead, ConsoleEvent);
  }

  // When the user prints data using the console, or attempts to read
  // this will be the event type that we send out (for callbacks)
  class ZeroShared ConsoleEvent : public EventData
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // The state invoking the console event
    ExecutableState* State;

    // The text of the console's WriteLine (to be printed)
    // If this is a read event, it is up to the user to set this text
    String Text;
  };

  // The default write text callback that prints to stdio
  ZeroShared void DefaultWriteText(ConsoleEvent* event);

  // The default read text callback that reads text from stdin
  ZeroShared void DefaultReadText(ConsoleEvent* event);

  // The type that we use to bind a console to the language
  class ZeroShared Console
  {
  public:
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    // Write to the console (not bound to Zilch)
    static void Write(AnyParam value0);
    static void Write(AnyParam value0, AnyParam value1);
    static void Write(AnyParam value0, AnyParam value1, AnyParam value2);
    static void Write(AnyParam value0, AnyParam value1, AnyParam value2, AnyParam value3);
    static void Write(AnyParam value0, AnyParam value1, AnyParam value2, AnyParam value3, AnyParam value4);
    static void Write(StringParam value);
    static void Write(StringRange value);
    static void Write(cstr value);
    static void Write(char value);
    static void Write(Boolean value);
    static void Write(Boolean2Param value);
    static void Write(Boolean3Param value);
    static void Write(Boolean4Param value);
    static void Write(Integer value);
    static void Write(Integer2Param value);
    static void Write(Integer3Param value);
    static void Write(Integer4Param value);
    static void Write(Real value);
    static void Write(Real2Param value);
    static void Write(Real3Param value);
    static void Write(Real4Param value);
    static void Write(DoubleInteger value);
    static void Write(DoubleReal value);
    static void Write(QuaternionParam value);
    static void WriteLine();
    static void WriteLine(AnyParam value0);
    static void WriteLine(AnyParam value0, AnyParam value1);
    static void WriteLine(AnyParam value0, AnyParam value1, AnyParam value2);
    static void WriteLine(AnyParam value0, AnyParam value1, AnyParam value2, AnyParam value3);
    static void WriteLine(AnyParam value0, AnyParam value1, AnyParam value2, AnyParam value3, AnyParam value4);
    static void WriteLine(StringParam value);
    static void WriteLine(StringRange value);
    static void WriteLine(cstr value);
    static void WriteLine(char value);
    static void WriteLine(Boolean value);
    static void WriteLine(Boolean2Param value);
    static void WriteLine(Boolean3Param value);
    static void WriteLine(Boolean4Param value);
    static void WriteLine(Integer value);
    static void WriteLine(Integer2Param value);
    static void WriteLine(Integer3Param value);
    static void WriteLine(Integer4Param value);
    static void WriteLine(Real value);
    static void WriteLine(Real2Param value);
    static void WriteLine(Real3Param value);
    static void WriteLine(Real4Param value);
    static void WriteLine(DoubleInteger value);
    static void WriteLine(DoubleReal value);
    static void WriteLine(QuaternionParam value);

    // Write out an object (1 level deep - only properties)
    static void DumpValue(AnyParam value);

    // Write out an object to a certain number of levels deep (used for debugging)
    static void DumpValue(AnyParam value, Integer howDeep);

    // Read from the console
    static String ReadString();
    static Integer ReadInteger();
    static Boolean ReadBoolean();
    static Real ReadReal();

  public:

    // Write out data (sends the write event)
    static void WriteData(StringParam text);

    // Read text from the console (sends the read event)
    // If no users handle this then it will return an empty string
    static String ReadData();

    // Helper for writing out objects
    static void DumpValue(StringBuilderExtended& builder, Type* type, const byte* value, Integer howDeep, Integer currentDepth);

  public:

    // Responsible for the console sending and receiveing events (how we hook up callbacks!)
    static EventHandler Events;
  };
}

#endif
