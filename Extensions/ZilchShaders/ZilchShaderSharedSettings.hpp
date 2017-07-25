///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------FieldKeyMap
/// A simple wrapper around a hash map that will delete the pointers in this map upon destruction
struct FieldKeyMap : public HashMap<ShaderFieldKey, ShaderField*>
{
  ~FieldKeyMap()
  {
    DeleteObjectsInContainer(*this);
  }
};

// What kind of shader stage an object is. The geometry stage is split into 2 parts:
// the primitive inputs (per shader) and the vertex input/outputs (the per-vertex data).
DeclareEnum4(ShaderStageType, Vertex, GeometryPrimitive, GeometryVertex, Pixel);

//-------------------------------------------------------------------NameSettings
// This class just caches a bunch of strings that are used in translation/compositing.
// This is technically faster than using string literals everywhere (only allocates once) and it allows easy renaming later.
class NameSettings
{
public:
  NameSettings();

  String mPerspectivePositionName;

  String mPixelAttributeName;
  String mVertexAttributeName;
  String mGeometryAttributeName;

  String mStaticAttribute;
  String mPreConstructorName;
  String mConstructorName;
  String mReferenceKeyword;
  String mMainAttribute;
  String mNoMangleAttributeName;
  String mEmitVertexHelperFunctionName;
  //@JoshD-ToDo (propagate this variable everywhere...)
  String mThisKeyword;

  String mMainAttributeTag;
  String mUniformTag;
  String mStageInputTag;
  String mStageOutputTag;
  String mSystemValueInputTag;
  String mSystemValueOutputTag;

  String mInputAttributeName;
  String mOutputAttributeName;
  String mInputTag;
  String mOutputTag;
  String mFragmentInputAttributeName;
  String mStageInputAttributeName;
  String mStageOutputAttributeName;
  String mBuiltinInputAttributeName;
  String mPropertyInputAttributeName;
  String mSystemValueInputAttributeName;
  String mSystemValueOutputAttributeName;
  // Attributes that are implied by the [Input] attribute
  Array<String> mInputSubAttributes;
  String mGeometryShaderOutputTag;

  String mImplementAttribute;
  String mExtensionAttribute;

  String mIntrinsicAttribute;
  String mLanguageParamName;
  String mTypeNameParamName;
  String mStorageQualifierParamName;
  String mRefTypeParamName;
  String mPropertyTypeParamName;
  String mNonCopyableParamName;
  String mForcedStaticParamName;
  String mMaxVerticesParamName;

  String mSharedInputAttributeName;
  String mSharedInputNameParamName;

  String mUniformName;
  String mGeometryShaderOutputAttribute;

  String mPixelShaderInputPrefix;
  String mGeometryShaderInputPrefix;
  String mAttributePrefix;

  HashSet<String> mAllowedClassAttributes;
  HashSet<String> mAllowedFunctionAttributes;
  HashSet<String> mAllowedFieldAttributes;

  static String mVertexIntrinsicAttributeName;
  static String mGeometryIntrinsicAttributeName;
  static String mPixelIntrinsicAttributeName;
  static String mNonCopyableAttribute;
  static String mForcedStaticAttribute;
};

//-------------------------------------------------------------------ShaderDefinitionSettings
// Shader settings for various definitions including render target names, built-in variables
// (uniforms like time) as well as vertex definitions. All of these are needed to know
// how to compose/translate zilch shaders together.
class ShaderDefinitionSettings
{
public:
  // Set what's the max number of render targets that can be used
  void SetMaxSimultaneousRenderTargets(size_t maxNumber);
  // Render targets are resolved via name.
  void SetRenderTargetName(StringParam varName, size_t targetIndex);

  //@JoshD: Add default value?
  void AddBuiltIn(StringParam name, StringParam type);
  void AddVertexDefinition(StringParam varName, StringParam varType);

  FieldKeyMap mBuiltIns;
  FieldKeyMap mVertexDefinitions;

  // The mapping of render target index to render target name
  Array<String> mRenderTargetNames;
};

//-------------------------------------------------------------------LanguageSystemValue
DeclareBitField3(SystemValueStates, Input, Output, Forced);
/// Information needed to translate a system value to a specific language
class LanguageSystemValue
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);

  LanguageSystemValue();

  // The input and output names can be different. For instance, in glsl there
  // cannot be an input/output with the same name.

  // Set the input type, name, and attribute for this system value.
  void SetInput(StringParam type, StringParam name, StringParam attribute);
  // Set the output type, name, and attribute for this system value.
  void SetOutput(StringParam type, StringParam name, StringParam attribute);
  // Set the input and output type, name, and attribute for this system value.
  void SetInputAndOutput(StringParam type, StringParam name, StringParam attribute);

  // Does this system value have to be written to? Controls if the compositor will force a declaration.
  bool GetForced();
  void SetForced(bool state);

  bool IsInput() const;
  bool IsOutput() const;

  BitField<SystemValueStates::Enum> mIoMode;
  int mMinVersion;
  int mMaxVersion;

  String mInputType;
  String mInputName;
  String mOutputType;
  String mOutputName;
};

//-------------------------------------------------------------------ShaderSystemValue
/// A collection of system values for all languages and version ranges.
class ShaderSystemValue
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);

  LanguageSystemValue* AddLanguageSystemValue(StringParam language);

  String mName;
  String mType;
  ShaderFieldKey mKey;
  typedef HashMap<String, LanguageSystemValue> LanguageSystemValuesMap;
  LanguageSystemValuesMap mLanguages;
};

//-------------------------------------------------------------------ShaderSystemValueStageSettings
class ShaderSystemValueStageSettings
{
public:
  ShaderSystemValue* AddSystemValue(StringParam type, StringParam name);
  bool ContainsSystemValue(ShaderFieldKey& fieldKey);
  ShaderSystemValue* FindSystemValue(ShaderFieldKey& fieldKey);

  typedef HashMap<ShaderFieldKey, ShaderSystemValue> LanguageSystemValuesMap;
  LanguageSystemValuesMap mSystemValues;
};

//-------------------------------------------------------------------ShaderSystemValueSettings
/// A collection of system values for all shader stages.
/// System values are special inputs (such as VertexId) for certain shader stages.
class ShaderSystemValueSettings
{
public:
  ZilchDeclareType(Zilch::TypeCopyMode::ReferenceType);

  ShaderSystemValueSettings();

  ShaderSystemValue* AddSystemValue(ShaderStageType::Enum fragmentStage, StringParam name, StringParam type);
  
  bool ContainsSystemValue(ShaderStageType::Enum fragmentStage, ShaderFieldKey& fieldKey, StringParam language, int languageVersion);
  // Find a shader system value (no language specifics)
  ShaderSystemValue * FindSystemValue(ShaderStageType::Enum fragmentStage, ShaderFieldKey& fieldKey);
  // Various helpers to find a system value for a language
  LanguageSystemValue* FindSystemValue(ShaderStageType::Enum fragmentStage, ShaderFieldKey& fieldKey, StringParam language);
  LanguageSystemValue* FindSystemValue(ShaderStageType::Enum fragmentStage, ShaderFieldKey& fieldKey, StringParam language, int languageVersion);
  LanguageSystemValue* FindSystemValue(ShaderField* shaderField, BaseShaderTranslator* translator);
  LanguageSystemValue* FindSystemValue(ShaderStageType::Enum fragmentStage, ShaderField* shaderField, BaseShaderTranslator* translator);
  LanguageSystemValue* FindSystemValue(ShaderStageType::Enum fragmentStage, ShaderFieldKey& shaderFieldKey, BaseShaderTranslator* translator);

  // Helper to make finding a system value given a field (which can look up it's fragment type) easier.
  ShaderStageType::Enum FragmentTypeToShaderStageType(FragmentType::Enum fragmentType);

  Array<ShaderSystemValueStageSettings> mShaderStages;
};

//-------------------------------------------------------------------ShaderSettings
// A collection of various settings to be used all though zilch shaders. This makes it easier to group together
// and pass around a lot of values as well as avoiding extra allocations.
class ZilchShaderSettings
{
public:
  // A special key is needed to lookup a default constructor (when it was implicitly created) via a pointer.
  static void* GetDefaultConstructorKey();

  ShaderDefinitionSettings mShaderDefinitionSettings;
  NameSettings mNameSettings;
  ShaderSystemValueSettings mSystemValueSettings;

  // To guarantee a unique key the address of this variable is used as the default constructor key
  static char mDefaultConstructorKey;

  // An intrusive reference count for memory handling
  ZilchRefLink(ZilchShaderSettings);
};

String GenerateMangledName(StringParam shaderTypeName, StringParam symbolName);
// A helper to generate a uniform's name for a field (not guaranteed to be what an
// actual uniform name is due to how it ends up being translated)
String GenerateFieldUniformName(ShaderField* shaderField);
String GenerateFieldUniformName(StringParam zilchTypeName, StringParam fieldName);

}//namespace Zero
