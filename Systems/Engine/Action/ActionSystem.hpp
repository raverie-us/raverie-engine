///////////////////////////////////////////////////////////////////////////////
///
/// \file ActionSystem.hpp
/// Declaration of the ActionSystem and ActionSpace.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

typedef InList<Actions> ActiveListsType;
class ActionSystem;
class UpdateEvent;


//------------------------------------------------------------------------------
/// Space component that synchronizes action queues.
class ActionSpace : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ActionSpace();
  ~ActionSpace();

  // Component Interface
  void Initialize(CogInitializer& initializer) override;
  
  // Time Space handler
  void OnActionFrameUpdate(UpdateEvent* updateEvent);
  void OnActionLogicUpdate(UpdateEvent* updateEvent);
  void UpdateActions(UpdateEvent* updateEvent, ActionExecuteMode::Enum mode);

  ActiveListsType ActiveLists;
  ActiveListsType ScheduledLists;

  IntrusiveLink(ActionSpace, link);
};

}//namespace Zero
