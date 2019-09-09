// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

namespace Events
{
// Sent out if scripts successfully compiled before setting mLibrary
DeclareEvent(ScriptsCompiledPrePatch);
// Sent out if scripts successfully compiled before setting mLibrary
// Sync point for other systems to commit their pending libraries
DeclareEvent(ScriptsCompiledCommit);
// Sent out immediately before 'ScriptsCompiledPostPatch', allowing the editor
// to build script objects back up before post patch event handlers execute.
DeclareEvent(ScriptsCompiledPatch);
// Sent out if scripts successfully compiled after setting mLibrary
DeclareEvent(ScriptsCompiledPostPatch);
// Sent if scripts failed to compile
DeclareEvent(ScriptCompilationFailed);
} // namespace Events

// Zilch Compile Event
class ZilchCompileEvent : public Event
{
public:
  ZilchDeclareType(ZilchCompileEvent, TypeCopyMode::ReferenceType);
  ZilchCompileEvent(HashSet<ResourceLibrary*>& modifiedLibraries);

  bool WasTypeModified(BoundType* type);
  BoundType* GetReplacingType(BoundType* oldType);

  HashSet<ResourceLibrary*>& mModifiedLibraries;
};

// Zilch Manager
DeclareEnum2(CompileResult, CompilationFailed, CompilationSucceeded);

class ZilchManager : public ExplicitSingleton<ZilchManager, EventObject>
{
public:
  typedef ZilchManager ZilchSelf;
  typedef ExplicitSingleton<ZilchManager, EventObject> ZilchBase;

  /// Constructor.
  ZilchManager();

  /// Compiles all Scripts and Fragments and allow duplicate errors to
  /// re-appear.
  void TriggerCompileExternally();

  /// Compiles all Scripts and Fragments.
  void InternalCompile();

  // If dirtied we attempt to compile every engine update (checks dirty flag)
  void OnEngineUpdate(UpdateEvent* event);

  // When the debugger first pauses on a line
  void OnDebuggerPause(Zilch::DebuggerEvent* event);

  // When the debugger resumes execution
  void OnDebuggerResume(Zilch::DebuggerEvent* event);

  // When the debugger is paused on a line and updates
  void OnDebuggerPauseUpdate(Zilch::DebuggerEvent* event);

  // When the debugger skips a breakpoint
  void OnDebuggerBreakNotAllowed(Zilch::DebuggerTextEvent* event);

  // The last library we properly built (set inside
  // CompileLoadedScriptsIntoLibrary) Once this library becomes in use by an
  // executable state, we CANNOT update it, or any ZilchMeta types
  LibraryRef mCurrentFragmentProjectLibrary;
  LibraryRef mPendingFragmentProjectLibrary;

  // @TrevorS: We need to remove libraries from here if we remove them from the
  // project.
  HashSet<ResourceLibrary*> mPendingLibraries;

  // If any scripts, fragments, or plugins have been modified we should attempt
  // to compile once on engine update
  bool mShouldAttemptCompile;

  // We need to store the last result because we don't always attempt to
  // recompile
  CompileResult::Enum mLastCompileResult;

  // Every time we recompile libraries we increment a version globally.
  // This lets us know elsewhere that anything related to types or scripts have
  // changed. For example: We prevent duplicate exceptions until this version
  // changes.
  int mVersion;

  // The debugger interface that we register states with
  Debugger mDebugger;
};

} // namespace Zero
