// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void DirectBuilderComponent::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeName(mResourceId);
}

bool DirectBuilderComponent::NeedsBuilding(BuildOptions& options)
{
  String destFile = FilePath::Combine(options.OutputPath, GetOutputFile());
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  return CheckFileAndMeta(options, sourceFile, destFile);
}

void DirectBuilderComponent::Rename(StringParam newName)
{
  Name = newName;
}

String DirectBuilderComponent::GetOutputFile()
{
  return BuildString(Name, Extension);
}

bool DirectBuilderComponent::NeedsBuildingTool(BuildOptions& options, StringParam tool)
{
  return CheckToolFile(options, GetOutputFile(), tool);
}

void DirectBuilderComponent::BuildListing(ResourceListing& listing)
{
  String destFile = GetOutputFile();
  listing.PushBack(ResourceEntry(Order, LoaderType, Name, destFile, mResourceId, this->mOwner, this));
}

void DirectBuilderComponent::Generate(ContentInitializer& initializer)
{
  Name = initializer.Name;
  mResourceId = GenerateUniqueId64();
}

void DirectBuilderComponent::BuildContent(BuildOptions& buildOptions)
{
  // Default behavior for direct builder is to just copy the content file.

  String destFile = FilePath::Combine(buildOptions.OutputPath, GetOutputFile());
  String sourceFile = FilePath::Combine(buildOptions.SourcePath, mOwner->Filename);

  // Copy the file.
  bool success = CopyFile(destFile, sourceFile);
  if (!success)
  {
    // Try to create any missing parent directories of the destination path and
    // try again
    CreateDirectoryAndParents(FilePath::GetDirectoryPath(destFile));
    success = CopyFile(destFile, sourceFile);
  }

  if (!success)
  {
    buildOptions.Failure = true;
    buildOptions.Message = String::Format("Failed to copy file '%s' to '%s'", sourceFile.c_str(), destFile.c_str());

    // Log what part of the source path was missing (if any)
    String sourceFileMissingDir = FindFirstMissingDirectory(sourceFile);
    if (sourceFileMissingDir != sourceFile)
      buildOptions.Message = String::Format(
          "%s. The source path '%s' doesn't exist", buildOptions.Message.c_str(), sourceFileMissingDir.c_str());

    // Log what part of the destination path was missing (if any)
    String destFileMissingDir = FindFirstMissingDirectory(destFile);
    if (destFileMissingDir != destFile)
      buildOptions.Message = String::Format(
          "%s. The destination path '%s' doesn't exist", buildOptions.Message.c_str(), destFileMissingDir.c_str());

    return;
  }

  // Update the file time so that NeedsBuilding works.
  SetFileToCurrentTime(destFile);
}

} // namespace Zero
