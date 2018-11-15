///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Engine/EngineStandard.hpp"
#include "Utilities.hpp"
#include "Thread.hpp"
#include "Meta/Event.hpp"

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

}
