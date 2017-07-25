///////////////////////////////////////////////////////////////////////////////
///
/// \file EngineEvents.cpp
/// Implementation of the basic event classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(GeneralEngine);
  DefineEvent(ScriptInitialize);
}//namespace Events

ZilchDefineType(TextEvent, builder, type)
{
  ZilchBindFieldProperty(Text);
}

ZilchDefineType(TextErrorEvent, builder, type)
{
}

ZilchDefineType(ProgressEvent, builder, type)
{
}

ZilchDefineType(BlockingTaskEvent, builder, type)
{
}

//--------------------------------------------------------------- Progress Event
ProgressEvent::ProgressEvent()
{
  ProgressType = ProgressType::Normal;
  Percentage = 0.0f;
}

}//namespace Zero
