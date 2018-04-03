///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
//-----------------------------------------------------------FileDialogFilter
DeclareBitField2(FileDialogFlags, MultiSelect, Folder);

struct FileDialogFilter
{
  FileDialogFilter();
  FileDialogFilter(StringParam filter);
  FileDialogFilter(StringParam description, StringParam filter);

  String mDescription;
  // i.e. "*.fbx"
  String mFilter;
};

typedef void(*FileDialogCallback)(Array<String>& files, bool success, void* userData);

/// FileDialogConfig is used to configure the Open File Dialog and the Save File Dialog.
struct FileDialogSetup
{
  FileDialogSetup();
  void AddFilter(StringParam description, StringParam filter);

  String Title;
  String StartingDirectory;
  Array<FileDialogFilter> mSearchFilters;
  // Should not include '.'
  String mDefaultSaveExtension;
  String DefaultFileName;
  FileDialogFlags::Type Flags;
  FileDialogCallback mCallback;
  void* mUserData;
};

// Returns true if we successfully choose one or more files, or false otherwise
bool FileDialog(OsHandle windowHandle, FileDialogSetup& config, bool opening);

void ShellSetClipboardText(OsHandle windowHandle, StringRange text);
String ShellGetClipboardText(OsHandle windowHandle);

bool ShellGetClipboardImage(OsHandle windowHandle, Image* imageBuffer);
bool ShellIsClipboardImageAvailable(OsHandle windowHandle);

bool ShellGetWindowImage(OsHandle windowHandle, Image* imageBuffer);
bool ShellGetDesktopImage(byte** imageOut, Image* imageBuffer);

}//namespace Zero
