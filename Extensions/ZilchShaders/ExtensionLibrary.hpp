///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ZilchSpirVFrontEnd;
class ZilchSpirVFrontEndContext;
class ZilchShaderExtensionImport;

typedef void(*ExtensionInstructionResolverFn)(ZilchSpirVFrontEnd* translator, Zilch::FunctionCallNode* functionCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderExtensionImport* importLibraryIR, ZilchSpirVFrontEndContext* context);

class SpirVExtensionLibrary;
class ExtensionInstruction
{
public:
  ExtensionInstruction();

  ExtensionInstructionResolverFn mResolverFn;

  Array<ZilchShaderIRType*> mSignature;
  SpirVExtensionLibrary* mLibrary;
};

class SpirVExtensionLibrary
{
public:
  ExtensionInstruction* CreateExtInst(Zilch::Function* zilchFn, ExtensionInstructionResolverFn resolverFn);

  String mName;
  HashMap<Zilch::Function*, ExtensionInstruction*> mExtensions;
  ZilchShaderIRLibrary* mOwningLibrary;
};

}//namespace Zero
