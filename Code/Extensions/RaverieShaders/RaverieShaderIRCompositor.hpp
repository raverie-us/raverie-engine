// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class ShaderCapabilities
{
public:
  ShaderCapabilities();
  BitField<ShaderStage::Enum> mSupportedStages;
};

DeclareEnum7(CompositorShaderStage, Cpu, Vertex, PreTesselation, PostTesselation, Geometry, Pixel, Gpu);
DeclareEnum7(LinkedFieldType, None, Fragment, Stage, AppBuiltIn, HardwareBuiltIn, Property, SpecConstant);

CompositorShaderStage::Enum FragmentTypeToCompostitorShaderStage(FragmentType::Enum fragmentType);

class CompositorCallbackData;

/// The compositor takes an input of fragments and composes them into a full
/// shader based upon the described inputs and outputs of each fragment. The
/// idea is to compose one shader from a bunch of small re-usable pieces.
class RaverieShaderIRCompositor : public ShaderCompilationErrors
{
public:
  /// Describes a input field on a fragment and how it's input was fulfilled.
  struct ShaderFieldDescription
  {
    /// The description of the field.
    ShaderIRFieldMeta* mMeta;
    /// What kind of input this resolved as.
    LinkedFieldType::Enum mLinkedType;
    /// If this field's LinkedType is property, then this is the name that the
    /// property was declared as.
    String mFieldPropertyName;
    /// If the input was resolved by anything other than property, this is the
    /// output that this input will match to. If this input comes from the
    /// vertex definition then this field's owner will be null.
    ShaderIRFieldMeta* mOutputFieldDependency;
  };
  /// Describes all of the fields of a fragment and how they were resolved.
  struct ShaderFragmentDescription
  {
    /// The reflection information about the fragment.
    ShaderIRTypeMeta* mMeta;
    /// The fields that were resolved as inputs.
    /// @JoshD: Currently this doesn't include non-copyable types (samplers).
    /// Should this?
    OrderedHashMap<ShaderIRFieldMeta*, ShaderFieldDescription> mFieldDescriptions;
  };
  /// A helper class that manages memory.
  struct FragmentDescriptionMap : public OrderedHashMap<ShaderIRTypeMeta*, ShaderFragmentDescription*>
  {
    // An intrusive reference count for memory handling
    RaverieRefLink(FragmentDescriptionMap);

    ~FragmentDescriptionMap();
    void Clear();
  };
  /// Describes the final shader for one shader stage.
  struct ShaderStageDescription
  {
    ~ShaderStageDescription();
    void Clear();

    /// The generated raverie shader string.
    String mShaderCode;
    /// The name of the generated raverie class.
    String mClassName;
    /// What shader stage this is.
    FragmentType::Enum mFragmentType;
    /// The description of each fragment in this stage. Describes how inputs
    /// were resolved for each fragment.
    Raverie::Ref<FragmentDescriptionMap> mFragmentDescriptions;
  };
  /// The definition of a shader used to composite.
  struct ShaderDefinition
  {
    ShaderDefinition();

    /// The base name to use for the composited shader. Must be unique within a
    /// library dependency chain.
    String mShaderName;
    /// The input fragments to composite together.
    Array<RaverieShaderIRType*> mFragments;
    /// The resultant shader stages. This includes the composited raverie shader
    /// and the description of each fragment that was used to create the shader.
    ShaderStageDescription mResults[FragmentType::Size];

    /// Any extra attributes to add to declared compositor types (mostly for
    /// unit tests)
    ShaderIRAttributeList mExtraAttributes;
  };
  /// Information needed to composite a compute shader. This is used to set properties that
  /// need to exist for the whole shader that don't make sense to set (and match) on individual fragments.
  struct ComputeShaderProperties
  {
    ComputeShaderProperties();
    ComputeShaderProperties(int localSizeX, int localSizeY, int localSizeZ);
    int mLocalSizeX;
    int mLocalSizeY;
    int mLocalSizeZ;
  };

  RaverieShaderIRCompositor();

  /// Composite a shader for the rendering pipeline. Doesn't handle compute
  /// fragments and only allows one geometry fragment.
  bool Composite(ShaderDefinition& shaderDef,
                 const ShaderCapabilities& capabilities,
                 RaverieShaderSpirVSettingsRef& settings);
  /// Composite a compute shader. Compute properties should be passed in to override workgroup sizes.
  /// If null is passed through, the first fragment's properties are used (mostly legacy for unit testing).
  bool CompositeCompute(ShaderDefinition& shaderDef,
                        ComputeShaderProperties* computeProperties,
                        const ShaderCapabilities& capabilities,
                        RaverieShaderSpirVSettingsRef& settings);

  struct StageLinkingInfo;
  struct CompositedShaderInfo;
  struct ResolvedFieldInfo;
  struct StageAttachmentLinkingInfo;
  struct FieldLinkingInfo;
  struct FragmentLinkingInfo;
  struct ExpectedOutputData;

  void CollectFragmentsPerStage(Array<RaverieShaderIRType*>& inputFragments, CompositedShaderInfo& compositeInfo);
  bool ValidateStages(CompositedShaderInfo& compositeInfo);
  void CollectExpectedOutputs(CompositedShaderInfo& compositeInfo);
  void CollectExpectedOutputs(Array<RaverieShaderIRType*>& fragmentTypes, StageAttachmentLinkingInfo& linkingInfo);
  void CreateCpuStage(CompositedShaderInfo& compositeInfo);

  void ResolveInputs(StageLinkingInfo* previousStage, StageLinkingInfo* currentStage);
  void Link(StageAttachmentLinkingInfo& prevStageInfo,
            StageLinkingInfo* currentStage,
            Array<RaverieShaderIRType*>& fragmentTypes,
            StageAttachmentLinkingInfo& currStageInfo);
  void AddStageInput(ExpectedOutputData* previousStageOutputData,
                     StageAttachmentLinkingInfo* currentStage,
                     StringParam fieldVarName,
                     StringParam fieldAttributeName);
  ShaderIRFieldMeta* FindUniform(ShaderFieldKey& fieldKey, FragmentType::Enum fragmentType);
  ShaderIRFieldMeta* FindHardwareBuiltInInput(ShaderFieldKey& fieldKey, FragmentType::Enum fragmentType);
  ShaderIRFieldMeta* FindHardwareBuiltInOutput(ShaderFieldKey& fieldKey, FragmentType::Enum fragmentType);
  ShaderFieldKey MakeFieldKey(ShaderIRFieldMeta* fieldMeta, ShaderIRAttribute* attribute);
  static String MakePropertyName(StringParam fieldName, StringParam ownerType);
  static String GetFieldInOutName(ShaderIRFieldMeta* fieldMeta, ShaderIRAttribute* attribute);
  static void GetStageFieldName(ShaderIRFieldMeta* fieldMeta,
                                ShaderIRAttribute* attribute,
                                String& fieldVarNameOut,
                                String& fieldAttributeNameOut);
  void ResolveGpuStage(CompositedShaderInfo& compositeInfo);

  void ResolveStageLinkOrder(CompositedShaderInfo& compositeInfo);

  void GenerateRaverieComposite(StageLinkingInfo* currentStage,
                              ShaderStageDescription& stageResults,
                              ShaderIRAttributeList& extraAttributes);
  void GenerateBasicRaverieComposite(StageLinkingInfo* currentStage,
                                   ShaderStageDescription& stageResults,
                                   ShaderIRAttributeList& extraAttributes);
  void GenerateGeometryRaverieComposite(StageLinkingInfo* currentStage,
                                      ShaderStageDescription& stageResults,
                                      ShaderIRAttributeList& extraAttributes);
  void GenerateComputeRaverieComposite(StageLinkingInfo* currentStage,
                                     ShaderStageDescription& stageResults,
                                     ShaderIRAttributeList& extraAttributes);
  void CreateFragmentAndCopyInputs(StageLinkingInfo* currentStage,
                                   ShaderCodeBuilder& builder,
                                   StringParam currentClassName,
                                   RaverieShaderIRType* fragmentType,
                                   StringParam fragmentVarName);
  void DeclareFieldsInOrder(ShaderCodeBuilder& builder,
                            StageAttachmentLinkingInfo& linkingInfo,
                            OrderedHashSet<ShaderFieldKey>& orderMap);
  void DeclareFieldsWithAttribute(ShaderCodeBuilder& builder,
                                  StageAttachmentLinkingInfo& linkingInfo,
                                  OrderedHashSet<ShaderFieldKey>& fieldSet,
                                  StringParam attributeName);
  String MakeFragmentVarName(ShaderIRTypeMeta* typeMeta);

  void GenerateStageDescriptions(CompositedShaderInfo& compositeInfo, ShaderDefinition& shaderDef);

  RaverieShaderSpirVSettingsRef mSettings;
  ShaderCapabilities mCapabilities;
  String mShaderCompositeName;
  ComputeShaderProperties* mComputeShaderProperties;

  /// Used to store how each field on a fragment is linked together in the final
  /// shader.
  struct FieldLinkingInfo
  {
    FieldLinkingInfo();

    /// The description of the field.
    ShaderIRFieldMeta* mFieldMeta;
    /// What kind of input this resolved as.
    LinkedFieldType::Enum mLinkedType;
    /// If this field's LinkedType is property, then this is the name that the
    /// property was declared as.
    String mFieldPropertyName;
    /// If the input was resolved by anything other than property, this is the
    /// output that this input will match to. If this input comes from the
    /// vertex definition then this field's owner will be null.
    ShaderIRFieldMeta* mOutputFieldDependency;
    /// The name of the field this comes from could be different than the name
    /// of the field and it's impossible to re-map due to supporting multiple
    /// attributes.
    /// @JoshD: Potentially make a field meta for composites and link to that
    /// instead.
    String mOutputFieldDependencyName;
  };

  /// Stores linking information about each fragment, in particular how each
  /// field is resolved.
  struct FragmentLinkingInfo
  {
    void CopyTo(ShaderFragmentDescription& fragDesc);
    void CopyFieldTo(FieldLinkingInfo& fieldInfo, ShaderFragmentDescription& fragDesc);

    ShaderIRTypeMeta* mMeta;
    HashMap<ShaderFieldKey, FieldLinkingInfo> mFieldMap;
    /// Fields that are resolved as inputs (in order). The field key maps into
    /// FieldMap
    Array<ShaderFieldKey> mFieldList;
    /// Some properties are non-copyable (like images) so they can't be declared
    /// on the composite, we do need to generate reflection data for this
    /// property though so we store the extra data here.
    Array<FieldLinkingInfo> mNonCopyableProperties;
  };

  /// When a field is added to the shader stage it can fulfill multiple in/outs
  /// (e.g. [StageInput][FragmentOutput][StageOutput]). This keeps track of all
  /// attributes that need to be applied to a field for final declaration.
  struct ResolvedFieldInfo
  {
    ShaderIRAttributeList mAttributes;
    String mFieldType;
    String mFieldName;
  };

  /// Information about a resolved field. In particular, this contains the name
  /// of the stage output to use as name overrides may make this different then
  /// the actual field's name. It's only during linking that the actual name is
  /// known since multiple attribute can exist, each with their own overriding
  /// name.
  struct ResolvedFieldOutputInfo
  {
    ResolvedFieldOutputInfo();
    ResolvedFieldOutputInfo(StringParam fieldName, ShaderIRFieldMeta* outputDependency);

    String mFieldName;
    ShaderIRFieldMeta* mOutputFieldDependency;
  };

  /// Information about an expected output. In particular, this stores the
  /// fragment stage type for the expected field output. This is important to
  /// distinguish pass-through from actual outputs.
  struct ExpectedOutputData
  {
    ExpectedOutputData()
    {
      mOutputFieldDependency = nullptr;
      mFragmentType = FragmentType::None;
    }
    ExpectedOutputData(ShaderIRFieldMeta* meta, FragmentType::Enum fragmentType)
    {
      mOutputFieldDependency = meta;
      mFragmentType = fragmentType;
    }

    FragmentType::Enum mFragmentType;
    ShaderIRFieldMeta* mOutputFieldDependency;
  };

  /// Describes a stage's attachment point (e.g. vertex/primitive) linking info.
  struct StageAttachmentLinkingInfo
  {
    StageAttachmentLinkingInfo();

    void AddResolvedField(ShaderIRFieldMeta* fieldMeta, StringParam attributeName);
    void AddResolvedFieldProperty(ShaderIRFieldMeta* fieldMeta, ShaderIRAttribute* attribute);
    void AddResolvedStageField(SpirVNameSettings& nameSettings,
                               ShaderIRFieldMeta* fieldMeta,
                               StringParam fieldName,
                               StringParam attributeName,
                               StringParam attributeParameter);
    // Create or return a field by the given name/type. Creates or finds the
    // attribute by the given name and returns it.
    ShaderIRAttribute* AddResolvedField(StringParam fieldName, StringParam fieldType, StringParam attributeName);

    // Internal helper that creates a resolved field by name and type.
    // Returns a pointer to the created field (should not be stored, the pointer
    // is not safe).
    ResolvedFieldInfo* CreateResolvedField(StringParam fieldName, StringParam fieldType);

    /// Potential output from a this stage. These may become outputs if a later
    /// stage inputs them.
    OrderedHashMap<ShaderFieldKey, ExpectedOutputData> mExpectedOutputs;
    /// Actually resolved outputs (e.g. outputs that are input later).
    OrderedHashMap<ShaderFieldKey, ResolvedFieldOutputInfo> mResolvedOutputs;
    /// Stored information about each field that is resolved.
    OrderedHashMap<ShaderFieldKey, ResolvedFieldInfo> mResolvedFields;
    OrderedHashSet<ShaderFieldKey> mInputs;
    OrderedHashSet<ShaderFieldKey> mOutputs;
    OrderedHashSet<ShaderFieldKey> mHardwareInputs;
    OrderedHashSet<ShaderFieldKey> mHardwareOutputs;

    /// The stage that owns this attachment point. Used to find what shader
    /// stage this is.
    StageLinkingInfo* mOwningStage;
    /// The previous/next stage attachment points that are currently active.
    StageAttachmentLinkingInfo* mPreviousStage;
    StageAttachmentLinkingInfo* mNextStage;
  };

  /// Describes all info about a stage being composited. This includes the final
  /// properties on the composite as well as how each fragment's inputs/outputs
  /// are resolved.
  struct StageLinkingInfo
  {
    StageLinkingInfo();

    void SetFragmentType(FragmentType::Enum fragType);
    void Clear();

    /// All fragments belonging to the current stage.
    Array<RaverieShaderIRType*> mFragmentTypes;
    /// Vertex types are types that transfer vertex data via the standard
    /// rendering pipeline. This is the all vertex fragments, all pixel
    /// fragments, and the input/output types for the geometry streams.
    /// Input/Output are split due to having separate input/output resolution
    /// for geometry stages (and later tessellation).
    Array<RaverieShaderIRType*> mInputVertexTypes;
    Array<RaverieShaderIRType*> mOutputVertexTypes;
    /// Any fragments that transfer data for a primitive. This is the primary
    /// geometry fragment (and later tessellation).
    Array<RaverieShaderIRType*> mPrimitiveTypes;

    /// Describes what each fragment's inputs were resolved to.
    HashMap<RaverieShaderIRType*, FragmentLinkingInfo> mFragmentLinkInfoMap;

    /// Linking information for the vertex and primitive data.
    /// This describes what the input/outputs are for this stage.
    StageAttachmentLinkingInfo mVertexLinkingInfo;
    StageAttachmentLinkingInfo mPrimitiveLinkingInfo;

    /// What fragment type is this stage for?
    FragmentType::Enum mFragmentType;
    /// Shader stage flag (needed for validation)
    ShaderStage::Enum mShaderStage;
  };

  // Describes all of the stages for the shader being composited.
  struct CompositedShaderInfo
  {
    StageLinkingInfo mStages[CompositorShaderStage::Size];
    // All stages to actually process for this shader (pointers into mStages)
    Array<StageLinkingInfo*> mActiveStages;
  };

  // Default callback we use for api perspective position.
  static void ApiPerspectivePositionCallback(CompositorCallbackData& callbackData, void* userData);
};

class CompositorCallbackData
{
public:
  RaverieShaderIRCompositor* mCompositor;
  RaverieShaderIRCompositor::StageLinkingInfo* mStageLinkingInfo;
  RaverieShaderSpirVSettings* mSettings;
};

} // namespace Raverie
