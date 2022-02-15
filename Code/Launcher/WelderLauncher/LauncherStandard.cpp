// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
const String mLauncherRegularFont = "NotoSans-Regular";
const String mLauncherBoldFont = "NotoSans-Bold";

ZilchDefineEnum(LauncherStartupArguments);

ZilchDefineStaticLibrary(LauncherLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  ZilchInitializeEnum(LauncherStartupArguments);

  // Events
  ZilchInitializeType(LauncherBuildEvent);

  // Components
  ZilchInitializeType(ZeroBuildContent);
  ZilchInitializeType(ZeroBuildReleaseNotes);
  ZilchInitializeType(ZeroBuildDeprecated);
  ZilchInitializeType(ZeroTemplate);
  ZilchInitializeType(LauncherProjectInfo);
}

void LauncherLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void LauncherLibrary::Shutdown()
{
}

} // namespace Zero
