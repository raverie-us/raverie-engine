// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class TypeDependencyCollector
{
public:
  TypeDependencyCollector(ZilchShaderIRLibrary* owningLibrary);
  void Collect(ZilchShaderIRType* type);

  void Collect(ZilchShaderIRFunction* function);
  void Collect(BasicBlock* block);
  void Collect(ZilchShaderIROp* op);
  void CollectArguments(ZilchShaderIROp* op);
  void Collect(ZilchShaderExtensionImport* op);
  void Collect(IZilchShaderIR* instruction);

  void AddTypeReference(ZilchShaderIRType* type);
  void AddConstantReference(ZilchShaderIROp* op);
  void AddGlobalReference(ZilchShaderIROp* op);

  bool IsGlobalStorageClass(spv::StorageClass storageClass);

  OrderedHashSet<ZilchShaderIRType*> mReferencedTypes;
  OrderedHashSet<ZilchShaderIROp*> mReferencedConstants;
  OrderedHashSet<ZilchShaderIROp*> mReferencedGlobals;
  OrderedHashSet<ZilchShaderIRFunction*> mReferencedFunctions;
  OrderedHashSet<ZilchShaderExtensionImport*> mReferencedImports;
  OrderedHashSet<spv::Capability> mCapabilities;

  // Keep track of the order we found types, constants, and globals. These are
  // all grouped in one section of the module and have to be in the correct
  // usage order (no forward references of ids)
  OrderedHashSet<IZilchShaderIR*> mTypesConstantsAndGlobals;

  // Specifies if an op requires a certain capability that must be added.
  // @JoshD: Parse from the spirv grammar file at some point?
  HashMap<OpType, spv::Capability> mRequiredCapabilities;

  // All global variable initializer functions that need to be called for the
  // given type we processed.
  Array<ZilchShaderIRFunction*> mGlobalInitializers;

  // The library of the type we're processing. Needed to find global variables.
  ZilchShaderIRLibrary* mOwningLibrary;
};

} // namespace Zero
