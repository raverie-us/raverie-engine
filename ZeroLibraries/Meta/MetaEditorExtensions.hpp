////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------------ Editor Property Extension 
// Property Extensions
class EditorPropertyExtension : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  virtual ~EditorPropertyExtension() {};
};

//---------------------------------------------------------------------- Editor Indexed String Array
typedef void (*GetStringsFunc)(HandleParam instance, Property* property, Array<String>& strings);
extern const String StringArrayEdit;
class EditorIndexedStringArray : public EditorPropertyExtension
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ~EditorIndexedStringArray(){}

  EditorIndexedStringArray(GetStringsFunc sourceArray)
    : AccessSourceArray(sourceArray){};
  
  void Enumerate(HandleParam instance, Property* property, Array<String>& strings)
  {
    if(AccessSourceArray)
      AccessSourceArray(instance, property, strings);
    else
      strings = mFixedValues;
  }

  // If the function pointer is present, it will be called
  // otherwise the array of values will be used
  GetStringsFunc AccessSourceArray;
  Array<String> mFixedValues;
};


//------------------------------------------------------------------------------------- Editor Range
class EditorRange : public EditorPropertyExtension
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  float MinValue;
  float MaxValue;
  float Increment;

  //Takes a value between zero and one and returns the value
  float NormalizedToValue(float n);

  //Take the range value and converts it to between zero and one.
  float NormalizeValue(float v);

  EditorRange(float minValue, float maxValue, float increment)
    : MinValue(minValue), MaxValue(maxValue), Increment(increment)
  {

  }
};

//------------------------------------------------------------------------------ EditorRotationBasis
/// Editor for creating a gizmo to modify a basis. The int is occasionally used to denote what
/// "property" is being modified. Currently it's not easy to set just a property as several
/// properties need to be sampled (see RevoluteBasisGizmo).
class EditorRotationBasis : public EditorPropertyExtension
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EditorRotationBasis()
    : mGizmoName("EditorGizmo"), mIntData(0)
  {
  }

  EditorRotationBasis(StringParam archetypeName)
    : mArchetypeName(archetypeName), mGizmoName("EditorGizmo"), mIntData(0)
  { 
  }

  EditorRotationBasis(StringParam archetypeName, StringParam gizmoName, int intData)
    : mArchetypeName(archetypeName), mGizmoName(gizmoName), mIntData(intData)
  {
  }

  int mIntData;
  String mGizmoName;
  String mArchetypeName;
};

//---------------------------------------------------------------------------------- Editor Resource
extern const String ResourceEdit;
class EditorResource : public EditorPropertyExtension
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// If not empty only match resources with same FilterTag
  String FilterTag;
  /// Allows a plus button for easy adding of new resource
  bool AllowAdd;
  /// Are null resources allowed?
  bool AllowNone;

  EditorResource(bool allowAdd = false, bool allowNone = false, StringParam filterTag = "")
    : AllowAdd(allowAdd), AllowNone(allowNone), FilterTag(filterTag)
  {
  }
};

}//namespace Zero
