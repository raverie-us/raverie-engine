// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class RaverieShaderSpirVSettings;
class BuiltInStageDescription;
class EntryPointGeneration;
class RaverieShaderIRCompositor;
class AppendCallbackData;
class CompositorCallbackData;

// Helper macro to add bitwise operators to enums
#define DeclareBitFieldBitwiseOperators(bitFieldEnum)                                                                                                                                                  \
  inline bitFieldEnum operator|(bitFieldEnum a, bitFieldEnum b)                                                                                                                                        \
  {                                                                                                                                                                                                    \
    return bitFieldEnum(int(a) | int(b));                                                                                                                                                              \
  }                                                                                                                                                                                                    \
  inline void operator|=(bitFieldEnum& a, bitFieldEnum b)                                                                                                                                              \
  {                                                                                                                                                                                                    \
    a = a | b;                                                                                                                                                                                         \
  }                                                                                                                                                                                                    \
  inline bitFieldEnum operator&(bitFieldEnum a, bitFieldEnum b)                                                                                                                                        \
  {                                                                                                                                                                                                    \
    return bitFieldEnum(int(a) & int(b));                                                                                                                                                              \
  }                                                                                                                                                                                                    \
  inline void operator&=(bitFieldEnum& a, bitFieldEnum b)                                                                                                                                              \
  {                                                                                                                                                                                                    \
    a = a & b;                                                                                                                                                                                         \
  }

/// @JoshD: Unify later with the FragmentType enum. This needs to be a bitfield
/// due to the buffer stage binding.
DeclareBitField6(ShaderStage, Vertex, PreTesselation, PostTesselation, Geometry, Pixel, Compute);
DeclareBitFieldBitwiseOperators(ShaderStage::Enum);

ShaderStage::Enum FragmentTypeToShaderStage(FragmentType::Enum fragmentType);
/// Converts the shader stage bit field to a fragment type.
/// This will pick the first found shader stage
/// (since multiple can be set) using the defined order.
FragmentType::Enum ShaderStageToFragmentType(ShaderStage::Enum shaderStage);

/// Extra data to store for our allowed attributes.
/// Currently only used to hide attributes from code completion.
struct AttributeInfo
{
  typedef Raverie::ConstantType::Enum ParamType;
  struct ParameterSignature
  {
    Array<ParamType> mTypes;
  };

  AttributeInfo()
  {
    mHidden = false;
  }
  AttributeInfo(bool hidden)
  {
    mHidden = hidden;
  }

  AttributeInfo& AddSignature(ParamType p0)
  {
    return AddSignature(Array<ParamType>(RaverieInit, p0));
  }
  AttributeInfo& AddSignature(ParamType p0, ParamType p1)
  {
    return AddSignature(Array<ParamType>(RaverieInit, p0, p1));
  }
  AttributeInfo& AddSignature(ParamType p0, ParamType p1, ParamType p2)
  {
    return AddSignature(Array<ParamType>(RaverieInit, p0, p1, p2));
  }
  AttributeInfo& AddSignature(ParamType p0, ParamType p1, ParamType p2, ParamType p3)
  {
    return AddSignature(Array<ParamType>(RaverieInit, p0, p1, p2, p3));
  }
  AttributeInfo& AddSignature(Array<ParamType> types)
  {
    ParameterSignature& signature = mOverloads.PushBack();
    signature.mTypes = types;
    return *this;
  }

  bool mHidden;
  bool mCheckSignature = true;

  Array<ParameterSignature> mOverloads;
};

/// Name settings used in the RaverieSpirV translator. This allows
/// configuring various attribute names, function names, etc...
class SpirVNameSettings
{
public:
  SpirVNameSettings();

  static String mNonCopyableAttributeName;
  static String mStorageClassAttribute;
  static String mNameOverrideParam;
  static String mRequiresPixelAttribute;
  static String mRuntimeArrayTypeName;

  String mInputAttribute;
  String mFragmentInputAttribute;
  String mStageInputAttribute;
  String mHardwareBuiltInInputAttribute;
  String mAppBuiltInInputAttribute;
  String mPropertyInputAttribute;
  String mSpecializationConstantInputAttribute;
  /// Used to represent that a property should be shared between all
  /// fragments (instead of a unique copy per fragments). For instance,
  /// this can be used to have multiple fragments reference the same RuntimeArrays.
  String mFragmentSharedAttribute;

  String mOutputAttribute;
  String mFragmentOutputAttribute;
  String mStageOutputAttribute;
  String mHardwareBuiltInOutputAttribute;

  String mStaticAttribute;
  String mExtensionAttribute;
  String mImplementsAttribute;
  String mVertexAttribute;
  String mGeometryAttribute;
  String mPixelAttribute;
  String mComputeAttribute;
  String mComputeLocalSizeXParam;
  String mComputeLocalSizeYParam;
  String mComputeLocalSizeZParam;

  Array<String> mFragmentTypeAttributes;
  Array<String> mRequiresAttributes;

  String mMaxVerticesParam;

  String mMainFunctionName;
  String mEntryPointAttributeName;
  String mUnitTestAttribute;
  String mSpecializationConstantAttribute;
  String mApiPerspectivePositionName;
  String mPerspectiveToApiPerspectiveName;

  Array<String> mInputSubAttributes;
  Array<String> mOutputSubAttributes;

  HashMap<String, AttributeInfo> mAllowedClassAttributes;
  HashMap<String, AttributeInfo> mAllowedFunctionAttributes;
  HashMap<String, AttributeInfo> mAllowedFieldAttributes;
};

/// Describes a uniform buffer (constant buffer). A buffer is given a binding
/// location, descriptor set, and a collection of fields. Additionally a buffer
/// can be only bound for certain shader stages (possibly save memory +
/// performance). Fields are only allowed in one uniform buffer per shader
/// stage.
class UniformBufferDescription
{
public:
  UniformBufferDescription();
  UniformBufferDescription(u32 bindingId, u32 descriptorSetId = 0);
  UniformBufferDescription(const UniformBufferDescription& rhs);
  ~UniformBufferDescription();

  void operator=(const UniformBufferDescription& rhs);
  void CopyFrom(const UniformBufferDescription& source);

  /// Set the common description terms for this uniform buffer.
  void Set(u32 bindingId, u32 descriptorSetId, ShaderStage::Enum allowedStages, StringParam debugName = String());

  /// Add a field to this buffer. Fields are laid out in the order they are
  /// added.
  void AddField(Raverie::BoundType* type, StringParam fieldName);

  /// The register id that this buffer will be bound to.
  u32 mBindingId;
  /// The descriptor set (hlsl: space) for this buffer.
  u32 mDescriptorSetId;
  /// Debug name to emit this buffer with.
  String mDebugName;
  /// The fields that belong to this buffer.
  Array<ShaderIRFieldMeta*> mFields;
  /// What stages this buffer can be considered for.
  BitField<ShaderStage::Enum> mAllowedStages;

  /// Helper mask that represents all stages (default value for a uniform
  /// buffer).
  static ShaderStage::Enum mAllStagesMask;
};

/// Describes a block (might not actually be grouped in a struct) of built-in
/// field descriptions.
class BuiltInBlockDescription
{
public:
  BuiltInBlockDescription();

private:
  friend RaverieShaderSpirVSettings;
  friend BuiltInStageDescription;
  friend EntryPointGeneration;
  friend RaverieShaderIRCompositor;

  /// Represents a built-in field (just the meta type and the id)
  class BuiltInFieldMeta
  {
  public:
    BuiltInFieldMeta();
    BuiltInFieldMeta(const BuiltInFieldMeta& rhs);
    ~BuiltInFieldMeta();

    void operator=(const BuiltInFieldMeta& rhs);
    void CopyFrom(const BuiltInFieldMeta& source);

    ShaderIRFieldMeta* mMeta;
    spv::BuiltIn mId;
  };

  /// Adds a field that maps to the current built-in.
  void AddField(Raverie::BoundType* type, StringParam fieldName, spv::BuiltIn builtInId, StringParam attribute);
  /// Overrides the raverie name that is used to map to a spirv built-in variable.
  void SetBuiltInName(spv::BuiltIn builtInId, StringParam name);
  /// Finds a built-in by key if it exists.
  BuiltInFieldMeta* FindField(ShaderFieldKey fieldKey);

  Array<BuiltInFieldMeta> mFields;
  /// Is this block grouped together in an interface block (a struct with the
  /// Block decoration).
  bool mInterfaceBlock;
};

/// Represents one shader stage's description of built-ins. Each stage consists
/// of inputs and outputs, but additionally some of these may be required to be
/// in an interface block while others may not. Only one interface block per
/// in/out is allowed per stage.
class BuiltInStageDescription
{
public:
  BuiltInStageDescription();

  bool IsValidHardwareBuiltIn(ShaderFieldKey& fieldKey, bool isInput);

private:
  typedef HashMap<ShaderFieldKey, BuiltInBlockDescription*> FieldKeyToBlockMap;

  BuiltInBlockDescription mInputInterfaceBlock;
  BuiltInBlockDescription mInputGlobals;
  BuiltInBlockDescription mOutputInterfaceBlock;
  BuiltInBlockDescription mOutputGlobals;

  /// Cached mappings from each field's key to the description block it came
  /// from
  FieldKeyToBlockMap mInternalInputMappings;
  FieldKeyToBlockMap mInternalOutputMappings;

  friend RaverieShaderSpirVSettings;
  friend EntryPointGeneration;
  friend RaverieShaderIRCompositor;

  void Finalize();
  void Finalize(BuiltInBlockDescription& block, FieldKeyToBlockMap& mappings);

  // Validation functions. These happen before finalization so they
  // must check the un-mapped data. Used for setup error checking.
  bool ValidateIfHardwareBuiltIn(ShaderFieldKey& fieldKey);
  bool ValidateIfHardwareBuiltIn(ShaderFieldKey& fieldKey, BuiltInBlockDescription& block);
};

/// Defines the vertex inputs for the vertex shader stage.
/// Once this has been populated you cannot modify this structure~
class VertexDefinitionDescription
{
public:
  ~VertexDefinitionDescription();

  void AddField(Raverie::BoundType* type, StringParam fieldName);

  friend RaverieShaderSpirVSettings;
  friend EntryPointGeneration;
  Array<ShaderIRFieldMeta*> mFields;
};

/// Various callbacks used throughout shader translation for customization of
/// code emission.
class CallbackSettings
{
public:
  CallbackSettings();

  typedef void (*ShaderCompositeCallback)(CompositorCallbackData& callbackData, void* userData);
  /// Set a callback that is called right before the composited shader is
  /// emitted. Allows modifying inputs and outputs, in particular allows forced
  /// HardwareBuiltIns like Position.
  void SetCompositeCallback(ShaderCompositeCallback callback, void* userData);

  typedef void (*AppendVertexCallback)(AppendCallbackData& callbackData, void* userData);
  /// Callback to allow custom spirv emission in the Append function for
  /// geometry shader output streams. Allows custom handling of things like the
  /// BuiltIn Position to account for different api transforms.
  void SetAppendCallback(AppendVertexCallback callback, void* userData);

  void* mCompositeCallbackUserData;
  ShaderCompositeCallback mCompositeCallback;

  void* mAppendCallbackUserData;
  AppendVertexCallback mAppendCallback;
};

/// A collection of error settings. Currently, mostly for dealing with errors
/// that only matter if the compositor is not part of the expected work-flow.
class RaverieShaderErrorSettings
{
public:
  RaverieShaderErrorSettings();

  /// Should the front-end translator emit errors if a fragment type is missing
  /// the 'Main' function. This is only an error if the compositor is run on the
  /// fragment. The error can be moved to the front-end to make errors
  /// immediately known instead of only upon compositing.
  bool mFrontEndErrorOnNoMainFunction;
};

/// A collection of common settings for Raverie shader translation (current SpirV
/// specific).
class RaverieShaderSpirVSettings
{
public:
  RaverieShaderSpirVSettings();
  RaverieShaderSpirVSettings(const SpirVNameSettings& nameSettings);

  /// Adds a uniform buffer description (constant buffer)
  void AddUniformBufferDescription(UniformBufferDescription& description);
  /// Sets the default uniform buffer description to the last available binding
  /// id (based upon number bound).
  void AutoSetDefaultUniformBufferDescription(int descriptorSetId = 0, StringParam debugName = "Material");
  /// Sets the default uniform buffer description values.
  void SetDefaultUniformBufferDescription(int bindingId, int descriptorSetId, StringParam debugName);
  /// Checks if the given field matches any uniform constant buffer
  /// for the given fragment type. Used for attribute validation.
  bool IsValidUniform(FragmentType::Enum fragmentType, StringParam fieldType, StringParam fieldName);

  /// Overrides the raverie name that is used to map to a spirv built-in variable
  /// for all shader stages.
  void SetHardwareBuiltInName(spv::BuiltIn builtInId, StringParam name);
  bool IsValidHardwareBuiltIn(FragmentType::Enum fragmentType, StringParam fieldType, StringParam fieldName, bool isInput);

  // Set what's the max number of render targets that can be used
  void SetMaxSimultaneousRenderTargets(size_t maxNumber);
  // Render targets are resolved via name.
  void SetRenderTargetName(StringParam varName, size_t targetIndex);
  // Override what the render target type is.
  // @JoshD: This needs testing beyond using Real4 and how to deal with changing this per compilation.
  void SetRenderTargetType(Raverie::BoundType* targetType);

  /// The name of the current language's specialization variable name.
  /// Used to find the spec id to override this variable.
  static String GetLanguageSpecializationName();
  /// The name of the current language version's specialization variable name.
  /// Used to find the spec id to override this variable.
  static String GetLanguageVersionSpecializationName();
  /// Internal. The unique key for finding the current language's specialization
  /// variable.
  static void* GetLanguageSpecializationKey();
  /// Internal. The unique key for finding the current language version's
  /// specialization variable.
  static void* GetLanguageVersionSpecializationKey();

  /// This must be called once before being used for translation and will be
  /// auto-called by the translator if it hasn't already been. This is used to
  /// validate data and cache data in a more run-time friendly format.
  void Finalize();
  /// Has this already been finalized?
  bool IsFinalized() const;

  void Validate();

  /// Should the material buffer for each shader stage use the same binding id
  /// or not? If overlap is not allowed, then a buffer's binding id will be the
  /// default uniform buffer's binding id plus the current stage's value
  /// (FragmentType::Enum). This is relevant depending on if a graphics api
  /// allows separate bindings for each shader stage or if the entire program
  /// uses the same ids. In DirectX and Vulkan each shader stage allows separate
  /// binding points. OpenGl shares binding points for the entire program
  /// (without ARB_separate_shader_objects).
  bool mAllowUniformMaterialBufferIndexOverap;

  SpirVNameSettings mNameSettings;
  /// All of the uniform buffers that should be used if possible.
  Array<UniformBufferDescription> mUniformBufferDescriptions;
  // What binding/descriptor set should be used for material blocks. No fields
  // matter here.
  UniformBufferDescription mDefaultUniformBufferDescription;
  /// Mappings of raverie names and spirv built-in types for each shader stage.
  BuiltInStageDescription mBuiltIns[FragmentType::Size];
  VertexDefinitionDescription mVertexDefinitions;

  /// The bound render targets. Each index in the array maps to the given render
  /// target name.
  Array<String> mRenderTargetNames;
  Raverie::BoundType* mRenderTargetType;

  CallbackSettings mCallbackSettings;
  RaverieShaderErrorSettings mErrorSettings;

private:
  /// Validates that uniform descriptions don't have any
  /// errors (duplicate fields, duplicate binding ids, etc...)
  void ValidateUniformsDescriptions();
  void ValidateBuiltInNames();
  void ValidateBuiltInNames(BuiltInBlockDescription& blockDescription, HashMap<String, spv::BuiltIn>& keyMappings);
  void ValidateAppBuiltInsAgainstHardwareBuiltIns();

  void InitializeBuiltIns();

  bool IsValidHardwareBuiltInAnyStage(ShaderFieldKey& fieldKey, bool isInput);
  bool ValidateAgainstHardwareBuiltIns(ShaderFieldKey& fieldKey);

  // An intrusive reference count for memory handling
  RaverieRefLink(RaverieShaderSettings);
  bool mFinalized;

  static String mLanguageSpecConstantName;
  static String mLanguageVersionSpecConstantName;
};

} // namespace Raverie
