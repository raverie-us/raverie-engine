// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Editor Property Extension
// Property Extensions
class EditorPropertyExtension : public MetaAttribute
{
public:
  ZilchDeclareType(EditorPropertyExtension, TypeCopyMode::ReferenceType);

  virtual ~EditorPropertyExtension(){};
};

// Indexed String Array
typedef void (*GetStringsFunc)(HandleParam instance, Property* property, Array<String>& strings);
extern const String StringArrayEdit;

class EditorIndexedStringArray : public EditorPropertyExtension
{
public:
  ZilchDeclareType(EditorIndexedStringArray, TypeCopyMode::ReferenceType);

  EditorIndexedStringArray(GetStringsFunc sourceArray);
  ~EditorIndexedStringArray()
  {
  }

  void Enumerate(HandleParam instance, Property* property, Array<String>& strings);

  // If the function pointer is present, it will be called
  // otherwise the array of values will be used
  GetStringsFunc AccessSourceArray;
  Array<String> mFixedValues;
};

// Editor Range
class EditorRange : public EditorPropertyExtension
{
public:
  ZilchDeclareType(EditorRange, TypeCopyMode::ReferenceType);

  EditorRange();
  EditorRange(float minValue, float maxValue, float increment);

  void PostProcess(Status& status, ReflectionObject* owner) override;

  float Min;
  float Max;
  float Increment;
};

// Editor Slider
class EditorSlider : public EditorRange
{
public:
  ZilchDeclareType(EditorSlider, TypeCopyMode::ReferenceType);

  EditorSlider();

  using EditorRange::EditorRange;
};

// EditorRotationBasis
/// Editor for creating a gizmo to modify a basis. The int is occasionally used
/// to denote what "property" is being modified. Currently it's not easy to set
/// just a property as several properties need to be sampled (see
/// RevoluteBasisGizmo).
class EditorRotationBasis : public EditorPropertyExtension
{
public:
  ZilchDeclareType(EditorRotationBasis, TypeCopyMode::ReferenceType);

  EditorRotationBasis();
  EditorRotationBasis(StringParam archetypeName);
  EditorRotationBasis(StringParam archetypeName, StringParam gizmoName, int intData);

  int mIntData;
  String mGizmoName;
  String mArchetypeName;
};

// Editor Resource
class MetaEditorResource : public EditorPropertyExtension
{
public:
  ZilchDeclareType(MetaEditorResource, TypeCopyMode::ReferenceType);

  typedef bool (*SearchFilter)(HandleParam object, Property* property, HandleParam result, Status& status);

  MetaEditorResource(bool allowAdd = false,
                     bool allowNone = false,
                     StringParam filterTag = "",
                     bool forceCompact = false,
                     bool searchPreview = true);
  MetaEditorResource(SearchFilter filter);

  /// Custom filter for the Resource search for this property. Return false to
  /// not show the result. Set the status to failed to make it not selectable,
  /// but still shown.
  virtual bool FilterPropertySearchResult(HandleParam object, Property* property, HandleParam result, Status& status);

  /// If not empty only match resources with same FilterTag
  String FilterTag;
  /// Allows a plus button for easy adding of new resource
  bool AllowAdd;
  /// Are null resources allowed?
  bool AllowNone;
  bool ForceCompact;
  /// If true, when mousing over a Resource in the search view for this
  /// property, it will temporarily set the property to the current Resource as
  /// to preview the outcome.
  bool SearchPreview;
  SearchFilter Filter;
};

// Meta Property Filter
class MetaPropertyFilter : public MetaAttribute
{
public:
  ZilchDeclareType(MetaPropertyFilter, TypeCopyMode::ReferenceType);

  virtual ~MetaPropertyFilter()
  {
  }

  // Return false to hide the property
  // (prop will be either a Property or Function with no parameters)
  virtual bool Filter(Member* prop, HandleParam instance) = 0;
};

// Template Filter Base
class TemplateFilterBase
{
public:
  virtual bool Filter(Member* prop, HandleParam instance) = 0;
};

// Template Filter Bool
template <typename ClassType, bool ClassType::*ClassMember>
class TemplateFilterBool : public TemplateFilterBase
{
public:
  bool Filter(Member* prop, HandleParam instance) override
  {
    ClassType* pointer = instance.Get<ClassType*>(GetOptions::AssertOnNull);
    return pointer->*ClassMember;
  }
};

// Template Filter Not Bool
template <typename ClassType, bool ClassType::*ClassMember>
class TemplateFilterNotBool : public TemplateFilterBase
{
public:
  bool Filter(Member* prop, HandleParam instance) override
  {
    ClassType* pointer = instance.Get<ClassType*>(GetOptions::AssertOnNull);
    return !(pointer->*ClassMember);
  }
};

// Template Filter Equality
template <typename ClassType, typename ValueType, ValueType ClassType::*ClassMember, ValueType Value>
class TemplateFilterEquality : public TemplateFilterBase
{
public:
  bool Filter(Member* prop, HandleParam instance) override
  {
    ClassType* pointer = instance.Get<ClassType*>(GetOptions::AssertOnNull);
    return pointer->*ClassMember == Value;
  }
};
// Property Basic Filter
class MetaPropertyBasicFilter : public MetaPropertyFilter
{
public:
  ZilchDeclareType(MetaPropertyBasicFilter, TypeCopyMode::ReferenceType);

  MetaPropertyBasicFilter(TemplateFilterBase* filter = nullptr) : mActualFilter(filter)
  {
  }
  ~MetaPropertyBasicFilter()
  {
    if (mActualFilter)
      delete mActualFilter;
  }

  bool Filter(Member* prop, HandleParam instance)
  {
    return mActualFilter->Filter(prop, instance);
  }

  TemplateFilterBase* mActualFilter;
};

// Meta Shader Input
class MetaShaderInput : public MetaAttribute
{
public:
  ZilchDeclareType(MetaShaderInput, TypeCopyMode::ReferenceType);

  String mFragmentName;
  String mInputName;
};

// Meta Editor Gizmo
class MetaEditorGizmo : public MetaAttribute
{
public:
  ZilchDeclareType(MetaEditorGizmo, TypeCopyMode::ReferenceType);
  String mGizmoArchetype;
};

#define ZeroFilterBool(Member) Add(new MetaPropertyBasicFilter(new TemplateFilterBool<ZilchSelf, &ZilchSelf::Member>()))

#define ZeroFilterNotBool(Member)                                                                                      \
  Add(new MetaPropertyBasicFilter(new TemplateFilterNotBool<ZilchSelf, &ZilchSelf::Member>()))

#define ZeroFilterEquality(Member, MemberType, ConstantValue)                                                          \
  Add(new MetaPropertyBasicFilter(                                                                                     \
      new TemplateFilterEquality<ZilchSelf, MemberType, &ZilchSelf::Member, ConstantValue>()))

// Meta Group
/// Used for grouping properties in the property grid.
class MetaGroup : public MetaAttribute
{
public:
  ZilchDeclareType(MetaGroup, TypeCopyMode::ReferenceType);
  MetaGroup(StringParam name = String());
  String mName;
};

// Meta Custom Ui
/// Used for adding custom Ui to the property grid. This is currently only for
/// the old Ui and not exposed to script.
class MetaCustomUi : public MetaAttribute
{
public:
  ZilchDeclareType(MetaCustomUi, TypeCopyMode::ReferenceType);
  virtual void CreateUi(void* parentComposite, HandleParam object) = 0;
};

// Property Rename
/// Add to properties to handle old files with old property names.
class MetaPropertyRename : public MetaAttribute
{
public:
  ZilchDeclareType(MetaPropertyRename, TypeCopyMode::ReferenceType);

  MetaPropertyRename()
  {
  }
  MetaPropertyRename(StringParam oldName);

  String mOldName;
};

} // namespace Zero
