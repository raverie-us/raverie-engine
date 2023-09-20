// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

SpirVExtensionInstruction::SpirVExtensionInstruction()
{
  mResolverFn = nullptr;
}

SpirVExtensionInstruction* SpirVExtensionLibrary::CreateExtInst(Raverie::Function* raverieFn, SpirVExtensionInstructionResolverFn resolverFn)
{
  ErrorIf(raverieFn == nullptr, "Invalid raverie function");
  ErrorIf(mExtensions.ContainsKey(raverieFn), "Extension already exists");

  SpirVExtensionInstruction* instruction = new SpirVExtensionInstruction();
  instruction->mResolverFn = resolverFn;
  instruction->mLibrary = this;
  mExtensions[raverieFn] = instruction;
  // Store this extension instruction in the owning library
  mOwningLibrary->mExtensionInstructions.InsertOrError(raverieFn, instruction);
  return instruction;
}

} // namespace Raverie
