// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Importer.hpp"
#include "FileSystem.hpp"
#include "FilePath.hpp"
#include "ToString.hpp"
#include "Support/Archive.hpp"
#include "Utilities.hpp"

namespace Zero
{

const uint Importer::cEmptyPackageSize = 8;

ImporterResult::Type Importer::CheckForImport()
{
  // For the sake of plugins, our exe MUST be named ZeroEditor.exe
  // We solve this by copying our own executable to the output directory first,
  // run the executable there, and terminate our own
  static const String ZeroEngineExecutable("WelderEditor.exe");

  // Get the package as an executable resource
  ByteBufferBlock buffer;
  if (!GetExecutableResource(gPackName, gPackType, buffer))
    return ImporterResult::NotEmbeded;

  // Get the size to see if it is the empty resource (in editor) or larger
  if (buffer.Size() <= cEmptyPackageSize)
    return ImporterResult::NotEmbeded;

  // Read the unique export name  from the data section
  Status status;
  u32 uniqueNameSize = 0;
  buffer.Read(status, (byte*)&uniqueNameSize, sizeof(uniqueNameSize));
  StringNode* node = String::AllocateNode(uniqueNameSize);
  buffer.Read(status, (byte*)node->Data, node->Size);
  String uniqueName(node);

  mOutputDirectory = FilePath::Combine(GetUserLocalDirectory(), uniqueName);

  String applicationPath = GetApplication();
  String applicationName = FilePath::GetFileName(applicationPath);
  String destinationApplication = FilePath::Combine(mOutputDirectory, ZeroEngineExecutable);
  if (applicationName != ZeroEngineExecutable && !FileExists(destinationApplication))
  {
    ZPrint("Copying exe to output directory\n");

    CreateDirectoryAndParents(mOutputDirectory);
    CopyFile(destinationApplication, applicationPath);
  }

  SetWorkingDirectory(mOutputDirectory);

  static const String ProjectFile("Project.zeroproj");
  String projectFile = FilePath::Combine(mOutputDirectory, ProjectFile);
  if (!FileExists(projectFile))
  {
    ZPrint("Extracting packaged exe resources\n");

    // Extract critical blocking resources (loading, etc)
    Archive engineArchive(ArchiveMode::Decompressing);
    engineArchive.ReadBuffer(ArchiveReadFlags::All, buffer);
    engineArchive.ExportToDirectory(ArchiveExportMode::OverwriteIfNewer, mOutputDirectory);

    // Extract the project package
    byte* data = buffer.GetCurrent();
    size_t size = buffer.Size() - buffer.Tell();
    ByteBufferBlock importBuffer(data, size, false);
    DoImport(buffer);
  }

  if (applicationName != ZeroEngineExecutable)
  {
    ZPrint("Launching %s\n", destinationApplication.c_str());
    Zero::Os::ShellOpenApplication(destinationApplication);
    return ImporterResult::ExecutedAnotherProcess;
  }

  return ImporterResult::Embeded;
}

OsInt Importer::DoImport(ByteBufferBlock& buffer)
{
  Archive projectArchive(ArchiveMode::Decompressing);

  // Read all the entries
  projectArchive.ReadBuffer(ArchiveReadFlags::Enum(ArchiveReadFlags::Entries | ArchiveReadFlags::Data), buffer);
  // Only decompress and overwrite if the files are newer
  // this allows packaged exes to 'install' by running once
  projectArchive.ExportToDirectory(ArchiveExportMode::OverwriteIfNewer, mOutputDirectory);
  return 0;
}

} // namespace Zero
