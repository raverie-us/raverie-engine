///////////////////////////////////////////////////////////////////////////////
///
/// \file TagsContent.Cpp
/// Implementation of the TagsContent content component.
/// 
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "TagsContent.hpp"

namespace Zero
{

//------------------------------------------------------------------------- Tags
ZilchDefineType(ContentTags, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);
}

//******************************************************************************
void ContentTags::Serialize(Serializer& stream)
{
  SerializeName(mTags);
}

}// namespace Zero
