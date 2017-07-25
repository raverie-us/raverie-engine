///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(AllObjectsInitialized);
}

ZilchDefineType(CogInitializerEvent, builder, type)
{
  ZeroBindDocumented();
  ZilchBindFieldProperty(mCogInitializer);
}

ZilchDefineType(CogInitializer, builder, type)
{
  ZilchBindGetterProperty(Space);
  ZilchBindGetterProperty(Parent);
  ZilchBindGetterProperty(GameSession);
  ZilchBindMethod(DispatchEvent);
  ZeroBindEvent(Events::AllObjectsInitialized, CogInitializerEvent);
  type->HandleManager = ZilchManagerId(PointerManager);
}

CogInitializer::CogInitializer(Zero::Space* space, Zero::GameSession* gameSession)
{
  mSpace = space;
  mGameSession = gameSession;
  if(!gameSession && space)
    mGameSession = space->GetGameSession();
  Context = NULL;
  mParent = NULL;
  Flags = 0;
  AddCount = 0;
}

CogInitializer::~CogInitializer()
{
  ErrorIf(!CreationList.Empty(), "A cog was initialized and AllCreated was not called"
                                 " on the initializer to finish the creation.");
}

void CogInitializer::DispatchEvent(StringParam eventId, Event* event)
{
  this->GetDispatcher()->Dispatch(eventId, event);
}

SpaceCogList::range CogInitializer::AllCreated()
{
  // If empty do nothing otherwise looping will not work
  if (CreationList.Empty())
    return SpaceCogList::range();

  // Store the first and the last valid objects for iteration.
  // Normally a range would be used but initializing may
  // create more objects so the end would change when new
  // objects are created and added to the list
  Cog* first = CreationList.Begin();
  Cog* last = SpaceCogList::Prev(CreationList.End());

  if(mSpace)
  {
    // Move newly created objects to the space list
    SpaceCogList::iterator begin = CreationList.Begin();
    mSpace->mCogList.Splice(mSpace->mCogList.End(), CreationList);
    mSpace->mCogsInSpace += this->AddCount;
    mSpace->ChangedObjects();
  }

  // Call OnAllObjectsCreated on new objects
  Cog* current = first;
  for(;;)
  {
    PushErrorContextObject("Initializing Object", current);
    current->OnAllObjectsCreated(*this);
    if(current == last)
      break;
    current = SpaceCogList::Next(current);
  }

  // Call ScriptInitialize on new objects
  current = first;
  for(;;)
  {
    // Object not marked for deletion?
    if(!current->GetMarkedForDestruction())
    {
      PushErrorContextObject("Initializing Script Object", current);
      current->ScriptInitialize(*this);
    }
    if(current == last)
      break;
    current = SpaceCogList::Next(current);
  }

  // Send Post Initialize event
  SendAllObjectsInitialized();

  // Reset
  Context = NULL;
  AddCount = 0;
  CreationList.Clear();

  return SpaceCogList::range(first, SpaceCogList::Next(last));
}

void CogInitializer::SendAllObjectsInitialized()
{
  CogInitializerEvent toSend;
  toSend.mCogInitializer = this;
  this->DispatchEvent(Events::AllObjectsInitialized, &toSend);
}

}
