// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class DataNode;
struct DataTreeContext;

class LegacyDataTreeParser
{
public:
  static DataNode* BuildTree(DataTreeContext& context, StringRange data);
};

} // namespace Raverie
