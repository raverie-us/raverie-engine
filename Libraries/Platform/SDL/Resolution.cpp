// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

Resolution GetDesktopResolution()
{
  SDL_DisplayMode displayInfo;
  if (SDL_GetDesktopDisplayMode(0, &displayInfo) != 0)
  {
    String errorString = SDL_GetError();
    Warn("Failed to get desktop display info: %s", errorString.c_str());
    return Resolution(cMinimumMonitorSize.x, cMinimumMonitorSize.y);
  }

  return Resolution(displayInfo.w, displayInfo.h);
  ;
}

void Enumerate(Array<Resolution>& resolutions, uint bitDepth, Resolution aspect)
{
  HashSet<Resolution> ResolutionMap;
  resolutions.Clear();

  // Returns the window currently with input focus
  SDL_Window* activeWindow = SDL_GetGrabbedWindow();
  // Returns the display device index for the window provided
  int displayIndex = SDL_GetWindowDisplayIndex(activeWindow);
  // Display index is negative on error
  if (displayIndex < 0)
  {
    String errorString = SDL_GetError();
    Warn("Failed to get display index for the active window: %s", errorString.c_str());
    return;
  }

  int totalDisplayModes = SDL_GetNumDisplayModes(displayIndex);
  // Total display modes >= 1 on success
  if (totalDisplayModes < 1)
  {
    String errorString = SDL_GetError();
    Warn("Failed to get total display modes for the active display device: %s", errorString.c_str());
    return;
  }

  // Get all supported display modes for the active display device
  SDL_DisplayMode displayInfo = {SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0};
  for (int i = 0; i < totalDisplayModes; ++i)
  {
    if (SDL_GetDisplayMode(displayIndex, i, &displayInfo) != 0)
    {
      String errorString = SDL_GetError();
      Warn("Failed to get display mode info: %s", errorString.c_str());
      continue;
    }
    Resolution resolution(displayInfo.w, displayInfo.h);
    // Check the bit depth, see SDL_PixelFormatEnum for more detail
    if (bitDepth == SDL_PIXELFORMAT_UNKNOWN || displayInfo.format == bitDepth)
    {
      // Make sure the aspect rations match the
      if (resolution.SameAspect(aspect))
      {
        ResolutionMap.Insert(resolution);
      }
    }
  }

  // Transfer the unique resolutions to the provided array
  PushAll(resolutions, ResolutionMap.All());
  // Sort the resolutions
  Sort(resolutions.All());
}

} // namespace Zero
