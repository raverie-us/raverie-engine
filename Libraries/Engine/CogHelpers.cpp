///////////////////////////////////////////////////////////////////////////////
///
/// \file CogHelpers.cpp
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//******************************************************************************
Cog* LowestCommonAncestor(Cog* objectA, Cog* objectB)
{
  HashSet<Cog*> parents;

  Cog* aChain = objectA;
  while(aChain != nullptr)
  {
    parents.Insert(aChain);
    aChain = aChain->GetParent();
  }

  Cog* bChain = objectB;
  while(bChain != nullptr)
  {
    if(parents.Contains(bChain))
      return bChain;
    bChain = bChain->GetParent();
  }

  return nullptr;
}

//******************************************************************************
void SendHierarchyEvents(cstr op, Cog* oldObject, Cog* newObject, 
                         Event* outEvent, Event* inEvent,
                         StringParam outEventName, StringParam inEventName,
                         StringParam outHierarchyName, StringParam inHierarchyName,
                         uint flag, uint hierarchyFlag, FlagCallback callback)
{
  // Find the lowest common ancestor this object will 
  // will stop the bubble of the hierarchy events so
  Cog* lca = LowestCommonAncestor(newObject, oldObject);

  if(op)
  {
    ZPrint("%s %s -> %s , Lca %s\n", op,
           oldObject ? oldObject->GetName().c_str() : "None",
           newObject ? newObject->GetName().c_str() : "None",
           lca ? lca->GetName().c_str() : "None");
  }

  // Send the out event on the old object
  if(oldObject)
  {
    oldObject->DispatchEvent(outEventName, outEvent);
    //oldObject->DispatchUp(outEventName, outEvent);
    callback(oldObject, flag, FlagOperation::Clear);
  }

  // Now send the out hierarchy event up the old tree
  // until the lowest common ancestor
  Cog* oldChain = oldObject;
  while(oldChain != lca)
  {
    callback(oldChain, hierarchyFlag, FlagOperation::Clear);
    oldChain->DispatchEvent(outHierarchyName, outEvent);
    oldChain = oldChain->GetParent();
  }

  // Send the in event on the new object
  if(newObject)
  {
    newObject->DispatchEvent(inEventName, inEvent);
    //newObject->DispatchUp(inEventName, inEvent);
    callback(newObject, flag, FlagOperation::Set);
  }

  // Now send the in hierarchy event up the new tree
  // until the lowest common ancestor
  Cog* newChain = newObject;
  while(newChain != lca)
  {
    newChain->DispatchEvent(inHierarchyName, inEvent);
    callback(newChain, hierarchyFlag, FlagOperation::Set);
    newChain = newChain->GetParent();
  }

}

}//namespace Zero
