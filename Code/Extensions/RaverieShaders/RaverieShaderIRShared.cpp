// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieShaderIROp::RaverieShaderIROp(OpType opType) : IRaverieShaderIR(mStaticBaseType)
{
  mOpType = opType;
}

bool RaverieShaderIROp::IsTerminator()
{
  bool isTerminator = (mOpType == OpType::OpReturn) || (mOpType == OpType::OpReturnValue) ||
                      (mOpType == OpType::OpBranch) || (mOpType == OpType::OpBranchConditional);
  return isTerminator;
}

bool RaverieShaderIROp::IsResultPointerType()
{
  if (mResultType == nullptr)
    return false;
  return mResultType->IsPointerType();
}

RaverieShaderIRType* RaverieShaderIROp::GetValueType()
{
  return mResultType->GetValueType();
}

RaverieShaderIRType* RaverieShaderIROp::GetPointerType()
{
  return mResultType->GetPointerType();
}

BasicBlock::BasicBlock() : IRaverieShaderIR(mStaticBaseType)
{
  mTerminatorOp = nullptr;
  mBlockType = BlockType::Direct;

  mMergePoint = nullptr;
  mContinuePoint = nullptr;
}

BasicBlock::~BasicBlock()
{
  DeleteObjectsIn(mLines);
  DeleteObjectsIn(mLocalVariables);
}

void BasicBlock::AddOp(IRaverieShaderIR* op)
{
  mLines.PushBack(op);
}

EntryPointInfo::EntryPointInfo()
{
  mFragmentType = FragmentType::None;
  mEntryPointFn = nullptr;
  mGlobalsInitializerFunction = nullptr;
}

RaverieShaderIRFunction::RaverieShaderIRFunction() : IRaverieShaderIR(mStaticBaseType)
{
  mMeta = nullptr;
}

RaverieShaderIRFunction::~RaverieShaderIRFunction()
{
  DeleteObjectsIn(mBlocks.All());
  // mMeta owned by the type's meta
}

RaverieShaderIRType* RaverieShaderIRFunction::GetReturnType()
{
  // The return type is always the first sub-type
  return mFunctionType->GetSubType(0);
}

RaverieShaderIRType::RaverieShaderIRType() : IRaverieShaderIR(mStaticBaseType)
{
  mComponentType = nullptr;
  mRaverieType = nullptr;
  mComponents = 1;
  mBaseType = ShaderIRTypeBaseType::Unknown;
  mPointerType = nullptr;
  mDereferenceType = nullptr;
  mMeta = nullptr;
  mEntryPoint = nullptr;
  mAutoDefaultConstructor = nullptr;
  mStorageClass = spv::StorageClassGeneric;
  mHasMainFunction = false;
}

RaverieShaderIRType::~RaverieShaderIRType()
{
  delete mEntryPoint;
}

RaverieShaderIRFunction* RaverieShaderIRType::CreateFunction(RaverieShaderIRLibrary* library)
{
  RaverieShaderIRFunction* function = new RaverieShaderIRFunction();
  mFunctions.PushBack(function);
  library->mOwnedFunctions.PushBack(function);
  return function;
}

void RaverieShaderIRType::AddMember(RaverieShaderIRType* memberType, StringParam memberName)
{
  mParameters.PushBack(memberType);

  int index = mParameters.Size() - 1;
  mMemberNamesToIndex[memberName] = index;

  // Either use the use the raverie type name if possible, otherwise
  // this type doesn't actually exist as a raverie type
  // (e.g. RuntimeArray internal type) so use the spirv name instead.
  String memberTypeName = memberType->mName;
  if (memberType->mRaverieType != nullptr)
    memberTypeName = memberType->mRaverieType->ToString();

  mMemberKeysToIndex[ShaderFieldKey(memberName, memberTypeName)] = index;
}

String RaverieShaderIRType::GetMemberName(size_t memberIndex)
{
  // Currently we only store names to indices but not the other way around.
  // For now just iterate through this map to find the member index.
  // A type shouldn't be too big and this is rarely done so this is acceptable
  // for now.
  for (auto range = mMemberNamesToIndex.All(); !range.Empty(); range.PopFront())
  {
    auto pair = range.Front();
    if (pair.second == memberIndex)
      return pair.first;
  }
  return String();
}

RaverieShaderIRType* RaverieShaderIRType::GetSubType(int index) const
{
  bool supportsSubTypes = mBaseType == ShaderIRTypeBaseType::Struct || mBaseType == ShaderIRTypeBaseType::Function;

  ErrorIf(!supportsSubTypes,
          "Type '%s' does not support sub-types. The parameters on this type "
          "are not guaranteed to be types.",
          mName.c_str());

  IRaverieShaderIR* param = mParameters[index];
  RaverieShaderIRType* subType = param->As<RaverieShaderIRType>();
  return subType;
}

size_t RaverieShaderIRType::GetSubTypeCount()
{
  bool supportsSubTypes = mBaseType == ShaderIRTypeBaseType::Struct || mBaseType == ShaderIRTypeBaseType::Function;

  ErrorIf(!supportsSubTypes,
          "Type '%s' does not support sub-types. The parameters on this type "
          "are not guaranteed to be types.",
          mName.c_str());

  return mParameters.Size();
}

size_t RaverieShaderIRType::GetByteSize() const
{
  // Force everything to 4 bytes
  if (mBaseType == ShaderIRTypeBaseType::Bool || mBaseType == ShaderIRTypeBaseType::Int ||
      mBaseType == ShaderIRTypeBaseType::Float)
    return 4;
  // Don't pad matrices to weird byte alignments
  else if (mBaseType == ShaderIRTypeBaseType::Vector)
    return mComponents * mComponentType->GetByteSize();
  else if (mBaseType == ShaderIRTypeBaseType::Matrix)
  {
    return GetByteAlignment() * mComponents;
  }
  else if (mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    // The actual size of a fixed array is the number of elements times the
    // array stride. The array stride is the size of the contained item rounded
    // up based upon the max alignment
    RaverieShaderIRType* elementType = mParameters[0]->As<RaverieShaderIRType>();
    size_t elementByteSize = elementType->GetByteSize();
    size_t alignment = GetByteAlignment();
    size_t itemSize = GetSizeAfterAlignment(elementByteSize, alignment);
    return itemSize * mComponents;
  }
  else if (mBaseType == ShaderIRTypeBaseType::Struct)
  {
    // Each element has to actually be properly aligned in order to get the
    // correct size though otherwise this can drift very wildly.
    // For example struct { float A; vec3 B; float C; vec3 D; }
    // Is actually size 16 + 16 + 16 + 12 = 60 due to vec3 having
    // to be aligned on 16 byte boundaries.
    size_t size = 0;
    for (size_t i = 0; i < mParameters.Size(); ++i)
    {
      RaverieShaderIRType* memberType = GetSubType(i);
      size_t alignment = memberType->GetByteAlignment();
      size_t memberSize = memberType->GetByteSize();
      // Fix the current offset to be at the required alignment for this member.
      size = GetSizeAfterAlignment(size, alignment);
      // Then add the member size exactly as is (no padding
      // is required unless another element follows)
      size += memberSize;
    }
    // Vulkan Spec: A struct has a base alignment equal to the largest base
    // alignment of any of its memebers rounded up to a multiple of 16.
    size = GetSizeAfterAlignment(size, 16);
    return size;
  }
  Error("Unknown type for byte size");
  return 0;
}

size_t RaverieShaderIRType::GetByteAlignment() const
{
  if (mBaseType == ShaderIRTypeBaseType::Bool || mBaseType == ShaderIRTypeBaseType::Int ||
      mBaseType == ShaderIRTypeBaseType::Float)
    return 4;
  else if (mBaseType == ShaderIRTypeBaseType::Vector)
  {
    int components = mComponents;
    // Real3 has to be aligned to 16 bytes per Vulkan spec.
    if (components == 3)
      components = 4;
    return components * mComponentType->GetByteAlignment();
  }
  else if (mBaseType == ShaderIRTypeBaseType::Matrix)
  {
    // Via opengl/dx matrix types are treated as an array of the vector types
    // where the vector types are padded up to vec4s. This happens for
    // efficiency reason (at least with uniform buffers).
    RaverieShaderIRType* scalarType = mComponentType->mComponentType;
    return 4 * scalarType->GetByteAlignment();
  }
  else if (mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    // Via opengl/dx array of the vector types where the vector
    // types are padded up to vec4s. This happens for efficiency reason (at
    // least with uniform buffers).
    RaverieShaderIRType* elementType = mParameters[0]->As<RaverieShaderIRType>();
    if (elementType->mBaseType == ShaderIRTypeBaseType::Int || elementType->mBaseType == ShaderIRTypeBaseType::Float ||
        elementType->mBaseType == ShaderIRTypeBaseType::Bool || elementType->mBaseType == ShaderIRTypeBaseType::Vector)
      return 16;
    return elementType->GetByteAlignment();
  }
  else if (mBaseType == ShaderIRTypeBaseType::Struct)
  {
    // The alignment of a struct is the max alignment of all of its members
    size_t alignment = 0;
    for (size_t i = 0; i < mParameters.Size(); ++i)
    {
      RaverieShaderIRType* elementType = GetSubType(i);
      alignment = Math::Max(elementType->GetByteAlignment(), alignment);
    }
    return alignment;
  }
  // Ignore structs for now
  Error("Unknown type for byte size");
  return 0;
}

ShaderIRTypeBaseType::Enum RaverieShaderIRType::GetBasePrimitiveType() const
{
  if (mBaseType == ShaderIRTypeBaseType::Bool || mBaseType == ShaderIRTypeBaseType::Int ||
      mBaseType == ShaderIRTypeBaseType::Float)
    return mBaseType;
  else if (mBaseType == ShaderIRTypeBaseType::Vector || mBaseType == ShaderIRTypeBaseType::Matrix)
    return mComponentType->GetBasePrimitiveType();
  return mBaseType;
}

ShaderIRAttribute* RaverieShaderIRType::FindFirstAttribute(StringParam attributeName) const
{
  if (mMeta == nullptr)
    return nullptr;
  return mMeta->mAttributes.FindFirstAttribute(attributeName);
}

bool RaverieShaderIRType::IsPointerType()
{
  return mBaseType == ShaderIRTypeBaseType::Pointer;
}

bool RaverieShaderIRType::IsGlobalType() const
{
  // Find the storage class attribute
  ShaderIRAttribute* storageClassAttribute = FindFirstAttribute(SpirVNameSettings::mStorageClassAttribute);
  if (storageClassAttribute == nullptr)
    return false;

  // Check the value of the storage class
  spv::StorageClass storageClass = (spv::StorageClass)storageClassAttribute->mParameters[0].GetIntValue();
  if (storageClass == spv::StorageClass::StorageClassUniformConstant || storageClass == spv::StorageClassUniform ||
      storageClass == spv::StorageClassStorageBuffer)
    return true;
  return false;
}

RaverieShaderIRType* RaverieShaderIRType::GetValueType()
{
  if (mBaseType == ShaderIRTypeBaseType::Pointer)
    return mDereferenceType;
  return this;
}

RaverieShaderIRType* RaverieShaderIRType::GetPointerType()
{
  if (mBaseType == ShaderIRTypeBaseType::Pointer)
    return this;
  return mPointerType;
}

RaverieShaderIRType* GetComponentType(RaverieShaderIRType* compositeType)
{
  bool isMathType = compositeType->mBaseType == ShaderIRTypeBaseType::Int ||
                    compositeType->mBaseType == ShaderIRTypeBaseType::Float ||
                    compositeType->mBaseType == ShaderIRTypeBaseType::Vector ||
                    compositeType->mBaseType == ShaderIRTypeBaseType::Matrix;
  ErrorIf(!isMathType,
          "Invalid type to find component type on. Only math types "
          "(scalars/vectors/matrices) are allowed");
  return compositeType->mComponentType;
}

bool IsScalarType(RaverieShaderIRType* compositeType)
{
  bool isScalarType = compositeType->mBaseType == ShaderIRTypeBaseType::Int ||
                      compositeType->mBaseType == ShaderIRTypeBaseType::Float ||
                      compositeType->mBaseType == ShaderIRTypeBaseType::Bool;
  return isScalarType;
}

RaverieShaderIRType* GetImageTypeFromSampledImage(RaverieShaderIRType* samplerType)
{
  if (samplerType->mBaseType != ShaderIRTypeBaseType::SampledImage)
  {
    Error("Given type was expected to be a SampledImage but was '%s'", samplerType->mName.c_str());
    return nullptr;
  }

  IRaverieShaderIR* imageArg = samplerType->mParameters[0];
  RaverieShaderIRType* imageType = imageArg->As<RaverieShaderIRType>();
  ErrorIf(imageType->mBaseType != ShaderIRTypeBaseType::Image, "Sampler's image type parameter isn't an image");
  return imageType;
}

int GetStride(RaverieShaderIRType* type, float baseAlignment)
{
  size_t typeSize = type->GetByteSize();
  int stride = (int)(baseAlignment * Math::Ceil(typeSize / baseAlignment));
  return stride;
}

size_t GetSizeAfterAlignment(size_t size, size_t baseAlignment)
{
  // Get the remainder to add
  size_t remainder = baseAlignment - (size % baseAlignment);
  // Mod with the required alignment to get offset 0 when needed
  size_t alignmentOffset = remainder % baseAlignment;
  size += alignmentOffset;
  return size;
}

TemplateTypeKey GenerateTemplateTypeKey(Raverie::BoundType* raverieType)
{
  StringBuilder builder;
  builder.Append(raverieType->TemplateBaseName);
  builder.Append("[");

  size_t size = raverieType->TemplateArguments.Size();
  for (size_t i = 0; i < size; ++i)
  {
    Raverie::Constant& arg = raverieType->TemplateArguments[i];
    builder.Append(Raverie::ConstantType::Names[arg.Type]);
    if (i != size - 1)
      builder.Append(",");
  }
  builder.Append("]");
  return builder.ToString();
}

String GenerateSpirVPropertyName(StringParam fieldName, StringParam ownerType)
{
  return BuildString(fieldName, "_", ownerType);
}

String GenerateSpirVPropertyName(StringParam fieldName, RaverieShaderIRType* ownerType)
{
  return GenerateSpirVPropertyName(fieldName, ownerType->mName);
}

Array<String> GetOpcodeNames()
{
  // Get all opcodes from spirv (hardcode the language for now...)
  spv_opcodes_t opCodeNames;
  spvGetOpcodeNames(SPV_ENV_UNIVERSAL_1_4, &opCodeNames);

  // Convert each opcode to string
  Array<String> results;
  for (size_t i = 0; i < opCodeNames.count; ++i)
  {
    results.PushBack(opCodeNames.opcode_names[i]);
  }
  spvDestroyOpcodeNames(&opCodeNames);

  return results;
}

} // namespace Raverie
