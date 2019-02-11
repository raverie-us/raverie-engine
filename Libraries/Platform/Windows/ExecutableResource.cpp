// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

bool GetExecutableResource(const char* name,
                           const char* type,
                           ByteBufferBlock& output)
{
  HRSRC resource =
      FindResource(nullptr, Widen(name).c_str(), Widen(type).c_str());
  if (resource == INVALID_HANDLE_VALUE)
    return false;

  DWORD size = SizeofResource(nullptr, resource);

  HGLOBAL resoureData = LoadResource(nullptr, resource);
  if (!resoureData)
    return false;

  byte* data = (byte*)LockResource(resoureData);

  output.SetData(data, size, false);
  return true;
}

ExecutableResourceUpdater::ExecutableResourceUpdater(Status& status,
                                                     const char* fileName)
{
  mHandle = BeginUpdateResource(Widen(fileName).c_str(), FALSE);

  if (!mHandle)
    FillWindowsErrorStatus(status);
}

ExecutableResourceUpdater::~ExecutableResourceUpdater()
{
  EndUpdateResource(mHandle, FALSE);
}

void ExecutableResourceUpdater::Update(const char* name,
                                       const char* type,
                                       const byte* data,
                                       size_t size)
{
  BOOL result = UpdateResource((HANDLE)mHandle,
                               Widen(type).c_str(),
                               Widen(name).c_str(),
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                               (void*)data,
                               size);
  ErrorIf(
      !result,
      "UpdateResource should never fail unless a parameter was invalid/null");
}


#pragma pack(push, 2)

// Icon Entry Structure in .ico file
typedef struct
{
  BYTE bWidth;         // Width, in pixels, of the image
  BYTE bHeight;        // Height, in pixels, of the image
  BYTE bColorCount;    // Number of colors in image (0 if >=8bpp)
  BYTE bReserved;      // Reserved ( must be 0)
  WORD wPlanes;        // Color Planes
  WORD wBitCount;      // Bits per pixel
  DWORD dwBytesInRes;  // How many bytes in this resource?
  DWORD dwImageOffset; // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

// Icon Directory Structure in .ico file
typedef struct
{
  WORD idReserved;           // Reserved (must be 0)
  WORD idType;               // Resource Type (1 for icons)
  WORD idCount;              // How many images?
  ICONDIRENTRY idEntries[1]; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;
const uint IconDirSize = sizeof(ICONDIR) - sizeof(ICONDIRENTRY);

// Group Icon Entry structure in Resource section
typedef struct
{
  BYTE bWidth;        // Width, in pixels, of the image
  BYTE bHeight;       // Height, in pixels, of the image
  BYTE bColorCount;   // Number of colors in image (0 if >=8bpp)
  BYTE bReserved;     // Reserved
  WORD wPlanes;       // Color Planes
  WORD wBitCount;     // Bits per pixel
  DWORD dwBytesInRes; // how many bytes in this resource?
  WORD nID;           // the ID
} GRPICONDIRENTRY, *LPGRPICONDIRENTRY;

// Group Icon Directory structure in Resource section
typedef struct
{
  WORD idReserved;              // Reserved (must be 0)
  WORD idType;                  // Resource type (1 for icons)
  WORD idCount;                 // How many images?
  GRPICONDIRENTRY idEntries[1]; // The entries for each image
} GRPICONDIR, *LPGRPICONDIR;
uint GroupIconSize = sizeof(GRPICONDIR) - sizeof(GRPICONDIRENTRY) * 1;

#pragma pack(pop)

void ExecutableResourceUpdater::UpdateIcon(const byte* buffer, size_t size)
{
  /// Add/Update a icon resources from the icon file.
  ICONDIR& iconDir = *(ICONDIR*)buffer;

  uint iconCount = iconDir.idCount;

  // Offset the new icon ids starting at one.
  const uint cIconOffset = 1;

  for (uint i = 0; i < iconCount; ++i)
  {
    // Add the new icon to resources
    BOOL result =
        UpdateResource(mHandle,
                       RT_ICON,
                       MAKEINTRESOURCE(i + cIconOffset),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                       (LPVOID)(buffer + iconDir.idEntries[i].dwImageOffset),
                       iconDir.idEntries[i].dwBytesInRes);

    CheckWin(result, "Failed to update resource");
  }

  // Compute the size of the group index structure
  uint groupIconSize = GroupIconSize + iconCount * sizeof(GRPICONDIRENTRY);

  // Now the GRPICONDIR resource has to be updated to refer to the new icons.
  // Build a new group directory and update the icon resource.
  GRPICONDIR& groupIcon = *(GRPICONDIR*)alloca(groupIconSize);
  ZeroMemory(&groupIcon, sizeof(groupIcon));
  groupIcon.idType = 1;
  groupIcon.idCount = iconCount;

  // Copy over icon data info
  for (uint i = 0; i < iconCount; ++i)
  {
    GRPICONDIRENTRY& entry = groupIcon.idEntries[i];
    ICONDIRENTRY& iconEntry = iconDir.idEntries[i];
    entry.bReserved = 0;
    entry.bWidth = iconEntry.bWidth;
    entry.bHeight = iconEntry.bHeight;
    entry.bColorCount = iconEntry.bColorCount;
    entry.wPlanes = iconEntry.wPlanes;
    entry.wBitCount = iconEntry.wBitCount;
    entry.dwBytesInRes = iconEntry.dwBytesInRes;
    entry.nID = i + cIconOffset;
  }

  // Update the resource
  // Note: If explorer is looking at the exe when this happens it can fail
  // and/or have cached the old icons.
  BOOL result = UpdateResource(mHandle,
                               RT_GROUP_ICON,
                               L"MAINICON",
                               MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
                               &groupIcon,
                               groupIconSize);

  CheckWin(result, "Failed to update resource");
}

} // namespace Zero
