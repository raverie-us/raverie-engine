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
  ZilchBindFieldProperty(Min);
  ZilchBindFieldProperty(Max);
  ZilchBindFieldProperty(Increment)->AddAttribute(PropertyAttributes::cOptional);
}

//**************************************************************************************************
EditorRange::EditorRange()
  : Min(-Math::PositiveMax())
  , Max(Math::PositiveMax())
  , Increment(0.01f)
{

}

//**************************************************************************************************
EditorRange::EditorRange(float minValue, float maxValue, float increment)
  : Min(minValue)
  , Max(maxValue)
  , Increment(increment)
{

}

//------------------------------------------------------------------------------------ Editor Slider
//**************************************************************************************************
ZilchDefineType(EditorSlider, builder, type)
{
}

//**************************************************************************************************
EditorSlider::EditorSlider()
{
  Min = 0;
  Max = 1;
  Increment = 0.01f;
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
  ZilchBindField(FilterTag)->AddAttribute(PropertyAttributes::cOptional);
  ZilchBindField(AllowAdd)->AddAttribute(PropertyAttributes::cOptional);
  ZilchBindField(AllowNone)->AddAttribute(PropertyAttributes::cOptional);
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
  ZilchBindFieldProperty(mGizmoArchetype);
}

//-------------------------------------------------------------------------------- Meta Shader Input
//**************************************************************************************************
ZilchDefineType(MetaShaderInput, builder, type)
{
  ZilchBindFieldProperty(mFragmentName)->AddAttribute(PropertyAttributes::cOptional);
  ZilchBindFieldProperty(mInputName)->AddAttribute(PropertyAttributes::cOptional);
}

//--------------------------------------------------------------------------------------- Meta Group
//**************************************************************************************************
ZilchDefineType(MetaGroup, builder, type)
{
  ZilchBindFieldProperty(mName);
}

//**************************************************************************************************
MetaGroup::MetaGroup(StringParam name)
  : mName(name)
{

}

//----------------------------------------------------------------------------------- Meta Custom Ui
//**************************************************************************************************
ZilchDefineType(MetaCustomUi, builder, type)
{
}

//---------------------------------------------------------------------------------- Property Rename
//**************************************************************************************************
ZilchDefineType(MetaPropertyRename, builder, type)
{
}

//**************************************************************************************************
MetaPropertyRename::MetaPropertyRename(StringParam oldName)
  : mOldName(oldName)
{

}

}//namespace Zero
