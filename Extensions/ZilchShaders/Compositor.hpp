///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------ShaderVariableLinkData
// Data needed to link shader variables together (to compose the zilch shader). Cleanup/remove later!
struct ShaderVariableLinkData
{
  // Merge all attributes into one string for declaration
  String GetAttributeString() const;

  String mName;
  String mType;
  HashSet<String> mAttributes;
};

//-------------------------------------------------------------------OrderedVariableMap
// A map that keeps track of insertion order. Needed to guarantee consistent variable order
// generation in compositing which is necessary for determinism in unit tests.
// This is only a base wrapper and is missing many functions!
class OrderedVariableMap
{
public:
  typedef HashMap<ShaderFieldKey, ShaderVariableLinkData*> MapType;
  typedef Pair<ShaderFieldKey, ShaderVariableLinkData*> PairType;
  typedef Array<PairType> ArrayType;

  ~OrderedVariableMap();

  void Clear();

  void InsertOrOverride(const ShaderFieldKey& fieldKey, ShaderVariableLinkData* value);
  void InsertOrOverride(PairType& pair);
  
  bool ContainsKey(const ShaderFieldKey& Key);
  ShaderVariableLinkData* FindOrCreate(const ShaderFieldKey& fieldKey);
  ShaderVariableLinkData* FindOrCreate(ShaderField* field);
  ShaderVariableLinkData* FindValue(const ShaderFieldKey& fieldKey, ShaderVariableLinkData* defaultValue);

  // Merge all variables from the other map into myself
  void Merge(OrderedVariableMap& otherMap);
  
  typedef ArrayType::range range;
  range All();

  MapType mMap;
  ArrayType mList;
};

//-------------------------------------------------------------------ShaderLinkDefinition
// Definition of inputs/outputs for use during linking shader stages together.
struct ShaderLinkDefinition
{
  void AddOutputDef(StringParam name, StringParam type);

  typedef OrderedVariableMap LinkDataMap;
  // The inputs/outputs are compiled into these variables
  LinkDataMap Inputs;
  LinkDataMap Outputs;

  // And after linking the results are stored in these variables
  LinkDataMap mLinkedInputs;
  LinkDataMap mLinkedOutputs;
};

DeclareEnum5(FieldInputType, Property, Stage, Fragment, BuiltInUniform, Unknown);

//-------------------------------------------------------------------ShaderFieldCompositeInfo
struct ShaderFieldCompositeInfo
{
  String mZilchName;
  String mZilchType;
  FieldInputType::Enum mInputType;
  String mFragmentDependencyName;
  bool mInputError;

  ShaderFieldCompositeInfo();
};

//-------------------------------------------------------------------ShaderFragmentCompositeInfo
struct ShaderFragmentCompositeInfo
{
  ShaderFieldCompositeInfo& GetField(ShaderFieldKey fieldKey);
  ShaderFieldCompositeInfo& GetField(ShaderField* field);

  String mFragmentName;

  typedef OrderedHashMap<ShaderFieldKey, ShaderFieldCompositeInfo> FieldMap;
  FieldMap mFieldMap;
};

//-------------------------------------------------------------------ShaderStageInfo
struct ShaderFragmentsInfoMap
{
  ~ShaderFragmentsInfoMap();
  void Generate(Array<ShaderType*>& fragmentsOfType);
  void Clear();

  // <fragmentName, info>
  typedef HashMap<String, ShaderFragmentCompositeInfo*> FragmentInfoMap;
  FragmentInfoMap mFragments;

  // An intrusive reference count for memory handling
  ZilchRefLink(ShaderFragmentsInfoMap);
};

//-------------------------------------------------------------------ShaderStageInfo
struct ShaderStageInfo
{
  void Generate(Array<ShaderType*>& fragmentsOfType);

  typedef ShaderFragmentsInfoMap::FragmentInfoMap::range FragmentInfoRange;
  ShaderFragmentCompositeInfo* GetFragment(StringParam zilchName);
  FragmentInfoRange All();

  Zilch::Ref<ShaderFragmentsInfoMap> mFragmentMap;
  ShaderLinkDefinition::LinkDataMap mExpectedOutputs;
  HashMap<ShaderFieldKey, String> mLastOutputFragments;
  // System variables actually used by this stage
  ShaderLinkDefinition::LinkDataMap mStageSystemVariables;
  ShaderLinkDefinition::LinkDataMap mInputs;
  ShaderLinkDefinition::LinkDataMap mComputedOutputs;
  ShaderLinkDefinition::LinkDataMap mProperties;
  ShaderStageType::Enum mFragmentStage;
};

//-------------------------------------------------------------------ZilchFragmentInfo
// Information about a fragment type after compositing. This contains the auto-generated
// fragment's name so the type can be fetched from the library it is built in.
// The code to be inserted into a ZilchShaderProject for compiling is also here.
// @JoshD: The file name isn't really used properly right now. Do something with it?
class ZilchFragmentInfo
{
public:
  String mZilchClassName;
  String mZilchCode;
  String mFileName;

  // The information about this shader stage, such as what fragment inputs are properties, fragment inputs and so on.
  // Use this to determine how to display each fragment input type for this composite.
  Zilch::Ref<ShaderFragmentsInfoMap> mFragmentMap;
};

//-------------------------------------------------------------------ZilchShaderDefinition
// A definition of a shader to compose together. This controls what fragments are put together into "one shader"
// (Actually produces two: vertex and pixel. The two are put together under one name though).
// The resultant composited zilch scripts are stored back on this class in their respective index on mShaderData.
class ZilchShaderDefinition
{
public:
  String mShaderName;
  Array<ShaderType*> mFragmentTypes;

  ZilchFragmentInfo mShaderData[FragmentType::Size];
};

//-------------------------------------------------------------------GeometryCompositeInfo
// Extra info needed during geometry shader generation.
// Used to pass data through to helper functions more easily.
struct GeometryCompositeInfo
{
  ShaderType* mFragmentInputStreamType;
  ShaderType* mFragmentOutputStreamType;
  ShaderType* mFragmentInputStructType;
  ShaderType* mFragmentOutputStructType;

  String mCompositeShaderTypeName;
  String mCompositeInputStreamTypeName;
  String mCompositeOutputStreamTypeName;
  String mCompositeInputStructTypeName;
  String mCompositeOutputStructTypeName;
};

//-------------------------------------------------------------------EmitVertexCallbackData
// Data sent about the composite used to help generate any extra logic for
// a composite's emit vertex function in a geometry shader.
struct EmitVertexCallbackData
{
  // The name of the composite's shader type
  String mCompositedShaderTypeName;
  // The variable name of the composite's data type. Use this when you
  // need to access the composite's struct local variable.
  String mCompositeOutVarName;
  // The variable name of the fragment's data type. Use this when you
  // need to access the fragment's struct  local variable.
  String mFragmentOutVarName;
  // The shader type of the output struct used in the fragment. Use when you need to
  // determine if a variable exists in the fragment's out data type.
  ShaderType* mFragmentOutputStructShaderType;
};

//-------------------------------------------------------------------CompositeData
// Shared information needed to generate a composited zilch shader.
// Mostly used to more easily pass data around.
struct CompositeData
{
  CompositeData() {};
  CompositeData(StringParam compositeName, StringParam fragmentAttributeName);

  // All of the fragments to build this composite from. These should only be of the correct fragment type for this composite.
  Array<ShaderType*> mFragments;
  
  // The composite material name (not the composited shader's name). Used to generate the shader type name.
  String mCompositeName;
  // The fragment's attribute name. Should be set from the current name settings.
  String mFragmentAttributeName;
  // The type name of the shader composite being generated.
  String mShaderTypeName;
};

//-------------------------------------------------------------------ZilchCompositor
// The compositor takes a input library and a definition of fragments and composes them into a full shader.
// The idea is to take a bunch of small fragments such as texture sampling, phong lighting, normal mapping, etc..
// and dynamically compose them into one shader that performs all of these operations in order.
// This compositor currently produces two zilch shader strings during the process of compositing: one for the
// vertex shader and one for the pixel shader. These strings are filled out on the shader definition passed in
// and it is left to the user to add them to whatever ZilchShaderProject they want for compilation.
// The settings passed in are mostly to control what attributes are used to denote things such as what's a vertex or pixel shader.
// They also control forced built-in uniforms, vertex definitions (varyings), render target names, and so on.
class ZilchCompositor
{
public:
  ZilchCompositor();

  // Takes a library to use (with all of it's dependencies) and a shader definition (a list of vertex/pixel fragments)
  // and composites two shaders. These shaders strings are filled out in their respective index on the shader definition's mShaderData.
  // A translator is needed to determine if invalid shader types are being composited (ie, geometry shaders in glsl130).
  void BuildCompositedShader(BaseShaderTranslator* translator, ZilchShaderLibraryRef inputLibraryRef, ZilchShaderDefinition& shaderDef, ZilchShaderSettingsRef& settings);

  // Callback helper used to generate extra logic when generating a geometry shader's
  // emit vertex helper. For instance, this is used to add zero's api perspective transform.
  typedef void (*CustomEmitVertexCallbackFn)(ShaderCodeBuilder& builder, ZilchShaderSettingsRef& settings, EmitVertexCallbackData& data);
  CustomEmitVertexCallbackFn mEmitVertexCallback;

private:
  typedef ShaderLinkDefinition::LinkDataMap LinkDataMap;

  void GenerateZilchShader(ZilchShaderDefinition& shaderDef);
  void CollectFragmentsOfType(ZilchShaderDefinition& shaderDef, uint fragmentType, Array<ShaderType*>& fragmentsOfType);
  void AddForcedSystemValues(ShaderStageInfo& stageInfo);
  
  //-------------------------------------------------------------------Compositor stage information generation
  void GenerateCpuStage(ShaderStageInfo& gpuStage);
  void GenerateGpuStage(ShaderStageInfo& previousStage, ShaderStageInfo& gpuStage);
  void CollectExpectedStageOutputs(Array<ShaderType*>& fragmentsOfType, ShaderStageInfo& stageInfo);
  void DetermineFragmentInputTypes(ShaderStageInfo& previousStage, ShaderStageInfo& currentStage, Array<ShaderType*>& fragmentsOfType);
  void DetermineGeometryStageInputs(ShaderStageInfo& vertexStageInfo, ShaderStageInfo& geometryStageInfo, ShaderStageInfo& pixelStageInfo);
  void LinkStage(ShaderStageInfo& previousStage, ShaderStageInfo& currentStage, const String& extraAttribute = String());
  //-------------------------------------------------------------------Zilch script emission helpers
  void DeclareBuiltIns(ShaderCodeBuilder& builder, ShaderStageInfo& stageInfo);
  void DeclareStageInputsAndOutputs(ShaderCodeBuilder& builder, ShaderStageInfo& stageInfo);
  void DeclareFragmentProperties(CompositeData& compositeData, ShaderStageInfo& stageInfo, ShaderCodeBuilder& builder);
  void DeclareFragmentInMain(ShaderType* shaderType, CompositeData& compositeData, ShaderStageInfo& stageInfo, ShaderCodeBuilder& builder, HashMap<String, String>& fragmentVarNames);
  //-------------------------------------------------------------------Zilch script vertex/pixel emission
  void CopyFinalStageOutputs(ShaderStageInfo& stageInfo, ShaderCodeBuilder& builder, HashMap<String, String>& fragmentVarNames);
  void DeclareCompositeMain(CompositeData& compositeData, ShaderStageInfo& stageInfo, ShaderCodeBuilder& builder);
  String GenerateZilchComposite(CompositeData& compositeData, ShaderStageInfo& stageInfo);
  //-------------------------------------------------------------------Zilch script geometry emission
  void DeclareGeometryDataStruct(ShaderCodeBuilder& builder, StringParam structName, LinkDataMap& linkedDef);
  void GenerateEmitVertexHelper(ShaderCodeBuilder& builder, CompositeData& compositeData, ShaderStageInfo& vertexInfo, GeometryCompositeInfo& geometryInfo);
  void GenerateGeometryFragmentInputsAndStream(ShaderCodeBuilder& builder, GeometryCompositeInfo& geometryInfo, ShaderStageInfo& vertexInfo);
  void DeclareGeometryMain(GeometryCompositeInfo& geometryInfo, CompositeData& compositeData, ShaderStageInfo& vertexInfo, ShaderStageInfo& primitiveInfo, ShaderCodeBuilder& builder);
  String GenerateZilchComposite(CompositeData& compositeData, ShaderStageInfo& stageInfo, ShaderStageInfo& fragmentInfo);

  // This function should ideally be removed or cleaned up later.
  // Figure out if this type (by string name) is forced to be static.
  bool IsTypeForceStatic(StringParam typeName);

  ZilchShaderSettingsRef mSettings;
  ZilchShaderLibraryRef mInputLibraryRef;
  BaseShaderTranslator* mTranslator;
  String mCurrentLanguageName;
  int mCurrentLanguageVersionNumber;
};

}//namespace Zero
