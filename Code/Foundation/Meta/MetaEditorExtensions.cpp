// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// Editor Property Extension
ZilchDefineType(EditorPropertyExtension, builder, type)
{
}

// Indexed String Array
ZilchDefineType(EditorIndexedStringArray, builder, type)
{
}

EditorIndexedStringArray::EditorIndexedStringArray(GetStringsFunc sourceArray) : AccessSourceArray(sourceArray){};

void EditorIndexedStringArray::Enumerate(HandleParam instance, Property* property, Array<String>& strings)
{
  if (AccessSourceArray)
    AccessSourceArray(instance, property, strings);
  else
    strings = mFixedValues;
}

// Editor Range
ZilchDefineType(EditorRange, builder, type)
{
  ZilchBindFieldProperty(Min);
  ZilchBindFieldProperty(Max);
  ZilchBindFieldProperty(Increment)->AddAttribute(PropertyAttributes::cOptional);
}

EditorRange::EditorRange() : Min(-Math::PositiveMax()), Max(Math::PositiveMax()), Increment(0.01f)
{
}

EditorRange::EditorRange(float minValue, float maxValue, float increment) :
    Min(minValue),
    Max(maxValue),
    Increment(increment)
{
}

void EditorRange::PostProcess(Status& status, ReflectionObject* owner)
{
  if (Increment == 0)
  {
    status.SetFailed("'Increment' cannot be 0.");
    return;
  }
  if (Min >= Max)
  {
    status.SetFailed("'Max' must be greater than or equal to 'Min'.");
    return;
  }
}

// Editor Slider
ZilchDefineType(EditorSlider, builder, type)
{
}

EditorSlider::EditorSlider()
{
  Min = 0;
  Max = 1;
  Increment = 0.01f;
}

// EditorRotationBasis
ZilchDefineType(EditorRotationBasis, builder, type)
{
}

EditorRotationBasis::EditorRotationBasis() : mIntData(0), mGizmoName("EditorGizmo")
{
}

EditorRotationBasis::EditorRotationBasis(StringParam archetypeName) :
    mIntData(0),
    mGizmoName("EditorGizmo"),
    mArchetypeName(archetypeName)
{
}

EditorRotationBasis::EditorRotationBasis(StringParam archetypeName, StringParam gizmoName, int intData) :
    mIntData(intData),
    mGizmoName(gizmoName),
    mArchetypeName(archetypeName)
{
}

// Editor Resource
ZilchDefineType(MetaEditorResource, builder, type)
{
  ZilchBindField(FilterTag)->AddAttribute(PropertyAttributes::cOptional);
  ZilchBindField(AllowAdd)->AddAttribute(PropertyAttributes::cOptional);
  ZilchBindField(AllowNone)->AddAttribute(PropertyAttributes::cOptional);
  ZilchBindField(SearchPreview)->AddAttribute(PropertyAttributes::cOptional);
}

MetaEditorResource::MetaEditorResource(
    bool allowAdd, bool allowNone, StringParam filterTag, bool forceCompact, bool searchPreview) :
    FilterTag(filterTag),
    AllowAdd(allowAdd),
    AllowNone(allowNone),
    ForceCompact(forceCompact),
    SearchPreview(searchPreview),
    Filter(nullptr)
{
}

MetaEditorResource::MetaEditorResource(SearchFilter filter) :
    AllowAdd(false),
    AllowNone(false),
    FilterTag(""),
    ForceCompact(false),
    SearchPreview(true),
    Filter(filter)
{
}

bool MetaEditorResource::FilterPropertySearchResult(HandleParam object,
                                                    Property* property,
                                                    HandleParam result,
                                                    Status& status)
{
  if (Filter)
    return Filter(object, property, result, status);
  return true;
}

// Meta Property Filter
ZilchDefineType(MetaPropertyFilter, builder, type)
{
}

// Property Basic Filter
ZilchDefineType(MetaPropertyBasicFilter, builder, type)
{
}

// Meta Editor Gizmo
ZilchDefineType(MetaEditorGizmo, builder, type)
{
  ZilchBindFieldProperty(mGizmoArchetype);
}

// Meta Shader Input
ZilchDefineType(MetaShaderInput, builder, type)
{
  ZilchBindFieldProperty(mFragmentName)->AddAttribute(PropertyAttributes::cOptional);
  ZilchBindFieldProperty(mInputName)->AddAttribute(PropertyAttributes::cOptional);
}

// Meta Group
ZilchDefineType(MetaGroup, builder, type)
{
  ZilchBindFieldProperty(mName);
}

MetaGroup::MetaGroup(StringParam name) : mName(name)
{
}

// Meta Custom Ui
ZilchDefineType(MetaCustomUi, builder, type)
{
}

// Property Rename
ZilchDefineType(MetaPropertyRename, builder, type)
{
}

MetaPropertyRename::MetaPropertyRename(StringParam oldName) : mOldName(oldName)
{
}

} // namespace Zero
