// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

class ZilchShaderToSpirVContext
{
public:
  ZilchShaderToSpirVContext();

  // Get an id for the next IR.
  int GetAndAdvanceId();
  void GenerateId(IZilchShaderIR* ir);

  int FindId(IZilchShaderIR* instruction, bool assertOnZero = true);

  ShaderStreamWriter* mStreamWriter;
  ShaderStageInterfaceReflection* mReflectionData;

  // Id mapping of an instruction
  int mId;
  HashMap<IZilchShaderIR*, int> mGeneratedId;
  Array<EntryPointInfo*> mEntryPoints;
  ZilchShaderIRFunction* mMain;
};

class ZilchShaderSpirVBinaryBackend
{
public:
  ~ZilchShaderSpirVBinaryBackend();

  void TranslateType(ZilchShaderIRType* type, ShaderStreamWriter& writer);
  void TranslateType(ZilchShaderIRType* type,
                     ShaderStreamWriter& writer,
                     ShaderStageInterfaceReflection& reflectionData);

private:
  // Prototype of generating one library with multiple entry points.
  // Not currently used or tested. Also would need to be updated for multiple
  // reflection objects.
  void TranslateLibrary(ZilchShaderIRLibrary* library,
                        ShaderStreamWriter& writer,
                        ShaderStageInterfaceReflection& reflectionData);

  // Debugging helper
  void ValidateIdMap(ZilchShaderToSpirVContext* context);

  typedef OrderedHashSet<IZilchShaderIR*> IRList;
  typedef OrderedHashSet<ZilchShaderIRType*> TypeList;
  typedef OrderedHashSet<ZilchShaderIRFunction*> FunctionList;
  typedef OrderedHashSet<ZilchShaderIROp*> OpList;
  typedef OrderedHashSet<ZilchShaderExtensionImport*> ImportList;
  typedef OrderedHashMap<ZilchShaderIRFunction*, ZilchShaderIRFunction*> LateBoundFunctionMap;

  /// Helper function to emit the given entry points and their dependencies out
  /// to spirv binary.
  void EmitSpirvBinary(TypeDependencyCollector& collector, ZilchShaderToSpirVContext* context);

  void GenerateDummyMain(ZilchShaderIRType* type,
                         ZilchShaderIRLibrary* library,
                         TypeDependencyCollector& collector,
                         ZilchShaderToSpirVContext* context);
  void GenerateGlobalsInitializerFunction(TypeDependencyCollector& collector, ZilchShaderToSpirVContext* context);
  void RegisterLateBoundFunctions(LateBoundFunctionMap& lateBoundFunctionMap,
                                  TypeDependencyCollector& collector,
                                  ZilchShaderToSpirVContext* context);
  void Clear();

  void AddDecorationCapabilities(TypeDependencyCollector& collector, ZilchShaderToSpirVContext* context);
  void AddDecorationCapabilities(EntryPointInfo* entryPoint,
                                 TypeDependencyCollector& collector,
                                 ZilchShaderToSpirVContext* context);
  void AddDecorationCapabilities(ZilchShaderIROp* decorationOp,
                                 TypeDependencyCollector& collector,
                                 ZilchShaderToSpirVContext* context);
  void AddMemberDecorationCapabilities(ZilchShaderIROp* memberDecorationOp,
                                       TypeDependencyCollector& collector,
                                       ZilchShaderToSpirVContext* context);

  template <typename T>
  void GenerateListIds(OrderedHashSet<T>& input, ZilchShaderToSpirVContext* context);
  void GenerateFunctionIds(FunctionList& functions, ZilchShaderToSpirVContext* context);
  void GenerateFunctionBlockIds(ZilchShaderIRFunction* function, ZilchShaderToSpirVContext* context);
  void GenerateBlockLineIds(BasicBlock* block, ZilchShaderToSpirVContext* context);

  void WriteHeader(ZilchShaderToSpirVContext* context, TypeDependencyCollector& typeCollector);
  void WriteDebug(TypeList& types, ZilchShaderToSpirVContext* context);
  void WriteDebug(ZilchShaderIRType* type, ZilchShaderToSpirVContext* context);
  void WriteDebug(FunctionList& functions, ZilchShaderToSpirVContext* context);
  void WriteDebug(ZilchShaderIRFunction* function, ZilchShaderToSpirVContext* context);
  void WriteDebug(BasicBlock* block, ZilchShaderToSpirVContext* context);
  void WriteDebug(OpList& ops, ZilchShaderToSpirVContext* context);
  void WriteDebugName(IZilchShaderIR* resultIR, StringParam debugName, ZilchShaderToSpirVContext* context);
  void WriteDecorations(ZilchShaderToSpirVContext* context);
  void WriteSpecializationConstantBindingDecorations(TypeDependencyCollector& typeCollector,
                                                     ZilchShaderToSpirVContext* context);
  ZilchShaderIROp* FindSpecialiationConstantCompositeId(ZilchShaderIROp* op);

  void WriteTypesGlobalsAndConstants(IRList& typesGlobalsAndConstants, ZilchShaderToSpirVContext* context);
  void WriteType(ZilchShaderIRType* type, ZilchShaderToSpirVContext* context);
  void WriteConstant(ZilchShaderIROp* constantOp, ZilchShaderToSpirVContext* context);
  void WriteSpecConstant(ZilchShaderIROp* constantOp, ZilchShaderToSpirVContext* context);
  void WriteGlobal(ZilchShaderIROp* globalVarOp, ZilchShaderToSpirVContext* context);
  void WriteFunctions(FunctionList& functions, ZilchShaderToSpirVContext* context);
  void WriteFunction(ZilchShaderIRFunction* function, ZilchShaderToSpirVContext* context);

  void WriteBlock(BasicBlock* block, ZilchShaderToSpirVContext* context);
  void WriteBlockInstructions(BasicBlock* block,
                              Array<IZilchShaderIR*>& instructions,
                              ZilchShaderToSpirVContext* context);
  void WriteIROp(BasicBlock* block, ZilchShaderIROp* op, ZilchShaderToSpirVContext* context);
  void WriteIROpGeneric(ZilchShaderIROp* op, ZilchShaderToSpirVContext* context);
  void WriteIROpGenericNoReturnType(ZilchShaderIROp* op, ZilchShaderToSpirVContext* context);
  void WriteIROpArguments(ZilchShaderIROp* op, ZilchShaderToSpirVContext* context);
  void WriteIRArguments(Array<IZilchShaderIR*>& mArguments, ZilchShaderToSpirVContext* context);
  void WriteIRId(IZilchShaderIR* ir, ZilchShaderToSpirVContext* context);
  void WriteImport(ZilchShaderExtensionImport* importLibrary, ZilchShaderToSpirVContext* context);

  Array<IZilchShaderIR*> mOwnedInstructions;
  Array<EntryPointInfo*> mOwnedEntryPoints;
  LateBoundFunctionMap mExtraLateBoundFunctions;
  ZilchShaderIRLibraryRef mLastLibrary;
};

} // namespace Zero
