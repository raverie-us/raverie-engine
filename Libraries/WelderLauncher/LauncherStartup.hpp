// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

bool ZeroLauncherStartup(Engine* engine,
                         StringMap& arguments,
                         StringParam dllPath);

class LauncherStartup : public ZeroStartup
{
protected:
  void InitializeExternal() override;
  void InitializeConfig(Cog* configCog) override;
  void ShutdownExternal() override;
};

} // namespace Zero
