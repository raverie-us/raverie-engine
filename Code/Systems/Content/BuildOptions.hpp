// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Options
typedef Array<ContentItem*> ContentItemArray;

// Options used to control content building
// Treat this as an immutable object once we pass it to the
// content items to be processed, it's used my multiple threads!
class BuildOptions
{
public:
  BuildOptions(ContentLibrary* library);

  // Any Content Item Failed?
  bool Failure = false;

  String OutputPath;
  String SourcePath;
  String ToolPath;
  // The error message if the build fails.
  String Message;
};

} // namespace Zero
