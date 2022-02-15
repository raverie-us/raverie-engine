// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class LauncherStartup : public ZeroStartup
{
protected:
  void UserInitializeLibraries() override;
  void UserInitializeConfig(Cog* configCog) override;
  void UserInitialize() override;
  void UserStartup() override;
  void UserCreation() override;
  void UserShutdown() override;
  void UserShutdownLibraries() override;
};

} // namespace Zero
