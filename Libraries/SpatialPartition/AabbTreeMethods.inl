///////////////////////////////////////////////////////////////////////////////
///
/// \file AabbTreeMethods.inl
/// Implementation of the helper functions used by Aabb Trees in order to
/// cut down on code re-use.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

template <typename NodeType>
bool NodeSortX(const NodeType* left, const NodeType* right)
{
  return left->mAabb.GetCenter().x < right->mAabb.GetCenter().x;
}

template <typename NodeType>
bool NodeSortY(const NodeType* left, const NodeType* right)
{
  return left->mAabb.GetCenter().y < right->mAabb.GetCenter().y;
}

template <typename NodeType>
bool NodeSortZ(const NodeType* left, const NodeType* right)
{
  return left->mAabb.GetCenter().z < right->mAabb.GetCenter().z;
}


template <typename NodeType>
NodeType* BuildTreeTopDownNodes(Array<NodeType*>& leafNodes,
  typename PartitionNodeMethod<NodeType>::PartitionNodeAxisMethod partitionMethod)
{
  //this case seems to be happening for some reason. If we continue down
  //this path, we will crash in minimize volume sum.
  if(leafNodes.Size() == 0)
    return nullptr;

  //If there is only one object
  if(leafNodes.Size() == 1)
    return leafNodes[0];

  //If there are only two objects, create two leaf nodes 
  //and add them to a single root node
  if(leafNodes.Size() == 2)
  {
    NodeType* node = new NodeType();
    node->SetChildren(leafNodes[0],leafNodes[1]);
    return node;
  }

  int separationIndex = partitionMethod(leafNodes);

  //Split up the array
  Array<NodeType*> left, right;
  left.Assign(leafNodes.Begin(), leafNodes.Begin() + separationIndex);
  right.Assign(leafNodes.Begin() + separationIndex, leafNodes.End());

  NodeType* leftNode = BuildTreeTopDownNodes<NodeType>(left,partitionMethod);
  NodeType* rightNode = BuildTreeTopDownNodes<NodeType>(right,partitionMethod);

  NodeType* parentNode = new NodeType();
  parentNode->SetChildren(leftNode,rightNode);
  return parentNode;
}

template <typename NodeType>
Aabb CalculateAabbNodes(Array<NodeType*>& leafNodes)
{
  Aabb total;
  total.Zero();

  for(uint i = 0; i < leafNodes.Size(); ++i)
    total.Combine(leafNodes[i]->mAabb);

  return total;
}

template <typename NodeType>
uint MinimizeVolumeSumNodes(Array<NodeType*>& leafNodes)
{
  //The axis we are using.
  uint axis = 0;
  //The index we are splitting at.
  uint index = 0;
  //The amount of objects.
  uint size = leafNodes.Size();
  //Default the cost to the max float.
  real cost = Math::PositiveMax();

  Array<real> leftVolume, rightVolume;

  leftVolume.Resize(size);
  Fill(leftVolume.All(), real(0.0));

  rightVolume.Resize(size);
  Fill(rightVolume.All(), real(0.0));

  //Sort on the x axis first.
  Sort(leafNodes.All(), NodeSortX<NodeType>);

  //For each axis
  for(uint currAxis = 0; currAxis < 3; ++currAxis)
  {
    //Holds the current volume of each side.
    Aabb leftAabb, rightAabb;
    leftAabb.Zero();
    rightAabb.Zero();

    //determine which axis to sort on
    if(currAxis == 1)
      Sort(leafNodes.All(), NodeSortY<NodeType>);
    else if(currAxis == 2)
      Sort(leafNodes.All(), NodeSortZ<NodeType>);

    //Loop through each object
    for(uint i = 0; i < leafNodes.Size(); ++i)
    {
      //Add to the total of the leftAabb
      leftAabb = leftAabb.Combined(leafNodes[i]->mAabb);
      leftVolume[i] = leftAabb.GetVolume();

      rightAabb = rightAabb.Combined(leafNodes[size - i - 1]->mAabb);
      rightVolume[size - i - 1] = rightAabb.GetVolume();
    }

    //The minimum volume is 0.1.  If it's any lower, it may possibly 
    //split at index 0, causing infinite recursion.
    real totalVolume = Math::Max(leftAabb.GetVolume(), real(0.1));

    //Walk through each aabb and calculate the cost.
    //We don't do the last one because the volume will be the same.
    for(uint i = 1; i < size - 1; ++i)
    {
      //Get the current cost
      real currentCost = ((leftVolume[i] / totalVolume) * i) + 
        ((rightVolume[i] / totalVolume) * (size - i - 1));

      //If the current cost is cheaper than the previous cost, record it.
      if(currentCost < cost)
      {
        cost = currentCost;
        //Record the axis and index
        axis = currAxis;
        index = i;
      }
    }
  }

  //Sort on the chosen axis (it is sorted on the z axis when we 
  //get here, so no need to sort in that case).
  if(axis == 0)
    Sort(leafNodes.All(), NodeSortX<NodeType>);
  else if(axis == 1)
    Sort(leafNodes.All(), NodeSortY<NodeType>);

  return index;
}


template <typename NodeType>
uint MinimizeSurfaceAreaSumNodes(Array<NodeType*>& leafNodes)
{
  //The axis we are using.
  uint axis = 0;
  //The index we are splitting at.
  uint index = 0;
  //The amount of objects.
  uint size = leafNodes.Size();
  //Default the cost to the max float.
  real cost = Math::PositiveMax();

  Array<real> leftArea, rightArea;

  leftArea.Resize(size);
  Fill(leftArea.All(), real(0.0));
  rightArea.Resize(size);
  Fill(rightArea.All(), real(0.0));

  //Sort on the x axis first.
  Sort(leafNodes.All(), NodeSortX<NodeType>);

  //For each axis
  for(uint currAxis = 0; currAxis < 3; ++currAxis)
  {
    //Holds the current volume of each side.
    Aabb leftAabb, rightAabb;
    leftAabb.Zero();
    rightAabb.Zero();

    //determine which axis to sort on
    if(currAxis == 1)
      Sort(leafNodes.All(), NodeSortY<NodeType>);
    else if(currAxis == 2)
      Sort(leafNodes.All(), NodeSortZ<NodeType>);

    //Loop through each object
    for(uint i = 0; i < leafNodes.Size(); ++i)
    {
      //Add to the total of the leftAabb
      leftAabb = leftAabb.Combined(leafNodes[i]->mAabb);
      leftArea[i] = leftAabb.GetSurfaceArea();

      rightAabb = rightAabb.Combined(leafNodes[size - i - 1]->mAabb);
      rightArea[size - i - 1] = rightAabb.GetSurfaceArea();
    }

    //The minimum surface area is 0.1.  If it's any lower, it may possibly 
    //split at index 0, causing infinite recursion.
    real totalSurfaceArea = Math::Max(leftAabb.GetSurfaceArea(), real(0.1));

    //Walk through each aabb and calculate the cost.
    //We don't do the last one because the volume will be the same.
    for(uint i = 1; i < size - 1; ++i)
    {
      //Get the current cost
      real currentCost = ((leftArea[i] / totalSurfaceArea) * i) + 
                         ((rightArea[i] / totalSurfaceArea) * (size - i - 1));

      //If the current cost is cheaper than the previous cost, record it.
      if(currentCost < cost)
      {
        cost = currentCost;
        //Record the axis and index
        axis = currAxis;
        index = i;
      }
    }
  }

  //Sort on the chosen axis (it is sorted on the z axis when 
  //we get here, so no need to sort in that case).
  if(axis == 0)
    Sort(leafNodes.All(), NodeSortX<NodeType>);
  else if(axis == 1)
    Sort(leafNodes.All(), NodeSortY<NodeType>);

  return index;
}

template <typename NodeType>
uint MidPointNodes(Array<NodeType*>& leafNodes)
{
  //Calculate the total Aabb
  Aabb total = CalculateAabbNodes(leafNodes);

  //Find the best axis
  int axis = 0;
  real dist = 0;

  for(uint i = 0; i < 3; ++i)
  {
    real currDist = total.mMax[i] - total.mMin[i];
    if(currDist > dist)
    {
      axis = i;
      dist = currDist;
    }
  }

  //Sort on the x axis.
  if(axis == 0)
    Sort(leafNodes.All(), NodeSortX<NodeType>);
  else if(axis == 1)
    Sort(leafNodes.All(), NodeSortY<NodeType>);
  else
    Sort(leafNodes.All(), NodeSortZ<NodeType>);

  //Divide it by two and return
  return leafNodes.Size() >> 1;
}



//------------------------------------- Old functions (still used, can't remove)

template <typename NodeType, typename ObjectType>
NodeType* BuildTreeTopDown(Array<ObjectType>& leafNodes,
      typename PartitionMethod<ObjectType>::PartitionAxisMethod partitionMethod)
{
  //If there is only one object
  if(leafNodes.Size() == 1)
    return new NodeType(leafNodes[0]);

  //If there are only two objects, create two leaf nodes 
  //and add them to a single root node
  if(leafNodes.Size() == 2)
    return new NodeType(new NodeType(leafNodes[0]), new NodeType(leafNodes[1]));

  int separationIndex = partitionMethod(leafNodes);

  //Split up the array
  Array<ObjectType> left, right;
  left.Assign(leafNodes.Begin(), leafNodes.Begin() + separationIndex);
  right.Assign(leafNodes.Begin() + separationIndex, leafNodes.End());

  //Allocate a new node
  return new NodeType(BuildTreeTopDown<NodeType,ObjectType>(left, partitionMethod), 
                      BuildTreeTopDown<NodeType,ObjectType>(right,partitionMethod));
}

template <typename ObjectType>
Aabb CalculateAabb(Array<ObjectType>& proxies)
{
  Aabb total;
  total.Zero();

  for(uint i = 0; i < proxies.Size(); ++i)
    total = total.Combined(proxies[i].mAabb);

  return total;
}

template <typename ObjectType>
bool ProxySortX(const ObjectType& left, const ObjectType& right)
{
  return left.mAabb.GetCenter().x < right.mAabb.GetCenter().x;
}

template <typename ObjectType>
bool ProxySortY(const ObjectType& left, const ObjectType& right)
{
  return left.mAabb.GetCenter().y < right.mAabb.GetCenter().y;
}

template <typename ObjectType>
bool ProxySortZ(const ObjectType& left, const ObjectType& right)
{
  return left.mAabb.GetCenter().z < right.mAabb.GetCenter().z;
}

template <typename ObjectType>
uint MinimizeVolumeSum(Array<ObjectType>& proxies)
{
  //The axis we are using.
  uint axis = 0;
  //The index we are splitting at.
  uint index = 0;
  //The amount of objects.
  uint size = proxies.Size();
  //Default the cost to the max float.
  real cost = Math::PositiveMax();

  Array<real> leftVolume, rightVolume;
  
  leftVolume.Resize(size);
  Fill(leftVolume.All(), real(0.0));

  rightVolume.Resize(size);
  Fill(rightVolume.All(), real(0.0));

  //Sort on the x axis first.
  Sort(proxies.All(), ProxySortX<ObjectType>);

  //For each axis
  for(uint currAxis = 0; currAxis < 3; ++currAxis)
  {
    //Holds the current volume of each side.
    Aabb leftAabb, rightAabb;
    leftAabb.Zero();
    rightAabb.Zero();

    //determine which axis to sort on
    if(currAxis == 1)
      Sort(proxies.All(), ProxySortY<ObjectType>);
    else if(currAxis == 2)
      Sort(proxies.All(), ProxySortZ<ObjectType>);

    //Loop through each object
    for(uint i = 0; i < proxies.Size(); ++i)
    {
      //Add to the total of the leftAabb
      leftAabb = leftAabb.Combined(proxies[i].mAabb);
      leftVolume[i] = leftAabb.GetVolume();

      rightAabb = rightAabb.Combined(proxies[size - i - 1].mAabb);
      rightVolume[size - i - 1] = rightAabb.GetVolume();
    }

    //The minimum volume is 0.1.  If it's any lower, it may possibly 
    //split at index 0, causing infinite recursion.
    real totalVolume = Math::Max(leftAabb.GetVolume(), real(0.1));

    //Walk through each aabb and calculate the cost.
    //We don't do the last one because the volume will be the same.
    for(uint i = 1; i < size - 1; ++i)
    {
      //Get the current cost
      real currentCost = ((leftVolume[i] / totalVolume) * i) + 
        ((rightVolume[i] / totalVolume) * (size - i - 1));

      //If the current cost is cheaper than the previous cost, record it.
      if(currentCost < cost)
      {
        cost = currentCost;
        //Record the axis and index
        axis = currAxis;
        index = i;
      }
    }
  }

  //Sort on the chosen axis (it is sorted on the z axis when we 
  //get here, so no need to sort in that case).
  if(axis == 0)
    Sort(proxies.All(), ProxySortX<ObjectType>);
  else if(axis == 1)
    Sort(proxies.All(), ProxySortY<ObjectType>);

  return index;
}

template <typename ObjectType>
uint MinimizeSurfaceAreaSum(Array<ObjectType>& proxies)
{
  //The axis we are using.
  uint axis = 0;
  //The index we are splitting at.
  uint index = 0;
  //The amount of objects.
  uint size = proxies.Size();
  //Default the cost to the max float.
  real cost = Math::PositiveMax();

  Array<real> leftArea, rightArea;
  
  leftArea.Resize(size);
  Fill(leftArea.All(), real(0.0));
  rightArea.Resize(size);
  Fill(rightArea.All(), real(0.0));

  //Sort on the x axis first.
  Sort(proxies.All(), ProxySortX<ObjectType>);

  //For each axis
  for(uint currAxis = 0; currAxis < 3; ++currAxis)
  {
    //Holds the current volume of each side.
    Aabb leftAabb, rightAabb;
    leftAabb.Zero();
    rightAabb.Zero();

    //determine which axis to sort on
    if(currAxis == 1)
      Sort(proxies.All(), ProxySortY<ObjectType>);
    else if(currAxis == 2)
      Sort(proxies.All(), ProxySortZ<ObjectType>);

    //Loop through each object
    for(uint i = 0; i < proxies.Size(); ++i)
    {
      //Add to the total of the leftAabb
      leftAabb = leftAabb.Combined(proxies[i].mAabb);
      leftArea[i] = leftAabb.GetSurfaceArea();

      rightAabb = rightAabb.Combined(proxies[size - i - 1].mAabb);
      rightArea[size - i - 1] = rightAabb.GetSurfaceArea();
    }

    //The minimum surface area is 0.1.  If it's any lower, it may possibly 
    //split at index 0, causing infinite recursion.
    real totalSurfaceArea = Math::Max(leftAabb.GetSurfaceArea(), real(0.1));

    //Walk through each aabb and calculate the cost.
    //We don't do the last one because the volume will be the same.
    for(uint i = 1; i < size - 1; ++i)
    {
      //Get the current cost
      real currentCos = ((leftArea[i] / totalSurfaceArea) * i) + 
        ((rightArea[i] / totalSurfaceArea) * (size - i - 1));

      //If the current cost is cheaper than the previous cost, record it.
      if(currentCos < cost)
      {
        cost = currentCos;
        //Record the axis and index
        axis = currAxis;
        index = i;
      }
    }
  }

  //Sort on the chosen axis (it is sorted on the z axis when 
  //we get here, so no need to sort in that case).
  if(axis == 0)
    Sort(proxies.All(), ProxySortX<ObjectType>);
  else if(axis == 1)
    Sort(proxies.All(), ProxySortY<ObjectType>);

  return index;
}

template <typename ObjectType>
uint MidPoint(Array<ObjectType>& proxies)
{
  //Calculate the total Aabb
  Aabb total = CalculateAabb(proxies);

  //Find the best axis
  int axis = 0;
  real dist = 0;

  for(uint i = 0; i < 3; ++i)
  {
    real currDist = total.mMax[i] - total.mMin[i];
    if(currDist > dist)
    {
      axis = i;
      dist = currDist;
    }
  }

  //Sort on the x axis.
  if(axis == 0)
    Sort(proxies.All(), ProxySortX<ObjectType>);
  else if(axis == 1)
    Sort(proxies.All(), ProxySortY<ObjectType>);
  else
    Sort(proxies.All(), ProxySortZ<ObjectType>);

  //Divide it by two and return
  return proxies.Size() >> 1;
}

//---------------------------------------------------------------- Ray Functions
///Callbacks for simple tests to see if we should traverse further down the tree.  
///Returns the time of collision only for ray casts.
template <typename NodeType>
bool RayNodeTest(NodeType* tree, CastDataParam castData, real& t)
{
  return IBroadPhase::TestRayVsAabb(tree->mAabb, castData.GetRay().Start, 
                                   castData.GetRay().Direction, t);
}

template <typename NodeType>
bool SegmentNodeTest(NodeType* tree, CastDataParam castData, real& t)
{
  return IBroadPhase::TestSegmentVsAabb(tree->mAabb, castData.GetSegment().Start, 
                                       castData.GetSegment().End, t);
}

//-------------------------------------------------------- Volume Test Functions
///Callbacks for simple tests to see if we should traverse further down the tree.  
///Returns the time of collision only for ray casts.
template <typename NodeType>
bool AabbNodeTest(NodeType* tree, CastDataParam castData, real& t)
{
  if(Intersection::None != Intersection::AabbAabb(tree->mAabb.mMin,
                                                  tree->mAabb.mMax,
                                                  castData.GetAabb().mMin,
                                                  castData.GetAabb().mMax))
  {
    t = (tree->mAabb.GetCenter() - castData.GetSphere().mCenter).Length();
    return true;
  }

  return false;
}

template <typename NodeType>
bool SphereNodeTest(NodeType* tree, CastDataParam castData, real& t)
{
  if(Intersection::AabbSphere(tree->mAabb.mMin, tree->mAabb.mMax, 
    castData.GetSphere().mCenter, castData.GetSphere().mRadius, nullptr))
  {
    t = (tree->mAabb.GetCenter() - castData.GetSphere().mCenter).Length();
    return true;
  }

  return false;
}

template <typename NodeType>
bool FrustumNodeTest(NodeType* tree, CastDataParam castData, real& t)
{
  if(Intersection::None != Intersection::AabbFrustumApproximation(tree->mAabb.mMin, 
                                                     tree->mAabb.mMax, 
                                                     &castData.GetFrustum().Planes[0].GetData()))
  {
    t = (tree->mAabb.GetCenter() - castData.GetSphere().mCenter).Length();
    return true;
  }

  return false;
}
