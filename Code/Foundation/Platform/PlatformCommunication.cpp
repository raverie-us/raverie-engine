// MIT Licensed (see LICENSE.md).
#include "PlatformCommunication.hpp"

extern "C" {
void ZeroExportNamed(ExportQuit)() {
  Zero::Shell* shell = Zero::Shell::sInstance;
  Zero::ShellWindow* mainWindow = shell->mMainWindow;

  if (mainWindow && mainWindow->mOnClose)
    mainWindow->mOnClose(mainWindow);
}

}
