////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(ScriptsCompiledPrePatch);
DefineEvent(ScriptsCompiledCommit);
DefineEvent(ScriptsCompiledPostPatch);
DefineEvent(ScriptCompilationFailed);
}//namespace Events

//------------------------------------------------------------------------------ Zilch Compile Event
ZilchDefineType(ZilchCompileEvent, builder, type)
{

}

//**************************************************************************************************
ZilchCompileEvent::ZilchCompileEvent(HashSet<ResourceLibrary*>& modifiedLibraries) :
  mModifiedLibraries(modifiedLibraries)
{

}

//------------------------------------------------------------------------------------ Zilch Manager
//**************************************************************************************************
ZilchManager::ZilchManager()
{

}

//**************************************************************************************************
CompileResult::Enum ZilchManager::Compile()
{
  forRange(ResourceLibrary* resourceLibrary, Z::gResources->LoadedResourceLibraries.Values())
  {
    if(resourceLibrary->CompileScripts(mPendingLibraries) == false)
    {
      Event eventToSend;
      this->DispatchEvent(Events::ScriptCompilationFailed, &eventToSend);
      return CompileResult::CompilationFailed;
    }
  }

  // If there are no pending libraries, nothing was compiled
  if(mPendingLibraries.Empty())
    return CompileResult::CompilationNotRequired;

  // Since we binary cache archetypes (in a way that is NOT saving the data tree, but rather a 'known serialization format'
  // then if we moved any properties around in any script it would completely destroy how the archetypes were cached
  // The simplest solution is to clear the cache
  ArchetypeManager::GetInstance()->FlushBinaryArchetypes();

  // Scripts were successfully compiled
  ZilchCompileEvent compileEvent(mPendingLibraries);

  this->DispatchEvent(Events::ScriptsCompiledPrePatch, &compileEvent);
  // Library commits must happen after all systems have handle PrePatch
  this->DispatchEvent(Events::ScriptsCompiledCommit, &compileEvent);

  forRange(ResourceLibrary* resourceLibrary, compileEvent.mModifiedLibraries.All())
    resourceLibrary->Commit();

  if(mPendingScriptProjectLibrary)
  {
    mCurrentScriptProjectLibrary = mPendingScriptProjectLibrary;
    mPendingScriptProjectLibrary = nullptr;
  }

  if(mPendingFragmentProjectLibrary)
  {
    mCurrentFragmentProjectLibrary = mPendingFragmentProjectLibrary;
    mPendingFragmentProjectLibrary = nullptr;
  }

  this->DispatchEvent(Events::ScriptsCompiledPostPatch, &compileEvent);

  mPendingLibraries.Clear();

  return CompileResult::CompilationSucceeded;
}

}//namespace Zero
