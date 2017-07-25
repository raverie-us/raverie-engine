/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  namespace Events
  {
    ZilchDefineEvent(ConsoleWrite);
    ZilchDefineEvent(ConsoleRead);
  }

  //***************************************************************************
  // Initialize static variables
  EventHandler Console::Events;
  static String ConsoleSeparator(", ");
  static String NewLine("\n");

  //***************************************************************************
  ZilchDefineType(ConsoleEvent, builder, type)
  {
  }

  //***************************************************************************
  void DefaultWriteText(ConsoleEvent* event)
  {
    // Print the text to the console
    printf("%s", event->Text.c_str());
  }

  //***************************************************************************
  void DefaultReadText(ConsoleEvent* event)
  {
    // Attempt to read a string from the console
    const size_t BufferSize = 1024;
    char buffer[BufferSize + 1] = {0};

    // Read data into the buffer
    fgets(buffer, BufferSize, stdin);

    // Loop through all characters in the buffer
    for (size_t i = 0; i < BufferSize; ++i)
    {
      // If we hit a newline...
      if (buffer[i] == '\r' || buffer[i] == '\n')
      {
        // Terminate the string at the newline
        buffer[i] = '\0';
      }
    }
    
    // Return the read in data
    event->Text = String(buffer);
  }

  //***************************************************************************
  ZilchDefineType(Console, builder, type)
  {
    ZilchFullBindMethod(builder, type, &Console::Write, (void (*)(AnyParam)),                                         "Write", nullptr);
    ZilchFullBindMethod(builder, type, &Console::Write, (void (*)(AnyParam, AnyParam)),                               "Write", nullptr);
    ZilchFullBindMethod(builder, type, &Console::Write, (void (*)(AnyParam, AnyParam, AnyParam)),                     "Write", nullptr);
    ZilchFullBindMethod(builder, type, &Console::Write, (void (*)(AnyParam, AnyParam, AnyParam, AnyParam)),           "Write", nullptr);
    ZilchFullBindMethod(builder, type, &Console::Write, (void (*)(AnyParam, AnyParam, AnyParam, AnyParam, AnyParam)), "Write", nullptr);
    
    ZilchFullBindMethod(builder, type, &Console::WriteLine, (void (*)()),                                                 "WriteLine", nullptr);
    ZilchFullBindMethod(builder, type, &Console::WriteLine, (void (*)(AnyParam)),                                         "WriteLine", nullptr);
    ZilchFullBindMethod(builder, type, &Console::WriteLine, (void (*)(AnyParam, AnyParam)),                               "WriteLine", nullptr);
    ZilchFullBindMethod(builder, type, &Console::WriteLine, (void (*)(AnyParam, AnyParam, AnyParam)),                     "WriteLine", nullptr);
    ZilchFullBindMethod(builder, type, &Console::WriteLine, (void (*)(AnyParam, AnyParam, AnyParam, AnyParam)),           "WriteLine", nullptr);
    ZilchFullBindMethod(builder, type, &Console::WriteLine, (void (*)(AnyParam, AnyParam, AnyParam, AnyParam, AnyParam)), "WriteLine", nullptr);

    ZilchFullBindMethod(builder, type, &Console::DumpValue, (void (*)(AnyParam)),          "DumpValue", nullptr);
    ZilchFullBindMethod(builder, type, &Console::DumpValue, (void (*)(AnyParam, Integer)), "DumpValue", nullptr);

    ZilchFullBindMethod(builder, type, &Console::ReadString,  ZilchNoOverload, "ReadString",   nullptr);
    ZilchFullBindMethod(builder, type, &Console::ReadInteger, ZilchNoOverload, "ReadInteger",  nullptr);
    ZilchFullBindMethod(builder, type, &Console::ReadBoolean, ZilchNoOverload, "ReadBoolean",  nullptr);
    ZilchFullBindMethod(builder, type, &Console::ReadReal,    ZilchNoOverload, "ReadReal",     nullptr);
  }
  
  //***************************************************************************
  void Console::WriteData(StringParam text)
  {
    // Send out the event with the given text data
    ConsoleEvent toSend;
    toSend.State = ExecutableState::CallingState;
    toSend.Text = text;
    EventSend(&Events, Events::ConsoleWrite, &toSend);
  }
  
  //***************************************************************************
  String Console::ReadData()
  {
    // Send out the event (the user must fill out the text field)
    ConsoleEvent toSend;
    toSend.State = ExecutableState::CallingState;
    EventSend(&Events, Events::ConsoleRead, &toSend);
    return toSend.Text;
  }

  //***************************************************************************
  void Console::Write(AnyParam value0)
  {
    WriteData(value0.ToString());
  }

  //***************************************************************************
  void Console::Write(AnyParam value0, AnyParam value1)
  {
    WriteData(BuildString(value0.ToString(), ConsoleSeparator, value1.ToString()));
  }

  //***************************************************************************
  void Console::Write(AnyParam value0, AnyParam value1, AnyParam value2)
  {
    WriteData(BuildString(value0.ToString(), ConsoleSeparator, value1.ToString(), ConsoleSeparator, value2.ToString()));
  }

  //***************************************************************************
  void Console::Write(AnyParam value0, AnyParam value1, AnyParam value2, AnyParam value3)
  {
    StringBuilder builder;
    builder.Append(value0.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value1.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value2.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value3.ToString());
    WriteData(builder.ToString());
  }

  //***************************************************************************
  void Console::Write(AnyParam value0, AnyParam value1, AnyParam value2, AnyParam value3, AnyParam value4)
  {
    StringBuilder builder;
    builder.Append(value0.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value1.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value2.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value3.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value4.ToString());
    WriteData(builder.ToString());
  }

  //***************************************************************************
  void Console::Write(StringParam value)
  {
    WriteData(value);
  }
  
  //***************************************************************************
  void Console::Write(StringRange value)
  {
    WriteData(value);
  }

  //***************************************************************************
  void Console::Write(cstr value)
  {
    WriteData(String(value));
  }

  //***************************************************************************
  void Console::Write(char value)
  {
    WriteData(String(value));
  }

  //***************************************************************************
  void Console::Write(Integer value)
  {
    WriteData(IntegerToString(value));
  }

  //***************************************************************************
  void Console::Write(Integer2Param value)
  {
    WriteData(Integer2ToString(value));
  }

  //***************************************************************************
  void Console::Write(Integer3Param value)
  {
    WriteData(Integer3ToString(value));
  }

  //***************************************************************************
  void Console::Write(Integer4Param value)
  {
    WriteData(Integer4ToString(value));
  }

  //***************************************************************************
  void Console::Write(DoubleInteger value)
  {
    WriteData(DoubleIntegerToString(value));
  }

  //***************************************************************************
  void Console::Write(Boolean value)
  {
    WriteData(BooleanToString(value));
  }

  //***************************************************************************
  void Console::Write(Boolean2Param value)
  {
    WriteData(Boolean2ToString(value));
  }

  //***************************************************************************
  void Console::Write(Boolean3Param value)
  {
    WriteData(Boolean3ToString(value));
  }

  //***************************************************************************
  void Console::Write(Boolean4Param value)
  {
    WriteData(Boolean4ToString(value));
  }

  //***************************************************************************
  void Console::Write(Real value)
  {
    WriteData(RealToString(value));
  }

  //***************************************************************************
  void Console::Write(DoubleReal value)
  {
    WriteData(DoubleRealToString(value));
  }

  //***************************************************************************
  void Console::Write(Real2Param value)
  {
    WriteData(Real2ToString(value));
  }

  //***************************************************************************
  void Console::Write(Real3Param value)
  {
    WriteData(Real3ToString(value));
  }

  //***************************************************************************
  void Console::Write(Real4Param value)
  {
    WriteData(Real4ToString(value));
  }

  //***************************************************************************
  void Console::Write(QuaternionParam value)
  {
    WriteData(QuaternionToString(value));
  }

  //***************************************************************************
  void Console::WriteLine()
  {
    WriteData(NewLine);
  }

  //***************************************************************************
  void Console::WriteLine(AnyParam value0)
  {
    WriteData(BuildString(value0.ToString(), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(AnyParam value0, AnyParam value1)
  {
    WriteData(BuildString(value0.ToString(), ConsoleSeparator, value1.ToString(), NewLine));
  }
  
  //***************************************************************************
  void Console::WriteLine(AnyParam value0, AnyParam value1, AnyParam value2)
  {
    StringBuilder builder;
    builder.Append(value0.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value1.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value2.ToString());
    builder.Append(NewLine);
    WriteData(builder.ToString());
  }
  
  //***************************************************************************
  void Console::WriteLine(AnyParam value0, AnyParam value1, AnyParam value2, AnyParam value3)
  {
    StringBuilder builder;
    builder.Append(value0.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value1.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value2.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value3.ToString());
    builder.Append(NewLine);
    WriteData(builder.ToString());
  }
  
  //***************************************************************************
  void Console::WriteLine(AnyParam value0, AnyParam value1, AnyParam value2, AnyParam value3, AnyParam value4)
  {
    StringBuilder builder;
    builder.Append(value0.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value1.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value2.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value3.ToString());
    builder.Append(ConsoleSeparator);
    builder.Append(value4.ToString());
    builder.Append(NewLine);
    WriteData(builder.ToString());
  }
  
  //***************************************************************************
  void Console::WriteLine(StringParam value)
  {
    WriteData(BuildString(value, NewLine));
  }
  
  //***************************************************************************
  void Console::WriteLine(StringRange value)
  {
    WriteData(BuildString(value, NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(cstr value)
  {
    WriteData(BuildString(String(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(char value)
  {
    WriteData(BuildString(String(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Integer value)
  {
    WriteData(BuildString(IntegerToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Integer2Param value)
  {
    WriteData(BuildString(Integer2ToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Integer3Param value)
  {
    WriteData(BuildString(Integer3ToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Integer4Param value)
  {
    WriteData(BuildString(Integer4ToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(DoubleInteger value)
  {
    WriteData(BuildString(DoubleIntegerToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Boolean value)
  {
    WriteData(BuildString(BooleanToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Boolean2Param value)
  {
    WriteData(BuildString(Boolean2ToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Boolean3Param value)
  {
    WriteData(BuildString(Boolean3ToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Boolean4Param value)
  {
    WriteData(BuildString(Boolean4ToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Real value)
  {
    WriteData(BuildString(RealToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(DoubleReal value)
  {
    WriteData(BuildString(DoubleRealToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Real2Param value)
  {
    WriteData(BuildString(Real2ToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Real3Param value)
  {
    WriteData(BuildString(Real3ToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(Real4Param value)
  {
    WriteData(BuildString(Real4ToString(value), NewLine));
  }

  //***************************************************************************
  void Console::WriteLine(QuaternionParam value)
  {
    WriteData(BuildString(QuaternionToString(value), NewLine));
  }

  //***************************************************************************
  void Console::DumpValue(StringBuilderExtended& builder, Type* type, const byte* value, Integer howDeep, Integer currentDepth)
  {
    // First write the generic value of the type...
    String baseValueString = type->GenericToString(value);
    builder.WriteLine(baseValueString);

    // If this is the level of depth we wanted to traverse, exit out
    if (currentDepth == howDeep)
      return;

    // If this is a bound type, lets print out its property/member values
    BoundType* boundType = Type::DynamicCast<BoundType*>(type);
    if (boundType != nullptr)
    {
      byte* instanceData = boundType->GenericGetMemory(value);
      if (instanceData)
      {
        // Loop through all the instance properties
        PropertyArray& properties = boundType->AllProperties;
        for (size_t i = 0; i < properties.Size(); ++i)
        {
          // Grab the current property
          Property* property = properties[i];

          // If it's a static or hidden property, skip it
          if (property->IsStatic || property->IsHidden)
            continue;

          // TEMPORARY - Because we do not have the ExecutableState in bound C++ functions, we can only
          // print out fields (because we know their type and know where they exist in memory
          if (Field* field = Type::DynamicCast<Field*>(property))
          {
            // Write tabs up to the current depth
            for (Integer i = 0; i <= currentDepth; ++i)
              builder.Write("  ");

            // Write out the property name
            builder.Write(field->Name);
            builder.Write(": ");

            byte* fieldData = instanceData + field->Offset;
          
            // Recursively dumping will imediately print out the property value
            DumpValue(builder, field->PropertyType, fieldData, howDeep, currentDepth + 1);
          }
        }
      }
    }
  }

  //***************************************************************************
  void Console::DumpValue(AnyParam value)
  {
    DumpValue(value, 1);
  }

  //***************************************************************************
  void Console::DumpValue(AnyParam value, Integer howDeep)
  {
    // Create the string builder we'll use to fully build the object
    StringBuilderExtended builder;

    // Call the helper (recursive)
    DumpValue(builder, value.StoredType, value.Data, howDeep, 0);

    // Write out the debug text for the object
    String finalText = builder.ToString();
    Write(finalText);
  }
  
  //***************************************************************************
  String Console::ReadString()
  {
    // Just read the string directly (no interpreting, may be empty)
    return ReadData();
  }
  
  //***************************************************************************
  Integer Console::ReadInteger()
  {
    // Just read the string directly (may be empty)
    String readText = ReadData();

    // Convert the text to the value type
    Integer returnValue = 0;
    ToValue(readText, returnValue);

    // Return the resulting read value, or a default if we had no callback
    return returnValue;
  }

  //***************************************************************************
  Boolean Console::ReadBoolean()
  {
    // Just read the string directly (may be empty)
    String readText = ReadData();

    // Convert the text to the value type
    Boolean returnValue = false;
    ToValue(readText, returnValue);

    // Return the resulting read value, or a default if we had no callback
    return returnValue;
  }
  
  //***************************************************************************
  Real Console::ReadReal()
  {
    // Just read the string directly (may be empty)
    String readText = ReadData();

    // Convert the text to the value type
    Real returnValue = 0.0f;
    ToValue(readText, returnValue);

    // Return the resulting read value, or a default if we had no callback
    return returnValue;
  }
}
