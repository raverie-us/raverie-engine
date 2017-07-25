///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
namespace Z
{
  bool gEnableOsWindowProcedure = true;
}

//-------------------------------------------------------------------OsShell
ZilchDefineType(OsShell, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindMethod(IsClipboardText);
  ZilchBindGetterSetterProperty(ClipboardText);

  ZilchBindGetterProperty(WindowCount);
  ZilchBindMethod(GetWindow);

  ZilchBindMethod(DumpMemoryDebuggerStats);
}

void OsShell::DumpMemoryDebuggerStats()
{
  Memory::DumpMemoryDebuggerStats("MyProject");
}

//-------------------------------------------------------------------OsFileSelection
ZilchDefineType(OsFileSelection, builder, type)
{
}

//-------------------------------------------------------------------FileDialogFilter
FileDialogFilter::FileDialogFilter()
{

}

FileDialogFilter::FileDialogFilter(StringParam filter) 
  : mDescription(filter)
  , mFilter(filter)
{

}

FileDialogFilter::FileDialogFilter(StringParam description, StringParam filter)
  : mDescription(description)
  , mFilter(filter)
{

}

//-------------------------------------------------------------------FileDialogConfig
FileDialogConfig::FileDialogConfig()
{
  Flags = 0;
  CallbackObject = nullptr;
}

void FileDialogConfig::AddFilter(StringParam description, StringParam filter)
{
  mSearchFilters.PushBack(FileDialogFilter(description, filter));
}

}//namespace Zero
