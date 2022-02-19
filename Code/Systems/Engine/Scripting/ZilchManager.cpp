// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(ScriptsCompiledPrePatch);
DefineEvent(ScriptsCompiledCommit);
DefineEvent(ScriptsCompiledPatch);
DefineEvent(ScriptsCompiledPostPatch);
DefineEvent(ScriptCompilationFailed);
} // namespace Events

// Zilch Compile Event
ZilchDefineType(ZilchCompileEvent, builder, type)
{
}

ZilchCompileEvent::ZilchCompileEvent(HashSet<ResourceLibrary*>& modifiedLibraries) :
    mModifiedLibraries(modifiedLibraries)
{
}

bool ZilchCompileEvent::WasTypeModified(BoundType* type)
{
  forRange (ResourceLibrary* lib, mModifiedLibraries.All())
  {
    if (lib->BuiltType(type))
      return true;
  }

  return false;
}

BoundType* ZilchCompileEvent::GetReplacingType(BoundType* oldType)
{
  if (!WasTypeModified(oldType))
    return nullptr;

  forRange (ResourceLibrary* lib, mModifiedLibraries.All())
  {
    if (BoundType* newType = lib->GetReplacingType(oldType))
      return newType;
  }

  return nullptr;
}

// Zilch Manager
ZilchManager::ZilchManager() :
    mVersion(0),
    mShouldAttemptCompile(true),
    mLastCompileResult(CompileResult::CompilationSucceeded)
{
  ConnectThisTo(Z::gEngine, Events::EngineUpdate, OnEngineUpdate);

  EventConnect(&mDebugger, Zilch::Events::DebuggerPause, &ZilchManager::OnDebuggerPause, this, &mDebugger);
  EventConnect(&mDebugger, Zilch::Events::DebuggerResume, &ZilchManager::OnDebuggerResume, this, &mDebugger);
  EventConnect(&mDebugger, Zilch::Events::DebuggerPauseUpdate, &ZilchManager::OnDebuggerPauseUpdate, this, &mDebugger);
  EventConnect(
      &mDebugger, Zilch::Events::DebuggerBreakNotAllowed, &ZilchManager::OnDebuggerBreakNotAllowed, this, &mDebugger);
}

void ZilchManager::TriggerCompileExternally()
{
  // Currently the version is used to detect duplicate errors
  // If something is externally triggering a compile (such as saving, project
  // loading, playing a game, etc) then we want to show duplicate errors again.
  ++mVersion;
  InternalCompile();
}

void ZilchManager::InternalCompile()
{
  if (Z::gEngine->IsReadOnly())
  {
    DoNotifyWarning("Zilch", "Cannot recompile scripts while in read-only mode.");
    return;
  }

  if (!mShouldAttemptCompile)
    return;
  mShouldAttemptCompile = false;

  forRange (ResourceLibrary* resourceLibrary, Z::gResources->LoadedResourceLibraries.Values())
  {
    if (resourceLibrary->CompileScripts(mPendingLibraries) == false)
    {
      Event eventToSend;
      this->DispatchEvent(Events::ScriptCompilationFailed, &eventToSend);
      mLastCompileResult = CompileResult::CompilationFailed;
      return;
    }
  }

  // If there are no pending libraries, nothing was compiled
  ErrorIf(mPendingLibraries.Empty(),
          "If the mShouldAttemptCompile flag was set, we should always have "
          "pending libraries (even at startup with no scripts)!");

  // Since we binary cache archetypes (in a way that is NOT saving the data
  // tree, but rather a 'known serialization format' then if we moved any
  // properties around in any script it would completely destroy how the
  // archetypes were cached The simplest solution is to clear the cache
  ArchetypeManager::GetInstance()->FlushBinaryArchetypes();

  // Scripts were successfully compiled
  ZilchCompileEvent compileEvent(mPendingLibraries);

  // If Events::ScriptsCompiledPrePatch is dispatched, we MUST dispatch the
  // PostPatch event after. There cannot be a return in between them. This is
  // due to how we re-initialize Cogs and rebuild Archetype's (see
  // Archetype::sRebuilding)
  this->DispatchEvent(Events::ScriptsCompiledPrePatch, &compileEvent);
  // Library commits must happen after all systems have handle PrePatch
  this->DispatchEvent(Events::ScriptsCompiledCommit, &compileEvent);

  // Unload ALL affected libraries before committing any of them.
  forRange (ResourceLibrary* resourceLibrary, compileEvent.mModifiedLibraries.All())
    resourceLibrary->PreCommitUnload();

  forRange (ResourceLibrary* resourceLibrary, compileEvent.mModifiedLibraries.All())
    resourceLibrary->Commit();

  // @TrevorS: Refactor this to remove a global dependence on a single library.
  if (mPendingFragmentProjectLibrary)
  {
    mCurrentFragmentProjectLibrary = mPendingFragmentProjectLibrary;
    mPendingFragmentProjectLibrary = nullptr;
  }

  this->DispatchEvent(Events::ScriptsCompiledPatch, &compileEvent);
  this->DispatchEvent(Events::ScriptsCompiledPostPatch, &compileEvent);

  MetaDatabase::GetInstance()->ClearRemovedLibraries();

  mPendingLibraries.Clear();

  mLastCompileResult = CompileResult::CompilationSucceeded;
}

void ZilchManager::OnEngineUpdate(UpdateEvent* event)
{
  InternalCompile();
}

void ZilchManager::OnDebuggerPause(Zilch::DebuggerEvent* event)
{
  if (GetApplicationName() != sEditorName)
    return;

  ScriptEvent toSend;
  toSend.Location = *event->Location;
  toSend.Script = (DocumentResource*)event->Location->CodeUserData;
  Z::gResources->DispatchEvent(Events::DebuggerPaused, &toSend);
}

void ZilchManager::OnDebuggerResume(Zilch::DebuggerEvent* event)
{
  if (GetApplicationName() != sEditorName)
    return;

  ScriptEvent toSend;
  toSend.Location = *event->Location;
  toSend.Script = (DocumentResource*)event->Location->CodeUserData;
  Z::gResources->DispatchEvent(Events::DebuggerResumed, &toSend);
}

void ZilchManager::OnDebuggerPauseUpdate(Zilch::DebuggerEvent* event)
{
  if (GetApplicationName() != sEditorName)
    return;

  Z::gEngine->mIsDebugging = true;
  Z::gEngine->Update();
  Z::gEngine->mIsDebugging = false;
}

void ZilchManager::OnDebuggerBreakNotAllowed(Zilch::DebuggerTextEvent* event)
{
  if (GetApplicationName() != sEditorName)
    return;

  DoNotifyWarning("Debugger", event->Text);
}

} // namespace Zero
