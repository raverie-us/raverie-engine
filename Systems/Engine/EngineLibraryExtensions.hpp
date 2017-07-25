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
class EngineLibraryExtensions
{
public:
  // Only called for native libraries.
  static void AddNativeExtensions(LibraryBuilder& builder);

  // Called before each library is compiled (only called for Resource Libraries)
  static void AddExtensionsPreCompilation(LibraryBuilder& builder, ResourceLibrary* resources);

  // Called after each library is called (only called for Resource Libraries)
  static void AddExtensionsPostCompilation(LibraryBuilder& builder);

  // Called after each type is parsed (only called for Resource Libraries)
  static void TypeParsedCallback(Zilch::ParseEvent* e, void* userData);

  // Used to search for where a Proxy came from. This is helpful when scripts failed to
  // compile on startup
  static void FindProxiedTypeOrigin(BoundType* proxiedType);

private:

  // Adds easier access to resources in the given resource library (e.g. Sprite.Fireball)
  static void AddResourceExtensions(LibraryBuilder& builder, ResourceLibrary* resources);
};

} // namespace Zero
