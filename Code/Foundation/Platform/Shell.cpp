// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Foundation/Platform/PlatformCommunication.hpp"

namespace Raverie
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

void RaverieExportNamed(ExportOpenFileDialogAdd)(void* dialog, const char* filePath) {
  FileDialogInfo& config = *(FileDialogInfo*)dialog;
  config.mFiles.PushBack(filePath);
}

void RaverieExportNamed(ExportOpenFileDialogFinish)(void* dialog) {
  FileDialogInfo& config = *(FileDialogInfo*)dialog;
  config.mCallback(config.mFiles, config.mUserData);
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

void Shell::Close()
{
}

void Shell::SetProgress(const char* textOrNull, float percent) {
  // We batch all loading of multiple content libraries in the beginning under a single "loading" window
  // If we finished initial loading, then let code hide and show loading screens
  if (mInitialLoadingComplete) {
    ImportProgressUpdate(textOrNull, percent);
  } else {
    // Otherwise we always show the loading screen and ignore if it's null
    ImportProgressUpdate(textOrNull == nullptr ? "" : textOrNull, percent);
  }
}

GamepadRawState& Shell::GetOrCreateGamepad(uint32_t gamepadIndex) {
  if (mGamepads.Size() <= gamepadIndex) {
    mGamepads.Resize(gamepadIndex + 1);
  }

  return mGamepads[gamepadIndex];
}

GamepadRawButtonState& GamepadRawState::GetOrCreateButton(uint32_t buttonIndex) {
  if (mButtons.Size() < buttonIndex) {
    mButtons.Resize(buttonIndex + 1);
  }

  return mButtons[buttonIndex];
}

GamepadRawAxisState& GamepadRawState::GetOrCreateAxis(uint32_t axisIndex) {
  if (mAxes.Size() < axisIndex) {
    mAxes.Resize(axisIndex + 1);
  }

  return mAxes[axisIndex];
}

} // namespace Raverie
