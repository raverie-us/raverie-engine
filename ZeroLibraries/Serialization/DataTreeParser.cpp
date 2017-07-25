////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//--------------------------------------------------------------------------------- Data Tree Parser
//**************************************************************************************************
DataNode* DataTreeParser::BuildTree(DataTreeContext& context, StringRange data)
{
  DataTreeParser parser(context);
  return parser.Parse(data);
}

//*************************************************************************************************
DataTreeParser::DataTreeParser(DataTreeContext& context) : 
  mContext(context),
  mRoot(nullptr),
  mCurrentIndex(0),
  mLastPoppedNode(nullptr)
{

}

//*************************************************************************************************
DataNode* DataTreeParser::Parse(StringRange text)
{
  // Read all tokens into an array
  DataTreeTokenizer tokenizer(text);
  Status status;
  DataToken token;
  while(tokenizer.ReadToken(token, status))
    mTokens.PushBack(token);

  // Return if it failed
  if(status.Failed())
  {
    mContext.Error = true;
    mContext.Message = status.Message;
    return nullptr;
  }

  Start();

  return mRoot;
}

//**************************************************************************************************
bool DataTreeParser::Start()
{
  // Accept any amount of attributes at the start of the file
  while(Attribute());

  // There must be at least one Object
  Expect(Object(), "There must be a single object in the file");

  return true;
}

//**************************************************************************************************
bool DataTreeParser::Object()
{
  // Example:
  /*
    Transform [LocallyAdded]
    {
      var Translation = Real3(0,0,0);
    }
  */

  // Object |= P("Identifier", P(Identifier)) << *Attribute << P(OpenCurley) << *((Property | Value | Object) << ~P(Comma)) << P(CloseCurley);

  // Objects must start with the type name
  if(Accept(DataTokenType::Identifier) == false)
    return false;

  // The object was started, so create a node
  DataNode* node = CreateNewNode(DataNodeType::Object);

  // Set the typename of the node
  node->mTypeName = GetLastAcceptedToken().mText;

  // Any amount of attributes can follow the type name
  while(Attribute());

  // Open the object
  Expect(DataTokenType::OpenCurley, "Objects must be opened with '{'");

  // Accept any amount of properties, values, or objects
  while(true)
  {
    bool acceptedChild = (Property() || Object() || Value());
    if(!acceptedChild)
      break;

    // We're considered an array if we have non-property values as children
    bool lastNodeIsValue = mLastPoppedNode->mNodeType == DataNodeType::Value;
    if(lastNodeIsValue && !mLastPoppedNode->mFlags.IsSet(DataNodeFlags::Property))
      node->mFlags.SetFlag(DataNodeFlags::Array);

    // We're adding an optional comma between each value to handle separated values in an array
    Accept(DataTokenType::Comma);
  }

  // We're done with this Object
  PopNode();

  // Close the object
  return Expect(DataTokenType::CloseCurley, "Objects must be closed with '}'");
}

//**************************************************************************************************
bool DataTreeParser::Attribute()
{
  // Example:
  /*
    [LocallyAdded]
    [ChildId:52]
  */

  // Attribute |= P(OpenBracket) << P("Name", P(Identifier)) << ~(P(Colon) << (P("Value", Value) | Object)) << P(CloseBracket);

  if(Accept(DataTokenType::OpenBracket) == false)
    return false;

  Expect(DataTokenType::Identifier, "Attributes must have an identifier after '['");

  StringRange attributeName = GetLastAcceptedToken().mText;
  StringRange attributeValue;

  // Optionally accept an attribute value
  if(Accept(DataTokenType::Colon))
  {
    // Don't create a node for the value
    Expect(Value(false), "Attributes must have a value after the ':'");
    attributeValue = GetLastAcceptedToken().mText;
  }

  // Add the attribute to the appropriate node
  if(DataNode* currentNode = GetCurrentNode())
  {
    currentNode->AddAttribute(attributeName, attributeValue);

    // Set node values based on the attribute
    if(attributeName == SerializationAttributes::Id)
    {
      ToValue(attributeValue, currentNode->mUniqueNodeId);
    }
    else if(attributeName == SerializationAttributes::InheritId)
    {
      currentNode->mInheritedFromId = attributeValue;
      mContext.PatchRequired = true;
    }
    else if(attributeName == SerializationAttributes::LocallyAdded)
    {
      currentNode->mFlags.SetFlag(DataNodeFlags::LocallyAdded);
      mContext.PatchRequired = true;
    }
    else if(attributeName == SerializationAttributes::LocallyRemoved)
    {
      currentNode->mPatchState = PatchState::ShouldRemove;
      mContext.PatchRequired = true;
    }
    else if(attributeName == SerializationAttributes::ChildOrderOverride)
    {
      currentNode->mFlags.SetFlag(DataNodeFlags::ChildOrderOverride);
      mContext.PatchRequired = true;
    }
  }
  else
  {
    // Add it to the root attributes
    mContext.Loader->mRootAttributes.PushBack(DataAttribute(attributeName, attributeValue));
  }

  return Expect(DataTokenType::CloseBracket, "Attributes must be closed with ']'");
}

//**************************************************************************************************
bool DataTreeParser::Property()
{
  // Example:
  /*
    var Visible = true
  */

  // Property |= P(Var) << P("Name", P(Identifier)) << P(Assignment) << (P("Value", Value) | Object);

  if(!Accept(DataTokenType::Var))
    return false;

  Expect(DataTokenType::Identifier, "Incomplete property. An identifier must come after 'var' ");

  String propertyName = GetLastAcceptedToken().mText;

  Expect(DataTokenType::Assignment, "A property must be assigned a value with '='");
  
  // Properties can only be values or objects
  if(!Value() && !Object())
  {
    mContext.Error = true;
    return false;
  }

  // This node will either be the value node or object node
  DataNode* lastNode = mLastPoppedNode;
  lastNode->mFlags.SetFlag(DataNodeFlags::Property);
  lastNode->mPropertyName = propertyName;

  return true;
}

//**************************************************************************************************
bool DataTreeParser::Value(bool createNode)
{
  // Value |= P("Value", P(Integer) | P(Float) | P(Hex) | P(StringLiteral) | P(Enum) | P(True) | P(False));
  return AcceptValue(createNode, DataTokenType::Integer)       ||
         AcceptValue(createNode, DataTokenType::Float)         ||
         AcceptValue(createNode, DataTokenType::Hex)           ||
         AcceptValue(createNode, DataTokenType::StringLiteral) ||
         AcceptValue(createNode, DataTokenType::Enumeration)   ||
         AcceptValue(createNode, DataTokenType::True)          ||
         AcceptValue(createNode, DataTokenType::False);
}

//*************************************************************************************************
bool DataTreeParser::Accept(DataTokenType::Enum token)
{
  if(mCurrentIndex >= mTokens.Size())
    return false;

  if(mTokens[mCurrentIndex].mType == token)
  {
    ++mCurrentIndex;
    return true;
  }

  return false;
}

//**************************************************************************************************
bool DataTreeParser::Expect(bool succeeded, cstr errorMessage)
{
  if(succeeded)
    return true;

  mContext.Error = true;
  Error(BuildString("Parsing error: ", errorMessage).c_str());
  return false;
}

//**************************************************************************************************
bool DataTreeParser::Expect(DataTokenType::Enum token, cstr errorMessage)
{
  return Expect(Accept(token), errorMessage);
}

//*************************************************************************************************
bool DataTreeParser::AcceptValue(bool createNode, DataTokenType::Enum tokenType)
{
  if(!Accept(tokenType))
    return false;

  if(!createNode)
    return true;
  
  DataNode* node = CreateNewNode(DataNodeType::Value);
  DataToken& token = GetLastAcceptedToken();

  node->mTextValue = token.mText;

  // Integer
  if(token.mType == DataTokenType::Integer)
  {
    node->mTypeName = Serialization::Trait<int>::TypeName();
  }
  // Hex
  else if(token.mType == DataTokenType::Hex)
  {
    node->mTypeName = Serialization::Trait<u64>::TypeName();
  }
  // Float
  else if(token.mType == DataTokenType::Float)
  {
    node->mTypeName = Serialization::Trait<float>::TypeName();
  }
  // Boolean
  else if(token.mType == DataTokenType::True || token.mType == DataTokenType::False)
  {
    node->mTypeName = Serialization::Trait<bool>::TypeName();
  }
  else if(token.mType == DataTokenType::StringLiteral)
  {
    node->mTypeName = ZilchTypeId(String)->Name;

    StringBuilder builder;
    StringRange text = token.mText;

    // Remove all escaped slashes and quotes that were added when saved
    while(!text.Empty())
    {
      Rune rune = text.Front();
      
      // We escaped all slashes when saving, so we need to remove them
      if(rune == '\\')
      {
        text.PopFront();

        // Temporary solution, only do this extra step if the next character is 
        // either a slash or a quote (the special case we're trying to solve here)
        // If we updated all text files, this check would not be needed
        Rune next = text.Front();
        if(next == '\\' || next == '\"')
        {
          rune = next;
        }
        else
        {
          builder.Append(rune);
          rune = next;
        }
      }

      builder.Append(rune);
      text.PopFront();
    }

    node->mTextValue = builder.ToString();
  }
  // Enum
  else if(token.mType == DataTokenType::Enumeration)
  {
    // The enum comes in as 'Type.Value' (ie. 'LightType.PointLight'), so we need
    // to separate the type name from the value
    StringTokenRange r(token.mText, '.');
    node->mTypeName = r.Front();
    r.PopFront();
    node->mTextValue = r.Front();
    node->mFlags.SetFlag(DataNodeFlags::Enumeration);
  }

  PopNode();

  return true;
}

//**************************************************************************************************
DataNode* DataTreeParser::CreateNewNode(DataNodeType::Enum nodeType)
{
  DataNode* newNode =  new DataNode(nodeType, GetCurrentNode());
  mNodeStack.PushBack(newNode);

  if(mRoot == nullptr)
    mRoot = newNode;

  return newNode;
}

//**************************************************************************************************
void DataTreeParser::PopNode()
{
  mLastPoppedNode = mNodeStack.Back();
  mNodeStack.PopBack();
}

//**************************************************************************************************
DataNode* DataTreeParser::GetCurrentNode()
{
  if(mNodeStack.Empty())
    return nullptr;
  return mNodeStack.Back();
}

//**************************************************************************************************
DataToken& DataTreeParser::GetLastAcceptedToken()
{
  return mTokens[mCurrentIndex - 1];
}

}//namespace Zero
