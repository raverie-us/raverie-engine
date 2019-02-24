// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

bool LoadLauncherContent()
{
  Z::gContentSystem->DefaultBuildStream = new TextStreamDebugPrint();
  Z::gContentSystem->EnumerateLibraries();

  ZPrint("Loading Editor Content...\n");
  EditorPackageLoader* loader = EditorPackageLoader::GetInstance();

  Timer timer;
  timer.Update();

  String docDirectory = GetUserDocumentsDirectory();

  LoadContentLibrary("FragmentCore", true);
  bool coreContent = LoadContentLibrary("Loading", true);

  Array<String> coreLibs;

  coreLibs.PushBack("ZeroCore");
  coreLibs.PushBack("ZeroLauncherResources");

  forRange(String libraryName, coreLibs.All())
  {
    coreContent = coreContent && LoadContentLibrary(libraryName, true);
  }

  if (!coreContent)
  {
    FatalEngineError("Failed to load core content library for editor. Resources"
                     " need to be in the working directory.");
    return false;
  }

  float time = (float)timer.UpdateAndGetTime();
  ZPrint("Finished Loading Editor Content in %.2f\n", time);
  return true;
}

void LauncherStartup::InitializeExternal()
{
  LauncherDllLibrary::Initialize();
}

void LauncherStartup::InitializeConfig(Cog* configCog)
{
  // Force certain config components to exist. There's a few upgrade cases
  // where one of these could be missing otherwise.
  HasOrAdd<TextEditorConfig>(configCog);
  HasOrAdd<RecentProjects>(configCog);

  LauncherConfig* versionConfig = HasOrAdd<LauncherConfig>(configCog);
  versionConfig->mLauncherLocation = GetApplication();

  // Apply any command line arguments (mostly auto-run settings).
  versionConfig->ApplyCommandLineArguments();
}

void LauncherStartup::ShutdownExternal()
{
  LauncherDllLibrary::Shutdown();
  LauncherDllLibrary::GetInstance().ClearLibrary();
}

} // namespace Zero
