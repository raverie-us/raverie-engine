// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class DataNode;
struct DataTreeContext;

// Data Tree Parser
class DataTreeParser
{
public:
  static bool BuildTree(DataTreeContext& context, StringRange data, DataNode* fileRoot);

private:
  DataTreeParser(DataTreeContext& context);

  bool Parse(StringRange text, DataNode* fileRoot);

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

  DataNode* mLastPoppedNode;
  Array<DataNode*> mNodeStack;

  uint mCurrentIndex;
  Array<DataToken> mTokens;
  DataTreeContext& mContext;
};

} // namespace Zero
