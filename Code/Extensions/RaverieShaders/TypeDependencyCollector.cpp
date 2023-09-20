// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "extensions.h"

namespace Raverie
{

TypeDependencyCollector::TypeDependencyCollector(RaverieShaderIRLibrary* owningLibrary)
{
  mOwningLibrary = owningLibrary;
  // @JoshD: This can't always be on in vulkan. Figure out how to experiment with linkage later.
  // mCapabilities.InsertOrIgnore(spv::CapabilityLinkage);
  mCapabilities.InsertOrIgnore(spv::CapabilityShader);
  // @JoshD: Parse from spirv grammar later. Just prototyping now.
  mRequiredCapabilities[spv::OpImageQuerySize] = spv::CapabilityImageQuery;
  mRequiredCapabilities[spv::OpImageQuerySizeLod] = spv::CapabilityImageQuery;
  mRequiredCapabilities[spv::OpImageQueryLod] = spv::CapabilityImageQuery;
  mRequiredCapabilities[spv::OpImageQueryLevels] = spv::CapabilityImageQuery;
}

void TypeDependencyCollector::Collect(RaverieShaderIRType* type)
{
  if (type == nullptr)
    return;

  if (mReferencedTypes.ContainsKey(type))
    return;

  // @JoshD: Value types should always be visited before pointers...
  if (!mReferencedTypes.ContainsKey(type->mDereferenceType))
    Collect(type->mDereferenceType);

  if (type->mComponentType != nullptr)
    Collect(type->mComponentType);
  // Walk all parameters of this type
  for (size_t i = 0; i < type->mParameters.Size(); ++i)
    Collect(type->mParameters[i]);

  if (!mReferencedTypes.ContainsKey(type))
    AddTypeReference(type);

  // Walk any pointer types if necessary
  Collect(type->mPointerType);

  for (size_t i = 0; i < type->mFunctions.Size(); ++i)
  {
    Collect(type->mFunctions[i]);
  }
}

void TypeDependencyCollector::Collect(RaverieShaderIRFunction* function)
{
  if (mReferencedFunctions.ContainsKey(function))
    return;
  mReferencedFunctions.InsertOrError(function);

  Collect(function->mFunctionType);
  Collect(&function->mParameterBlock);
  for (size_t i = 0; i < function->mBlocks.Size(); ++i)
  {
    Collect(function->mBlocks[i]);
  }
}

void TypeDependencyCollector::Collect(BasicBlock* block)
{
  for (size_t i = 0; i < block->mLines.Size(); ++i)
  {
    Collect(block->mLines[i]);
  }
  // @JoshD: Flip the order of these two later when a diff is cleaner to do.
  for (size_t i = 0; i < block->mLocalVariables.Size(); ++i)
  {
    Collect(block->mLocalVariables[i]);
  }
}

void TypeDependencyCollector::Collect(RaverieShaderIROp* op)
{
  if (op->mResultType != nullptr)
    Collect(op->mResultType);

  // Check for global variables
  if (op->mOpType == OpType::OpVariable)
  {
    RaverieShaderIRConstantLiteral* storageClassLiteral = op->mArguments[0]->As<RaverieShaderIRConstantLiteral>();
    spv::StorageClass storageClass = (spv::StorageClass)storageClassLiteral->mValue.Get<int>();
    // If this is a global variable the add it to the global variable list and
    // collect all arguments as normal
    if (IsGlobalStorageClass(storageClass))
    {
      AddGlobalReference(op);
      CollectArguments(op);
      return;
    }
  }

  // Check if an op requires a capability, if so add it.
  // @JoshD: Can an op require more than one capability? They're nested so I'm
  // not sure (primarily with Kernel)
  spv::Capability* requiredCapability = mRequiredCapabilities.FindPointer(op->mOpType);
  if (requiredCapability != nullptr)
    mCapabilities.InsertOrIgnore(*requiredCapability);

  if (op->mOpType == OpType::OpBranchConditional)
  {
    // Only collect on the conditional, not on the branch targets in order to
    // avoid infinite loops
    Collect(op->mArguments[0]);
    return;
  }

  CollectArguments(op);

  // Handle constants (have to add them to a separate map). These should be
  // added after collecting all arguments so that composite instructions are
  // guaranteed to have already visited their constituents.
  if (op->mOpType == OpType::OpConstant || op->mOpType == OpType::OpConstantComposite || op->mOpType == OpType::OpSpecConstant || op->mOpType == OpType::OpSpecConstantComposite)
  {
    AddConstantReference(op);
  }
}

void TypeDependencyCollector::CollectArguments(RaverieShaderIROp* op)
{
  for (size_t i = 0; i < op->mArguments.Size(); ++i)
  {
    IRaverieShaderIR* arg = op->mArguments[i];
    // Don't walk blocks or we'll get infinite loops (covered by the function)
    if (arg->mIRType == RaverieShaderIRBaseType::Block)
      continue;
    Collect(arg);
  }
}

void TypeDependencyCollector::Collect(RaverieShaderExtensionImport* op)
{
  mReferencedImports.InsertOrIgnore(op);
}

void TypeDependencyCollector::Collect(IRaverieShaderIR* instruction)
{
  if (instruction->mIRType == RaverieShaderIRBaseType::DataType)
    Collect(instruction->As<RaverieShaderIRType>());
  else if (instruction->mIRType == RaverieShaderIRBaseType::Op)
    Collect(instruction->As<RaverieShaderIROp>());
  else if (instruction->mIRType == RaverieShaderIRBaseType::Function)
    Collect(instruction->As<RaverieShaderIRFunction>());
  else if (instruction->mIRType == RaverieShaderIRBaseType::Block)
    Collect(instruction->As<BasicBlock>());
  else if (instruction->mIRType == RaverieShaderIRBaseType::Extension)
    Collect(instruction->As<RaverieShaderExtensionImport>());
}

void TypeDependencyCollector::AddTypeReference(RaverieShaderIRType* type)
{
  mReferencedTypes.InsertOrIgnore(type);
  mTypesConstantsAndGlobals.InsertOrIgnore(type);

  // Storage buffers require an extension specification
  if (type->mStorageClass == spv::StorageClassStorageBuffer)
  {
    mRequiredExtensions.InsertOrIgnore(spvtools::kSPV_KHR_storage_buffer_storage_class);
  }
}

void TypeDependencyCollector::AddConstantReference(RaverieShaderIROp* op)
{
  mReferencedConstants.InsertOrIgnore(op);
  mTypesConstantsAndGlobals.InsertOrIgnore(op);
}

void TypeDependencyCollector::AddGlobalReference(RaverieShaderIROp* op)
{
  // Don't process a global more than once
  if (mTypesConstantsAndGlobals.ContainsKey(op))
    return;

  mReferencedGlobals.InsertOrIgnore(op);
  mTypesConstantsAndGlobals.InsertOrIgnore(op);

  // Try to add the global variable's initializer function
  GlobalVariableData* globalVarData = mOwningLibrary->FindGlobalVariable(op);
  if (globalVarData != nullptr && globalVarData->mInitializerFunction != nullptr)
  {
    // Make sure to collect all referenced objects in the initializer function
    Collect(globalVarData->mInitializerFunction);
    mGlobalInitializers.PushBack(globalVarData->mInitializerFunction);
  }
}

bool TypeDependencyCollector::IsGlobalStorageClass(spv::StorageClass storageClass)
{
  return storageClass == spv::StorageClassUniform || storageClass == spv::StorageClassUniformConstant || storageClass == spv::StorageClassStorageBuffer || storageClass == spv::StorageClassInput ||
         storageClass == spv::StorageClassOutput || storageClass == spv::StorageClassPrivate;
}

} // namespace Raverie
