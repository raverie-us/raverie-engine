////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class DataNode;
struct DataTreeContext;

//--------------------------------------------------------------------------------- Data Tree Parser
class DataTreeParser
{
public:
  static DataNode* BuildTree(DataTreeContext& context, StringRange data);

private:
  DataTreeParser(DataTreeContext& context);

  DataNode* Parse(StringRange text);

  bool Start();
  bool Object();
  bool Attribute();
  bool Property();
  bool Value(bool createNode = true);

  bool Accept(DataTokenType::Enum token);
  bool Expect(bool succeeded, cstr errorMessage);
  bool Expect(DataTokenType::Enum token, cstr errorMessage);
  bool AcceptValue(bool createNode, DataTokenType::Enum tokenType);

  DataNode* CreateNewNode(DataNodeType::Enum nodeType);
  void PopNode();
  DataNode* GetCurrentNode();
  DataToken& GetLastAcceptedToken();

  DataNode* mRoot;
  DataNode* mLastPoppedNode;
  Array<DataNode*> mNodeStack;

  uint mCurrentIndex;
  Array<DataToken> mTokens;
  DataTreeContext& mContext;
};

}//namespace Zero
