/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  JsonBuilder::JsonBuilder() :
    IsMember(false),
    IsWrittenTo(false),
    IsCompactMode(false)
  {
  }

  //***************************************************************************
  JsonMember::JsonMember() :
    Value(nullptr)
  {
  }

  //***************************************************************************
  JsonMember::~JsonMember()
  {
    delete this->Value;
  }

  //***************************************************************************
  JsonValue::JsonValue() :
    Type(JsonValueType::Invalid),
    RealValue(0.0),
    IntegralValue(0)
  {
  }
  
  //***************************************************************************
  String JsonValue::ToString()
  {
    JsonBuilder builder;
    builder.WriteTree(this);
    return builder.ToString();
  }
    
  //***************************************************************************
  JsonValue* JsonValue::GetMember(StringParam name, JsonErrorMode::Enum errorMode)
  {
    ErrorIf(this->Type != JsonValueType::Object && errorMode == JsonErrorMode::ReportError,
      "This value was not an object type, and therefore cannot have members");

    JsonValue* value = this->Members.FindValue(name, nullptr);

    ErrorIf(value == nullptr && errorMode == JsonErrorMode::ReportError,
      "Unable to find json member by the name of '%s'", name.c_str());

    return value;
  }
  
  //***************************************************************************
  JsonValue* JsonValue::IndexValue(size_t index, JsonErrorMode::Enum errorMode)
  {
    ErrorIf(this->Type != JsonValueType::Array && errorMode == JsonErrorMode::ReportError,
      "This value was not an array type, and therefore cannot be indexed");

    ReturnIf(index >= this->ArrayElements.Size(), nullptr,
      "The index given was outside the range provided by the array");
    
    return this->ArrayElements[index];
  }

  //***************************************************************************
  bool JsonValue::AsBool(bool defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (this->Type == JsonValueType::True)
    {
      return true;
    }
    else if (this->Type == JsonValueType::False)
    {
      return false;
    }
    else if (errorMode == JsonErrorMode::ReportError)
    {
      Error("The json value was not a bool value");
    }

    return defaultValue;
  }

  //***************************************************************************
  String JsonValue::AsString(StringParam defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (this->Type == JsonValueType::String)
    {
      return this->StringValue;
    }
    else if (errorMode == JsonErrorMode::ReportError)
    {
      Error("The json value was not a String value");
    }

    return defaultValue;
  }

  //***************************************************************************
  double JsonValue::AsDouble(double defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (this->Type == JsonValueType::Real)
    {
      return this->RealValue;
    }
    else if (errorMode == JsonErrorMode::ReportError)
    {
      Error("The json value was not a Real value");
    }

    return defaultValue;
  }

  //***************************************************************************
  float JsonValue::AsFloat(float defaultValue, JsonErrorMode::Enum errorMode)
  {
    return (float)AsDouble(defaultValue, errorMode);
  }

  //***************************************************************************
  long long JsonValue::AsLongLong(long long defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (this->Type == JsonValueType::Integer)
    {
      return this->IntegralValue;
    }
    else if (errorMode == JsonErrorMode::ReportError)
    {
      Error("The json value was not an Integer value");
    }

    return defaultValue;
  }

  //***************************************************************************
  int JsonValue::AsInteger(int defaultValue, JsonErrorMode::Enum errorMode)
  {
    return (int)AsLongLong(defaultValue, errorMode);
  }

  //***************************************************************************
  bool JsonValue::MemberAsBool(StringParam name, bool defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = GetMember(name, errorMode))
      return value->AsBool(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  String JsonValue::MemberAsString(StringParam name, StringParam defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = GetMember(name, errorMode))
      return value->AsString(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  double JsonValue::MemberAsDouble(StringParam name, double defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = GetMember(name, errorMode))
      return value->AsDouble(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  long long JsonValue::MemberAsLongLong(StringParam name, long long defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = GetMember(name, errorMode))
      return value->AsLongLong(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  int JsonValue::MemberAsInteger(StringParam name, int defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = GetMember(name, errorMode))
      return value->AsInteger(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  float JsonValue::MemberAsFloat(StringParam name, float defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = GetMember(name, errorMode))
      return value->AsFloat(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  bool JsonValue::IndexAsBool(size_t index, bool defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = IndexValue(index, errorMode))
      return value->AsBool(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  String JsonValue::IndexAsString(size_t index, StringParam defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = IndexValue(index, errorMode))
      return value->AsString(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  double JsonValue::IndexAsDouble(size_t index, double defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = IndexValue(index, errorMode))
      return value->AsDouble(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  long long JsonValue::IndexAsLongLong(size_t index, long long defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = IndexValue(index, errorMode))
      return value->AsLongLong(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  int JsonValue::IndexAsInteger(size_t index, int defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = IndexValue(index, errorMode))
      return value->AsInteger(defaultValue, errorMode);
    return defaultValue;
  }

  //***************************************************************************
  float JsonValue::IndexAsFloat(size_t index, float defaultValue, JsonErrorMode::Enum errorMode)
  {
    if (JsonValue* value = IndexValue(index, errorMode))
      return value->AsFloat(defaultValue, errorMode);
    return defaultValue;
  }
  
  //***************************************************************************
  JsonValue* JsonReader::ReadIntoTreeFromFile(CompilationErrors& errors, StringParam fileName, void* userData)
  {
    Status status;
    String json = Project::ReadTextFile(status, fileName);
    
    if (status.Failed())
      return nullptr;

    return ReadIntoTreeFromString(errors, json, fileName, userData);
  }

  //***************************************************************************
  JsonValue* JsonReader::ReadIntoTreeFromString(CompilationErrors& errors, StringParam json, StringParam origin, void* userData)
  {
    Array<UserToken> tokens;
    Array<UserToken> comments;

    Tokenizer tokenizer(errors);
    tokenizer.EnableStringInterpolation = false;

    CodeEntry entry;
    entry.Code = json;
    entry.CodeUserData = userData;
    entry.Origin = origin;

    if (tokenizer.Parse(entry, tokens, comments) == false)
      return nullptr;

    UniquePointer<JsonValue> root;

    Array<JsonValue*> objectArrayStack;

    // Normally we'd have to worry about this being a pointer to an element in an array that's resizing
    // however, this will only ever point to the back element and therefore will be valid
    JsonMember* member = nullptr;
    bool foundColon = false;
    bool isNegative = false;

    for (size_t i = 0; i < tokens.Size(); ++i)
    {
      UserToken& token = tokens[i];

      JsonValue* createdValue = nullptr;

      if (isNegative && token.TokenId != Grammar::IntegerLiteral && token.TokenId != Grammar::RealLiteral)
      {
        errors.Raise(token.Location, ErrorCode::GenericError, "The negative sign must be followed by an Integer or Real literal");
        return nullptr;
      }

      switch(token.TokenId)
      {
        case Grammar::True:
        {
          createdValue = new JsonValue();
          createdValue->Type = JsonValueType::True;
        }
        break;

        case Grammar::False:
        {
          createdValue = new JsonValue();
          createdValue->Type = JsonValueType::False;
        }
        break;

        case Grammar::Null:
        {
          createdValue = new JsonValue();
          createdValue->Type = JsonValueType::Null;
        }
        break;

        case Grammar::Negative:
        {
          isNegative = true;
          break;
        }

        case Grammar::IntegerLiteral:
        {
          createdValue = new JsonValue();
          createdValue->Type = JsonValueType::Integer;
          Zero::ToValue(token.Token, createdValue->IntegralValue);

          if (isNegative)
          {
            createdValue->IntegralValue = -createdValue->IntegralValue;
            isNegative = false;
          }
        }
        break;

        case Grammar::RealLiteral:
        {
          createdValue = new JsonValue();
          createdValue->Type = JsonValueType::Real;
          Zero::ToValue(token.Token, createdValue->RealValue);
          
          if (isNegative)
          {
            createdValue->RealValue = -createdValue->RealValue;
            isNegative = false;
          }
        }
        break;

        case Grammar::StringLiteral:
        {
          String strValue = ReplaceStringEscapesAndStripQuotes(token.Token);

          // If we're inside an object and not a member...
          if (objectArrayStack.Empty() == false && objectArrayStack.Back()->Type == JsonValueType::Object && member == nullptr)
          {
            // We're starting a member!
            member = new JsonMember();
            objectArrayStack.Back()->OrderedMembers.PushBack(member);
            member->Key = strValue;
          }
          else
          {
            createdValue = new JsonValue();
            createdValue->Type = JsonValueType::String;
            createdValue->StringValue = strValue;
          }
        }
        break;

        case Grammar::NameSpecifier:
        {
          if (member == nullptr)
          {
            errors.Raise(token.Location, ErrorCode::GenericError, "Invalid member ':' found");
            return nullptr;
          }

          foundColon = true;
        }
        break;

        case Grammar::BeginScope:
        {
          createdValue = new JsonValue();
          createdValue->Type = JsonValueType::Object;
        }
        break;

        case Grammar::BeginIndex:
        {
          createdValue = new JsonValue();
          createdValue->Type = JsonValueType::Array;
        }
        break;

        case Grammar::EndScope:
        case Grammar::EndIndex:
        {
          if (objectArrayStack.Empty())
          {
            errors.Raise(token.Location, ErrorCode::GenericError, "Unexpected end of an object or array");
            return nullptr;
          }

          objectArrayStack.PopBack();
        }
        break;

        case Grammar::ArgumentSeparator:
        {
          if (objectArrayStack.Empty())
          {
            errors.Raise(token.Location, ErrorCode::GenericError, "Unexpected argument separator");
            return nullptr;
          }
        }
        break;

        default:
        {
          errors.Raise(token.Location, ErrorCode::GenericError, "Unexpected token type");
          return nullptr;
        }
      }
        
      if (createdValue != nullptr)
      {
        if (member != nullptr)
        {
          if (foundColon == false)
          {
            errors.Raise(token.Location, ErrorCode::GenericError, "A colon ':' was not found when specifying the value");
            return nullptr;
          }

          foundColon = false;
          member->Value = createdValue;
          objectArrayStack.Back()->Members.Insert(member->Key, member->Value);
          member = nullptr;
        }
        else
        {
          if (objectArrayStack.Empty())
          {
            if(createdValue->Type != JsonValueType::Object)
            {
              errors.Raise(token.Location, ErrorCode::GenericError, "The root of a Json tree must be an object");
              return nullptr;
            }

            root = createdValue;
          }
          else
          {
            JsonValue* array = objectArrayStack.Back();

            if(array->Type != JsonValueType::Array)
            {
              errors.Raise(token.Location, ErrorCode::GenericError, "We can only add values to an array (not to an object unless we use the member syntax)");
              return nullptr;
            }

            array->ArrayElements.PushBack(createdValue);
          }
        }

        if (createdValue->Type == JsonValueType::Object || createdValue->Type == JsonValueType::Array)
        {
          objectArrayStack.PushBack(createdValue);
        }
      }
    }

    return root.Release();
  }

  //***************************************************************************
  String JsonBuilder::ToString() const
  {
    // Error checking
    ErrorIf(this->Stack.Size() != 0,
      "The resulting Json object will be incomplete");

    // Output the final string
    return this->Builder.ToString();
  }

  //***************************************************************************
  void JsonBuilder::WriteTree(JsonValue* value)
  {
    switch (value->Type)
    {
      case JsonValueType::True:
        this->Value(true);
        break;
      case JsonValueType::False:
        this->Value(false);
        break;
      case JsonValueType::Null:
        this->Null();
        break;
      case JsonValueType::String:
        this->Value(value->StringValue);
        break;
      case JsonValueType::Integer:
        this->Value(value->IntegralValue);
        break;
      case JsonValueType::Real:
        this->Value(value->RealValue);
        break;
      case JsonValueType::Object:
        this->Begin(JsonType::Object);
        for (size_t i = 0; i < value->OrderedMembers.Size(); ++i)
        {
          JsonMember* member = value->OrderedMembers[i];
          this->Key(member->Key);
          this->WriteTree(member->Value);
        }
        this->End();
        break;
      case JsonValueType::Array:
        this->Begin(JsonType::ArrayMultiLine);
        for (size_t i = 0; i < value->ArrayElements.Size(); ++i)
        {
          JsonValue* element = value->ArrayElements[i];
          this->WriteTree(element);
        }
        this->End();
        break;
      default:
        Error("Invalid json value type");
        break;
    }
  }

  //***************************************************************************
  void JsonBuilder::Key(StringRange name)
  {
    // If the stack size is zero...
    ReturnIf(this->Stack.Size() == 0,,
      "You must be in the middle of an object to start a key/member (currently at the root!)");

    // Error checking
    ErrorIf(this->Stack.Back() != JsonType::Object,
      "You must be in the middle of an object to start a key/member");

    // Error checking
    ErrorIf(this->IsMember,
      "A member of the object was already started");

    // Make sure to Append a trailing comma if it's needed (after the last value)
    this->AttemptComma();

    this->AttemptNewline();

    // Append the quoted name with a following ':'
    this->Builder.Write("\"");
    this->Builder.Write(name);
    this->Builder.Write("\":");

    if (this->IsCompactMode == false)
    {
      this->Builder.Write(" ");
    }

    // We're now writing to a member
    this->IsMember = true;

    // Mark that we have yet to write anything since we started a new member
    this->IsWrittenTo = false;
  }

  //***************************************************************************
  void JsonBuilder::Value(int value)
  {
    this->RawValue(String::Format("%d", value));
  }

  //***************************************************************************
  void JsonBuilder::Value(unsigned int value)
  {
    this->RawValue(String::Format("%u", value));
  }

  //***************************************************************************
  void JsonBuilder::Value(long long value)
  {
    this->RawValue(String::Format("%lld", value));
  }

  //***************************************************************************
  void JsonBuilder::Value(long value)
  {
    this->RawValue(String::Format("%ld", value));
  }
  
  //***************************************************************************
  void JsonBuilder::Value(unsigned long value)
  {
    this->RawValue(String::Format("%lu", value));
  }

  //***************************************************************************
  void JsonBuilder::Value(unsigned long long value)
  {
    this->RawValue(String::Format("%llu", value));
  }

  //***************************************************************************
  void JsonBuilder::Value(double value)
  {
    this->RawValue(String::Format("%f", value));
  }

  //***************************************************************************
  void JsonBuilder::Value(cstr value)
  {
    this->Value(StringRange(value));
  }

  //***************************************************************************
  void JsonBuilder::Value(StringRange value)
  {
    // We need to build an escaped string
    StringBuilder escapedString;

    // Start with quotes on the front (and we'll add one to the end later)
    escapedString.Append("\"");

    // Loop through all the characters in the string
    while (!value.Empty())
    {
      // Get the current character
      Rune r = value.Front();

      // Based on the character type...
      switch (r.mValue.value)
      {
      case '\\':
        // Escape the escape character
        escapedString.Append("\\\\");
        break;
      case '\n':
        // Escape the newline character
        escapedString.Append("\\n");
        break;
      case '\r':
        // Escape the carriage return character
        escapedString.Append("\\r");
        break;
      case '"':
        // Escape the quote character
        escapedString.Append("\\\"");
        break;
      case '\0':
        // Escape the quote character
        escapedString.Append("\\u0000");
        break;
      default:
        // Just Append the character like normal
        escapedString.Append(r.mValue);
        break;
      }
      value.PopBack();
    }

    // Finish with an ending quote
    escapedString.Append("\"");

    // Get the resulting string we just built
    String result = escapedString.ToString();

    // Set the string as a value
    this->RawValue(result);
  }

  //***************************************************************************
  void JsonBuilder::Value(Boolean value)
  {
    // Write true or false dependent upon the value
    if (value)
    {
      this->RawValue("true");
    }
    else
    {
      this->RawValue("false");
    }
  }

  //***************************************************************************
  void JsonBuilder::Null()
  {
      this->RawValue("null");
  }

  //***************************************************************************
  void JsonBuilder::AttemptComma()
  {
    // If we're at the root, no comma is needed
    if (this->Stack.Size() == 0)
    {
      return;
    }

    // If we're not written to
    if (this->IsWrittenTo)
    {
      this->Builder.Write(",");

      if (this->IsCompactMode == false)
      {
        this->Builder.Write(" ");
      }
    }
  }

  //***************************************************************************
  void JsonBuilder::AttemptNewline()
  {
    if (this->IsCompactMode == false)
    {
      // Get the last thing (object or array) that we started
      if (this->Stack.Empty() || this->Stack.Back() != JsonType::ArraySingleLine)
      {
        this->Builder.WriteLine();

        for (size_t i = 0; i < this->Stack.Size(); ++i)
        {
          this->Builder.Write("  ");
        }
      }
    }
  }
  
  //***************************************************************************
  void JsonBuilder::RawValue(StringParam value)
  {
    // Error checking
    this->VerifyCanWriteValue();

    // Make sure to Append a trailing comma if it's needed (after the last value)
    this->AttemptComma();

    if (this->IsMember == false)
    {
      this->AttemptNewline();
    }

    // Add the value to the builder
    this->Builder.Append(value);

    // For the node above us, we've always written to it when we get here
    this->IsWrittenTo = true;

    // No matter what, we either just wrote an array value or finished writing to a member
    this->IsMember = false;
  }

  //***************************************************************************
  void JsonBuilder::Begin(JsonType::Enum type)
  {
    // Error checking
    this->VerifyCanWriteValue();

    // Make sure to Append a trailing comma if it's needed (after the last value)
    this->AttemptComma();
    
    if (!(type == JsonType::ArraySingleLine && this->IsMember))
    {
      this->AttemptNewline();
    }
    
    // Push the type on the stack so we know later what we're doing
    this->Stack.PushBack(type);

    // If the type is an object...
    if (type == JsonType::Object)
    {
      // Start the object
      this->Builder.Write("{");
    }
    // Otherwise it's an array
    else
    {
      // Start the array
      this->Builder.Write("[");
    }

    // Mark that we have yet to write anything
    this->IsWrittenTo = false;

    // No matter what, we either just wrote an array value or finished writing to a member
    this->IsMember = false;
  }

  //***************************************************************************
  void JsonBuilder::End()
  {
    // Error checking
    ReturnIf(this->Stack.Empty(),,
      "There is no object or array to end (End was called one too many times)");

    // Get what we're in the top of the stack
    JsonType::Enum type = this->Stack.Back();

    // Push the type on the stack so we know later what we're doing
    this->Stack.PopBack();

    if (type != JsonType::ArraySingleLine)
    {
      this->AttemptNewline();
    }

    // If the type is an object...
    if (type == JsonType::Object)
    {
      // End the object
      this->Builder.Write("}");
    }
    // Otherwise it's an array
    else
    {
      // End the array
      this->Builder.Write("]");
    }

    // For the node above us, we've always written to it when we get here
    this->IsWrittenTo = true;
  }

  //***************************************************************************
  void JsonBuilder::VerifyCanWriteValue()
  {
    // We can always write a value if we are at the root
    if (this->Stack.Size() == 0)
    {
      // We can't write to the root twice...
      ErrorIf(this->Builder.GetSize() != 0,
        "Attempting to write to the root when something was already written");
      
      // Early out since there's nothing else to check
      return;
    }

    // Get the last thing (object or array) that we started
    JsonType::Enum last = this->Stack.Back();

    // Verify that, if we're in the middle of an object, it must be as a member
    ErrorIf(last == JsonType::Object && this->IsMember == false,
      "You must make a member in order to add values inside of an object");
  }
}