#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderIROp
ZilchShaderIROp::ZilchShaderIROp(OpType opType) : IZilchShaderIR(mStaticBaseType)
{
  mOpType = opType;
}

bool ZilchShaderIROp::IsTerminator()
{
  bool isTerminator =
    (mOpType == OpType::OpReturn) ||
    (mOpType == OpType::OpReturnValue) ||
    (mOpType == OpType::OpBranch) ||
    (mOpType == OpType::OpBranchConditional);
  return isTerminator;
}

bool ZilchShaderIROp::IsResultPointerType()
{
  if(mResultType == nullptr)
    return false;
  return mResultType->IsPointerType();
}

ZilchShaderIRType* ZilchShaderIROp::GetValueType()
{
  return mResultType->GetValueType();
}

ZilchShaderIRType* ZilchShaderIROp::GetPointerType()
{
  return mResultType->GetPointerType();
}

//-------------------------------------------------------------------BasicBlock
BasicBlock::BasicBlock() : IZilchShaderIR(mStaticBaseType)
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

void BasicBlock::AddOp(IZilchShaderIR* op)
{
  mLines.PushBack(op);
}

EntryPointInfo::EntryPointInfo()
{
  mEntryPointFn = nullptr;
  mGlobalsInitializerFunction = nullptr;
}

//-------------------------------------------------------------------ZilchShaderIRFunction
ZilchShaderIRFunction::ZilchShaderIRFunction() : IZilchShaderIR(mStaticBaseType)
{
  mMeta = nullptr;
}

ZilchShaderIRFunction::~ZilchShaderIRFunction()
{
  DeleteObjectsIn(mBlocks.All());
  // mMeta owned by the type's meta
}

ZilchShaderIRType* ZilchShaderIRFunction::GetReturnType()
{
  // The return type is always the first sub-type
  return mFunctionType->GetSubType(0);
}

//-------------------------------------------------------------------ZilchShaderIRType
ZilchShaderIRType::ZilchShaderIRType() : IZilchShaderIR(mStaticBaseType)
{
  mComponentType = nullptr;
  mZilchType = nullptr;
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

ZilchShaderIRType::~ZilchShaderIRType()
{
  delete mEntryPoint;
}

ZilchShaderIRFunction* ZilchShaderIRType::CreateFunction(ZilchShaderIRLibrary* library)
{
  ZilchShaderIRFunction* function = new ZilchShaderIRFunction();
  mFunctions.PushBack(function);
  library->mOwnedFunctions.PushBack(function);
  return function;
}

void ZilchShaderIRType::AddMember(ZilchShaderIRType* memberType, StringParam memberName)
{
  mParameters.PushBack(memberType);

  int index = mParameters.Size() - 1;
  mMemberNamesToIndex[memberName] = index;
  mMemberKeysToIndex[ShaderFieldKey(memberName, memberType->mZilchType->ToString())] = index;
}

String ZilchShaderIRType::GetMemberName(size_t memberIndex)
{
  // Currently we only store names to indices but not the other way around.
  // For now just iterate through this map to find the member index.
  // A type shouldn't be too big and this is rarely done so this is acceptable for now.
  for(auto range = mMemberNamesToIndex.All(); !range.Empty(); range.PopFront())
  {
    auto pair = range.Front();
    if(pair.second == memberIndex)
      return pair.first;
  }
  return String();
}

ZilchShaderIRType* ZilchShaderIRType::GetSubType(int index)
{
  bool supportsSubTypes = mBaseType == ShaderIRTypeBaseType::Struct ||
    mBaseType == ShaderIRTypeBaseType::Function;

  ErrorIf(!supportsSubTypes, "Type '%s' does not support sub-types. The parameters on this type are not guaranteed to be types.", mName.c_str());

  IZilchShaderIR* param = mParameters[index];
  ZilchShaderIRType* subType = param->As<ZilchShaderIRType>();
  return subType;
}

size_t ZilchShaderIRType::GetSubTypeCount()
{
  bool supportsSubTypes = mBaseType == ShaderIRTypeBaseType::Struct ||
    mBaseType == ShaderIRTypeBaseType::Function;

  ErrorIf(!supportsSubTypes, "Type '%s' does not support sub-types. The parameters on this type are not guaranteed to be types.", mName.c_str());

  return mParameters.Size();
}

size_t ZilchShaderIRType::GetByteSize() const
{
  // Force everything to 4 bytes
  if(mBaseType == ShaderIRTypeBaseType::Bool || mBaseType == ShaderIRTypeBaseType::Int || mBaseType == ShaderIRTypeBaseType::Float)
    return 4;
  // Don't pad matrices to weird byte alignments
  else if(mBaseType == ShaderIRTypeBaseType::Vector)
    return mComponents * mComponentType->GetByteSize();
  else if(mBaseType == ShaderIRTypeBaseType::Matrix)
  {
    return GetByteAlignment() * mComponents;
  }
  else if(mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    return GetByteAlignment() * mComponents;
  }
  // Ignore structs for now
  Error("Unknown type for byte size");
  return 0;
}

size_t ZilchShaderIRType::GetByteAlignment() const
{
  if(mBaseType == ShaderIRTypeBaseType::Bool || mBaseType == ShaderIRTypeBaseType::Int || mBaseType == ShaderIRTypeBaseType::Float)
    return 4;
  else if(mBaseType == ShaderIRTypeBaseType::Vector)
  {
    int components = mComponents;
    // Real3 has to be aligned to 16 bytes per Vulkan spec.
    if(components == 3)
      components = 4;
    return components * mComponentType->GetByteAlignment();
  }
  else if(mBaseType == ShaderIRTypeBaseType::Matrix)
  {
    // Via opengl/dx matrix types are treated as an array of the vector types where the vector
    // types are padded up to vec4s. This happens for efficiency reason (at least with uniform buffers).
    ZilchShaderIRType* scalarType = mComponentType->mComponentType;
    return 4 * scalarType->GetByteAlignment();
  }
  else if(mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    // Via opengl/dx array of the vector types where the vector
    // types are padded up to vec4s. This happens for efficiency reason (at least with uniform buffers).
    ZilchShaderIRType* elementType = mParameters[0]->As<ZilchShaderIRType>();
    if(elementType->mBaseType == ShaderIRTypeBaseType::Int ||
      elementType->mBaseType == ShaderIRTypeBaseType::Float ||
      elementType->mBaseType == ShaderIRTypeBaseType::Bool ||
      elementType->mBaseType == ShaderIRTypeBaseType::Vector)
      return 16;
    return elementType->GetByteAlignment();
  }
  // Ignore structs for now
  Error("Unknown type for byte size");
  return 0;
}

ShaderIRTypeBaseType::Enum ZilchShaderIRType::GetBasePrimitiveType() const
{
  if(mBaseType == ShaderIRTypeBaseType::Bool || mBaseType == ShaderIRTypeBaseType::Int || mBaseType == ShaderIRTypeBaseType::Float)
    return mBaseType;
  else if(mBaseType == ShaderIRTypeBaseType::Vector || mBaseType == ShaderIRTypeBaseType::Matrix)
    return mComponentType->GetBasePrimitiveType();
  return mBaseType;
}

ShaderIRAttribute* ZilchShaderIRType::FindFirstAttribute(StringParam attributeName) const
{
  if(mMeta == nullptr)
    return nullptr;
  return mMeta->mAttributes.FindFirstAttribute(attributeName);
}

bool ZilchShaderIRType::IsPointerType()
{
  return mBaseType == ShaderIRTypeBaseType::Pointer;
}

bool ZilchShaderIRType::IsGlobalType() const
{
  // Find the storage class attribute
  ShaderIRAttribute* storageClassAttribute = FindFirstAttribute(SpirVNameSettings::mStorageClassAttribute);
  if(storageClassAttribute == nullptr)
    return false;

  // Check the value of the storage class
  spv::StorageClass storageClass = (spv::StorageClass)storageClassAttribute->mParameters[0].GetIntValue();
  if(storageClass == spv::StorageClass::StorageClassUniformConstant ||
    storageClass == spv::StorageClassUniform)
    return true;
  return false;
}

ZilchShaderIRType* ZilchShaderIRType::GetValueType()
{
  if(mBaseType == ShaderIRTypeBaseType::Pointer)
    return mDereferenceType;
  return this;
}

ZilchShaderIRType* ZilchShaderIRType::GetPointerType()
{
  if(mBaseType == ShaderIRTypeBaseType::Pointer)
    return this;
  return mPointerType;
}

ZilchShaderIRType* GetComponentType(ZilchShaderIRType* compositeType)
{
  bool isMathType = compositeType->mBaseType == ShaderIRTypeBaseType::Int ||
    compositeType->mBaseType == ShaderIRTypeBaseType::Float ||
    compositeType->mBaseType == ShaderIRTypeBaseType::Vector ||
    compositeType->mBaseType == ShaderIRTypeBaseType::Matrix;
  ErrorIf(!isMathType, "Invalid type to find component type on. Only math types (scalars/vectors/matrices) are allowed");
  return compositeType->mComponentType;
}

bool IsScalarType(ZilchShaderIRType* compositeType)
{
  bool isScalarType = compositeType->mBaseType == ShaderIRTypeBaseType::Int ||
    compositeType->mBaseType == ShaderIRTypeBaseType::Float ||
    compositeType->mBaseType == ShaderIRTypeBaseType::Bool;
  return isScalarType;
}

ZilchShaderIRType* GetImageTypeFromSampledImage(ZilchShaderIRType* samplerType)
{
  if(samplerType->mBaseType != ShaderIRTypeBaseType::SampledImage)
  {
    Error("Given type was expected to be a SampledImage but was '%s'", samplerType->mName.c_str());
    return nullptr;
  }

  IZilchShaderIR* imageArg = samplerType->mParameters[0];
  ZilchShaderIRType* imageType = imageArg->As<ZilchShaderIRType>();
  ErrorIf(imageType->mBaseType != ShaderIRTypeBaseType::Image, "Sampler's image type parameter isn't an image");
  return imageType;
}

int GetStride(ZilchShaderIRType* type, float baseAlignment)
{
  size_t typeSize = type->GetByteSize();
  int stride = (int)(baseAlignment * Math::Ceil(typeSize / baseAlignment));
  return stride;
}

TemplateTypeKey GenerateTemplateTypeKey(Zilch::BoundType* zilchType)
{
  StringBuilder builder;
  builder.Append(zilchType->TemplateBaseName);
  builder.Append("[");

  size_t size = zilchType->TemplateArguments.Size();
  for(size_t i = 0; i < size; ++i)
  {
    Zilch::Constant& arg = zilchType->TemplateArguments[i];
    builder.Append(Zilch::ConstantType::Names[arg.Type]);
    if(i != size - 1)
      builder.Append(",");
  }
  builder.Append("]");
  return builder.ToString();
}

String GenerateSpirVPropertyName(StringParam fieldName, StringParam ownerType)
{
  return BuildString(fieldName, "_", ownerType);
}

String GenerateSpirVPropertyName(StringParam fieldName, ZilchShaderIRType* ownerType)
{
  return GenerateSpirVPropertyName(fieldName, ownerType->mName);
}

}//namespace Zero