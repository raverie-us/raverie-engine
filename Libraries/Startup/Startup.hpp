// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Startup
// Handles all platform agnostic initialization
class ZeroStartup
{
public:
  Engine* Initialize();
  void Startup();
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
