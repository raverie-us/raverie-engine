///////////////////////////////////////////////////////////////////////////////
///
/// \file CogHelpers.hpp
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

Cog* LowestCommonAncestor(Cog* objectA, Cog* objectB);

// When events occur on sub objects like mouse enter/exit it is useful to 
// have events that are only sent when the mouse leaves the object and all 
// children (MouseExitHierarchy). This function sends the correct events to 
// the base objects and up the trees to the lowest common ancestor of the object.
DeclareEnum2(FlagOperation, Set, Clear);
typedef void (*FlagCallback)(Cog*, uint, FlagOperation::Enum);
void SendHierarchyEvents(cstr op, Cog* oldObject, Cog* newObject, 
                         Event* outEvent, Event* inEvent,
                         StringParam outEventName, StringParam inEventName,
                         StringParam outHierarchyName, StringParam inHierarchyName,
                         uint flag, uint hierarchyFlag, FlagCallback callback);

}//namespace Zero
