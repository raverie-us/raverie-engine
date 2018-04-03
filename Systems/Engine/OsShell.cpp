///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
DefineEvent(OsShellUpdate);
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

OsShell::OsShell() :
  mOsShellHook(nullptr)
{
}
uint OsShell::GetScrollLineCount( )
{
  return 1;
}

void OsShell::DumpMemoryDebuggerStats()
{
  Memory::DumpMemoryDebuggerStats("MyProject");
}

//-------------------------------------------------------------------OsFileSelection
ZilchDefineType(OsFileSelection, builder, type)
{
}

//-------------------------------------------------------------------FileDialogConfig
FileDialogConfig::FileDialogConfig()
{
  CallbackObject = nullptr;
  mCallback = &Callback;
  mUserData = this;
}

void FileDialogConfig::Callback(Array<String>& files, bool success, void* userData)
{
  FileDialogConfig* self = (FileDialogConfig*)userData;
  
  if (self->CallbackObject)
  {
    OsFileSelection fileEvent;
    fileEvent.Files = files;
    fileEvent.Success = success;
    EventDispatcher* dispatcher = self->CallbackObject->GetDispatcherObject();
    dispatcher->Dispatch(self->EventName, &fileEvent);
  }
}

}//namespace Zero
