///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
//---------------------------------------------------------------------------------- Node Print Info

class NodePrintInfo
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  NodePrintInfo(int level, const String& name, bool hasOutput) : 
    mLevel(level),
    mName(name),
    mHasOutput(hasOutput),
    mMoved(false), 
    mPositionSet(false) 
  {}

  Array<NodePrintInfo*>::range GetConnections() { return mChildren.All(); }

  int mLevel;
  Array<NodePrintInfo*> mParents;
  Array<NodePrintInfo*> mChildren;
  String mName;
  Vec2 mPosition;
  float mConnectAvgPos;
  bool mHasOutput;
  bool mMoved;
  bool mPositionSet;
};

typedef Array<NodePrintInfo*> NodeInfoListType;

//----------------------------------------------------------------------- Node Info Sorting Position

class NodeInfoSortingPosition
{
public:
  NodeInfoSortingPosition() : mNodeInfo(nullptr) {}
  NodeInfoSortingPosition(NodePrintInfo* node) : mNodeInfo(node) {}

  bool operator<(const NodeInfoSortingPosition& other) const 
  { 
    return mNodeInfo->mConnectAvgPos < other.mNodeInfo->mConnectAvgPos; 
  }

  NodePrintInfo* mNodeInfo;
};

//--------------------------------------------------------------------------------- Sound Node Graph

class SoundNodeGraph
{
public:
  SoundNodeGraph() : 
    mNodeWidth(100),
    mNodeHeight(80),
    mMaxLevel(0)
  {}
  ~SoundNodeGraph();

  NodeInfoListType::range GetNodeInfoList();

private:
  NodeInfoListType mNodeInfoList;
  typedef HashMap<unsigned, NodePrintInfo*> NodeMapType;
  NodeMapType mNodeMap;
  unsigned mNodeWidth;
  unsigned mNodeHeight;
  int mMaxLevel;


  void CreateInfo(Audio::SoundNode* node, Audio::SoundNode* outputNode, int level);
  void CheckForCollision(NodeInfoListType& list, float& minXpos);
  void AddSpacePadding(NodePrintInfo* node, float addToXPos);
  void FirstPassPositioning(Array<NodeInfoListType>& infoByLevel, int largestLevel);
  void SecondPassPositioning(Array<NodeInfoListType> &infoByLevel, int largestLevel, float& minXpos);
  void PositionCleanUp(Array<NodeInfoListType> &infoByLevel, int largestLevel, float& minXpos);
};

}
