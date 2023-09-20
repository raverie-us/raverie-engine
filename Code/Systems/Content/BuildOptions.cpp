// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
BuildOptions::BuildOptions(ContentLibrary* library)
{
  ToolPath = Z::gContentSystem->ToolPath;

  SourcePath = library->SourcePath;
  OutputPath = library->GetOutputPath();
}
} // namespace Raverie
