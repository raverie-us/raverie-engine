////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------------------ Meta Resource
//**************************************************************************************************
ZilchDefineType(MetaResource, builder, type)
{
}

//**************************************************************************************************
MetaResource::MetaResource(Resource* resource)
{
  SetResource(resource);
}

//**************************************************************************************************
void MetaResource::SetResource(Resource* resource)
{
  mResourceId = resource->mResourceId;
}

//------------------------------------------------------------------------- MetaEditor Script Object
//**************************************************************************************************
ZilchDefineType(MetaEditorScriptObject, builder, type)
{
  ZilchBindField(mAutoRegister)->AddAttribute(PropertyAttributes::cOptional);
}

//**************************************************************************************************
MetaEditorScriptObject::MetaEditorScriptObject()
  : mAutoRegister(true)
{
  
}

}//namespace Zero
