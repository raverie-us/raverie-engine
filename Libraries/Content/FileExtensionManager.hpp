///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// An individual type's available extensions.
class TypeExtensionEntry
{
public:
  /// The current default extension for this type with no '.' in front.
  String GetDefaultExtensionNoDot() const;
  /// The current default extension for this type with a '.' in front.
  String GetDefaultExtensionWithDot() const;
  /// Check if the given extension matches any available
  /// extensions. Assumes the given extension starts with '.'
  bool IsValidExtensionWithDot(StringParam extension) const;
  /// Check if the given extension matches any available extensions.
  /// Assumes the given extension does not start with '.'
  bool IsValidExtensionNoDot(StringParam extension) const;

  /// All available extensions for this type. The types are assumed to not start with a '.'
  /// and the first one in the list is assumed to be the default value.
  Array<String> mExtensions;
};

/// Helper singleton to manage default and legacy type extension for given "file types".
/// The TypeExtensionEntries are not safe to store as they're pointers into a container. Maybe fix later?
class FileExtensionManager : public LazySingleton<FileExtensionManager, EventObject>
{
public:
  FileExtensionManager();

  static TypeExtensionEntry* GetZilchScriptTypeEntry();
  static TypeExtensionEntry* GetZilchFragmentTypeEntry();

  HashMap<String, TypeExtensionEntry> mTypeExtensionEntries;
};

}//namespace Zero
