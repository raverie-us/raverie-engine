// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void DownloadProjectFolder(ProjectSettings* project)
{
  Download(project->ProjectFolder);
}

void DownloadContentOutput(ProjectSettings* project)
{
  Download(project->ProjectContentLibrary->GetOutputPath());
}

void BindArchiveCommands(Cog* config, CommandManager* commands)
{
  commands->AddCommand("DownloadProjectFolder", BindCommandFunction(DownloadProjectFolder), true);
  commands->AddCommand("DownloadContentOutput", BindCommandFunction(DownloadContentOutput), true);
}

} // namespace Zero
