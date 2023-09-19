// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
namespace Events
{
DefineEvent(Cut);
DefineEvent(Copy);
DefineEvent(Paste);
DefineEvent(OsShellUpdate);
DefineEvent(FileDialogComplete);
} // namespace Events

ZilchDefineType(OsShell, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  ZilchBindSetter(MouseCursor);
}

OsShell* CreateOsShellSystem()
{
  return new OsShell();
}

OsShell::OsShell()
{
}

const char* ZeroExportNamed(ExportCopy)(bool isCut)
{
  ClipboardEvent toSend;
  Z::gEngine->has(OsShell)->DispatchEvent(isCut ? Events::Cut : Events::Copy, &toSend);

  // We store the copied string here until the next copy occurs
  static String sCopyBuffer;
  sCopyBuffer = toSend.mText;
  return sCopyBuffer.c_str();
}

void ZeroExportNamed(ExportPaste)(const char* text)
{
  ClipboardEvent toSend;
  toSend.mText = text;
  Z::gEngine->has(OsShell)->DispatchEvent(Events::Paste, &toSend);
}

cstr OsShell::GetName()
{
  return "OsShell";
}

void OsShell::Update(bool debugger)
{
  ProfileScopeTree("ShellSystem", "Engine", Color::Red);

  Keyboard* keyboard = Keyboard::GetInstance();
  keyboard->Update();

  // Zero the cursor movement before the windows message pump to clear last
  // frames movement
  Z::gMouse->mCursorMovement = Vec2::cZero;
  Z::gMouse->mRawMovement = Vec2(0, 0);

  // This is a special place to update for other systems like the
  // that may cause the message pump to run (originally used for CEF browser).
  Event toSend;
  DispatchEvent(Events::OsShellUpdate, &toSend);
  Z::gEngine->DispatchEvent(Events::OsShellUpdate, &toSend);
}

void OsShell::SetMouseCursor(Cursor::Enum cursorId)
{
  return mShell.SetMouseCursor(cursorId);
}

ZilchDefineType(ClipboardEvent, builder, type)
{
  ZilchBindMethodProperty(Clear);
  ZilchBindGetterSetterProperty(Text);
  ZilchBindMemberProperty(mHandled);
}

void ClipboardEvent::Clear()
{
  mText = String();
}

void ClipboardEvent::SetText(StringParam text)
{
  mText = text;
}

String ClipboardEvent::GetText()
{
  return mText;
}

ZilchDefineType(OsFileSelection, builder, type)
{
}

FileDialogConfig* FileDialogConfig::Create()
{
  return new FileDialogConfig();
}

FileDialogConfig::FileDialogConfig()
{
  EventName = Events::FileDialogComplete;
  CallbackObject = nullptr;
  mCallback = &Callback;
  mUserData = this;
}

void FileDialogConfig::Callback(Array<String>& files, void* userData)
{
  FileDialogConfig* self = (FileDialogConfig*)userData;

  if (Object* callbackObject = self->CallbackObject)
  {
    EventDispatcher* dispatcher = callbackObject->GetDispatcherObject();
    if (dispatcher)
    {
      OsFileSelection fileEvent;
      fileEvent.Files = files;
      fileEvent.Success = !files.Empty();
      dispatcher->Dispatch(self->EventName, &fileEvent);
    }
  }

  // At the end of the callback we need to free the memory for the config.
  delete self;
}

} // namespace Zero
