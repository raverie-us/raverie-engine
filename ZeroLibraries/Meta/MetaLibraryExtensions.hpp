////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------------- Zero Library Extensions
class MetaLibraryExtensions
{
public:
  // Only called for native libraries.
  static void AddNativeExtensions(LibraryBuilder& builder);

  // Called before each library is compiled (only called for Resource Libraries)
  static void AddExtensionsPreCompilation(LibraryBuilder& builder);

  // Called after each library is called (only called for Resource Libraries)
  static void AddExtensionsPostCompilation(LibraryBuilder& builder);

  // Called after each type is parsed (only called for Resource Libraries)
  static void TypeParsedCallback(Zilch::ParseEvent* e, void* userData);

private:

  // For types that have a meta composition, we want to add getters that return the component types
  // e.g. For Cogs, we want all Components to show up on the Cog type in script so that accessing
  // a Component looks like "this.Owner.RigidBody" in script.
  // Same applies for material blocks "this.Owner.Model.Material.TextureColor".
  static void ProcessComposition(LibraryBuilder& builder, BoundType* boundType);
  static void ProcessComponent(LibraryBuilder& builder, BoundType* boundType);
  static void AddCompositionExtension(LibraryBuilder& builder, BoundType* compositionType, 
                                      BoundType* componentType);
};

} // namespace Zero
