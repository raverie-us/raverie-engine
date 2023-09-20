// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
// Node Print Info

class NodePrintInfo
{
public:
  RaverieDeclareType(NodePrintInfo, TypeCopyMode::ReferenceType);
  NodePrintInfo(int level, const String& name, int ID, bool hasOutput, HandleOf<SoundNode> node) :
      mLevel(level), mName(name), mHasOutput(hasOutput), mMoved(false), mPositionSet(false), mPosition(Vec2(0.0f, 0.0f)), mID(ID), mNode(node)
  {
  }

  Array<NodePrintInfo*>::range GetConnections()
  {
    return mChildren.All();
  }

  int mLevel;
  Array<NodePrintInfo*> mParents;
  Array<NodePrintInfo*> mChildren;
  String mName;
  Vec2 mPosition;
  bool mHasOutput;
  bool mMoved;
  bool mPositionSet;
  int mID;
  HandleOf<SoundNode> mNode;
};

typedef Array<NodePrintInfo*> NodeInfoListType;

// Sound Node Graph

class SoundNodeGraph
{
public:
  SoundNodeGraph() : mNodeWidth(100), mNodeHeight(100), mMaxLevel(0)
  {
  }
  ~SoundNodeGraph();

  NodeInfoListType::range GetNodeInfoList();

private:
  NodeInfoListType mNodeInfoList;
  typedef HashMap<unsigned, NodePrintInfo*> NodeMapType;
  NodeMapType mNodeMap;
  unsigned mNodeWidth;
  unsigned mNodeHeight;
  int mMaxLevel;

  void CreateInfo(HandleOf<SoundNode> node, HandleOf<SoundNode> outputNode, int level);
  void CheckForCollision(NodeInfoListType& list, bool checkOrphanNodes);
  void AddSpacePadding(NodePrintInfo* node, float addToXPos);
  void FirstPassPositioning(Array<NodeInfoListType>& infoByLevel, int largestLevel);
  void SecondPassPositioning(Array<NodeInfoListType>& infoByLevel, int largestLevel);
  void PositionCleanUp(Array<NodeInfoListType>& infoByLevel, int largestLevel);
};

} // namespace Raverie
