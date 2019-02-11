// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Startup
// Handles all platform agnostic initialization
class ZeroStartup
{
public:
  Engine* Initialize(ZeroStartupSettings& settings);
  void Shutdown();

protected:
  virtual void InitializeLibraries(ZeroStartupSettings& settings);
  Engine* InitializeEngine();
  ExecutableState* mState;
  ZilchSetup* mZilchSetup;
};

} // namespace Zero
