// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
BuildOptions::BuildOptions(ContentLibrary* library)
{
  ToolPath = Z::gContentSystem->ToolPath;

  SourcePath = library->SourcePath;
  OutputPath = library->GetOutputPath();
}
} // namespace Zero
