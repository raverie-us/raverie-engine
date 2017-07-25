/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_JSON_HPP
#define ZILCH_JSON_HPP

namespace Zilch
{
  namespace JsonValueType
  {
    enum Enum
    {
      Invalid,
      String,
      Integer,
      Real,
      Object,
      Array,
      True,
      False,
      Null
    };
  }

  class JsonValue;

  // A member is basically just a key-value pair, where
  // the key is always a string and the value is generic
  class ZeroShared JsonMember
  {
  public:
    // Constructor
    JsonMember();

    // Destructor (cleans up value)
    ~JsonMember();

    // Effectively the name of the member
    String Key;

    // The value of our member
    JsonValue* Value;
  };

  namespace JsonErrorMode
  {
    enum Enum
    {
      DefaultValue,
      ReportError
    };
  }

  // A 'value' in Json can be a string, number, object, array, true/false, or null
  class ZeroShared JsonValue
  {
  public:

    // Constructor
    JsonValue();

    // Converts the value and it's children into Json
    String ToString();

    // Get a member (reports errors if this is not an object, or if the member doesn't exist)
    JsonValue* GetMember(StringParam name, JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);

    // Index a value of an array (reports errors if this is not an array, or if the index is out of bounds)
    JsonValue* IndexValue(size_t index, JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);

    // Helper functions for pulling native types out of a json value
    bool      AsBool    (bool         defaultValue = false,     JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    String    AsString  (StringParam  defaultValue = String(),  JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    double    AsDouble  (double       defaultValue = 0.0,       JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    float     AsFloat   (float        defaultValue = 0.0f,      JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    long long AsLongLong(long long    defaultValue = 0LL,       JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    int       AsInteger (int          defaultValue = 0,         JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);

    // Helper functions for pulling out members of a json object
    // Only valid when the 'Type' is set to 'Object'
    bool      MemberAsBool    (StringParam name, bool         defaultValue = false,     JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    String    MemberAsString  (StringParam name, StringParam  defaultValue = String(),  JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    double    MemberAsDouble  (StringParam name, double       defaultValue = 0.0,       JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    float     MemberAsFloat   (StringParam name, float        defaultValue = 0.0f,      JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    long long MemberAsLongLong(StringParam name, long long    defaultValue = 0LL,       JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    int       MemberAsInteger (StringParam name, int          defaultValue = 0,         JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);

    // Helper functions for indexing values of a json array
    // Only valid when the 'Type' is set to 'Array'
    bool      IndexAsBool     (size_t index, bool         defaultValue = false,     JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    String    IndexAsString   (size_t index, StringParam  defaultValue = String(),  JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    double    IndexAsDouble   (size_t index, double       defaultValue = 0.0,       JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    float     IndexAsFloat    (size_t index, float        defaultValue = 0.0f,      JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    long long IndexAsLongLong (size_t index, long long    defaultValue = 0LL,       JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);
    int       IndexAsInteger  (size_t index, int          defaultValue = 0,         JsonErrorMode::Enum errorMode = JsonErrorMode::ReportError);

    // Specifies what type of value this is
    JsonValueType::Enum Type;

    // Only valid when the 'Type' is set to 'String'
    String StringValue;

    // Only valid when the 'Type' is set to 'Real'
    double RealValue;

    // Only valid when the 'Type' is set to 'Integer'
    long long IntegralValue;

    // Only valid when the 'Type' is set to 'Object'
    OwnedArray<JsonMember*> OrderedMembers;

    // A map of names to the values
    // Only valid when the 'Type' is set to 'Object'
    HashMap<String, JsonValue*> Members;

    // Only valid when the 'Type' is set to 'Array'
    OwnedArray<JsonValue*> ArrayElements;
  };

  class ZeroShared JsonReader
  {
  public:

    // Reads a json text file into a tree format
    static JsonValue* ReadIntoTreeFromString(CompilationErrors& errors, StringParam json, StringParam origin, void* userData);
    
    // Reads json text into a tree format
    static JsonValue* ReadIntoTreeFromFile(CompilationErrors& errors, StringParam fileName, void* userData);
  };

  namespace JsonType
  {
    enum Enum
    {
      ArraySingleLine,
      ArrayMultiLine,
      Object
    };
  }

  class ZeroShared JsonBuilder
  {
  public:

    // Constructor
    JsonBuilder();

    // Get the resulting Json (only legal if it's completely closed)
    String ToString() const;

    // Writes the json tree out
    void WriteTree(JsonValue* value);

    // Start a key/member inside of an object (illegal to do at the root or inside an array)
    void Key(StringRange name);

    // Write a value to Json (only legal as a member or in an array)
    void Value(int value);
    void Value(unsigned int value);
    void Value(long value);
    void Value(unsigned long value);
    void Value(long long value);
    void Value(unsigned long long value);
    void Value(double value);
    void Value(StringRange value);
    void Value(cstr value);
    void Value(bool value);
    void Null();

    // The string provided is written exactly to the Json object as a value
    // Only legal as a member or in an array
    void RawValue(StringParam value);

    // Start either a Json object or array
    // When inside an object, it's only legal to call this after creating a 'Member'
    // When inside an array, it is always legal to call this
    void Begin(JsonType::Enum type);

    // End a Json object or array
    void End();

  private:

    // Checks if we can write a value (includes object or array)
    void VerifyCanWriteValue();

    // Returns an empty string or a string containing a
    // comma if it's needed to add a value/member for the current object or array
    void AttemptComma();

    void AttemptNewline();

  public:

    // In compact mode, we only output necessary spaces
    bool IsCompactMode;

  private:

    // Lets us know what's the last operation of the Json stack was
    Array<JsonType::Enum> Stack;

    // Lets us know if we started a member
    // This is only used when a Json object was the last thing on the stack
    bool IsMember;

    // Whether or not we wrote to the current object/array
    bool IsWrittenTo;

    // Allow us to efficiently build the json file
    StringBuilderExtended Builder;
  };
}

#endif
