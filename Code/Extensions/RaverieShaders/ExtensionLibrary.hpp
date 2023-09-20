// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class RaverieSpirVFrontEnd;
class RaverieSpirVFrontEndContext;
class RaverieShaderExtensionImport;

typedef void (*SpirVExtensionInstructionResolverFn)(RaverieSpirVFrontEnd* translator,
                                                    Raverie::FunctionCallNode* functionCallNode,
                                                    Raverie::MemberAccessNode* memberAccessNode,
                                                    RaverieShaderExtensionImport* importLibraryIR,
                                                    RaverieSpirVFrontEndContext* context);

class SpirVExtensionLibrary;

/// An extension intruction for an extension library (e.g. glsl contains
/// Matrix.Determinant).
class SpirVExtensionInstruction
{
public:
  SpirVExtensionInstruction();

  /// A callback function to implement whatever the operation is.
  SpirVExtensionInstructionResolverFn mResolverFn;
  /// The library that owns this instruction (needed to generate the spir-v
  /// instruction call)
  SpirVExtensionLibrary* mLibrary;
};

/// Represents an extension library which contains a collection of
/// non-core spir-v instructions (e.g. the glsl math extension functions).
class SpirVExtensionLibrary
{
public:
  /// Creates an extension instruction that translates a given raverie function
  SpirVExtensionInstruction* CreateExtInst(Raverie::Function* raverieFn, SpirVExtensionInstructionResolverFn resolverFn);

  /// The name of the library (mostly for debug)
  String mName;
  /// All of the extension functions owned by this library,
  /// mapped by the raverie function that they translate.
  HashMap<Raverie::Function*, SpirVExtensionInstruction*> mExtensions;
  RaverieShaderIRLibrary* mOwningLibrary;
};

} // namespace Raverie
