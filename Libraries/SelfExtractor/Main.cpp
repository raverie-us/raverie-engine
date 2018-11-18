///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
// This is hard-coded for now, but we could technically read this from the data
// section if we want to be able to pack more than one type of executable.
static const cstr cExecutableName = "ZeroEditor";

// As a backup strategy in case extraction fails, we should always package this executable with a copy of itself
// stored in it's own resource section. This doubles the size of the code section of this executable, but it's
// already trimmed down and very small. We can extract the small version of ourselves and launch it with special
// command line parameters that point back at the original executable so that we may extract that.
static const cstr cResourceName = "SelfExtractor";
static const cstr cResourceType = "EXECUTABLE";

// We expect tot see this signature at the end of the executable file.
static const cstr cSignature = "ZExtract";
static const u64 cSignatureSize = 8;

// For packing icons and possibly using the resource section, we should read this:
// https://en.wikibooks.org/wiki/X86_Disassembly/Windows_Executable_Files#Resources

void FatalError(const char* message)
{
  Error(message);
  CrashHandler::FatalError(1);
}

void Extract(const Array<String>& arguments)
{
  // Use the file size and modify date to build a hash for where to extract.
  // This can only fail if we have two executables with the exact same name,
  // same file size, and the same modification date.
  String applicationName = FilePath::GetFileNameWithoutExtension(GetApplication());
  u64 hash = GetFileSize(GetApplication()) * 15461;
  hash ^= GetFileModifiedTime(GetApplication()) * 17;

  // Build the path that we're going to extract to.
  String extractName = BuildString(applicationName, ToString(hash));
  String extractDirectoryPath = FilePath::Combine(GetUserLocalDirectory(), "ZeroSelfExtractor", extractName);

  // Get the executable name (hard-coded for now, but we could technically read this from our data section).
  String extractExecutablePath = FilePath::CombineWithExtension(extractDirectoryPath, cExecutableName, cExecutableExtensionWithDot);

  // This will point at our own executable with the data section at the end.
  File file;

  // If we just have the single argument (or somehow none) then perform regular extraction.
  if (arguments.Size() <= 1)
  {
    // Check to see if we already have the executable extracted.
    // We're technically writing to a user specific hidden directory that should
    // persist (GetUserLocalDirectory) which means that after extraction, all the
    // files should remain forever unless the user explicitly deletes them.
    if (FileExists(extractExecutablePath))
    {
      // Run the extracted executable, we're done!
      Zero::Os::SystemOpenFile(extractExecutablePath.c_str());
      return;
    }

    // The executable hasn't been extracted yet, so we need to read the archived contents from our data section.
    file.Open(GetApplication(), FileMode::Read, FileAccessPattern::Random, FileShare::Read);

    // If the file isn't openable, then look in our resource
    // section to see if we can extract a clone of this executable (without the packaged resources, very small).
    if (!file.IsOpen())
    {
      // Make sure we're able to extract a valid block.
      ByteBufferBlock block;
      if (GetExecutableResource(cResourceName, cResourceType, block) && block.GetBegin() && block.Size())
      {
        // Now write the extracted block to disk and verify it was written.
        String clonePath = FilePath::CombineWithExtension(GetTemporaryDirectory(), extractName, cExecutableExtensionWithDot);
        if (WriteToFile(clonePath.c_str(), block.GetBegin(), block.Size()) == block.Size() && FileExists(clonePath))
        {
          // We wrote the executable to disk, now we need to run it.
          String quotedApplication = BuildString("\"", GetApplication(), "\"");
          Status status;
          Zero::Os::SystemOpenFile(status, clonePath.c_str(), Os::Verb::Default, quotedApplication.c_str());

          // If we successfully ran the other process, shut down so that it can read our process.
          if (status.Succeeded())
            return;
        }
      }
    }

    // If we weren't able to open our own file, try and copy our executable
    // to a temporary location and attempt to open it there.
    if (!file.IsOpen())
    {
      // Copy the file to a temporary directory (the name doesn't matter, as long as it doesn't collide).
      String tempCopyPath = FilePath::CombineWithExtension(GetTemporaryDirectory(), extractName, ".tmp");
      if (CopyFile(tempCopyPath, GetApplication()) && FileExists(tempCopyPath))
        file.Open(tempCopyPath, FileMode::Read, FileAccessPattern::Random, FileShare::Read);
    }
  }
  // If we have an extra argument, then we're being run from ourselves (we are a clone).
  // The second argument should be a path back to the original parent process that's running us.
  // We need to extract that executable instead.
  else
  {
    // Attempt to open the original executable for read (we are a clone of it).
    String originalExecutablePath = arguments[1];
    file.Open(originalExecutablePath, FileMode::Read, FileAccessPattern::Random, FileShare::Read);
  }

  // If the file still wasn't open, check to see if there's a side by side executable we can use.
  if (!file.IsOpen())
    return FatalError("Unable to open our own file for reading.");

  // If we're here, it means we successfully opened the data file.
  // Check the signature at the end to be sure we can self extract.
  if (!file.Seek((u64)-(s64)cSignatureSize, SeekOrigin::End))
    return FatalError("Unable to seek to the end of the file to read the signature.");

  byte buffer[cSignatureSize];
  Status status;
  size_t amount = file.Read(status, buffer, cSignatureSize);

  if (status.Failed() || amount != cSignatureSize)
    return FatalError("Unable to read the signature at the end of the file.");

  if (memcmp(buffer, cSignature, cSignatureSize) != 0)
    return FatalError("End of the file did not match our signature bytes.");

  // Before the signature we should be able to read a size that tells us how much
  // data is in the data section at the end of the executable.
  // Note that the size is *always* in little endian format.
  u64 archiveSize = 0;
  if (!file.Seek((u64)-(s64)(cSignatureSize + sizeof(archiveSize)), SeekOrigin::End))
    return FatalError("Unable to seek to the end of the file to read the archive size.");

  amount = file.Read(status, (byte*)&archiveSize, sizeof(archiveSize));
  if (status.Failed() || amount != sizeof(archiveSize))
    return FatalError("Unable to read the size of the archive.");

  // The file format is little endian, so if we're on a big endian platform switch it.
  if (IsBigEndian())
    archiveSize = EndianSwap(archiveSize);

  // Now backup by the size of the archive.
  if (!file.Seek((u64)-(s64)(sizeof(archiveSize) + archiveSize), SeekOrigin::End))
    return FatalError("Unable to seek to the end of the file to read the archive size.");

  // Start reading the file at the specified location, and extract it to the directory.
  Archive archive(ArchiveMode::Decompressing);
  archive.mFileOriginBegin = (u64)file.Tell();
  archive.ReadZip((ArchiveReadFlags::Enum)(ArchiveReadFlags::Entries | ArchiveReadFlags::Data /* we dont want Data! */), file);

  archive.ExportToDirectory(ArchiveExportMode::Overwrite, extractDirectoryPath);

  // Verify that the target executable file exists.
  if (!FileExists(extractExecutablePath))
    return FatalError("Target executable not found after extracting archive.");

  // Run the extracted executable, we're done!
  Zero::Os::SystemOpenFile(extractExecutablePath.c_str());
}

void PackArchive(Status& status, StringParam executablePath, StringParam archivePath)
{
  File executable;
  if (!executable.Open(executablePath, FileMode::Append, FileAccessPattern::Random, FileShare::Unspecified, &status))
    return;

  // This should be a stream!
  File archive;
  if (!archive.Open(archivePath, FileMode::Read, FileAccessPattern::Sequential, FileShare::Read, &status))
    return;

  size_t amountWritten = 0;
  u64 size = 0;
  for (;;)
  {
    byte buffer[8192];
    size_t amountRead = archive.Read(status, buffer, sizeof(buffer));
    size += amountRead;

    if (status.Failed())
      return;

    amountWritten = executable.Write(buffer, amountRead);

    if (amountWritten != amountRead)
    {
      status.SetFailed("Unable to write chunk of archive data to the executable data section");
      return;
    }
  }

  // Write out the size in little endian form.
  if (IsBigEndian())
    size = EndianSwap(size);

  amountWritten = executable.Write((byte*)&size, sizeof(size));
  if (amountWritten != sizeof(size))
  {
    status.SetFailed("Unable to write size of archive");
    return;
  }

  // Finally, write the signature and we're done!
  amountWritten = executable.Write((byte*)cSignature, cSignatureSize);

  if (amountWritten != cSignatureSize)
  {
    status.SetFailed("Unable to write signature");
    return;
  }
}

int PlatformMain(const Array<String>& arguments)
{
  CommonLibrary::Initialize();

  Extract(arguments);
  
  CommonLibrary::Shutdown();
  return 0;
}

} // namespace Zero