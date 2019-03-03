// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// To allow platforms without threads / yields (such as Emscripten) to give time back to the OS/Browser
// we perform our initialization in phases
DeclareEnum10(
    StartupPhase,
    // All meta libraries are initialized, environment initialized, config loaded, and
    // engine created here. There will be no systems available on the engine.
    Initialize,
    // User may perform logic using Common/Meta/Config/Environment.
    // Use should set window creation settings, etc.
    UserInitialize,
    // Adds all the engine systems, initializes the content system (no loading yet), and creates the main window.
    Startup,
    // User may decide to queue any jobs such as processing content or loading packages.
    UserStartup,
    // All jobs are performed until there are none left (may yield back to the OS/Browser periodically).
    ProcessJobs,
    // Called when all jobs are complete (allows the engine to do anything with the results of completed jobs).
    JobsComplete,
    // After all jobs are completed, use this opportunity to create your version of a main object
    // (Editor/Game/Launcher).
    UserCreation,
    // Engine is automatically updated until it is no longer active / closed.
    EngineUpdate,
    // User may perform any cleanup logic (do not destroy the engine however).
    UserShutdown,
    // All libraries are shutdown and the engine is destroyed.
    Shutdown);

// Runs through phases of initialization, allowing platforms that don't support threading
// to yeild time back to the OS/Browser between updates. This also unifies Editor/Game/Launcher startup.
class ZeroStartup
{
public:
  // Called from main (returns the main return value).
  int Run();

protected:
  // The following options should be set by the user in UserInitialize.
  // The default options are all tailored for the Editor.
  // If changes are ever made to these flags (especially mWindowStyle), ALL platforms and programs
  // (Editor/Game/Launcher) must be considered.
  bool mLoadContent = true;
  WindowState::Enum mWindowState = WindowState::Maximized;
  // If this value is IntVec2::cZero, the primary monitor usable size will be used.
  IntVec2 mWindowSize = IntVec2::cZero;
  IntVec2 mMinimumWindowSize = cMinimumMonitorSize;
  bool mWindowCentered = false;
  WindowStyleFlags::Enum mWindowStyle =
      (WindowStyleFlags::Enum)(WindowStyleFlags::MainWindow | WindowStyleFlags::OnTaskBar | WindowStyleFlags::TitleBar |
                               WindowStyleFlags::Resizable | WindowStyleFlags::Close | WindowStyleFlags::ClientOnly);
  Cog* mWindowSettingsFromProjectCog = nullptr;
  bool mUseSplashScreen = false;

  // This will be available in UserStartup.
  OsWindow* mMainWindow = nullptr;

  // This may be set by the user at any time.
  // This must be static to properly handle the shutdown between regular platforms and Emscripten.
  static int sReturnCode;

  // During any point of initialization the user can set this to quit out.
  // Gracefull shutdown is not guaranteed.
  bool mExit = false;

  // The order these are declared is the order they will be called.
  virtual void UserInitializeLibraries();
  virtual void UserInitializeConfig(Cog* configCog);
  virtual void UserInitialize();
  virtual void UserStartup();
  virtual void UserCreation();
  virtual void UserShutdown();
  virtual void UserShutdownLibraries();

  // A helper that exits and sets the return code.
  void Exit(int returnCode = 0);

private:
  void MainLoop();
  static void MainLoopFunction(void* userData);

  void Initialize();
  static void InitializeConfigExternal(Cog* configCog, void* userData);
  void Startup();
  void ProcessJobs();
  void JobsComplete();
  void EngineUpdate();
  void Shutdown();

  void NextPhase();

  StartupPhase::Enum mPhase = StartupPhase::Initialize;

  ExecutableState* mState = nullptr;
  ZilchSetup* mZilchSetup = nullptr;

  // Initialize:
  UniquePointer<DebuggerListener> mDebuggerListener;
  UniquePointer<FileSystemInitializer> mFileSystemInitializer;
  UniquePointer<FileListener> mFileListener;
  UniquePointer<TimerBlock> mTotalEngineTimer;
  UniquePointer<StdOutListener> mStdoutListener;
};

} // namespace Zero
