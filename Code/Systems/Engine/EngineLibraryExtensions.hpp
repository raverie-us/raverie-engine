// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Raverie Library Extensions
class EngineLibraryExtensions
{
public:
  // Only called for native libraries.
  static void AddNativeExtensions(LibraryBuilder& builder);
  static void AddNativeExtensions(LibraryBuilder& builder, BoundTypeMap& boundTypes);

  // Called before each library is compiled (only called for Resource Libraries)
  static void AddExtensionsPreCompilation(LibraryBuilder& builder, ResourceLibrary* resources);

  // Called after each library is called (only called for Resource Libraries)
  static void AddExtensionsPostCompilation(LibraryBuilder& builder);

  // Called after each type is parsed (only called for Resource Libraries)
  static void TypeParsedCallback(Raverie::ParseEvent* e, void* userData);

  // Used to search for where a Proxy came from. This is helpful when scripts
  // failed to compile on startup
  static void FindProxiedTypeOrigin(BoundType* proxiedType);

private:
  // Adds easier access to resources in the given resource library (e.g.
  // Sprite.Fireball)
  static void AddResourceExtensions(LibraryBuilder& builder, ResourceLibrary* resources);
};

} // namespace Raverie
