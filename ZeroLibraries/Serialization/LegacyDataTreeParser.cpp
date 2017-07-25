#include "Precompiled.hpp"
#include "LegacyDataTreeParser.hpp"
#include "DataTreeNode.hpp"
#include "Tokenizer.hpp"

namespace Zero
{

DataNode* ReadField(DataTreeContext& c, Tokenizer& tokenizer, TempToken token, DataNode* parent);
DataNode* ReadValue(DataTreeContext& c, Tokenizer& tokenizer, TempToken token, DataNode* parent);

//------------------------------------------------------------ Data Tree Builder
//******************************************************************************
DataNode* LegacyDataTreeParser::BuildTree(DataTreeContext& context, StringRange data)
{
  Tokenizer tokenizer;
  tokenizer.Load(data);

  TempToken token;
  tokenizer.ReadToken(token);

  return ReadField(context, tokenizer, token, nullptr);
}

//******************************************************************************
DataNode* ParseError(DataTreeContext& c, Tokenizer& t, cstr parseError)
{
  c.Error = true;
  c.Message = String::Format("Error parsing file '%s' on line %u. Error: %s",
    c.Filename.c_str(), t.CurrentLine(), parseError);
  ErrorIf(true, "Error parsing file '%s' on line %u. Error: %s",
    c.Filename.c_str(), t.CurrentLine(), parseError);
  return NULL;
}

//******************************************************************************
DataNode* ReadObject(DataTreeContext& c, Tokenizer& tokenizer, DataNode* parent)
{
  DataNode* object = new DataNode(DataNodeType::Object, parent);
  TempToken token;
  //keep reading until the ending }
  for(;;)
  {
    tokenizer.ReadToken(token);

    if(token.Type == TempToken::Symbol)
    {
      if(token.Text == '}')
        return object;//done

      if(token.Text == ',')
        continue;
    }
    //if we ever get an invalid token, bail
    else if(token.Type == TempToken::None)
    {
      return ParseError(c, tokenizer, "End of file found trying to read object.");
    }

    //read the next field (may be a property or even another object)
    DataNode* node = ReadField(c, tokenizer, token, object);

    //If there was an error return, what was parsed.
    if(node == NULL)
      return object;
  }
}

//******************************************************************************
DataNode* ReadArray(DataTreeContext& c, Tokenizer& tokenizer, DataNode* parent)
{
  DataNode* dataArray = new DataNode(DataNodeType::Object, parent);
  dataArray->mFlags.SetFlag(DataNodeFlags::Array);

  TempToken token;
  //read until the ending ]
  for(;;)
  {
    //Read the next value
    tokenizer.ReadToken(token);

    if(token.Type == TempToken::Symbol)
    {
      if(token.Text == ']')
        return dataArray;//done

                         //move to next token
      if(token.Text == ',')
        continue;
    }
    else if(token.Type == TempToken::None)
    {
      return ParseError(c, tokenizer, "End of file found trying to read array.");
    }

    //Should be a value to read in the array Try to read it
    DataNode* node = ReadValue(c, tokenizer, token, dataArray);
    if(node == NULL)
      return NULL;
  }
}

//******************************************************************************
DataNode* ReadField(DataTreeContext& c, Tokenizer& tokenizer, TempToken token, DataNode* parent)
{
  // A '-' before the type name means that we should remove the node
  bool removeNode = false;
  bool addedNode = false;

  if(token.Text == '-')
  {
    removeNode = true;
    c.PatchRequired = true;
    tokenizer.ReadToken(token);
  }

  // A '+' before the type name means that the object should be added to
  // the inherited data
  else if(token.Text == '+')
  {
    addedNode = true;
    c.PatchRequired = true;
    tokenizer.ReadToken(token);
  }

  if(!(token.Type == TempToken::Word || token.Type == TempToken::String))
  {
    return ParseError(c, tokenizer, "Objects can only contain name-value pairs. "
      "Name is not a string.");
  }

  // Read type name if available
  StringRange typeName = token.Text;
  if(token.Type != TempToken::Word)
    return ParseError(c, tokenizer, "Bad token while reading object. Expected type name.");

  // Convert to "Real2, Real3, ..."
  if (typeName == "Vec2")
    typeName = ZilchTypeId(Vec2)->Name;
  else if (typeName == "Vec3")
    typeName = ZilchTypeId(Vec3)->Name;
  else if (typeName == "Vec4")
    typeName = ZilchTypeId(Vec4)->Name;
  else if (typeName == "Quat")
    typeName = ZilchTypeId(Quat)->Name;

  // Check for a child id next to the type name
  Guid childId = PolymorphicNode::cInvalidUniqueNodeId;
  tokenizer.ReadToken(token);
  if(token.Text == ':')
  {
    tokenizer.ReadToken(token);

    if(token.Type != TempToken::Number)
      return ParseError(c, tokenizer, "Bad child id while reading object. Expected u64.");

    childId = ReadHexString(String(token.Text));
    tokenizer.ReadToken(token);
  }

  // Read the field name text
  StringRange valuename;
  if(token.Type == TempToken::Word || token.Type == TempToken::String)
  {
    valuename = token.Text;
    tokenizer.ReadToken(token);
  }

  // If it wasn't a subtractive node, look for a data inheritance id
  String inheritedId;
  if(token.Text == '(')
  {
    tokenizer.ReadToken(token);
    ErrorIf(token.Type != TempToken::String, "Inheritance id's must be strings");
    inheritedId = token.Text;

    // Read the closing ')'
    tokenizer.ReadToken(token);

    // Read next
    tokenizer.ReadToken(token);


    //inheritedId = tokenizer.ReadUntil(')');
    c.PatchRequired = true;

    //while(token.Text != ')')
    //  tokenizer.ReadToken(token);
    //
    //tokenizer.ReadToken(token);
  }

  // If it wasn't a subtractive node, look for a data inheritance id
  //String inheritedId;
  //if(token.Type == Token::String)
  //{
  //  inheritedId = token.Text;
  //  tokenizer.ReadToken(token);
  //}

  DataNode* newValue = nullptr;

  // If the node is being removed, we'll just create an empty node for it
  // so that patching knows to remove it
  if(removeNode)
  {
    if(token.Text != ',')
    {
      return ParseError(c, tokenizer, "The node was being marked for removal "
        "must be immediately closed with a comma.");
    }

    newValue = new DataNode(DataNodeType::Object, parent);
    newValue->mPatchState = PatchState::ShouldRemove;
  }
  else
  {

    // There has to be an = or : since the field is being assigned to
    if(!(token.Text == '=' || token.Text == ':'))
    {
      return ParseError(c, tokenizer, "Bad token after name in field on object. "
        "Needed '=' or ':'");
    }

    // Read the value to assign the field to
    tokenizer.ReadToken(token);
    newValue = ReadValue(c, tokenizer, token, parent);
  }

  // Failed to read value pass parse error up
  if(!newValue)
    return nullptr;

  newValue->mPropertyName = valuename;
  newValue->mTypeName = typeName;
  newValue->mFlags.SetState(DataNodeFlags::LocallyAdded, addedNode);
  if(newValue->mNodeType == DataNodeType::Object)
  {
    newValue->mInheritedFromId = inheritedId;
    newValue->mUniqueNodeId = childId;
  }

  return newValue;
}

//******************************************************************************
DataNode* ReadValue(DataTreeContext& c, Tokenizer& tokenizer, TempToken token, DataNode* parent)
{
  // Deal with invalid case
  if(token.Type == TempToken::None)
  {
    return ParseError(c, tokenizer, "End of file found trying to read value.");
  }

  // If this is a single character symbol type, check which kind of bracket it is
  if(token.Type == TempToken::Symbol)
  {
    if(token.Text == '[')
      return ReadArray(c, tokenizer, parent);
    else if(token.Text == '{')
      return ReadObject(c, tokenizer, parent);
    else
      return ParseError(c, tokenizer, "Read symbol while trying to read value.");
  }
  else
  {
    DataNode* value = new DataNode(DataNodeType::Value, parent);
    value->mTextValue = token.Text;
    return value;
  }
}

}//namespace Zero
