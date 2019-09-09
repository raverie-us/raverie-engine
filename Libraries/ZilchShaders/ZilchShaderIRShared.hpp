// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

DeclareEnum7(ZilchShaderIRBaseType, Op, ConstantLiteral, DataType, Function, Block, Comment, Extension);
DeclareEnum3(BlockType, Direct, Selection, Loop);
DeclareEnum15(ShaderIRTypeBaseType,
              Bool,
              Float,
              Int,
              Vector,
              Matrix,
              Struct,
              Image,
              Sampler,
              SampledImage,
              FixedArray,
              RuntimeArray,
              Void,
              Function,
              Pointer,
              Unknown)

    typedef spv::Op OpType;
class ZilchShaderIRLibrary;
class ZilchShaderIRType;
class ZilchShaderIRFunction;
class SpirVExtensionLibrary;

class ZilchShaderIRComments
{
public:
  void Add(StringParam comment)
  {
    mComments.PushBack(comment);
  }
  Array<String> mComments;
};

class ZilchShaderSourceLocation
{
public:
  Array<Zilch::CodeLocation> mCodeLocations;
};

class ZilchShaderDebugInfo
{
public:
  void Clear()
  {
    mLocations.mCodeLocations.Clear();
    mComments.mComments.Clear();
  }
  ZilchShaderSourceLocation mLocations;
  ZilchShaderIRComments mComments;
};

class IZilchShaderIR
{
public:
  IZilchShaderIR(ZilchShaderIRBaseType::Enum baseType) : mIRType(baseType){};
  virtual ~IZilchShaderIR(){};

  template <typename AsType>
  AsType* As()
  {
    if (mIRType == AsType::mStaticBaseType)
      return (AsType*)this;
    return nullptr;
  }
  template <typename AsType>
  AsType* AsAssert()
  {
    ErrorIf(mIRType != AsType::mStaticBaseType, "Invalid type cast");
    return (AsType*)this;
  }

  ZilchShaderIRBaseType::Enum mIRType;
  ZilchShaderDebugInfo mDebugInfo;
  String mDebugResultName;
};

/// Represents a constant value literal such as the number 1.
class ZilchShaderIRConstantLiteral : public IZilchShaderIR
{
public:
  const static ZilchShaderIRBaseType::Enum mStaticBaseType = ZilchShaderIRBaseType::ConstantLiteral;

  ZilchShaderIRConstantLiteral(Zilch::Any value) : IZilchShaderIR(mStaticBaseType)
  {
    mValue = value;
  }

  Zilch::Any mValue;
};

class ZilchShaderExtensionImport : public IZilchShaderIR
{
public:
  const static ZilchShaderIRBaseType::Enum mStaticBaseType = ZilchShaderIRBaseType::Extension;
  ZilchShaderExtensionImport(SpirVExtensionLibrary* library) : IZilchShaderIR(mStaticBaseType)
  {
    mLibrary = library;
  }

  SpirVExtensionLibrary* mLibrary;
};

class ZilchShaderIROp : public IZilchShaderIR
{
public:
  const static ZilchShaderIRBaseType::Enum mStaticBaseType = ZilchShaderIRBaseType::Op;

  ZilchShaderIROp(OpType opType);

  bool IsTerminator();
  bool IsResultPointerType();

  /// If this is a pointer type then the dereference type is returned, otherwise
  /// this type itself is returned.
  ZilchShaderIRType* GetValueType();
  /// If this is a value type then the pointer type is returned, otherwise this
  /// type itself is returned.
  ZilchShaderIRType* GetPointerType();

  OpType mOpType;
  Array<IZilchShaderIR*> mArguments;
  ZilchShaderIRType* mResultType;
};

class BasicBlock : public IZilchShaderIR
{
public:
  const static ZilchShaderIRBaseType::Enum mStaticBaseType = ZilchShaderIRBaseType::Block;

  BasicBlock();
  ~BasicBlock();

  void AddOp(IZilchShaderIR* op);

  // Local variable must declared before all other lines in the starting block.
  // This separation makes this easier.
  Array<IZilchShaderIR*> mLocalVariables;
  Array<IZilchShaderIR*> mLines;

  /// What kind of a block is this? A basic block with simple
  /// statements? A if block (selection)? A loop?
  BlockType::Enum mBlockType;
  // If this is a loop/selection then these points are used to generate the code
  // needed for declaring structured control flow. Additionally these are used
  // to help generate required branching instructions.
  BasicBlock* mMergePoint;
  BasicBlock* mContinuePoint;

  IZilchShaderIR* mTerminatorOp;
};

/// Stores information about any found entry point on a type.
/// This is necessary to generate the first few sections of a valid spir-v file.
class EntryPointInfo
{
public:
  EntryPointInfo();

  /// The entry point function
  ZilchShaderIRFunction* mEntryPointFn;
  /// What kind of fragment type this entry point is.
  FragmentType::Enum mFragmentType;
  /// The global variable initializer function that the entry point will call.
  /// The back-end translation will replace this with a new function that
  /// actually calls all referenced global variable initializers.
  ZilchShaderIRFunction* mGlobalsInitializerFunction;
  /// Any extra capability modes for this entry point (e.g. Geometry)
  Array<spv::Capability> mCapabilities;
  /// Any execution mode instructions for this entry point (e.g. geometry max
  /// vertices)
  BasicBlock mExecutionModes;
  /// A block of all of the decorations for variables
  BasicBlock mDecorations;
  /// The declared global variables
  BasicBlock mVariables;
  /// The declared variables that are part of the interface for the
  /// entry point function (required for OpEntryPoint)
  Array<ZilchShaderIROp*> mInterface;

  /// The reflection data of all of the resources tied to this entry point.
  ShaderStageInterfaceReflection mStageReflectionData;

  /// Late bound functions are used to replace all instances of function
  /// reference with another during back-end generation. This is needed in
  /// particular for the Geometry shader stage's Append function which needs to
  /// be unique to each composite (due to pass-through).
  OrderedHashMap<ZilchShaderIRFunction*, ZilchShaderIRFunction*> mLateBoundFunctions;
};

/// Represents a function type for spir-v.
class ZilchShaderIRFunction : public IZilchShaderIR
{
public:
  const static ZilchShaderIRBaseType::Enum mStaticBaseType = ZilchShaderIRBaseType::Function;

  ZilchShaderIRFunction();
  ~ZilchShaderIRFunction();

  /// Get the shader type that this function returns.
  ZilchShaderIRType* GetReturnType();

  String mName;

  /// The type of this function. Arg0 is Return type, remaining args are the
  /// parameters
  ZilchShaderIRType* mFunctionType;

  // Parameters must be declared before the first block of a function.
  // This block makes it easier to deal with this requirement.
  BasicBlock mParameterBlock;
  Array<BasicBlock*> mBlocks;

  /// Meta information about this functions, such as attributes.
  ShaderIRFunctionMeta* mMeta;
};

/// Represents a type, such as Real or a struct.
class ZilchShaderIRType : public IZilchShaderIR
{
public:
  const static ZilchShaderIRBaseType::Enum mStaticBaseType = ZilchShaderIRBaseType::DataType;

  ZilchShaderIRType();
  ~ZilchShaderIRType();

  ZilchShaderIRFunction* CreateFunction(ZilchShaderIRLibrary* library);

  /// Adds a member variable to this type.
  void AddMember(ZilchShaderIRType* memberType, StringParam memberName);

  /// Gets a sub-type at the given index from this type. Sub types are only
  /// valid on structs/functions.
  ZilchShaderIRType* GetSubType(int index) const;
  /// Returns the number of sub-types contained. Only valid on
  /// structs/functions.
  size_t GetSubTypeCount();
  /// Finds a sub-member's name via the member index. For debug names only now.
  String GetMemberName(size_t memberIndex);

  /// How many bytes does this type require? Includes sub-type padding.
  size_t GetByteSize() const;
  /// What starting byte alignment does this type require?
  size_t GetByteAlignment() const;
  /// What is the base primitive type. If this is a vector or matrix it'll
  /// return the underlying component type. If this is a bool, integer, or float
  /// then that is returned. Otherwise the stored type (e.g. struct) will be
  /// returned.
  ShaderIRTypeBaseType::Enum GetBasePrimitiveType() const;

  /// Find the first attribute by the given name if it exists.
  ShaderIRAttribute* FindFirstAttribute(StringParam attributeName) const;
  /// Is this type a pointer or value type?
  bool IsPointerType();
  /// Does this type have to be a global? This is determined by the
  /// "StorageClass" attribute.
  bool IsGlobalType() const;
  /// If this is a pointer type then the dereference type is returned, otherwise
  /// this type itself is returned.
  ZilchShaderIRType* GetValueType();
  /// If this is a value type then the pointer type is returned, otherwise this
  /// type itself is returned.
  ZilchShaderIRType* GetPointerType();

  String mName;
  ShaderIRTypeBaseType::Enum mBaseType;

  /// The functions owned by this type
  Array<ZilchShaderIRFunction*> mFunctions;
  /// If this type doesn't have an explicit default constructor then this is the
  /// auto-generated one that does the same thing as zilch's default constructor
  /// (i.e. invoke the pre-constructor)
  ZilchShaderIRFunction* mAutoDefaultConstructor;

  /// The zilch type this came from
  Zilch::BoundType* mZilchType;
  /// What library owns this type
  ZilchShaderIRLibrary* mShaderLibrary;
  /// Any meta information about this type such as attributes.
  ShaderIRTypeMeta* mMeta;

  /// Only valid on scalar/vector/matrix types
  size_t mComponents;
  ZilchShaderIRType* mComponentType;
  // @JoshD: Unify with subtypes later?
  Array<IZilchShaderIR*> mParameters;
  HashMap<String, int> mMemberNamesToIndex;
  HashMap<ShaderFieldKey, int> mMemberKeysToIndex;
  /// If this is a value type then this points to the corresponding pointer
  /// type. Otherwise this is null.
  ZilchShaderIRType* mPointerType;
  /// If this is a pointer type then this points to the corresponding value
  /// type. Otherwise this is null.
  ZilchShaderIRType* mDereferenceType;

  spv::StorageClass mStorageClass;
  EntryPointInfo* mEntryPoint;

  // @JoshD: Turn into flags later
  /// Does this type have a main function necessary for compositing?
  bool mHasMainFunction;
};

/// Returns what the component type is (only valid for scalars (null), vectors,
/// and matrices)
ZilchShaderIRType* GetComponentType(ZilchShaderIRType* compositeType);
bool IsScalarType(ZilchShaderIRType* compositeType);
ZilchShaderIRType* GetImageTypeFromSampledImage(ZilchShaderIRType* samplerType);
/// Returns the required stride of the given type with a base alignment factor
/// (rounds up to nearest multiple of alignment)
int GetStride(ZilchShaderIRType* type, float baseAlignment);
size_t GetSizeAfterAlignment(size_t size, size_t baseAlignment);

// Generate a unique identifier for a template type with no filled in arguments.
// Used for finding 'base' template types.
typedef String TemplateTypeKey;
TemplateTypeKey GenerateTemplateTypeKey(Zilch::BoundType* zilchType);

/// Given a field name (after any extra mangling) and the owning class type,
/// generate the property name used to uniquely identify the field.
String GenerateSpirVPropertyName(StringParam fieldName, StringParam ownerType);
/// Given a field name (after any extra mangling) and the owning class type,
/// generate the property name used to uniquely identify the field.
String GenerateSpirVPropertyName(StringParam fieldName, ZilchShaderIRType* ownerType);

/// Get all spirv opocode string names. The returned strings don't contain
/// the leading "Op" that the disassembler generates.
Array<String> GetOpcodeNames();

} // namespace Zero
