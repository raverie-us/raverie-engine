// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class StartupOptions
{
public:
  // The default options are all tailored for the Editor.
  bool mLoadContent = true;
  WindowState::Enum mWindowState = WindowState::Maximized;
  IntVec2 mWindowSize = IntVec2(1280, 720);
  IntVec2 mMinimumWindowSize = cMinimumMonitorSize;
  bool mWindowCentered = false;

  // If changes are ever made to these flags, all platforms must be considered.
  WindowStyleFlags::Enum mWindowStyle = (WindowStyleFlags::Enum)(
      WindowStyleFlags::MainWindow | WindowStyleFlags::OnTaskBar |
      WindowStyleFlags::TitleBar | WindowStyleFlags::Resizable |
      WindowStyleFlags::Close | WindowStyleFlags::ClientOnly);
  Cog* mWindowSettingsFromProjectCog = nullptr;

  bool mUseSplashScreen = false;
};

// Handles all platform agnostic initialization
class ZeroStartup
{
public:
  // All meta libraries are initialized, environment initialized, config loaded, and
  // engine created here. There will be no systems available on the engine, however
  // logic may be performed after this that depends on Common/Meta/Config/Environment.
  Engine* Initialize();

  // Adds all the engine systems, initializes the content system (no loading yet),
  // and creates the main window. After this point content should be loaded iteratively.
  OsWindow* Startup(StartupOptions& options);

  void Shutdown();

protected:
  virtual void InitializeExternal();
  virtual void InitializeConfig(Cog* configCog);
  virtual void ShutdownExternal();

private:
  static void InitializeConfigExternal(Cog* configCog, void* userData);

  ExecutableState* mState;
  ZilchSetup* mZilchSetup;

  // Initialize:
  UniquePointer<DebuggerListener> mDebuggerListener;
  UniquePointer<FileSystemInitializer> mFileSystemInitializer;
  UniquePointer<FileListener> mFileListener;
  UniquePointer<TimerBlock> mTotalEngineTimer;
  UniquePointer<StdOutListener> mStdoutListener;

  // Startup:
};

} // namespace Zero
