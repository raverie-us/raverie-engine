////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
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
// Sent out if scripts successfully compiled after setting mLibrary
DeclareEvent(ScriptsCompiledPostPatch);
// Sent if scripts failed to compile
DeclareEvent(ScriptCompilationFailed);
}

//------------------------------------------------------------------------------ Zilch Compile Event
class ZilchCompileEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ZilchCompileEvent(HashSet<ResourceLibrary*>& modifiedLibraries);
  HashSet<ResourceLibrary*>& mModifiedLibraries;
};

//------------------------------------------------------------------------------------ Zilch Manager
DeclareEnum3(CompileResult, CompilationFailed, CompilationSucceeded, CompilationNotRequired);

class ZilchManager : public ExplicitSingleton<ZilchManager, EventObject>
{
public:
  /// Constructor.
  ZilchManager();

  /// Compiles all Scripts and Fragments.
  CompileResult::Enum Compile();

  // The last library we properly built (set inside CompileLoadedScriptsIntoLibrary)
  // Once this library becomes in use by an executable state, we CANNOT update it, or any ZilchMeta types
  LibraryRef mCurrentFragmentProjectLibrary;
  LibraryRef mPendingFragmentProjectLibrary;

  // @TrevorS: We need to remove libraries from here if we remove them from the project.
  HashSet<ResourceLibrary*> mPendingLibraries;

  // Every time we recompile libraries we increment a version globally.
  // This lets us know elsewhere that anything related to types or scripts have changed.
  // For example: We prevent duplicate exceptions until this version changes.
  int mVersion;
};

}//namespace Zero
