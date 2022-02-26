// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class GameStartup : public ZeroStartup
{
private:
  Cog* mProjectCog = nullptr;
  String mProjectFile;

  void UserInitializeConfig(Cog* configCog) override;
  void UserInitialize() override;
  void UserStartup() override;
  void UserCreation() override;
};

} // namespace Zero
