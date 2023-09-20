// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class TypeDependencyCollector
{
public:
  TypeDependencyCollector(RaverieShaderIRLibrary* owningLibrary);
  void Collect(RaverieShaderIRType* type);

  void Collect(RaverieShaderIRFunction* function);
  void Collect(BasicBlock* block);
  void Collect(RaverieShaderIROp* op);
  void CollectArguments(RaverieShaderIROp* op);
  void Collect(RaverieShaderExtensionImport* op);
  void Collect(IRaverieShaderIR* instruction);

  void AddTypeReference(RaverieShaderIRType* type);
  void AddConstantReference(RaverieShaderIROp* op);
  void AddGlobalReference(RaverieShaderIROp* op);

  bool IsGlobalStorageClass(spv::StorageClass storageClass);

  OrderedHashSet<RaverieShaderIRType*> mReferencedTypes;
  OrderedHashSet<RaverieShaderIROp*> mReferencedConstants;
  OrderedHashSet<RaverieShaderIROp*> mReferencedGlobals;
  OrderedHashSet<RaverieShaderIRFunction*> mReferencedFunctions;
  OrderedHashSet<RaverieShaderExtensionImport*> mReferencedImports;
  OrderedHashSet<spv::Capability> mCapabilities;
  // Any required extensions that need to declare OpExtension. This is of type spvtools::Extension
  OrderedHashSet<int> mRequiredExtensions;

  // Keep track of the order we found types, constants, and globals. These are
  // all grouped in one section of the module and have to be in the correct
  // usage order (no forward references of ids)
  OrderedHashSet<IRaverieShaderIR*> mTypesConstantsAndGlobals;

  // Specifies if an op requires a certain capability that must be added.
  // @JoshD: Parse from the spirv grammar file at some point?
  HashMap<OpType, spv::Capability> mRequiredCapabilities;

  // All global variable initializer functions that need to be called for the
  // given type we processed.
  Array<RaverieShaderIRFunction*> mGlobalInitializers;

  // The library of the type we're processing. Needed to find global variables.
  RaverieShaderIRLibrary* mOwningLibrary;
};

} // namespace Raverie
