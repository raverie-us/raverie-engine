///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------ File Dialog

// Documentation says the character limit is 32k limit for ANSI
// http://msdn.microsoft.com/en-us/library/ms646927(v=vs.85).aspx
// only using a fourth of the limit
const int cFileBufferSize = 8192;

void FileDialog(OsHandle windowHandle, FileDialogConfig& config, bool opening)
{
  //Create Windows OPENFILENAME structure and zero it
  OPENFILENAME fileDialog = {0};
  fileDialog.lStructSize = sizeof (OPENFILENAME);
  fileDialog.hwndOwner = (HWND)windowHandle;

  //Set up buffers.
  wchar_t fileName[cFileBufferSize]  = {0};
  fileDialog.lpstrFile = fileName;
  fileDialog.nMaxFile = cFileBufferSize;

  wchar_t fileFilter[cFileBufferSize] = {0};
  fileDialog.lpstrFilter = fileFilter;

  wchar_t* fileFilterPosition = fileFilter;
  forRange(FileDialogFilter& fileFilter, config.mSearchFilters.All())
  {
    WString description = Widen(fileFilter.mDescription);
    WString filter = Widen(fileFilter.mFilter);
    ZeroStrCpyW(fileFilterPosition, description.Size(), description.c_str());
    fileFilterPosition += description.Size();
    ZeroStrCpyW(fileFilterPosition, filter.Size(), filter.c_str());
    fileFilterPosition += filter.Size();
  }

  // File filters are double null terminated, so add one more at the end
  WString nullChar = Widen("\0");
  ZeroStrCpyW(fileFilterPosition, 1, nullChar.c_str());

  //Set the default file name
  WString defaultFileName = Widen(config.DefaultFileName);
  ZeroStrCpyW(fileName, cFileBufferSize, defaultFileName.c_str());

  //Object containing the data is needed to properly keep it alive while the dialog is open
  WString title = Widen(config.Title);
  WString startingDirectory = Widen(config.StartingDirectory);

  fileDialog.lpstrFileTitle = NULL;
  fileDialog.lpstrInitialDir = startingDirectory.c_str();
  fileDialog.lpstrTitle = title.c_str();

  //Extensions is ".ext" so skip one character
  if (!config.mDefaultSaveExtension.Empty())
  {
    WString defaultExtension = Widen(config.mDefaultSaveExtension);
    fileDialog.lpstrDefExt = defaultExtension.c_str();
  }

  //Use explorer interface
  fileDialog.Flags |= OFN_EXPLORER;

  if(config.Flags & FileDialogFlags::Folder)
  {
    ZeroStrCpyW(fileName, cFileBufferSize, L"Folder");
  }

  if(config.Flags & FileDialogFlags::MultiSelect)
    fileDialog.Flags |= OFN_ALLOWMULTISELECT;

  BOOL success = 0;
  if(opening)
    success = GetOpenFileName(&fileDialog);
  else
    success = GetSaveFileName(&fileDialog);

  OsFileSelection fileEvent;
  fileEvent.EventName = config.EventName;
  fileEvent.Success = success!=0;

  if(success)
  {
    //If nFileExtension is zero, there is multiple files 
    //from a multiselect
    if(fileDialog.nFileExtension == 0 && 
       config.Flags & FileDialogFlags::MultiSelect)
    {
      //lpstrFile will be a multi null terminated string
      //beginning with the directory name then a list of
      //files and finally a null terminator. (so two at the end)
      wchar_t* current = fileDialog.lpstrFile;

      String directoryName;

      //Check for null termination
      //or double null termination which signals the 
      //end of the file list
      while(*current != '\0')
      {
        cwstr currentFile = current;

        //On the first loop this sets the directory name
        if(directoryName.Empty())
          directoryName = Narrow(currentFile);
        else
        {
          //Append a full file path Append the file name to the path
          fileEvent.Files.PushBack(FilePath::Combine(directoryName, Narrow(currentFile)));
        }

        //Find null terminator
        while(*current != '\0')
          ++current;
        //Skip the terminator
        ++current;
      }
    }
    else
    {
      fileEvent.Files.PushBack(Narrow(fileDialog.lpstrFile).c_str());
    }
  }
  
  //We assume that the first item is the path if we were multi selecting, but if the user
  //types in a random string then we can get something that looks like a path (no extension)
  //and then get no files. So if we didn't get any files for some reason then mark that we actually failed.
  if(fileEvent.Files.Size() == 0)
  {
    fileEvent.Success = false;
  }

  if(config.CallbackObject)
  {
    EventDispatcher* dispatcher = config.CallbackObject->GetDispatcherObject();
    dispatcher->Dispatch(config.EventName, &fileEvent);
  }
}

//-------------------------------------------------------------------- Clipboard
String ShellGetClipboardText(OsHandle windowHandle)
{
  if(!IsClipboardFormatAvailable(CF_UNICODETEXT))
    return String();
  if(!OpenClipboard((HWND)windowHandle))
    return String();

  LPWSTR clipboardCstr = NULL;

  //Get OS global handle
  HGLOBAL globalHandle = GetClipboardData(CF_UNICODETEXT);
  if(globalHandle != NULL) 
  {
    clipboardCstr = (LPWSTR)GlobalLock(globalHandle);
    if(clipboardCstr != NULL) 
    {
      GlobalUnlock(globalHandle);
    }
  } 
  CloseClipboard();
  return Narrow(clipboardCstr);
}

void ShellSetClipboardText(OsHandle windowHandle, StringRange text)
{
  WString wideText = Widen(text);
  if(!OpenClipboard((HWND)windowHandle))
    return;
  EmptyClipboard();

  uint numCharacters = wideText.SizeInBytes();
  // Allocate a global memory object for the text.

  HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (numCharacters + 1) * sizeof(wchar_t));
  if(hglbCopy == NULL)
  {
    CloseClipboard();
    return;
  }

  // Lock the handle and copy the text to the buffer.
  wchar_t* data = (wchar_t*)GlobalLock(hglbCopy);
  memcpy(data, wideText.Data(), wideText.SizeInBytes());

  //Null terminate the buffer
  data[numCharacters] = (wchar_t)0;
  GlobalUnlock(hglbCopy);

  // Place the handle on the clipboard.
  SetClipboardData(CF_UNICODETEXT, hglbCopy);

  CloseClipboard();
}


// Convert bitmap to image. Hdc is hdc bitmap was created in
void ConvertImage(HDC hdc, HBITMAP bitmapHandle, Image* image)
{
  BITMAP bmpScreen;
  GetObject(bitmapHandle, sizeof(BITMAP), &bmpScreen);

  BITMAPINFOHEADER bi;
  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = bmpScreen.bmWidth;
  bi.biHeight = bmpScreen.bmHeight;
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  // Row size is padded to 32 bits
  DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;
  byte* rawBitmapData = (byte*)zAllocate(dwBmpSize) ;

  // Gets the "bits" from the bitmap and copies them into a buffer 
  // which is pointed to by lpbitmap.
  GetDIBits(hdc, bitmapHandle, 0,
    (UINT)bmpScreen.bmHeight, rawBitmapData,
    (BITMAPINFO *)&bi, DIB_RGB_COLORS);

  // Allocate the image and copy over pixels
  image->Allocate(bmpScreen.bmWidth, bmpScreen.bmHeight);

  for(int y=0;y<bmpScreen.bmHeight;++y)
  {
    for(int x=0;x<bmpScreen.bmWidth;++x)
    {
      byte* data = rawBitmapData + bmpScreen.bmWidth * 4 * (bmpScreen.bmHeight-y-1) + x * 4;
      image->GetPixel(x, y) = ByteColorRGBA(data[2], data[1], data[0], 255);
    }
  }
}

bool ShellIsClipboardImageAvailable(OsHandle windowHandle)
{
  if(!OpenClipboard((HWND)windowHandle)) 
    return false;

  bool result = (IsClipboardFormatAvailable(CF_BITMAP) != 0);
  CloseClipboard();
  return result;
}

bool ShellGetClipboardImage(OsHandle windowHandle, Image* image)
{
  if(!OpenClipboard((HWND)windowHandle)) 
    return false;
  if(!IsClipboardFormatAvailable(CF_BITMAP))
  {
    CloseClipboard();
    return false;
  }

  HBITMAP hbmScreen  = (HBITMAP)GetClipboardData(CF_BITMAP);
  HDC hdcScreen = GetDC(NULL);
  ConvertImage(hdcScreen, hbmScreen, image);

  CloseClipboard();
  return true;
}

bool ShellGetDesktopImage(Image* image)
{
  return ShellGetWindowImage(GetDesktopWindow(), image);
}

bool ShellGetWindowImage(OsHandle windowHandle, Image* image)
{
  RECT windowRect;
  GetWindowRect((HWND)windowHandle, &windowRect);

  int width = windowRect.right - windowRect.left;
  int height = windowRect.bottom - windowRect.top;

  HDC hdcScreen = GetDC(NULL);

  HDC hdcDest = CreateCompatibleDC(hdcScreen);
  HBITMAP hBitmap  = CreateCompatibleBitmap(hdcScreen, width, height);

  // Select the compatible bitmap into the compatible memory DC.
  HGDIOBJ old = SelectObject(hdcDest, hBitmap);

  // Bit block transfer into our compatible memory DC.
  if(!BitBlt(hdcDest, 
    0,
    0,
    width, 
    height, 
    hdcScreen, 
    windowRect.left, windowRect.top,
    SRCCOPY))
  {
    return false;
  }

  SelectObject(hdcDest, old);
  ConvertImage(hdcScreen, hBitmap, image);
  DeleteDC(hdcDest);
  ReleaseDC((HWND)windowHandle, hdcScreen);

  DeleteObject(hBitmap);

  return true;
}


DeclareEnum4(ReturnCode, Continue, DebugBreak, Terminate, Ignore);

bool WindowsErrorProcessHandler(ErrorSignaler::ErrorData& errorData) 
{
  char commandLine[4096];
  char* message = (char*)errorData.Message;

  //No message provided
  if(message==NULL)
    message = "No message";

  ZPrint("%s\n", message);

  uint msgLength = strlen(message);

  //Remove quotes from message
  for(uint i = 0; i < msgLength; ++i)
  {
    if(message[i] == '"')
      message[i] = '\'';
  }

  ZeroSPrintf(commandLine, "ErrorDialog.exe \"%s\" \"%s\" \"%s:%d\" %s", 
          message, errorData.Expression, errorData.File, errorData.Line, 
          "Default");

  STARTUPINFO startUpInfo;
  memset(&startUpInfo, 0, sizeof(startUpInfo));

  PROCESS_INFORMATION processInfo;
  memset(&processInfo, 0, sizeof(processInfo));

  // Start the child process.
  CreateProcess(NULL,                 // No module name (use command line)
                (LPTSTR)Widen(commandLine).c_str(),  // Command line
                NULL,                 // Process handle not inheritable
                NULL,                 // Thread handle not inheritable
                FALSE,                // Set handle inheritance to FALSE
                CREATE_NO_WINDOW,     // Creation flags
                NULL,                 // Use parent's environment block
                NULL,                 // Use parent's starting directory
                &startUpInfo,         // Pointer to STARTUPINFO structure
                &processInfo);

  DWORD exitCode = 0;

  WaitForSingleObject(processInfo.hProcess, INFINITE);
  BOOL success = GetExitCodeProcess(processInfo.hProcess, &exitCode);

  //Failed debug break
  if(!success)
    return true;

  ReturnCode::Enum returnCode = (ReturnCode::Enum)exitCode;

  //Exit code 3 is ignore
  switch (returnCode)
  {
    case ReturnCode::Continue:
      return false;
    case ReturnCode::DebugBreak:
      return true;
    case ReturnCode::Terminate:
      TerminateProcess(GetCurrentProcess(), 0);
      return false;
    case ReturnCode::Ignore:
      errorData.IgnoreFutureAssert = true;
      return false;
  }

  //Debug break if invalid code
  return true;
}

}//namespace Zero
