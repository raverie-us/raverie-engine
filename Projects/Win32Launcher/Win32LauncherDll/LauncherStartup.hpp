///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

bool ZeroLauncherStartup(Engine* engine, StringMap& arguments, StringParam dllPath);

class ZeroLauncherStartupSettings : public ZeroStartupSettings
{
public:
  String mDllPath;
  Cog* LoadConfig() override;
};

/// Helper to initialize libraries for the launcher
class LauncherStartup : public ZeroStartup
{
public:
  void Shutdown();

protected:
  void InitializeLibraries(ZeroStartupSettings& settings) override;
};

}//namespace Zero
