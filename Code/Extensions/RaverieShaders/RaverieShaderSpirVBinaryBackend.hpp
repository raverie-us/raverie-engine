// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class RaverieShaderToSpirVContext
{
public:
  RaverieShaderToSpirVContext();

  // Get an id for the next IR.
  int GetAndAdvanceId();
  void GenerateId(IRaverieShaderIR* ir);

  int FindId(IRaverieShaderIR* instruction, bool assertOnZero = true);

  ShaderStreamWriter* mStreamWriter;
  ShaderStageInterfaceReflection* mReflectionData;

  // Id mapping of an instruction
  int mId;
  HashMap<IRaverieShaderIR*, int> mGeneratedId;
  Array<EntryPointInfo*> mEntryPoints;
  RaverieShaderIRFunction* mMain;
};

class RaverieShaderSpirVBinaryBackend
{
public:
  ~RaverieShaderSpirVBinaryBackend();

  void TranslateType(RaverieShaderIRType* type, ShaderStreamWriter& writer);
  void TranslateType(RaverieShaderIRType* type,
                     ShaderStreamWriter& writer,
                     ShaderStageInterfaceReflection& reflectionData);

private:
  // Prototype of generating one library with multiple entry points.
  // Not currently used or tested. Also would need to be updated for multiple
  // reflection objects.
  void TranslateLibrary(RaverieShaderIRLibrary* library,
                        ShaderStreamWriter& writer,
                        ShaderStageInterfaceReflection& reflectionData);

  // Debugging helper
  void ValidateIdMap(RaverieShaderToSpirVContext* context);

  typedef OrderedHashSet<IRaverieShaderIR*> IRList;
  typedef OrderedHashSet<RaverieShaderIRType*> TypeList;
  typedef OrderedHashSet<RaverieShaderIRFunction*> FunctionList;
  typedef OrderedHashSet<RaverieShaderIROp*> OpList;
  typedef OrderedHashSet<RaverieShaderExtensionImport*> ImportList;
  typedef OrderedHashMap<RaverieShaderIRFunction*, RaverieShaderIRFunction*> LateBoundFunctionMap;

  /// Helper function to emit the given entry points and their dependencies out
  /// to spirv binary.
  void EmitSpirvBinary(TypeDependencyCollector& collector, RaverieShaderToSpirVContext* context);

  void GenerateDummyMain(RaverieShaderIRType* type,
                         RaverieShaderIRLibrary* library,
                         TypeDependencyCollector& collector,
                         RaverieShaderToSpirVContext* context);
  void GenerateGlobalsInitializerFunction(TypeDependencyCollector& collector, RaverieShaderToSpirVContext* context);
  void RegisterLateBoundFunctions(LateBoundFunctionMap& lateBoundFunctionMap,
                                  TypeDependencyCollector& collector,
                                  RaverieShaderToSpirVContext* context);
  void Clear();

  void AddDecorationCapabilities(TypeDependencyCollector& collector, RaverieShaderToSpirVContext* context);
  void AddDecorationCapabilities(EntryPointInfo* entryPoint,
                                 TypeDependencyCollector& collector,
                                 RaverieShaderToSpirVContext* context);
  void AddDecorationCapabilities(RaverieShaderIROp* decorationOp,
                                 TypeDependencyCollector& collector,
                                 RaverieShaderToSpirVContext* context);
  void AddMemberDecorationCapabilities(RaverieShaderIROp* memberDecorationOp,
                                       TypeDependencyCollector& collector,
                                       RaverieShaderToSpirVContext* context);

  template <typename T>
  void GenerateListIds(OrderedHashSet<T>& input, RaverieShaderToSpirVContext* context);
  void GenerateFunctionIds(FunctionList& functions, RaverieShaderToSpirVContext* context);
  void GenerateFunctionBlockIds(RaverieShaderIRFunction* function, RaverieShaderToSpirVContext* context);
  void GenerateBlockLineIds(BasicBlock* block, RaverieShaderToSpirVContext* context);

  void WriteHeader(RaverieShaderToSpirVContext* context, TypeDependencyCollector& typeCollector);
  void WriteDebug(TypeList& types, RaverieShaderToSpirVContext* context);
  void WriteDebug(RaverieShaderIRType* type, RaverieShaderToSpirVContext* context);
  void WriteDebug(FunctionList& functions, RaverieShaderToSpirVContext* context);
  void WriteDebug(RaverieShaderIRFunction* function, RaverieShaderToSpirVContext* context);
  void WriteDebug(BasicBlock* block, RaverieShaderToSpirVContext* context);
  void WriteDebug(OpList& ops, RaverieShaderToSpirVContext* context);
  void WriteDebugName(IRaverieShaderIR* resultIR, StringParam debugName, RaverieShaderToSpirVContext* context);
  void WriteDecorations(RaverieShaderToSpirVContext* context);
  void WriteSpecializationConstantBindingDecorations(TypeDependencyCollector& typeCollector,
                                                     RaverieShaderToSpirVContext* context);
  RaverieShaderIROp* FindSpecialiationConstantCompositeId(RaverieShaderIROp* op);

  void WriteTypesGlobalsAndConstants(IRList& typesGlobalsAndConstants, RaverieShaderToSpirVContext* context);
  void WriteType(RaverieShaderIRType* type, RaverieShaderToSpirVContext* context);
  void WriteConstant(RaverieShaderIROp* constantOp, RaverieShaderToSpirVContext* context);
  void WriteSpecConstant(RaverieShaderIROp* constantOp, RaverieShaderToSpirVContext* context);
  void WriteGlobal(RaverieShaderIROp* globalVarOp, RaverieShaderToSpirVContext* context);
  void WriteFunctions(FunctionList& functions, RaverieShaderToSpirVContext* context);
  void WriteFunction(RaverieShaderIRFunction* function, RaverieShaderToSpirVContext* context);

  void WriteBlock(BasicBlock* block, RaverieShaderToSpirVContext* context);
  void WriteBlockInstructions(BasicBlock* block,
                              Array<IRaverieShaderIR*>& instructions,
                              RaverieShaderToSpirVContext* context);
  void WriteIROp(BasicBlock* block, RaverieShaderIROp* op, RaverieShaderToSpirVContext* context);
  void WriteIROpGeneric(RaverieShaderIROp* op, RaverieShaderToSpirVContext* context);
  void WriteIROpGenericNoReturnType(RaverieShaderIROp* op, RaverieShaderToSpirVContext* context);
  void WriteIROpArguments(RaverieShaderIROp* op, RaverieShaderToSpirVContext* context);
  void WriteIRArguments(Array<IRaverieShaderIR*>& mArguments, RaverieShaderToSpirVContext* context);
  void WriteIRId(IRaverieShaderIR* ir, RaverieShaderToSpirVContext* context);
  void WriteImport(RaverieShaderExtensionImport* importLibrary, RaverieShaderToSpirVContext* context);

  Array<IRaverieShaderIR*> mOwnedInstructions;
  Array<EntryPointInfo*> mOwnedEntryPoints;
  LateBoundFunctionMap mExtraLateBoundFunctions;
  RaverieShaderIRLibraryRef mLastLibrary;
};

} // namespace Raverie
