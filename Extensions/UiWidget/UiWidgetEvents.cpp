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
  DefineEvent(UiFocusGainedPreview);
  DefineEvent(UiFocusLost);
  DefineEvent(UiFocusGained);
  DefineEvent(UiFocusLostHierarchy);
  DefineEvent(UiFocusGainedHierarchy);
  DefineEvent(UiFocusReset);

}//namespace Events

//------------------------------------------------------------------ Focus Event
ZilchDefineType(UiFocusEvent, builder, type)
{
  ZeroBindDocumented();
}

}//namespace Zero
