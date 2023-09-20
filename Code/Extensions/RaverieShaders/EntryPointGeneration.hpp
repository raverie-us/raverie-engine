// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
struct GeometryStreamUserData;
} // namespace Raverie

class EntryPointGeneration;

namespace Raverie
{

class TypeDependencyCollector;

/// Simple class to group common data together to make it easier to pass between
/// functions.
class EntryPointHelperFunctionData
{
public:
  RaverieShaderIRFunction* mFunction;
  RaverieShaderIROp* mSelfParam;
  BasicBlock* mBlock;
};

/// Helper data passed around for generating a geometry shader's append function
class GeometryAppendFunctionData
{
public:
  RaverieShaderIRFunction* mFunction;
  BasicBlock* mBlock;
  IRaverieShaderIR* mDefaultVertexId;
  RaverieShaderIROp* mOutputDataInstance;
};

/// Represents a grouping of fields for entry point generation to make
/// input/output interface generation easier. These fields could be grouped
/// together in a struct or could be globals. These fields could be inputs or
/// outputs. Additionally, each field and the parent struct (if it exists) can
/// contain decoration attributes.
class InterfaceInfoGroup
{
public:
  /// Represents a decoration for an object. If mValue is -1 then there is no
  /// value to the decoration (some decorations take parameters)
  struct DecorationParam
  {
    DecorationParam()
    {
    }
    explicit DecorationParam(spv::Decoration type)
    {
      mDecorationType = type;
      mValue = -1;
    }
    explicit DecorationParam(spv::Decoration type, int value)
    {
      mDecorationType = type;
      mValue = value;
    }
    spv::Decoration mDecorationType;
    int mValue;
  };

  /// A field in this group that can contain individual decorations.
  struct FieldInfo
  {
    ShaderIRFieldMeta* mFieldMeta;
    // The metas of all of the fields that were matched against this interface
    // field. Needed as the matching is not 1 to 1 and the names don't have to
    // match.
    Array<ShaderIRFieldMeta*> mLinkedFields;
    Array<DecorationParam> mDecorations;
    Array<DecorationParam> mTypeDecorations;
    ShaderResourceReflectionData mReflectionData;
  };

  InterfaceInfoGroup()
  {
    mIsStruct = true;
    mIsBuiltIn = false;
    mStorageClass = spv::StorageClassGeneric;
  }

  /// Find a field descriptor based upon its key. Returns null if there is no
  /// matching field.
  FieldInfo* FindFieldInfo(const ShaderFieldKey& fieldKey);
  /// Find a field descriptor based upon its key. Creates the descriptor if it
  /// doesn't exist.
  FieldInfo* FindOrCreateFieldInfo(const ShaderFieldKey& fieldKey);

  /// The fields being grouped together
  typedef Array<FieldInfo> FieldList;
  FieldList mFields;

  /// Should these fields be grouped together in a struct?
  /// This is determined by the stage and the storage type.
  bool mIsStruct;
  /// The name of this struct to emit. Purely for debugging.
  String mName;
  /// Is this a collection of built-ins. Needed to know if booleans are allowed.
  bool mIsBuiltIn;
  /// What storage class to declare these fields with (e.g. input/output)?
  spv::StorageClass mStorageClass;
  /// Any decorations on the struct (if it is one).
  Array<DecorationParam> mTypeDecorations;
  Array<DecorationParam> mInstanceDecorations;

  /// The reflection data for this group (struct). If this type isn't
  /// a struct then this data is not filled out.
  ShaderResourceReflectionData mReflectionData;
};

struct ShaderInterfaceField
{
  ShaderInterfaceField();

  // The index of this field within it's owning data structure. Needed to get a
  // pointer to the actual memory address.
  int mFieldIndex;
  // Name of the field to generate. Should not be used for linking purposes.
  String mFieldName;
  // The meta (uniquely owned by this type) for the field. Stores all attributes
  // needed for resolution.
  ShaderIRFieldMeta* mFieldMeta;

  // The actual type of the interface field
  RaverieShaderIRType* mFieldType;
  // The original field type that was declared for this field. This might be
  // different from the actual field type if the interface type had to be
  // replaced (e.g. bool -> int)
  RaverieShaderIRType* mOriginalFieldType;

  Array<ShaderIRFieldMeta*> mLinkedFields;
};

DeclareEnum3(ShaderInterfaceKind, Struct, Globals, Array);

struct ShaderInterfaceType
{
  virtual ~ShaderInterfaceType(){};

  virtual ShaderInterfaceKind::Enum GetInterfaceKind() = 0;

  template <typename T>
  T* As()
  {
    if (GetInterfaceKind() != T::StaticGetInterfaceKind())
    {
      Error("Invalid As");
      return nullptr;
    }
    return (T*)this;
  }

  // Returns how many fields this type contains.
  virtual size_t GetFieldCount() = 0;
  // Returns the description of the field at the given index.
  virtual ShaderInterfaceField* GetFieldAtIndex(size_t index) = 0;
  // Finds a field by key. Returns null if the value is invalid.
  virtual ShaderInterfaceField* GetField(const ShaderFieldKey& fieldKey) = 0;
  // Returns a pointer to the field at the given index. If needed, instructions
  // will be written to the given block.
  virtual RaverieShaderIROp* GetFieldPointerByIndex(size_t index, EntryPointGeneration* entryPointGeneration, BasicBlock* block, spv::StorageClass storageClass) = 0;

  // Declare an interface struct type to contain the given interface group
  // variables.
  virtual void DeclareInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo){};
  // Decorates the interface type and all of its sub-members.
  virtual void DecorateInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo){};
  // Create the instance for the given interface struct. Also applies instance
  // decorations to the newly created instance.
  virtual void DefineInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo){};
  // Copies the given interface type instance to/from the copyHelper's self type
  // (based upon the storage class).
  virtual void CopyInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo, EntryPointHelperFunctionData& copyHelperData){};
};

struct ShaderInterfaceStruct : public ShaderInterfaceType
{
  ShaderInterfaceStruct();
  virtual ShaderInterfaceKind::Enum GetInterfaceKind() override;
  static ShaderInterfaceKind::Enum StaticGetInterfaceKind();

  size_t GetFieldCount() override;
  ShaderInterfaceField* GetFieldAtIndex(size_t index) override;
  ShaderInterfaceField* GetField(const ShaderFieldKey& fieldKey) override;
  RaverieShaderIROp* GetFieldPointerByIndex(size_t index, EntryPointGeneration* entryPointGeneration, BasicBlock* block, spv::StorageClass storageClass) override;

  // Get the field by index off of the given instance. Used internally and when
  // this is a sub-type in an array.
  RaverieShaderIROp* GetFieldPointerByIndex(RaverieShaderIROp* instance, size_t index, EntryPointGeneration* entryPointGeneration, BasicBlock* block, spv::StorageClass storageClass);

  void DeclareInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo) override;
  void DecorateInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo) override;
  void DefineInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo) override;
  void CopyInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo, EntryPointHelperFunctionData& copyHelperData) override;

  RaverieShaderIRType* mType;
  RaverieShaderIROp* mInstance;

  Array<ShaderInterfaceField> mFields;
};

struct ShaderInterfaceGlobals : public ShaderInterfaceType
{
  ShaderInterfaceGlobals();
  virtual ShaderInterfaceKind::Enum GetInterfaceKind() override;
  static ShaderInterfaceKind::Enum StaticGetInterfaceKind();

  size_t GetFieldCount() override;
  ShaderInterfaceField* GetFieldAtIndex(size_t index) override;
  ShaderInterfaceField* GetField(const ShaderFieldKey& fieldKey) override;
  RaverieShaderIROp* GetFieldPointerByIndex(size_t index, EntryPointGeneration* entryPointGeneration, BasicBlock* block, spv::StorageClass storageClass) override;

  // Declare an interface struct type to contain the given interface group
  // variables.
  void DeclareInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo) override;
  // Decorates the interface type and all of its sub-members.
  void DecorateInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo) override;
  // Create the instance for the given interface struct. Also applies instance
  // decorations to the newly created instance.
  void DefineInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo) override;
  // Copies the given interface type instance to/from the copyHelper's self type
  // (based upon the storage class).
  void CopyInterfaceType(EntryPointGeneration* entryPointGeneration, InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo, EntryPointHelperFunctionData& copyHelperData) override;

  Array<ShaderInterfaceField> mFields;
  Array<RaverieShaderIRType*> mFieldTypes;
  Array<RaverieShaderIROp*> mFieldInstances;
};

struct ShaderInterfaceStructArray : public ShaderInterfaceStruct
{
  ShaderInterfaceStructArray();
  virtual ShaderInterfaceKind::Enum GetInterfaceKind() override;
  static ShaderInterfaceKind::Enum StaticGetInterfaceKind();

  // Does the contained struct type have a field with the given name/type?
  bool ContainsField(const ShaderFieldKey& fieldKey);
  // Get a pointer to a field in the sub-struct at the given index. Emits access
  // code to the given block
  RaverieShaderIROp* GetPointerByIndex(IRaverieShaderIR* index, ShaderFieldKey& fieldKey, EntryPointGeneration* entryPointGeneration, BasicBlock* block, spv::StorageClass storageClass);

  // The contained struct sub-type.
  ShaderInterfaceStruct* mStructType;
};

/// Common information needed about the input/output types for a geometry shader
struct GeometryStageInfo
{
  GeometryStageInfo(RaverieSpirVFrontEnd* translator, RaverieShaderIRType* shaderType, Raverie::GenericFunctionNode* node);

  RaverieShaderIRType* mShaderType;
  RaverieShaderIRType* mInputStreamType;
  RaverieShaderIRType* mOutputStreamType;
  RaverieShaderIRType* mInputVertexType;
  RaverieShaderIRType* mOutputVertexType;
  Raverie::GeometryStreamUserData* mInputStreamUserData;
  Raverie::GeometryStreamUserData* mOutputStreamUserData;
  // All output streams used by the entry point.
  // Each one of these needs to generate late-bound 'Append' functions.
  Array<RaverieShaderIRType*> mOutputStreamTypes;
};

/// Structure to help collect what variables belong in what interface blocks
class ShaderInterfaceInfo
{
public:
  typedef OrderedHashMap<void*, InterfaceInfoGroup> InterfaceGroupMap;

  InterfaceInfoGroup mInputs;
  InterfaceInfoGroup mOutputs;
  InterfaceGroupMap mBuiltInGroups;
  InterfaceGroupMap mUniformGroups;
};

class AppendCallbackData
{
public:
  EntryPointGeneration* mEntryPointGeneration;
  RaverieShaderSpirVSettings* mSettings;
  GeometryStageInfo* mStageInfo;
  EntryPointInfo* mEntryPointInfo;
  GeometryAppendFunctionData* mAppendFnData;
  Array<ShaderInterfaceType*>* mOutputVertexInterfaceTypes;
  Array<ShaderInterfaceType*>* mInputStreamInterfaceTypes;
};

class EntryPointGeneration
{
public:
  EntryPointGeneration();
  ~EntryPointGeneration();

  void DeclareVertexInterface(RaverieSpirVFrontEnd* translator, Raverie::GenericFunctionNode* node, RaverieShaderIRFunction* function, RaverieSpirVFrontEndContext* context);
  void DeclareGeometryInterface(RaverieSpirVFrontEnd* translator, Raverie::GenericFunctionNode* node, RaverieShaderIRFunction* function, RaverieSpirVFrontEndContext* context);
  void DeclarePixelInterface(RaverieSpirVFrontEnd* translator, Raverie::GenericFunctionNode* node, RaverieShaderIRFunction* function, RaverieSpirVFrontEndContext* context);
  void DeclareComputeInterface(RaverieSpirVFrontEnd* translator, Raverie::GenericFunctionNode* node, RaverieShaderIRFunction* function, RaverieSpirVFrontEndContext* context);

  void Clear();

  friend ShaderInterfaceStruct;
  friend ShaderInterfaceGlobals;
  friend ShaderInterfaceStructArray;

  // Shared information for declaring a in/out interface type for a geometry
  // shader to make passing data to functions easier
  struct GeometryInOutTypeInfo
  {
    String mInstanceName;
    String mItemTypeName;
    String mArrayTypeName;
    IRaverieShaderIR* mArraySize;
  };

  // Returns the self variable instance.
  void CreateEntryPointFunction(Raverie::GenericFunctionNode* node, RaverieShaderIRFunction* function, RaverieShaderIROp*& selfVarOp, BasicBlock*& entryPointBlock);
  void BuildBasicEntryPoint(Raverie::GenericFunctionNode* node, RaverieShaderIRFunction* function, RaverieShaderIRFunction* copyInputsFn, RaverieShaderIRFunction* copyOutputsFn);
  RaverieShaderIRFunction* CreateGlobalsInitializerFunction(Raverie::GenericFunctionNode* node);
  EntryPointHelperFunctionData GenerateCopyHelper(RaverieShaderIRFunction* userMainFn, StringParam name);

  // Geometry Shader Stage helpers
  EntryPointHelperFunctionData GenerateGeometryCopyHelper(RaverieShaderIRFunction* userMainFn, StringParam name, RaverieShaderIRType* inputStreamType, RaverieShaderIROp*& inputStreamVar);
  void WriteGeometryStageInterface(
      RaverieShaderIRFunction* function, GeometryStageInfo& stageInfo, EntryPointInfo* entryPointInfo, EntryPointHelperFunctionData& copyInputsData, RaverieShaderIROp* copyInputsStreamVar);
  void CollectGeometryStreamTypes(RaverieShaderIRFunction* function, GeometryStageInfo& stageInfo);
  void DeclareGeometryVertexInputs(GeometryStageInfo& stageInfo,
                                   EntryPointInfo* entryPointInfo,
                                   ShaderInterfaceInfo& vertexInputInterfaceInfo,
                                   EntryPointHelperFunctionData& copyInputsData,
                                   RaverieShaderIROp* copyInputsStreamVar,
                                   Array<ShaderInterfaceType*>& inputStreamInterfaceTypes,
                                   Array<ShaderInterfaceType*>& inputVertexInterfaceTypes);
  ShaderInterfaceStructArray* DeclareGeometryVertexInput(InterfaceInfoGroup& interfaceGroup,
                                                         EntryPointInfo* entryPointInfo,
                                                         GeometryInOutTypeInfo& info,
                                                         Array<ShaderInterfaceType*>& inputStreamInterfaceTypes,
                                                         Array<ShaderInterfaceType*>& inputVertexInterfaceTypes);
  void
  DeclareGeometryVertexOutputs(GeometryStageInfo& stageInfo, EntryPointInfo* entryPointInfo, ShaderInterfaceInfo& vertexOutputInterfaceInfo, Array<ShaderInterfaceType*>& inputStreamInterfaceTypes);
  void WriteGeometryInterfaceOutput(InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo, Array<ShaderInterfaceType*>& interfaceTypes);
  void WriteGeometryAppendFunctions(GeometryStageInfo& stageInfo,
                                    EntryPointInfo* entryPointInfo,
                                    Array<ShaderInterfaceType*>& outputVertexInterfaceTypes,
                                    Array<ShaderInterfaceType*>& inputStreamInterfaceTypes);
  void GenerateProvokingVertexAppend(GeometryStageInfo& stageInfo, EntryPointInfo* entryPointInfo, GeometryAppendFunctionData& appendFnData, RaverieShaderIRType* outputStreamType);
  RaverieShaderIRFunction* CloneAppendFn(RaverieShaderIRFunction* originalAppendFn);
  void CopyGeometryOutputInterface(GeometryStageInfo& stageInfo,
                                   EntryPointInfo* entryPointInfo,
                                   GeometryAppendFunctionData& appendFnData,
                                   ShaderInterfaceType& interfaceType,
                                   Array<ShaderInterfaceType*>& inputStreamInterfaceTypes);

  void CollectInterfaceVariables(RaverieShaderIRFunction* function, ShaderInterfaceInfo& interfaceInfo, ShaderStage::Enum shaderStage);
  void CollectInputInterfaceVariables(RaverieShaderIRFunction* function, ShaderInterfaceInfo& interfaceInfo, RaverieShaderIRType* type, ShaderStage::Enum shaderStage);
  void CollectOutputInterfaceVariables(RaverieShaderIRFunction* function, ShaderInterfaceInfo& interfaceInfo, RaverieShaderIRType* type, ShaderStage::Enum shaderStage);
  void CollectUniformInterfaceVariables(RaverieShaderIRFunction* function, ShaderInterfaceInfo& interfaceInfo, RaverieShaderIRType* type, ShaderStage::Enum shaderStage);

  void ProcessUniformBlockSettings(ShaderStage::Enum stageToProcess, HashMap<ShaderFieldKey, UniformBufferDescription*>& mapping);
  InterfaceInfoGroup* ProcessBuiltIn(RaverieShaderIRFunction* function,
                                     ShaderInterfaceInfo& interfaceInfo,
                                     ShaderStage::Enum shaderStage,
                                     ShaderIRFieldMeta* fieldMeta,
                                     ShaderIRAttribute* attribute,
                                     spv::StorageClass storageClass,
                                     StringParam name);
  InterfaceInfoGroup* ProcessUniformBlock(
      RaverieShaderIRFunction* function, ShaderInterfaceInfo& interfaceInfo, ShaderIRFieldMeta* fieldMeta, ShaderIRAttribute* attribute, HashMap<ShaderFieldKey, UniformBufferDescription*>& mapping);

  void DeclareStageBlocks(ShaderInterfaceInfo& interfaceInfo, EntryPointInfo* entryPointInfo, EntryPointHelperFunctionData& copyInputsData, EntryPointHelperFunctionData& copyOutputsData);
  void DeclareGroupBlocks(ShaderInterfaceInfo::InterfaceGroupMap& interfaceGroups,
                          EntryPointInfo* entryPointInfo,
                          EntryPointHelperFunctionData& copyInputsData,
                          EntryPointHelperFunctionData& copyOutputsData,
                          Array<ShaderInterfaceType*>& outArray);
  void DeclareBlock(InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo, EntryPointHelperFunctionData& copyHelperData, Array<ShaderInterfaceType*>& outArray);
  void DeclareBlockStruct(InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo, EntryPointHelperFunctionData& copyHelperData, Array<ShaderInterfaceType*>& outArray);
  void DeclareBlockNoStruct(InterfaceInfoGroup& interfaceGroup, EntryPointInfo* entryPointInfo, EntryPointHelperFunctionData& copyHelperData, Array<ShaderInterfaceType*>& outArray);

  void CopyField(BasicBlock* targetFnBlock, RaverieShaderIRType* sourceLoadType, RaverieShaderIROp* source, RaverieShaderIROp* dest);
  void CopyFromInterfaceType(
      BasicBlock* block, RaverieShaderIROp* dest, RaverieShaderIROp* source, ShaderInterfaceType* sourceInterface, spv::StorageClass destStorageClass, spv::StorageClass sourceStorageClass);
  RaverieShaderIROp* GetMemberInstanceFrom(BasicBlock* block, RaverieShaderIROp* source, int sourceOffset, spv::StorageClass sourceStorageClass);
  RaverieShaderIROp* GetNamedMemberInstanceFrom(BasicBlock* block, RaverieShaderIROp* source, StringParam memberName, spv::StorageClass sourceStorageClass);
  RaverieShaderIROp* GetNamedMemberInstanceFrom(BasicBlock* block, RaverieShaderIROp* source, const ShaderFieldKey& fieldKey, spv::StorageClass sourceStorageClass);

  void DecorateUniformGroups(ShaderInterfaceInfo& interfaceInfo);
  void DecorateUniformGroup(InterfaceInfoGroup& infoGroup);
  void AddOffsetDecorations(InterfaceInfoGroup& infoGroup);
  void AddMemberTypeDecorations(RaverieShaderIRType* memberType, InterfaceInfoGroup::FieldInfo& fieldInfo, ShaderResourceReflectionData& memberReflection);
  void AddVertexLocationDecorations(InterfaceInfoGroup& infoGroup);
  void AddPixelLocationDecorations(InterfaceInfoGroup& infoGroup);
  void AddFlatDecorations(InterfaceInfoGroup& infoGroup);
  void WriteTypeDecorations(Array<InterfaceInfoGroup::DecorationParam>& decorations, BasicBlock* decorationBlock, IRaverieShaderIR* toDecorate);
  void WriteMemberDecorations(Array<InterfaceInfoGroup::DecorationParam>& decorations, BasicBlock* decorationBlock, IRaverieShaderIR* toDecorate, RaverieShaderIRConstantLiteral* memberIndexLiteral);
  void FindAndDecorateGlobals(RaverieShaderIRType* currentType, EntryPointInfo* entryPointInfo);
  void AddInterfaceTypesToEntryPoint(TypeDependencyCollector& collector, EntryPointInfo* entryPointInfo);
  void DecorateImagesAndSamplers(TypeDependencyCollector& collector, EntryPointInfo* entryPointInfo);
  void DecorateRuntimeArrays(TypeDependencyCollector& collector, EntryPointInfo* entryPointInfo);
  /// Add decorations for a runtime array struct.
  void AddRuntimeArrayDecorations(
      BasicBlock* decorationBlock, RaverieShaderIRType* raverieRuntimeArrayType, RaverieShaderIRType* spirvRuntimeArrayType, RaverieShaderIRType* elementType, ShaderStageResource& stageResource);
  /// Recursively decorate a struct (currently setup for runtime arrays)
  void RecursivelyDecorateStructType(BasicBlock* decorationBlock, RaverieShaderIRType* structType, ShaderStageResource& stageResource);

  u32 FindBindingId();

  // Copy reflection data from the internal interface info to the entry point
  void CopyReflectionDataToEntryPoint(EntryPointInfo* entryPointInfo, ShaderInterfaceInfo& interfaceInfo);
  void CopyReflectionData(Array<ShaderStageResource>& resourceList, ShaderInterfaceInfo& interfaceInfo, InterfaceInfoGroup& group);
  void CopyReflectionDataStruct(Array<ShaderStageResource>& resourceList, ShaderInterfaceInfo& interfaceInfo, InterfaceInfoGroup& group);
  void CopyReflectionDataGlobals(Array<ShaderStageResource>& resourceList, ShaderInterfaceInfo& interfaceInfo, InterfaceInfoGroup& group);

  // Create a shader interface field from the interface group and the field
  // index.
  void CreateShaderInterfaceField(ShaderInterfaceField& interfaceField, InterfaceInfoGroup& interfaceGroup, int index);

  // Some types aren't allowed in any interface declarations (uniform/in/out).
  // This function converts them to the next best thing (e.g. bool -> int)
  Raverie::BoundType* ConvertInterfaceType(Raverie::BoundType* inputType);

  // Write the execution mode for the pixel origin for this entry point.
  void WriteExecutionModeOriginUpperLeft(EntryPointInfo* entryPointInfo);

  // Finds a field from a collection of interface types. If necessary, any new
  // op-codes will be written into the given block and the provided storage
  // class will be used.
  RaverieShaderIROp* FindField(ShaderFieldKey& fieldKey, Array<ShaderInterfaceType*>& interfaces, BasicBlock* block, spv::StorageClass storageClass);
  // Walks the given type meta to find an attribute with the given field meta
  // (for name overrides). Returns the first field that fulfills the meta,
  // otherwise null.
  ShaderIRFieldMeta* FindFieldViaAttribute(ShaderIRTypeMeta* typeMeta, StringParam attributeName, ShaderFieldKey& fieldKey);
  // Default callback we use for api perspective position.
  static void PerspectiveTransformAppendVertexCallback(AppendCallbackData& callbackData, void* userData);

  RaverieSpirVFrontEnd* mTranslator;
  RaverieSpirVFrontEndContext* mContext;

  Array<ShaderInterfaceType*> mUniforms;
  Array<ShaderInterfaceType*> mBuiltIns;
  Array<ShaderInterfaceType*> mInputs;
  Array<ShaderInterfaceType*> mOutputs;
  HashSet<RaverieShaderIRType*> mUniqueTypes;
  HashSet<RaverieShaderIROp*> mUniqueOps;
  HashSet<u32> mUsedBindingIds;
};

} // namespace Raverie
