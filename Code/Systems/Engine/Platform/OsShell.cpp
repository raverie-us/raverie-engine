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

  ZilchBindGetterProperty(WindowCount);
  ZilchBindMethod(GetWindow);
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

  mShell.Update();

  // This is a special place to update for other systems like the
  // that may cause the message pump to run (originally used for CEF browser).
  Event toSend;
  DispatchEvent(Events::OsShellUpdate, &toSend);
  Z::gEngine->DispatchEvent(Events::OsShellUpdate, &toSend);
}

IntVec2 OsShell::GetPrimaryMonitorSize()
{
  return mShell.GetPrimaryMonitorSize();
}

ByteColor OsShell::GetColorAtMouse()
{
  return mShell.GetColorAtMouse();
}

void OsShell::SetMouseCursor(Cursor::Enum cursorId)
{
  return mShell.SetMouseCursor(cursorId);
}

void OsShell::OpenFile(FileDialogConfig* config)
{
  return mShell.OpenFile(*config);
}

void OsShell::SaveFile(FileDialogConfig* config)
{
  return mShell.SaveFile(*config);
}

void OsShell::ShowMessageBox(StringParam title, StringParam message)
{
  return mShell.ShowMessageBox(title, message);
}

void OsShell::ScanInputDevices()
{
  // DeactivateAll because joysticks may have been removed in device changed
  Z::gJoysticks->DeactivateAll();

  const Array<PlatformInputDevice>& devices = mShell.ScanInputDevices();
  forRange (PlatformInputDevice& device, devices)
  {
    // Tell the Joysticks system that a Joystick is present
    Z::gJoysticks->AddJoystickDevice(device);
  }

  Z::gJoysticks->JoysticksChanged();
}

size_t OsShell::GetWindowCount()
{
  return mShell.mWindows.Size();
}

OsWindow* OsShell::GetWindow(size_t index)
{
  if (index >= mShell.mWindows.Size())
  {
    DoNotifyException("Shell", "Invalid window index");
    return nullptr;
  }
  return (OsWindow*)mShell.mWindows[index]->mUserData;
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
