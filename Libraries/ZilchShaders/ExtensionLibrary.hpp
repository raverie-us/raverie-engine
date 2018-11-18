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

typedef void(*SpirVExtensionInstructionResolverFn)(ZilchSpirVFrontEnd* translator, Zilch::FunctionCallNode* functionCallNode, Zilch::MemberAccessNode* memberAccessNode, ZilchShaderExtensionImport* importLibraryIR, ZilchSpirVFrontEndContext* context);

class SpirVExtensionLibrary;

//-------------------------------------------------------------------SpirVExtensionInstruction
/// An extension intruction for an extension library (e.g. glsl contains Matrix.Determinant).
class SpirVExtensionInstruction
{
public:
  SpirVExtensionInstruction();

  /// A callback function to implement whatever the operation is.
  SpirVExtensionInstructionResolverFn mResolverFn;
  /// The library that owns this instruction (needed to generate the spir-v instruction call)
  SpirVExtensionLibrary* mLibrary;
};

//-------------------------------------------------------------------SpirVExtensionLibrary
/// Represents an extension library which contains a collection of
/// non-core spir-v instructions (e.g. the glsl math extension functions).
class SpirVExtensionLibrary
{
public:
  /// Creates an extension instruction that translates a given zilch function
  SpirVExtensionInstruction* CreateExtInst(Zilch::Function* zilchFn, SpirVExtensionInstructionResolverFn resolverFn);

  /// The name of the library (mostly for debug)
  String mName;
  /// All of the extension functions owned by this library,
  /// mapped by the zilch function that they translate.
  HashMap<Zilch::Function*, SpirVExtensionInstruction*> mExtensions;
  ZilchShaderIRLibrary* mOwningLibrary;
};

}//namespace Zero
