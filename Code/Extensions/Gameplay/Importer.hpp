// MIT Licensed (see LICENSE.md).
#pragma once
#include "Systems/Engine/EngineStandard.hpp"
#include "Utilities.hpp"
#include "Thread.hpp"
#include "Foundation/Meta/Event.hpp"

namespace Zero
{
class OsFileSelection;
class File;
class Thread;

DeclareEnum3(ImporterResult, NotEmbeded, Embeded, ExecutedAnotherProcess);

class Importer
{
public:
  static const uint cEmptyPackageSize;

  ImporterResult::Type CheckForImport();
  String mOutputDirectory;

private:
  OsInt DoImport(ByteBufferBlock& buffer);
};

} // namespace Zero
