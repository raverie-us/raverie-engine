///////////////////////////////////////////////////////////////////////////////
///
/// \file WinUtility.cpp
/// Windows Utility Functions.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "WinUtility.hpp"
#include "Support/Image.hpp"

namespace Zero
{

LPSTR StripQuotes(LPSTR input)
{
  int length = strlen(input);
  if(length > 2)
  {
    char* last = input+(length-1);
    //Eat first quote
    if(*input == '\"')
      ++input;

    //Eat last quote
    if(*last == '\"')
      *last = '\0';
  }
  return input;
}

LPCSTR* CommandLineToArgvA(LPCSTR CmdLine, int* _argc)
{
  LPCSTR* argv;
  PCHAR  _argv;
  ULONG   len;
  ULONG   argc;
  CHAR   a;
  ULONG   i, j;

  BOOLEAN  in_QM;
  BOOLEAN  in_TEXT;
  BOOLEAN  in_SPACE;

  len = strlen(CmdLine);
  i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

  argv = (LPCSTR*)GlobalAlloc(GMEM_FIXED,
    i + (len+2)*sizeof(CHAR));

  _argv = (PCHAR)(((PUCHAR)argv)+i);

  argc = 0;
  argv[argc] = _argv;
  in_QM = FALSE;
  in_TEXT = FALSE;
  in_SPACE = TRUE;
  i = 0;
  j = 0;

  while( a = CmdLine[i] ) {
    if(in_QM) {
      if(a == '\"') {
        in_QM = FALSE;
      } else {
        _argv[j] = a;
        j++;
      }
    } else {
      switch(a) {
      case '\"':
        in_QM = TRUE;
        in_TEXT = TRUE;
        if(in_SPACE) {
          argv[argc] = _argv+j;
          argc++;
        }
        in_SPACE = FALSE;
        break;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        if(in_TEXT) {
          _argv[j] = '\0';
          j++;
        }
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        break;
      default:
        in_TEXT = TRUE;
        if(in_SPACE) {
          argv[argc] = _argv+j;
          argc++;
        }
        _argv[j] = a;
        j++;
        in_SPACE = FALSE;
        break;
      }
    }
    i++;
  }
  _argv[j] = '\0';
  argv[argc] = NULL;

  (*_argc) = argc;
  return argv;
}

PWCHAR* CommandLineToArgvW(PWCHAR CmdLine, int* _argc)
{
  PWCHAR* argv;
  PWCHAR  _argv;
  ULONG   len;
  ULONG   argc;
  WCHAR   a;
  ULONG   i, j;

  BOOLEAN  in_QM;
  BOOLEAN  in_TEXT;
  BOOLEAN  in_SPACE;

  len = wcslen(CmdLine);
  i = ((len + 2) / 2) * sizeof(PVOID) + sizeof(PVOID);

  argv = (PWCHAR*)GlobalAlloc(GMEM_FIXED,
                              i + (len + 2) * sizeof(WCHAR));

  _argv = (PWCHAR)(((PUCHAR)argv) + i);

  argc = 0;
  argv[argc] = _argv;
  in_QM = FALSE;
  in_TEXT = FALSE;
  in_SPACE = TRUE;
  i = 0;
  j = 0;

  while (a = CmdLine[i]) {
    if (in_QM) {
      if (a == '\"') {
        in_QM = FALSE;
      }
      else {
        _argv[j] = a;
        j++;
      }
    }
    else {
      switch (a) {
      case '\"':
        in_QM = TRUE;
        in_TEXT = TRUE;
        if (in_SPACE) {
          argv[argc] = _argv + j;
          argc++;
        }
        in_SPACE = FALSE;
        break;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        if (in_TEXT) {
          _argv[j] = '\0';
          j++;
        }
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        break;
      default:
        in_TEXT = TRUE;
        if (in_SPACE) {
          argv[argc] = _argv + j;
          argc++;
        }
        _argv[j] = a;
        j++;
        in_SPACE = FALSE;
        break;
      }
    }
    i++;
  }
  _argv[j] = '\0';
  argv[argc] = NULL;

  (*_argc) = argc;
  return argv;
}

void CreateBitmapBuffer(Image* image, byte*& outputBuffer, uint& outSize)
{
  //Create a bitmap in memory
  uint width = image->Width;
  uint height = image->Height;
  //Each line is padded
  uint pitch = (3 * width + 3) & ~3;

  byte* sourceData = (byte*)image->Data;

  //A Bitmap is BITMAPFILEHEADER a BITMAPINFOHEADER and 
  //then the padded image bytes.
  uint headerSize = sizeof(BITMAPFILEHEADER);
  uint infoSize = sizeof(BITMAPINFOHEADER);
  uint imageBytes = pitch * height;

  uint bufferSize = imageBytes+headerSize+infoSize;
  byte* bitmapBuffer = (byte*)zAllocate(bufferSize);

  //Offset to the image data
  byte* imageData = bitmapBuffer+headerSize+infoSize;

  //Create the header sections
  BITMAPFILEHEADER* fileHeader = (BITMAPFILEHEADER*)(bitmapBuffer);
  BITMAPINFOHEADER* infoHeader = (BITMAPINFOHEADER*)(bitmapBuffer+headerSize);

  //Fill in the BITMAPFILEHEADER
  ZeroMemory(fileHeader, sizeof(BITMAPFILEHEADER));
  fileHeader->bfType = 'MB';
  fileHeader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  fileHeader->bfSize = bufferSize;

  //Fill in BITMAPINFOHEADER
  ZeroMemory(infoHeader, sizeof(BITMAPINFOHEADER));
  infoHeader->biSize = sizeof(BITMAPINFOHEADER);
  infoHeader->biWidth = width;
  infoHeader->biHeight = height;
  infoHeader->biPlanes = 1;
  infoHeader->biBitCount = 24;
  infoHeader->biCompression = BI_RGB;

  //Copy over pixels from image source to the bitmap
  //pixels
  for(uint line=0;line<height;++line)
  {
    //All lines are flipped
    byte* lineStart = imageData + (pitch * (height-line-1));
    for(uint p=0;p<width;++p)
    {
      byte* dest = lineStart + p * 3;
      byte* source = sourceData + (line * width + p)*4;
      //RGB is converted to BGR
      dest[0] = source[2];
      dest[1] = source[1];
      dest[2] = source[0];
    }
  }

  //Store the data
  outputBuffer = bitmapBuffer;
  outSize = bufferSize;
}

}

