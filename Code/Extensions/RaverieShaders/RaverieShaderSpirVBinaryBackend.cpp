// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "extensions.h"
#include "enum_string_mapping.h"

namespace Raverie
{

RaverieShaderToSpirVContext::RaverieShaderToSpirVContext()
{
  // Id 0 is not valid in SpirV so start with 1
  mId = 1;
  mMain = nullptr;
  mStreamWriter = nullptr;
  mReflectionData = nullptr;
}

int RaverieShaderToSpirVContext::GetAndAdvanceId()
{
  return mId++;
}

void RaverieShaderToSpirVContext::GenerateId(IRaverieShaderIR* ir)
{
  if (!mGeneratedId.ContainsKey(ir))
    mGeneratedId[ir] = GetAndAdvanceId();
}

int RaverieShaderToSpirVContext::FindId(IRaverieShaderIR* instruction, bool assertOnZero)
{
  int id = mGeneratedId.FindValue(instruction, 0);
  ErrorIf(id == 0 && assertOnZero, "Invalid instruction");
  return id;
}

RaverieShaderSpirVBinaryBackend::~RaverieShaderSpirVBinaryBackend()
{
  Clear();
}

void RaverieShaderSpirVBinaryBackend::TranslateType(RaverieShaderIRType* type, ShaderStreamWriter& writer)
{
  ShaderStageInterfaceReflection reflectionData;
  TranslateType(type, writer, reflectionData);
}

void RaverieShaderSpirVBinaryBackend::TranslateType(RaverieShaderIRType* type,
                                                  ShaderStreamWriter& writer,
                                                  ShaderStageInterfaceReflection& reflectionData)
{
  if (type->mMeta == nullptr)
    return;

  Clear();
  mLastLibrary = type->mShaderLibrary;

  // Set the name of the type that we're translating
  reflectionData.mShaderTypeName = type->mMeta->mRaverieName;

  // Setup the context
  RaverieShaderToSpirVContext context;
  context.mStreamWriter = &writer;
  context.mReflectionData = &reflectionData;

  TypeDependencyCollector collector(type->mShaderLibrary);
  // Generate a dummy main if none exists for unit testing purposes (remove
  // later?)
  GenerateDummyMain(type, type->mShaderLibrary, collector, &context);
  collector.Collect(type);

  // Walk the global initializers and generate and apply late bound functions
  GenerateGlobalsInitializerFunction(collector, &context);

  // @JoshD: Late bound functions 'potentially' require each entry point to be
  // independently generated
  for (size_t i = 0; i < context.mEntryPoints.Size(); ++i)
  {
    EntryPointInfo* entryPoint = context.mEntryPoints[i];
    reflectionData = entryPoint->mStageReflectionData;
    RegisterLateBoundFunctions(entryPoint->mLateBoundFunctions, collector, &context);
  }
  RegisterLateBoundFunctions(mExtraLateBoundFunctions, collector, &context);

  // Now that we've collected all entry points and
  // referenced data we can emit the spirv binary.
  EmitSpirvBinary(collector, &context);
}

void RaverieShaderSpirVBinaryBackend::TranslateLibrary(RaverieShaderIRLibrary* library,
                                                     ShaderStreamWriter& writer,
                                                     ShaderStageInterfaceReflection& reflectionData)
{
  Clear();
  mLastLibrary = library;

  // Setup the context
  RaverieShaderToSpirVContext context;
  context.mStreamWriter = &writer;
  context.mReflectionData = &reflectionData;

  TypeDependencyCollector collector(library);

  for (size_t i = 0; i < library->mOwnedTypes.Size(); ++i)
  {
    RaverieShaderIRType* type = library->mOwnedTypes[i];
    if (type->mEntryPoint != nullptr)
    {
      collector.Collect(type);
      context.mEntryPoints.PushBack(type->mEntryPoint);
    }
  }

  // Walk the global initializers and generate and apply late bound functions
  GenerateGlobalsInitializerFunction(collector, &context);

  // @JoshD: Late bound functions 'potentially' require each entry point to be
  // independently generated
  for (size_t i = 0; i < context.mEntryPoints.Size(); ++i)
  {
    EntryPointInfo* entryPoint = context.mEntryPoints[i];
    reflectionData = entryPoint->mStageReflectionData;
    RegisterLateBoundFunctions(entryPoint->mLateBoundFunctions, collector, &context);
  }
  RegisterLateBoundFunctions(mExtraLateBoundFunctions, collector, &context);

  // Now that we've collected all entry points and
  // referenced data we can emit the spirv binary.
  EmitSpirvBinary(collector, &context);
}

void RaverieShaderSpirVBinaryBackend::ValidateIdMap(RaverieShaderToSpirVContext* context)
{
  for (auto range = context->mGeneratedId.All(); !range.Empty(); range.PopFront())
  {
    ErrorIf(range.Front().second == 0, "Invalid Id?");
  }
}

void RaverieShaderSpirVBinaryBackend::EmitSpirvBinary(TypeDependencyCollector& collector,
                                                    RaverieShaderToSpirVContext* context)
{
  // Shouldn't need to go over these since the copy inputs/outputs functions
  // should contain these, but leave these in in case generation changes
  for (size_t i = 0; i < context->mEntryPoints.Size(); ++i)
  {
    EntryPointInfo* entryPoint = context->mEntryPoints[i];
    collector.Collect(&entryPoint->mVariables);
    // Make sure that every variable was added as a global
    for (size_t i = 0; i < entryPoint->mVariables.mLines.Size(); ++i)
    {
      IRaverieShaderIR* varIR = entryPoint->mVariables.mLines[i];
      RaverieShaderIROp* varOp = varIR->As<RaverieShaderIROp>();
      ErrorIf(!collector.mReferencedGlobals.ContainsKey(varOp), "Entry point variable wasn't added to globals");
    }
  }

  // Collect any capability requirements from decorations
  AddDecorationCapabilities(collector, context);

  // Now we have everything we need to reference so generate ids for everything

  GenerateListIds(collector.mReferencedImports, context);
  GenerateListIds(collector.mReferencedTypes, context);
  GenerateListIds(collector.mReferencedConstants, context);
  GenerateListIds(collector.mReferencedGlobals, context);
  GenerateFunctionIds(collector.mReferencedFunctions, context);

  // Write out the boiler-plate header
  WriteHeader(context, collector);
  // Write all debug information (names, source code, etc...)
  WriteDebug(collector.mReferencedTypes, context);
  WriteDebug(collector.mReferencedGlobals, context);
  WriteDebug(collector.mReferencedConstants, context);
  WriteDebug(collector.mReferencedFunctions, context);
  // Decorations (declaring inputs, uniforms, etc...)
  WriteDecorations(context);
  WriteSpecializationConstantBindingDecorations(collector, context);

  // Write out types, globals, and constants in one "block" based upon the order
  // they were found.
  WriteTypesGlobalsAndConstants(collector.mTypesConstantsAndGlobals, context);
  WriteFunctions(collector.mReferencedFunctions, context);
}

void RaverieShaderSpirVBinaryBackend::GenerateDummyMain(RaverieShaderIRType* type,
                                                      RaverieShaderIRLibrary* library,
                                                      TypeDependencyCollector& collector,
                                                      RaverieShaderToSpirVContext* context)
{
  if (type->mEntryPoint != nullptr)
  {
    context->mEntryPoints.PushBack(type->mEntryPoint);
    return;
  }

  EntryPointInfo* entryPointInfo = new EntryPointInfo();
  context->mEntryPoints.PushBack(entryPointInfo);

  RaverieShaderIRFunction* main = new RaverieShaderIRFunction();
  main->mDebugResultName = "auto_main";

  BasicBlock* block = new BasicBlock();
  main->mBlocks.PushBack(block);

  RaverieShaderIROp* op = new RaverieShaderIROp(OpType::OpReturn);
  op->mIRType = RaverieShaderIRBaseType::Op;
  block->mLines.PushBack(op);
  block->mTerminatorOp = op;

  RaverieShaderIRType* functionType = library->FindType("() : Void");
  if (functionType == nullptr)
  {
    functionType = new RaverieShaderIRType();

    RaverieShaderIRType* voidType = library->FindType("Void");
    functionType->mParameters.PushBack(voidType);
    functionType->mBaseType = ShaderIRTypeBaseType::Function;
    mOwnedInstructions.PushBack(functionType);
  }
  main->mFunctionType = functionType;
  collector.Collect(functionType);

  collector.mReferencedFunctions.InsertOrError(main);
  entryPointInfo->mEntryPointFn = main;

  // Force add the execution mode for the dummy entry point
  RaverieShaderIROp* executionModeOp = new RaverieShaderIROp(OpType::OpExecutionMode);
  RaverieShaderIRConstantLiteral* executionModeLiteral =
      new RaverieShaderIRConstantLiteral((int)spv::ExecutionModeOriginUpperLeft);
  executionModeOp->mResultType = nullptr;
  executionModeOp->mArguments.PushBack(main);
  executionModeOp->mArguments.PushBack(executionModeLiteral);
  entryPointInfo->mExecutionModes.AddOp(executionModeOp);
  // We own the literal instruction
  mOwnedInstructions.PushBack(executionModeLiteral);

  // Mark geometry shaders as such
  if (type->mMeta->mFragmentType == FragmentType::Geometry)
    entryPointInfo->mCapabilities.PushBack(spv::CapabilityGeometry);

  mOwnedInstructions.PushBack(main);
  mOwnedEntryPoints.PushBack(entryPointInfo);
}

void RaverieShaderSpirVBinaryBackend::GenerateGlobalsInitializerFunction(TypeDependencyCollector& collector,
                                                                       RaverieShaderToSpirVContext* context)
{
  for (size_t i = 0; i < context->mEntryPoints.Size(); ++i)
  {
    EntryPointInfo* entryPoint = context->mEntryPoints[i];
    // Check if this entry point has a initialization function and if the
    // collector found any global variables to initialize
    if (entryPoint->mGlobalsInitializerFunction == nullptr)
      continue;

    RaverieShaderIRFunction* originalFn = entryPoint->mGlobalsInitializerFunction;

    // Create a new late-bound function to replace the given globals
    // initializer. We do this just in-case this function is called more than
    // once which would append extra data each time to the function.
    RaverieShaderIRFunction* lateBoundFn = new RaverieShaderIRFunction();
    lateBoundFn->mDebugResultName = lateBoundFn->mName = originalFn->mName;
    lateBoundFn->mFunctionType = originalFn->mFunctionType;
    mOwnedInstructions.PushBack(lateBoundFn);
    mExtraLateBoundFunctions[originalFn] = lateBoundFn;

    // Make the starting block
    BasicBlock* block = new BasicBlock();
    lateBoundFn->mBlocks.PushBack(block);

    // Call every global variable's initializer function (if it exists)
    for (size_t j = 0; j < collector.mGlobalInitializers.Size(); ++j)
    {
      RaverieShaderIRFunction* initializerFn = collector.mGlobalInitializers[j];
      if (initializerFn == nullptr)
        continue;

      // Write out a function call to the variable's initialization function
      RaverieShaderIROp* op = new RaverieShaderIROp(OpType::OpFunctionCall);
      op->mResultType = originalFn->GetReturnType();
      op->mArguments.PushBack(initializerFn);
      block->AddOp(op);
    }
    // Finally, add the block terminator
    RaverieShaderIROp* returnOp = new RaverieShaderIROp(OpType::OpReturn);
    returnOp->mResultType = nullptr;
    block->AddOp(returnOp);
  }
}

void RaverieShaderSpirVBinaryBackend::RegisterLateBoundFunctions(LateBoundFunctionMap& lateBoundFunctionMap,
                                                               TypeDependencyCollector& collector,
                                                               RaverieShaderToSpirVContext* context)
{
  AutoDeclare(range, lateBoundFunctionMap.All());
  for (; !range.Empty(); range.PopFront())
  {
    RaverieShaderIRFunction* functionToReplace = range.Front().first;
    RaverieShaderIRFunction* replacingFunction = range.Front().second;

    // If we have a function to replace
    if (collector.mReferencedFunctions.ContainsKey(functionToReplace))
    {
      // Remove the old function
      collector.mReferencedFunctions.Erase(functionToReplace);
      // Walk the late-bound function to get all
      // variables/types/functions/etc...
      collector.Collect(replacingFunction);

      // Generate the id of the function that we're replacing and then give the
      // new function the same id. This will make them use the same id which
      // will be given the contents of the late bound function.
      context->GenerateId(functionToReplace);
      int id = context->mGeneratedId[functionToReplace];
      context->mGeneratedId[replacingFunction] = id;
    }
  }
}

void RaverieShaderSpirVBinaryBackend::Clear()
{
  DeleteObjectsIn(mOwnedEntryPoints);
  DeleteObjectsIn(mOwnedInstructions);
  mOwnedEntryPoints.Clear();
  mOwnedInstructions.Clear();
  mExtraLateBoundFunctions.Clear();
}

void RaverieShaderSpirVBinaryBackend::AddDecorationCapabilities(TypeDependencyCollector& collector,
                                                              RaverieShaderToSpirVContext* context)
{
  for (size_t i = 0; i < context->mEntryPoints.Size(); ++i)
  {
    EntryPointInfo* entryPoint = context->mEntryPoints[i];
    AddDecorationCapabilities(entryPoint, collector, context);
  }
}

void RaverieShaderSpirVBinaryBackend::AddDecorationCapabilities(EntryPointInfo* entryPoint,
                                                              TypeDependencyCollector& collector,
                                                              RaverieShaderToSpirVContext* context)
{
  for (size_t j = 0; j < entryPoint->mDecorations.mLines.Size(); ++j)
  {
    IRaverieShaderIR* decorationLine = entryPoint->mDecorations.mLines[j];
    RaverieShaderIROp* op = decorationLine->As<RaverieShaderIROp>();
    if (op->mOpType == OpType::OpDecorate)
      AddDecorationCapabilities(op, collector, context);
    else if (op->mOpType == OpType::OpMemberDecorate)
      AddMemberDecorationCapabilities(op, collector, context);
  }
}

void RaverieShaderSpirVBinaryBackend::AddDecorationCapabilities(RaverieShaderIROp* decorationOp,
                                                              TypeDependencyCollector& collector,
                                                              RaverieShaderToSpirVContext* context)
{
  RaverieShaderIRConstantLiteral* literal = decorationOp->mArguments[1]->As<RaverieShaderIRConstantLiteral>();
  int decorationType = literal->mValue.Get<int>();
  if (decorationType == spv::DecorationBuiltIn)
  {
    RaverieShaderIRConstantLiteral* builtInIdLiteral = decorationOp->mArguments[2]->As<RaverieShaderIRConstantLiteral>();
    int builtInId = builtInIdLiteral->mValue.Get<int>();

    if (builtInId == spv::BuiltInClipDistance)
      collector.mCapabilities.InsertOrIgnore(spv::CapabilityClipDistance);
    else if (builtInId == spv::BuiltInCullDistance)
      collector.mCapabilities.InsertOrIgnore(spv::CapabilityCullDistance);
    else if (builtInId == spv::BuiltInPrimitiveId)
      collector.mCapabilities.InsertOrIgnore(spv::CapabilityGeometry);
    else if (builtInId == spv::BuiltInInvocationId)
      collector.mCapabilities.InsertOrIgnore(spv::CapabilityGeometry);
  }
}

void RaverieShaderSpirVBinaryBackend::AddMemberDecorationCapabilities(RaverieShaderIROp* memberDecorationOp,
                                                                    TypeDependencyCollector& collector,
                                                                    RaverieShaderToSpirVContext* context)
{
  // @JoshD: Figure out what to do here later
}

template <typename T>
void RaverieShaderSpirVBinaryBackend::GenerateListIds(OrderedHashSet<T>& input, RaverieShaderToSpirVContext* context)
{
  AutoDeclare(range, input.All());
  for (; !range.Empty(); range.PopFront())
  {
    T& item = range.Front();
    context->GenerateId(item);
  }
}

void RaverieShaderSpirVBinaryBackend::GenerateFunctionIds(FunctionList& functions, RaverieShaderToSpirVContext* context)
{
  AutoDeclare(range, functions.All());
  for (; !range.Empty(); range.PopFront())
  {
    RaverieShaderIRFunction* function = range.Front();
    GenerateFunctionBlockIds(function, context);
  }
}

void RaverieShaderSpirVBinaryBackend::GenerateFunctionBlockIds(RaverieShaderIRFunction* function,
                                                             RaverieShaderToSpirVContext* context)
{
  context->GenerateId(function);

  GenerateBlockLineIds(&function->mParameterBlock, context);

  for (size_t bI = 0; bI < function->mBlocks.Size(); ++bI)
  {
    BasicBlock* block = function->mBlocks[bI];
    context->GenerateId(block);

    GenerateBlockLineIds(block, context);
  }
}

void RaverieShaderSpirVBinaryBackend::GenerateBlockLineIds(BasicBlock* block, RaverieShaderToSpirVContext* context)
{
  for (size_t i = 0; i < block->mLocalVariables.Size(); ++i)
  {
    IRaverieShaderIR* ir = block->mLocalVariables[i];
    context->GenerateId(ir);
  }

  for (size_t i = 0; i < block->mLines.Size(); ++i)
  {
    IRaverieShaderIR* ir = block->mLines[i];
    // Ignore if we've already visited (could be a global)
    if (context->mGeneratedId.ContainsKey(ir))
      continue;

    if (ir->mIRType == RaverieShaderIRBaseType::Op)
    {
      RaverieShaderIROp* op = (RaverieShaderIROp*)ir;
      if (op->mOpType == OpType::OpUndef)
        continue;

      bool hasResult = !op->IsTerminator() && op->mOpType != OpType::OpStore;
      if (hasResult)
      {
        context->GenerateId(op);
      }
    }
  }
}

void RaverieShaderSpirVBinaryBackend::WriteHeader(RaverieShaderToSpirVContext* context,
                                                TypeDependencyCollector& typeCollector)
{
  ShaderStreamWriter& streamWriter = *context->mStreamWriter;

  streamWriter.Write(spv::MagicNumber);
  // Major
  streamWriter.Write(0, 1, 4, 0);
  // Generator id
  streamWriter.Write(0);
  // Bound
  streamWriter.Write(context->mId);
  // Schema
  streamWriter.Write(0);

  // Capabilities
  // Add all entry point capabilities to our capabilities map (so we only
  // declare each once)
  for (size_t i = 0; i < context->mEntryPoints.Size(); ++i)
  {
    EntryPointInfo* entryPoint = context->mEntryPoints[i];
    for (size_t j = 0; j < entryPoint->mCapabilities.Size(); ++j)
      typeCollector.mCapabilities.InsertOrIgnore(entryPoint->mCapabilities[j]);
  }
  // Write all capabilities
  AutoDeclare(capabilitiesRange, typeCollector.mCapabilities.All());
  for (; !capabilitiesRange.Empty(); capabilitiesRange.PopFront())
  {
    streamWriter.WriteInstruction(2, OpType::OpCapability, capabilitiesRange.Front());
  }

  // Write Extensions
  AutoDeclare(extensionsRange, typeCollector.mRequiredExtensions.All());
  for (; !extensionsRange.Empty(); extensionsRange.PopFront())
  {
    // Convert the extension instruction id to a string (OpExtension requires
    // the extension string name instead of id for some reason)
    spvtools::Extension extensionId = (spvtools::Extension)extensionsRange.Front();
    String extString = spvtools::ExtensionToString(extensionId);

    // Compute the word count required for the op
    uint16 byteCount = (uint16)streamWriter.GetPaddedByteCount(extString);
    uint16 wordCount = byteCount / 4;

    streamWriter.Write(1 + wordCount, OpType::OpExtension);
    streamWriter.Write(extString);
  }

  // Imports
  AutoDeclare(importRange, typeCollector.mReferencedImports.All());
  for (; !importRange.Empty(); importRange.PopFront())
  {
    RaverieShaderExtensionImport* importLibrary = importRange.Front();
    WriteImport(importLibrary, context);
  }

  // Memory Model
  streamWriter.WriteInstruction(3, OpType::OpMemoryModel, spv::AddressingModelLogical, spv::MemoryModelGLSL450);

  // EntryPoints
  for (size_t i = 0; i < context->mEntryPoints.Size(); ++i)
  {
    EntryPointInfo* entryPoint = context->mEntryPoints[i];
    RaverieShaderIRFunction* entryPointFn = entryPoint->mEntryPointFn;
    String entryPointName = entryPointFn->mDebugResultName;
    int entryPointId = context->FindId(entryPointFn);

    size_t byteCount = streamWriter.GetPaddedByteCount(entryPointName);
    size_t wordCount = byteCount / 4;
    int16 totalSize = 3;
    totalSize += (int16)wordCount;
    totalSize += (int16)entryPoint->mInterface.Size();

    int executionModel = spv::ExecutionModelFragment;
    if (entryPoint->mFragmentType == FragmentType::Pixel)
      executionModel = spv::ExecutionModelFragment;
    else if (entryPoint->mFragmentType == FragmentType::Vertex)
      executionModel = spv::ExecutionModelVertex;
    else if (entryPoint->mFragmentType == FragmentType::Geometry)
      executionModel = spv::ExecutionModelGeometry;
    else if (entryPoint->mFragmentType == FragmentType::Compute)
      executionModel = spv::ExecutionModelGLCompute;

    streamWriter.WriteInstruction(totalSize, OpType::OpEntryPoint, executionModel, entryPointId);
    streamWriter.Write(entryPointName);
    for (size_t i = 0; i < entryPoint->mInterface.Size(); ++i)
    {
      RaverieShaderIROp* interfaceVar = entryPoint->mInterface[i];
      streamWriter.Write(context->FindId(interfaceVar));
    }
  }

  // ExecutionMode (per entry point)
  // Required to specify LowerLeft or UpperLeft if a pixel per the SpirV
  // validation rules
  for (size_t i = 0; i < context->mEntryPoints.Size(); ++i)
  {
    EntryPointInfo* entryPoint = context->mEntryPoints[i];
    RaverieShaderIRFunction* entryPointFn = entryPoint->mEntryPointFn;
    // Write out any extra execution mode instructions
    WriteBlockInstructions(&entryPoint->mExecutionModes, entryPoint->mExecutionModes.mLines, context);
  }

  // Source
  streamWriter.WriteInstruction(3, OpType::OpSource, 0, 100); // Source language unknown, Version 100
}

void RaverieShaderSpirVBinaryBackend::WriteDebug(TypeList& types, RaverieShaderToSpirVContext* context)
{
  AutoDeclare(range, types.All());
  for (; !range.Empty(); range.PopFront())
  {
    RaverieShaderIRType* type = range.Front();
    WriteDebug(type, context);
  }
}

void RaverieShaderSpirVBinaryBackend::WriteDebug(RaverieShaderIRType* type, RaverieShaderToSpirVContext* context)
{
  WriteDebugName(type, type->mDebugResultName, context);

  // Get indices to names for members
  HashMap<int, String> memberIndicesToNames;
  AutoDeclare(memberNameRange, type->mMemberNamesToIndex.All());
  for (; !memberNameRange.Empty(); memberNameRange.PopFront())
  {
    AutoDeclare(pair, memberNameRange.Front());
    memberIndicesToNames[pair.second] = pair.first;
  }

  // Walk all the member names in order
  for (int i = 0; i < (int)memberIndicesToNames.Size(); ++i)
  {
    String name = memberIndicesToNames.FindValue(i, String());

    ShaderStreamWriter& streamWriter = *context->mStreamWriter;
    size_t byteCount = streamWriter.GetPaddedByteCount(name);
    size_t wordCount = byteCount / 4;

    int typeId = context->FindId(type);
    streamWriter.WriteInstruction(3 + (int16)wordCount, OpType::OpMemberName);
    streamWriter.Write(typeId);
    streamWriter.Write(i);
    streamWriter.Write(name);
  }
}

void RaverieShaderSpirVBinaryBackend::WriteDebug(FunctionList& functions, RaverieShaderToSpirVContext* context)
{
  AutoDeclare(range, functions.All());
  for (; !range.Empty(); range.PopFront())
  {
    RaverieShaderIRFunction* function = range.Front();
    WriteDebug(function, context);
  }
}

void RaverieShaderSpirVBinaryBackend::WriteDebug(RaverieShaderIRFunction* function, RaverieShaderToSpirVContext* context)
{
  WriteDebugName(function, function->mDebugResultName, context);

  WriteDebug(&function->mParameterBlock, context);
  for (size_t i = 0; i < function->mBlocks.Size(); ++i)
  {
    BasicBlock* block = function->mBlocks[i];
    WriteDebug(block, context);
  }
}

void RaverieShaderSpirVBinaryBackend::WriteDebug(BasicBlock* block, RaverieShaderToSpirVContext* context)
{
  WriteDebugName(block, block->mDebugResultName, context);
  for (size_t i = 0; i < block->mLocalVariables.Size(); ++i)
  {
    IRaverieShaderIR* ir = block->mLocalVariables[i];
    WriteDebugName(ir, ir->mDebugResultName, context);
  }

  for (size_t i = 0; i < block->mLines.Size(); ++i)
  {
    IRaverieShaderIR* ir = block->mLines[i];
    WriteDebugName(ir, ir->mDebugResultName, context);
  }
}

void RaverieShaderSpirVBinaryBackend::WriteDebug(OpList& ops, RaverieShaderToSpirVContext* context)
{
  AutoDeclare(opRange, ops.All());
  for (; !opRange.Empty(); opRange.PopFront())
  {
    IRaverieShaderIR* ir = opRange.Front();
    WriteDebugName(ir, ir->mDebugResultName, context);
  }
}

void RaverieShaderSpirVBinaryBackend::WriteDebugName(IRaverieShaderIR* resultIR,
                                                   StringParam debugName,
                                                   RaverieShaderToSpirVContext* context)
{
  if (debugName.Empty())
    return;

  ShaderStreamWriter& streamWriter = *context->mStreamWriter;
  size_t byteCount = streamWriter.GetPaddedByteCount(debugName);
  size_t wordCount = byteCount / 4;

  int resultId = context->FindId(resultIR);
  ErrorIf(resultId == 0, "");
  streamWriter.WriteInstruction(2 + (int16)wordCount, OpType::OpName);
  streamWriter.Write(resultId);
  streamWriter.Write(debugName);
}

void RaverieShaderSpirVBinaryBackend::WriteDecorations(RaverieShaderToSpirVContext* context)
{
  // Write out all of the annotation instructions in each entry point
  for (size_t i = 0; i < context->mEntryPoints.Size(); ++i)
  {
    EntryPointInfo* entryPoint = context->mEntryPoints[i];
    WriteBlockInstructions(&entryPoint->mDecorations, entryPoint->mDecorations.mLines, context);
  }
}
void RaverieShaderSpirVBinaryBackend::WriteSpecializationConstantBindingDecorations(
    TypeDependencyCollector& typeCollector, RaverieShaderToSpirVContext* context)
{
  int specId = 1;
  ShaderStageInterfaceReflection& reflectionData = *context->mReflectionData;
  // Find all specialization constants so we can assign ids
  for (auto range = typeCollector.mReferencedConstants.All(); !range.Empty(); range.PopFront())
  {
    RaverieShaderIROp* op = range.Front();
    // If this is a spec constant (a scalar) then assign an id.
    // Also add reflection data so the constant's id can be looked up by name.
    if (op->mOpType == OpType::OpSpecConstant)
    {
      int opId = context->FindId(op);
      context->mStreamWriter->WriteInstruction(4, OpType::OpDecorate, opId, spv::DecorationSpecId, specId);

      reflectionData.mSpecializationConstants[op->mDebugResultName] = specId;
      ++specId;
    }
    // Composite specialization constants aren't actually assigned a decoration
    // binding id in spir-v. Instead, each scalar leaf constituent is given an
    // id. We guarantee that all constituents of a composite are assigned ids in
    // order, so instead of storing the id of every constituent we can find the
    // id of the first constituent and then the next n contiguous ids (based
    // upon the member count) all belong to this composite.
    else if (op->mOpType == OpType::OpSpecConstantComposite)
    {
      RaverieShaderIROp* leafConstituent = FindSpecialiationConstantCompositeId(op);
      int leafId = reflectionData.mSpecializationConstants[leafConstituent->mDebugResultName];
      reflectionData.mSpecializationConstants[op->mDebugResultName] = leafId;
    }
  }
}

RaverieShaderIROp* RaverieShaderSpirVBinaryBackend::FindSpecialiationConstantCompositeId(RaverieShaderIROp* op)
{
  // If we reached an OpSpecConstant then we found the leaf constituent and can
  // terminate
  if (op->mOpType == OpType::OpSpecConstant)
    return op;

  // Recursively find the first constituent of this composite
  RaverieShaderIROp* firstConstituent = op->mArguments[0]->As<RaverieShaderIROp>();
  return FindSpecialiationConstantCompositeId(firstConstituent);
}

void RaverieShaderSpirVBinaryBackend::WriteTypesGlobalsAndConstants(IRList& typesGlobalsAndConstants,
                                                                  RaverieShaderToSpirVContext* context)
{
  // Now spirv requires we write all types then all constants then all functions
  AutoDeclare(range, typesGlobalsAndConstants.All());
  for (; !range.Empty(); range.PopFront())
  {
    IRaverieShaderIR* ir = range.Front();
    // Write types
    if (ir->mIRType == RaverieShaderIRBaseType::DataType)
      WriteType(ir->As<RaverieShaderIRType>(), context);
    // This should never happen (there's always an ir op that points at a
    // constant (in the constant pool)
    else if (ir->mIRType == RaverieShaderIRBaseType::ConstantLiteral)
      WriteConstant(ir->As<RaverieShaderIROp>(), context);
    // Otherwise write
    else if (ir->mIRType == RaverieShaderIRBaseType::Op)
    {
      // Check if this op points at a constant or a global (generic op)
      RaverieShaderIROp* op = ir->As<RaverieShaderIROp>();
      if (op->mOpType == OpType::OpConstant)
        WriteConstant(op, context);
      else if (op->mOpType == OpType::OpSpecConstant)
        WriteSpecConstant(op, context);
      else
        WriteGlobal(op, context);
    }
    else
    {
      Error("This shouldn't happen");
    }
  }
}

void RaverieShaderSpirVBinaryBackend::WriteType(RaverieShaderIRType* type, RaverieShaderToSpirVContext* context)
{
  ShaderStreamWriter& streamWriter = *context->mStreamWriter;

  if (type->mBaseType == ShaderIRTypeBaseType::Void)
    streamWriter.WriteInstruction(2, OpType::OpTypeVoid, context->FindId(type));
  else if (type->mBaseType == ShaderIRTypeBaseType::Bool)
    streamWriter.WriteInstruction(2, OpType::OpTypeBool, context->FindId(type));
  else if (type->mBaseType == ShaderIRTypeBaseType::Int)
    streamWriter.WriteInstruction(4, OpType::OpTypeInt, context->FindId(type), 32, 1);
  else if (type->mBaseType == ShaderIRTypeBaseType::Uint)
    streamWriter.WriteInstruction(4, OpType::OpTypeInt, context->FindId(type), 32, 0);
  else if (type->mBaseType == ShaderIRTypeBaseType::Float)
    streamWriter.WriteInstruction(3, OpType::OpTypeFloat, context->FindId(type), 32);
  else if (type->mBaseType == ShaderIRTypeBaseType::Vector)
  {
    RaverieShaderIRType* componentType = GetComponentType(type);
    int componentTypeId = context->FindId(componentType);
    streamWriter.WriteInstruction(4, OpType::OpTypeVector, context->FindId(type), componentTypeId, type->mComponents);
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::Matrix)
  {
    RaverieShaderIRType* componentType = GetComponentType(type);
    int componentTypeId = context->FindId(componentType);
    streamWriter.WriteInstruction(4, OpType::OpTypeMatrix, context->FindId(type), componentTypeId, type->mComponents);
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::FixedArray)
  {
    RaverieShaderIRType* componentType = type->mParameters[0]->As<RaverieShaderIRType>();
    int componentTypeId = context->FindId(componentType);
    int lengthId = context->FindId(type->mParameters[1]);
    streamWriter.WriteInstruction(4, OpType::OpTypeArray, context->FindId(type), componentTypeId, lengthId);
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::RuntimeArray)
  {
    RaverieShaderIRType* componentType = type->mParameters[0]->As<RaverieShaderIRType>();
    int componentTypeId = context->FindId(componentType);
    streamWriter.WriteInstruction(3, OpType::OpTypeRuntimeArray, context->FindId(type), componentTypeId);
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::Struct)
  {
    streamWriter.WriteInstruction(2 + (int16)type->mParameters.Size(), OpType::OpTypeStruct, context->FindId(type));

    for (size_t i = 0; i < type->mParameters.Size(); ++i)
    {
      int memberId = context->FindId(type->mParameters[i]);
      streamWriter.Write(memberId);
    }
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::Function)
  {
    int typeId = context->FindId(type);
    streamWriter.WriteInstruction(2 + (int16)type->mParameters.Size(), OpType::OpTypeFunction, typeId);
    for (size_t i = 0; i < type->mParameters.Size(); ++i)
    {
      int subId = context->FindId(type->mParameters[i]);
      streamWriter.Write(subId);
    }
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::Pointer)
  {
    int dereferenceTypeId = context->FindId(type->mDereferenceType);
    streamWriter.WriteInstruction(
        4, OpType::OpTypePointer, context->FindId(type), type->mStorageClass, dereferenceTypeId);
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::Image)
  {
    int16 size = 2 + (int16)type->mParameters.Size();
    streamWriter.WriteInstruction(size, OpType::OpTypeImage, context->FindId(type));
    WriteIRArguments(type->mParameters, context);
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::SampledImage)
  {
    int imageTypeId = context->FindId(type->mParameters[0]);
    streamWriter.WriteInstruction(3, OpType::OpTypeSampledImage, context->FindId(type), imageTypeId);
  }
  else if (type->mBaseType == ShaderIRTypeBaseType::Sampler)
  {
    streamWriter.WriteInstruction(2, OpType::OpTypeSampler, context->FindId(type));
  }
}

void RaverieShaderSpirVBinaryBackend::WriteConstant(RaverieShaderIROp* constantOp, RaverieShaderToSpirVContext* context)
{
  ShaderStreamWriter& streamWriter = *context->mStreamWriter;

  RaverieShaderIRConstantLiteral* argConstant = (RaverieShaderIRConstantLiteral*)constantOp->mArguments[0];
  if (constantOp->mResultType->mBaseType == ShaderIRTypeBaseType::Bool)
  {
    bool value = argConstant->mValue.Get<bool>();
    if (value)
      streamWriter.WriteInstruction(
          3, OpType::OpConstantTrue, context->FindId(constantOp->mResultType), context->FindId(constantOp));
    else
      streamWriter.WriteInstruction(
          3, OpType::OpConstantFalse, context->FindId(constantOp->mResultType), context->FindId(constantOp));
  }
  else if (constantOp->mResultType->mBaseType == ShaderIRTypeBaseType::Int)
  {
    int value = argConstant->mValue.Get<int>();
    int resultId = context->FindId(constantOp->mResultType);
    int constantId = context->FindId(constantOp);
    streamWriter.WriteInstruction(4, OpType::OpConstant, resultId, constantId, value);
  }
  else if (constantOp->mResultType->mBaseType == ShaderIRTypeBaseType::Float)
  {
    float value = argConstant->mValue.Get<float>();
    int resultId = context->FindId(constantOp->mResultType);
    int constantId = context->FindId(constantOp);
    streamWriter.WriteInstruction(4, OpType::OpConstant, resultId, constantId, *(int*)&value);
  }
  else if (constantOp->mResultType->mBaseType == ShaderIRTypeBaseType::Vector)
  {
    // The below code is likely how this op should be translated but this hasn't
    // been tested.
    Error("Not supported");

    // size_t componentCount = constantOp->mResultType->mComponents;
    // int* data = (int*)argConstant->mValue.GetData();
    //
    // int resultId = context->FindId(constantOp->mResultType);
    // int constantId = context->FindId(constantOp);
    // int16 size = 3 + (int16)componentCount;
    // streamWriter.WriteInstruction(size, OpType::OpConstantComposite,
    // resultId, constantId);
    //
    // for(size_t i = 0; i < componentCount; ++i)
    //  streamWriter.Write(*(data + i));
  }
  else
  {
    Error("Unknown constant type %d", constantOp->mResultType->mBaseType);
  }
}

void RaverieShaderSpirVBinaryBackend::WriteSpecConstant(RaverieShaderIROp* constantOp, RaverieShaderToSpirVContext* context)
{
  ShaderStreamWriter& streamWriter = *context->mStreamWriter;

  // Handle bools. (They're currently written out as OpConstant with true/false
  // for the value but they actually have to be written as different
  // instructions
  if (constantOp->mResultType->mBaseType == ShaderIRTypeBaseType::Bool)
  {
    RaverieShaderIRConstantLiteral* argConstant = (RaverieShaderIRConstantLiteral*)constantOp->mArguments[0];
    bool value = argConstant->mValue.Get<bool>();
    OpType opType = OpType::OpSpecConstantFalse;
    if (value)
      opType = OpType::OpSpecConstantTrue;

    int32 resultTypeId = context->FindId(constantOp->mResultType);
    int32 resultId = context->FindId(constantOp);
    streamWriter.WriteInstruction(3, opType, resultTypeId, resultId);
    return;
  }

  // Otherwise, write out the op generically
  WriteIROpGeneric(constantOp, context);
}

void RaverieShaderSpirVBinaryBackend::WriteGlobal(RaverieShaderIROp* globalVarOp, RaverieShaderToSpirVContext* context)
{
  WriteIROp(nullptr, globalVarOp, context);
}

void RaverieShaderSpirVBinaryBackend::WriteFunctions(FunctionList& functions, RaverieShaderToSpirVContext* context)
{
  AutoDeclare(range, functions.All());
  for (; !range.Empty(); range.PopFront())
  {
    RaverieShaderIRFunction* function = range.Front();
    WriteFunction(function, context);
  }
}

void RaverieShaderSpirVBinaryBackend::WriteFunction(RaverieShaderIRFunction* function, RaverieShaderToSpirVContext* context)
{
  ShaderStreamWriter& streamWriter = *context->mStreamWriter;

  RaverieShaderIRType* functionType = function->mFunctionType;

  // WriteFunction
  int functionId = context->FindId(function);
  int functionTypeId = context->FindId(function->mFunctionType);
  int returnTypeId = context->FindId(function->GetReturnType());
  streamWriter.WriteInstruction(5, OpType::OpFunction);
  streamWriter.Write(returnTypeId);
  streamWriter.Write(functionId);
  streamWriter.Write(spv::FunctionControlMaskNone);
  streamWriter.Write(functionTypeId);

  // Write function args
  for (size_t i = 0; i < function->mParameterBlock.mLines.Size(); ++i)
  {
    RaverieShaderIROp* paramOp = function->mParameterBlock.mLines[i]->As<RaverieShaderIROp>();
    WriteIROp(&function->mParameterBlock, paramOp, context);
  }

  // Write blocks
  for (size_t i = 0; i < function->mBlocks.Size(); ++i)
    WriteBlock(function->mBlocks[i], context);

  // End function
  streamWriter.WriteInstruction(1, OpType::OpFunctionEnd);
}

void RaverieShaderSpirVBinaryBackend::WriteBlock(BasicBlock* block, RaverieShaderToSpirVContext* context)
{
  ShaderStreamWriter& streamWriter = *context->mStreamWriter;

  // Write the label for the block
  streamWriter.WriteInstruction((int16)2, (int16)OpType::OpLabel, context->FindId(block));

  // All local variables must be declared first
  WriteBlockInstructions(block, block->mLocalVariables, context);
  // Then we can write all instructions
  WriteBlockInstructions(block, block->mLines, context);
}

void RaverieShaderSpirVBinaryBackend::WriteBlockInstructions(BasicBlock* block,
                                                           Array<IRaverieShaderIR*>& instructions,
                                                           RaverieShaderToSpirVContext* context)
{
  for (size_t i = 0; i < instructions.Size(); ++i)
  {
    IRaverieShaderIR* ir = instructions[i];

    RaverieShaderIROp* op = ir->As<RaverieShaderIROp>();
    WriteIROp(block, op, context);
  }
}

void RaverieShaderSpirVBinaryBackend::WriteIROp(BasicBlock* block,
                                              RaverieShaderIROp* op,
                                              RaverieShaderToSpirVContext* context)
{
  ShaderStreamWriter& streamWriter = *context->mStreamWriter;

  switch (op->mOpType)
  {
  case OpType::OpReturn:
  case OpType::OpKill:
  case OpType::OpUnreachable:
  case OpType::OpEmitVertex:
  {
    streamWriter.WriteInstruction(1, op->mOpType);
    break;
  }
  case OpType::OpReturnValue:
  {
    streamWriter.WriteInstruction(2, OpType::OpReturnValue);
    WriteIROpArguments(op, context);
    break;
  }
  case OpType::OpStore:
  {
    streamWriter.WriteInstruction(3, OpType::OpStore);
    WriteIROpArguments(op, context);
    break;
  }
  case OpType::OpDecorate:
  {
    int16 baseSize = 1;
    baseSize += (int16)op->mArguments.Size();
    streamWriter.WriteInstruction(baseSize, OpType::OpDecorate);
    WriteIROpArguments(op, context);
    break;
  }
  case OpType::OpCapability:
  case OpType::OpEndPrimitive:
  case OpType::OpExecutionMode:
  {
    WriteIROpGenericNoReturnType(op, context);
    break;
  }
  case OpType::OpMemberDecorate:
  {
    int16 baseSize = 1;
    // Now count the arguments to get the total instruction size
    int16 totalSize = baseSize + (int16)op->mArguments.Size();

    streamWriter.WriteInstruction(totalSize, (int)op->mOpType);
    WriteIROpArguments(op, context);
    break;
  }
  case OpType::OpBranchConditional:
  {
    if (block->mBlockType == BlockType::Selection)
    {
      int mergePoint = context->FindId(block->mMergePoint);
      streamWriter.WriteInstruction(3, OpType::OpSelectionMerge);
      streamWriter.Write(mergePoint);
      streamWriter.Write(spv::SelectionControlMaskNone);
    }

    streamWriter.WriteInstruction(4, OpType::OpBranchConditional);
    WriteIROpArguments(op, context);
    break;
  }
  case OpType::OpBranch:
  {
    if (block->mBlockType == BlockType::Loop)
    {
      int mergePoint = context->FindId(block->mMergePoint);
      int continuePoint = context->FindId(block->mContinuePoint);
      streamWriter.WriteInstruction(4, OpType::OpLoopMerge);
      streamWriter.Write(mergePoint);
      streamWriter.Write(continuePoint);
      streamWriter.Write(spv::LoopControlMaskNone);

      streamWriter.WriteInstruction(2, OpType::OpBranch);
      WriteIROpArguments(op, context);
    }
    else
    {
      streamWriter.WriteInstruction(2, OpType::OpBranch);
      WriteIROpArguments(op, context);
    }
    break;
  }
  // Skip constants, they aren't instructions
  case OpType::OpConstant:
  case OpType::OpSpecConstant:
    return;
  case OpType::OpCopyMemory:
  {
    // Now count the arguments to get the total instruction size
    int16 totalSize = 1 + (int16)op->mArguments.Size();
    streamWriter.WriteInstruction(totalSize, (int)op->mOpType);
    WriteIROpArguments(op, context);
    break;
  }
  case OpType::OpVectorShuffle:
  {
    WriteIROpGeneric(op, context);
    break;
  }
  case OpType::OpAccessChain:
  {
    WriteIROpGeneric(op, context);
    break;
  }
  // Arguments that don't have a return type so the size of the opcode is 1 +
  // the number of arguments.
  case OpType::OpImageWrite:
  {
    // Now count the arguments to get the total instruction size
    int16 totalSize = 1 + (int16)op->mArguments.Size();
    streamWriter.WriteInstruction(totalSize, (int)op->mOpType);
    WriteIROpArguments(op, context);
    break;
  }
  default:
  {
    WriteIROpGeneric(op, context);
    break;
  }
  }
}

void RaverieShaderSpirVBinaryBackend::WriteIROpGeneric(RaverieShaderIROp* op, RaverieShaderToSpirVContext* context)
{
  // Base size of an instruction is 3 (Size+Instruction, result type, id).
  int16 baseSize = 3;
  // Now count the arguments to get the total instruction size
  int16 totalSize = baseSize + (int16)op->mArguments.Size();

  int32 resultTypeId = context->FindId(op->mResultType);
  int32 resultId = context->FindId(op);

  ShaderStreamWriter& streamWriter = *context->mStreamWriter;
  streamWriter.WriteInstruction(totalSize, (int)op->mOpType);
  streamWriter.Write(resultTypeId);
  streamWriter.Write(resultId);
  WriteIROpArguments(op, context);
}

void RaverieShaderSpirVBinaryBackend::WriteIROpGenericNoReturnType(RaverieShaderIROp* op,
                                                                 RaverieShaderToSpirVContext* context)
{
  // Base size of an instruction is 1 (Size+Instruction).
  int16 baseSize = 1;
  // Now count the arguments to get the total instruction size
  int16 totalSize = baseSize + (int16)op->mArguments.Size();

  ShaderStreamWriter& streamWriter = *context->mStreamWriter;
  streamWriter.WriteInstruction(totalSize, (int)op->mOpType);
  WriteIROpArguments(op, context);
}

void RaverieShaderSpirVBinaryBackend::WriteIROpArguments(RaverieShaderIROp* op, RaverieShaderToSpirVContext* context)
{
  WriteIRArguments(op->mArguments, context);
}

void RaverieShaderSpirVBinaryBackend::WriteIRArguments(Array<IRaverieShaderIR*>& arguments,
                                                     RaverieShaderToSpirVContext* context)
{
  for (size_t i = 0; i < arguments.Size(); ++i)
  {
    IRaverieShaderIR* arg = arguments[i];
    WriteIRId(arg, context);
  }
}

void RaverieShaderSpirVBinaryBackend::WriteIRId(IRaverieShaderIR* ir, RaverieShaderToSpirVContext* context)
{
  int id;
  if (ir->mIRType == RaverieShaderIRBaseType::ConstantLiteral)
  {
    // We want the raw bytes of constant literals (limited to size of int for
    // now)
    RaverieShaderIRConstantLiteral* constantLiteral = ir->As<RaverieShaderIRConstantLiteral>();
    int* rawId = (int*)constantLiteral->mValue.GetData();
    id = *rawId;
  }
  else
    id = context->FindId(ir);
  ShaderStreamWriter& streamWriter = *context->mStreamWriter;
  streamWriter.Write(id);
}

void RaverieShaderSpirVBinaryBackend::WriteImport(RaverieShaderExtensionImport* importLibrary,
                                                RaverieShaderToSpirVContext* context)
{
  ShaderStreamWriter& streamWriter = *context->mStreamWriter;

  uint id = context->FindId(importLibrary);
  String name = importLibrary->mLibrary->mName;
  size_t byteCount = streamWriter.GetPaddedByteCount(name);
  size_t wordCount = byteCount / 4;
  u16 size = 2 + (u16)wordCount;
  streamWriter.WriteInstruction(size, OpType::OpExtInstImport);
  streamWriter.Write(id);
  streamWriter.Write(name);
}

} // namespace Raverie
