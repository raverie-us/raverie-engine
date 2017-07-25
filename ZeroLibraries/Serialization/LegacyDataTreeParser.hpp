#pragma once

namespace Zero
{

class DataNode;
struct DataTreeContext;

//------------------------------------------------------------ Data Tree Builder
class LegacyDataTreeParser
{
public:
  static DataNode* BuildTree(DataTreeContext& context, StringRange data);
};

}//namespace Zero
