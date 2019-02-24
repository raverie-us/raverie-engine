// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
// Get a resource by name and type from the current executable
ZeroShared bool GetExecutableResource(const char* name, const char* type, ByteBufferBlock& output);

class ZeroShared ExecutableResourceUpdater
{
public:
  // Open an executable file to make changes to its resources
  ExecutableResourceUpdater(Status& status, const char* fileName);

  // On destruction the changes are committed to the executable
  ~ExecutableResourceUpdater();

  // If a resource does not exist, it will be added. If it exists, it will be
  // updated/overwritten. If the data is nullptr and the size is 0, the resource
  // will be removed.
  void Update(const char* name, const char* type, const byte* data, size_t size);

  // Make the executable appear with a different icon within the shell.
  void UpdateIcon(const byte* data, size_t size);

private:
  OsHandle mHandle;
};

const char* const gPackName = "Pack.bin";
const char* const gPackType = "PACK";

} // namespace Zero
