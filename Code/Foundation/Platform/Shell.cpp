// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Platform/PlatformCommunication.hpp"

namespace Zero
{
Shell* Shell::sInstance = nullptr;
IntVec2 Shell::sInitialClientSize = IntVec2::cZero;
bool Shell::sInitialFocused = false;

Shell::Shell() : mCursor(Cursor::Arrow), mClientSize(sInitialClientSize), mHasFocus(sInitialFocused)
{
  sInstance = this;
}

Shell::~Shell()
{
}

ByteColor Shell::GetColorAtMouse()
{
  return 0;
}

bool Shell::IsKeyDown(Keys::Enum key)
{
  return mKeyState[key];
}

bool Shell::IsMouseDown(MouseButtons::Enum button)
{
  return mMouseState[button];
}

Cursor::Enum Shell::GetMouseCursor()
{
  return mCursor;
}

void Shell::SetMouseCursor(Cursor::Enum cursor)
{
  mCursor = cursor;
  ImportMouseSetCursor(cursor);
}

void Shell::OpenFile(FileDialogInfo& config)
{
  StringBuilder acceptExtensions;
  forRange (FileDialogFilter& filter, config.mSearchFilters)
  {
    if (acceptExtensions.GetSize() != 0)
      acceptExtensions.Append(',');

    // Filter out all the wildcard stars '*' since (currently cannot use them)
    forRange (Rune rune, filter.mFilter)
    {
      if (rune == ';')
        acceptExtensions.Append(',');
      else if (rune != '*')
        acceptExtensions.Append(rune);
    }
  }

  String accept = acceptExtensions.ToString();
  ImportOpenFileDialog(&config, config.mMultiple, accept.c_str());
}

void ZeroExportNamed(ExportOpenFileDialogAdd)(void* dialog, const char* filePath) {
  FileDialogInfo& config = *(FileDialogInfo*)dialog;
  config.mFiles.PushBack(filePath);
}

void ZeroExportNamed(ExportOpenFileDialogFinish)(void* dialog) {
  FileDialogInfo& config = *(FileDialogInfo*)dialog;
  config.mCallback(config.mFiles, config.mUserData);
}

const Array<PlatformInputDevice>& Shell::ScanInputDevices()
{
  return mInputDevices;
}

IntVec2 Shell::GetClientSize()
{
  return mClientSize;
}

void Shell::SetMouseCapture(bool capture)
{
}

bool Shell::GetMouseCapture()
{
  return false;
}

bool Shell::GetMouseTrap()
{
  return mMouseTrapped;
}

void Shell::SetMouseTrap(bool mouseTrapped)
{
  mMouseTrapped = mouseTrapped;
  ImportMouseTrap(mouseTrapped);
}

bool Shell::GetImage(Image* image)
{
  return false;
}

void Shell::Close()
{
}

} // namespace Zero
