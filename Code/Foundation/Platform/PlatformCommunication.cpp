// MIT Licensed (see LICENSE.md).
#include "PlatformCommunication.hpp"

extern "C" {
void ExportKeyDown(Zero::Keys::Enum key, uint osKey, bool repeated) {
  printf("ExportKeyDown\n");
}

void ExportQuit() {
  Zero::Shell* shell = Zero::Shell::sInstance;
  Zero::ShellWindow* mainWindow = shell->mMainWindow;

  if (mainWindow && mainWindow->mOnClose)
    mainWindow->mOnClose(mainWindow);
}

}
