// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

class GameOrEditorStartup : public ZeroStartup
{
private:
  bool mPlayGame = false;
  Cog* mProjectCog = nullptr;
  String mProjectFile;
  String mNewProject;

  void UserInitializeConfig(Cog* configCog) override;
  void UserInitialize() override;
  void UserStartup() override;
  void UserCreation() override;
};

} // namespace Zero
