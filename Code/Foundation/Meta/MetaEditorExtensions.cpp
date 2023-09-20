// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Editor Property Extension
RaverieDefineType(EditorPropertyExtension, builder, type)
{
}

// Indexed String Array
RaverieDefineType(EditorIndexedStringArray, builder, type)
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
RaverieDefineType(EditorRange, builder, type)
{
  RaverieBindFieldProperty(Min);
  RaverieBindFieldProperty(Max);
  RaverieBindFieldProperty(Increment)->AddAttribute(PropertyAttributes::cOptional);
}

EditorRange::EditorRange() : Min(-Math::PositiveMax()), Max(Math::PositiveMax()), Increment(0.01f)
{
}

EditorRange::EditorRange(float minValue, float maxValue, float increment) : Min(minValue), Max(maxValue), Increment(increment)
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
RaverieDefineType(EditorSlider, builder, type)
{
}

EditorSlider::EditorSlider()
{
  Min = 0;
  Max = 1;
  Increment = 0.01f;
}

// EditorRotationBasis
RaverieDefineType(EditorRotationBasis, builder, type)
{
}

EditorRotationBasis::EditorRotationBasis() : mIntData(0), mGizmoName("EditorGizmo")
{
}

EditorRotationBasis::EditorRotationBasis(StringParam archetypeName) : mIntData(0), mGizmoName("EditorGizmo"), mArchetypeName(archetypeName)
{
}

EditorRotationBasis::EditorRotationBasis(StringParam archetypeName, StringParam gizmoName, int intData) : mIntData(intData), mGizmoName(gizmoName), mArchetypeName(archetypeName)
{
}

// Editor Resource
RaverieDefineType(MetaEditorResource, builder, type)
{
  RaverieBindField(FilterTag)->AddAttribute(PropertyAttributes::cOptional);
  RaverieBindField(AllowAdd)->AddAttribute(PropertyAttributes::cOptional);
  RaverieBindField(AllowNone)->AddAttribute(PropertyAttributes::cOptional);
  RaverieBindField(SearchPreview)->AddAttribute(PropertyAttributes::cOptional);
}

MetaEditorResource::MetaEditorResource(bool allowAdd, bool allowNone, StringParam filterTag, bool forceCompact, bool searchPreview) :
    FilterTag(filterTag), AllowAdd(allowAdd), AllowNone(allowNone), ForceCompact(forceCompact), SearchPreview(searchPreview), Filter(nullptr)
{
}

MetaEditorResource::MetaEditorResource(SearchFilter filter) : AllowAdd(false), AllowNone(false), FilterTag(""), ForceCompact(false), SearchPreview(true), Filter(filter)
{
}

bool MetaEditorResource::FilterPropertySearchResult(HandleParam object, Property* property, HandleParam result, Status& status)
{
  if (Filter)
    return Filter(object, property, result, status);
  return true;
}

// Meta Property Filter
RaverieDefineType(MetaPropertyFilter, builder, type)
{
}

// Property Basic Filter
RaverieDefineType(MetaPropertyBasicFilter, builder, type)
{
}

// Meta Editor Gizmo
RaverieDefineType(MetaEditorGizmo, builder, type)
{
  RaverieBindFieldProperty(mGizmoArchetype);
}

// Meta Shader Input
RaverieDefineType(MetaShaderInput, builder, type)
{
  RaverieBindFieldProperty(mFragmentName)->AddAttribute(PropertyAttributes::cOptional);
  RaverieBindFieldProperty(mInputName)->AddAttribute(PropertyAttributes::cOptional);
}

// Meta Group
RaverieDefineType(MetaGroup, builder, type)
{
  RaverieBindFieldProperty(mName);
}

MetaGroup::MetaGroup(StringParam name) : mName(name)
{
}

// Meta Custom Ui
RaverieDefineType(MetaCustomUi, builder, type)
{
}

// Property Rename
RaverieDefineType(MetaPropertyRename, builder, type)
{
}

MetaPropertyRename::MetaPropertyRename(StringParam oldName) : mOldName(oldName)
{
}

} // namespace Raverie
