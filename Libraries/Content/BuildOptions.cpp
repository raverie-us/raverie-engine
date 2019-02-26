// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
BuildOptions::BuildOptions(ContentLibrary* library)
{
  Cog* configCog = Z::gEngine->GetConfigCog();
  ContentConfig* contentConfig = configCog->has(ContentConfig);
  if (contentConfig != nullptr)
  {
    Verbosity = contentConfig->ContentVerbosity;
  }

  ToolPath = Z::gContentSystem->ToolPath;

  SourcePath = library->SourcePath;
  OutputPath = library->GetOutputPath();
}
} // namespace Zero
