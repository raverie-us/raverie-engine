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
    // After all jobs are completed, use this opportunity to create your version of a main object (Editor/Game).
    UserCreation,
    // Engine is automatically updated until it is no longer active / closed.
    EngineUpdate,
    // All libraries are shutdown and the engine is destroyed.
    Shutdown,
    // We delete this entire startup object
    Terminate);

// Runs through phases of initialization, allowing platforms that don't support threading
// to yeild time back to the OS/Browser between updates. This also unifies Editor/Game startup.
class GameOrEditorStartup
{
public:
  // Run a single iteration and return the new phase that we reached
  StartupPhase::Enum RunIteration();

protected:
  // The following options should be set by the user in UserInitialize.
  // The default options are all tailored for the Editor.
  bool mLoadContent = true;
  WindowState::Enum mWindowState = WindowState::Maximized;
  // If this value is IntVec2::cZero, the primary monitor usable size will be used.
  IntVec2 mWindowSize = IntVec2::cZero;
  IntVec2 mMinimumWindowSize = cMinimumMonitorSize;
  bool mWindowCentered = false;
  WindowStyleFlags::Enum mWindowStyle =
      (WindowStyleFlags::Enum)(WindowStyleFlags::MainWindow | WindowStyleFlags::OnTaskBar | WindowStyleFlags::TitleBar |
                               WindowStyleFlags::Resizable | WindowStyleFlags::Close | WindowStyleFlags::ClientOnly);
  bool mUseSplashScreen = false;

  // This will be available in UserStartup.
  OsWindow* mMainWindow = nullptr;

  // The order these are declared is the order they will be called.
  void UserInitializeConfig(Cog* configCog);
  void UserInitialize();
  void UserStartup();
  void UserCreation();

private:
  void Initialize();
  static void InitializeConfigExternal(Cog* configCog, void* userData);
  void Startup();
  void ProcessJobs();
  void JobsComplete();
  void EngineUpdate();
  void Shutdown();

  void NextPhase();

  StartupPhase::Enum mPhase = StartupPhase::Initialize;

  bool mPlayGame = false;
  Cog* mProjectCog = nullptr;
  String mProjectFile;
  String mNewProject;

  ExecutableState* mState = nullptr;
  ZilchSetup* mZilchSetup = nullptr;

  // Initialize:
  UniquePointer<DebuggerListener> mDebuggerListener;
  UniquePointer<FileSystemInitializer> mFileSystemInitializer;
  UniquePointer<FileListener> mFileListener;
  UniquePointer<StdOutListener> mStdoutListener;
};

} // namespace Zero
