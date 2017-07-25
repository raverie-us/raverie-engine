///////////////////////////////////////////////////////////////////////////////
///
/// \file FilePath.hpp
/// Declaration of the FilePath class.
///
/// Authors: Joshua Davis
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "String/FixedString.hpp"
#include "String/String.hpp"

namespace Zero
{

/// Represents info about a file path. This can be converted to the full
/// path by using FilePath.CombineWithExtension(Folder, FileName, Extension).
struct ZeroShared FilePathInfo
{
  /// The full folder path of the file without the file name or extension.
  /// Note this doesn't include the trailing folder separator.
  /// FileName of "A/B/File.txt" will be "A/B/".
  StringRange Folder;
  /// Name of the file without the extension or path.
  /// FileName of "A/B/File.txt" will be "File".
  StringRange FileName;
  /// The extension of the file (which is everything that follows after
  /// the last '.') without the '.'. So "A/B/File.txt" will return "txt".
  StringRange Extension;
};

// Helper class to combine paths and extract info out of a path.
class ZeroShared FilePath
{
public:
  static String Combine(StringRange path0, StringRange path1);
  static String Combine(StringRange path0, StringRange path1, StringRange path2);
  static String Combine(StringRange path0, StringRange path1, StringRange path2, StringRange path3);
  static String Combine(StringRange path0, StringRange path1, StringRange path2, StringRange path3, StringRange path4);
  static String Combine(const StringRange** paths, uint count, StringRange extension);
  static String CombineWithExtension(StringRange path, StringRange fileName, StringRange ext);

  /// Remove all different slashes
  static String Normalize(StringRange path);
  /// Get a path info object
  static FilePathInfo GetPathInfo(StringRange path);
  /// Return extension of file path so "A/B/File.txt" is "txt"
  static StringRange GetExtension(StringRange path);
  /// Returns the file name with the extension so "A/B/File.txt" is "File.txt"
  static StringRange GetFileName(StringRange path);
  /// Returns just the file name so "A/B/File.txt" is "File"
  static StringRange GetFileNameWithoutExtension(StringRange path);
  /// Returns the name of the directory "A/B/File.txt" is "A/B"
  static StringRange GetDirectoryPath(StringRange path);
  /// Returns the name of the directory "A/B/File.txt" is "B"
  static StringRange GetDirectoryName(StringRange path);
};

}//namespace Zero
