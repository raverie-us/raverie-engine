///////////////////////////////////////////////////////////////////////////////
///
/// \file StreamTexture.hpp
/// Declares the Texture class and Manager.
/// 
/// Authors: Clifford Garvis, Wylder Keane
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Engine/EngineContainers.hpp"
#include "Engine/Resource.hpp"
#include "Graphics/GraphicsApi/HardwareBuffer.hpp"
#include "Texture.hpp"

namespace Zero
{

///StreamTexture resource class.
class StreamTexture : public Texture
{
public:
  StreamTexture(StringRef id, s32 width, s32 height);
  void Write(const byte* imageData);
private:
  //hardware buffers
  HardwareBuffer mBuffer[2];
  u32 mWriteIndex;
};

}