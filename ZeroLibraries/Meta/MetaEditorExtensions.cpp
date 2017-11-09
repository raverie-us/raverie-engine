////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------ Editor Property Extension 
//**************************************************************************************************
ZilchDefineType(EditorPropertyExtension, builder, type)
{
}

//---------------------------------------------------------------------- Editor Indexed String Array
//**************************************************************************************************
ZilchDefineType(EditorIndexedStringArray, builder, type)
{
}

//**************************************************************************************************
EditorIndexedStringArray::EditorIndexedStringArray(GetStringsFunc sourceArray)
  : AccessSourceArray(sourceArray) {};

//**************************************************************************************************
void EditorIndexedStringArray::Enumerate(HandleParam instance, Property* property,
                                         Array<String>& strings)
{
  if (AccessSourceArray)
    AccessSourceArray(instance, property, strings);
  else
    strings = mFixedValues;
}

//------------------------------------------------------------------------------------- Editor Range
//**************************************************************************************************
ZilchDefineType(EditorRange, builder, type)
{
}

//**************************************************************************************************
EditorRange::EditorRange(float minValue, float maxValue, float increment)
  : MinValue(minValue)
  , MaxValue(maxValue)
  , Increment(increment)
{

}

//------------------------------------------------------------------------------ EditorRotationBasis
//**************************************************************************************************
ZilchDefineType(EditorRotationBasis, builder, type)
{
}

//**************************************************************************************************
EditorRotationBasis::EditorRotationBasis()
  : mGizmoName("EditorGizmo")
  , mIntData(0)
{
}

//**************************************************************************************************
EditorRotationBasis::EditorRotationBasis(StringParam archetypeName)
  : mArchetypeName(archetypeName)
  , mGizmoName("EditorGizmo")
  , mIntData(0)
{
}

//**************************************************************************************************
EditorRotationBasis::EditorRotationBasis(StringParam archetypeName, StringParam gizmoName, int intData)
  : mArchetypeName(archetypeName)
  , mGizmoName(gizmoName)
  , mIntData(intData)
{
}

//---------------------------------------------------------------------------------- Editor Resource
//**************************************************************************************************
ZilchDefineType(EditorResource, builder, type)
{
}

//**************************************************************************************************
EditorResource::EditorResource(bool allowAdd, bool allowNone, StringParam filterTag, bool forceCompact)
  : AllowAdd(allowAdd)
  , AllowNone(allowNone)
  , FilterTag(filterTag)
  , ForceCompact(forceCompact)
{

}

//----------------------------------------------------------------------------- Meta Property Filter
//**************************************************************************************************
ZilchDefineType(MetaPropertyFilter, builder, type)
{
}

//----------------------------------------------------------------------- Meta Property Basic Filter
//**************************************************************************************************
ZilchDefineType(MetaPropertyBasicFilter, builder, type)
{
}

//-------------------------------------------------------------------------------- Meta Editor Gizmo
//**************************************************************************************************
ZilchDefineType(MetaEditorGizmo, builder, type)
{
}

//----------------------------------------------------------------------------------- Meta Custom Ui
//**************************************************************************************************
ZilchDefineType(MetaCustomUi, builder, type)
{
}

}//namespace Zero
