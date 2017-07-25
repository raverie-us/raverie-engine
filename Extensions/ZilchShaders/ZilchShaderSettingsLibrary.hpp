///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

ZilchDeclareStaticLibrary(ShaderSettingsLibrary, ZilchNoNamespace, ZeroNoImportExport);

//-------------------------------------------------------------------ZilchShaderSettingsLoader
/// Loads a directory of zilch files, compiles it and runs various script to collect settings.
/// Currently only used for SystemValues (expects a SystemValueSettings struct to exists).
class ZilchShaderSettingsLoader : public Zilch::EventHandler
{
public:
  /// Load, compile, and run a settings directory to populate the shader settings
  void LoadSettings(ZilchShaderSettings* settings, StringParam settingsDirectoryPath);

private:
  void LoadSystemValueSettings(ZilchShaderSettings* settings, Zilch::LibraryRef& library, Zilch::ExecutableState* state);

  void OnForwardEvent(Zilch::EventData* e);
};

}//namespace Zero

