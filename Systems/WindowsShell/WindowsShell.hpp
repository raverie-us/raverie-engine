///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

void FileDialog(OsHandle windowHandle, FileDialogConfig& config, bool opening);

void ShellSetClipboardText(OsHandle windowHandle, StringRange text);
String ShellGetClipboardText(OsHandle windowHandle);

bool ShellGetClipboardImage(OsHandle windowHandle, Image* imageBuffer);
bool ShellIsClipboardImageAvailable(OsHandle windowHandle);

bool ShellGetWindowImage(OsHandle windowHandle, Image* imageBuffer);
bool ShellGetDesktopImage(Image* image);
bool WindowsErrorProcessHandler(ErrorSignaler::ErrorData& errorData);

}//namespace Zero
