///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ExtensionInstruction::ExtensionInstruction()
{
  mResolverFn = nullptr;
}

ExtensionInstruction* SpirVExtensionLibrary::CreateExtInst(Zilch::Function* zilchFn, ExtensionInstructionResolverFn resolverFn)
{
  ErrorIf(zilchFn == nullptr, "Invalid zilch function");
  ErrorIf(mExtensions.ContainsKey(zilchFn), "Extension already exists");
  ExtensionInstruction* instruction = new ExtensionInstruction();
  instruction->mResolverFn = resolverFn;
  instruction->mLibrary = this;
  mExtensions[zilchFn] = instruction;
  mOwningLibrary->mExtensionInstructions.InsertOrError(zilchFn, instruction);
  return instruction;
}

}//namespace Zero
