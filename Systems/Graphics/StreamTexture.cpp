///////////////////////////////////////////////////////////////////////////////
///
/// \file StreamTexture.cpp
/// Stream texture is a special texture for constantly updated textures
/// through a PBO
///
/// Authors: Clifford Garvis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Graphics/StreamTexture.hpp"
#include "Graphics/GraphicsApi/Gl/OpenGlSupport.hpp"

namespace Zero
{
const uint channelCount = 3;

StreamTexture::StreamTexture(StringRef id, s32 width, s32 height)
{
  this->SetUpTexture(TextureFormat::RGBA8, width, height, 0, TextureType::Tex2D);

  mWriteIndex = 0;

  //initialize buffers with no data to just reserve space.
  mBuffer[0].Initialize(HardwareBuffer::PixelData, HardwareBuffer::StreamBuffer,
                        width * height * channelCount, 0);
  mBuffer[1].Initialize(HardwareBuffer::PixelData, HardwareBuffer::DynamicBuffer,
                        width * height * channelCount, 0);
}

GLuint UploadFormat(LoadFormat::Enum loadFormat);
void ConvertToOpenGlFormatElement(TextureFormat::Enum format, uint& internalFormat, uint& pixelType);

//Write
void StreamTexture::Write(const byte* imageData)
{
  //Bind the current buffer and texture
  mBuffer[mWriteIndex % 2].Bind();
  Bind();

  GLuint internalFormat;
  GLuint pixelType;
  GLuint uploadFormat = UploadFormat(LoadFormat::RGB8);
  ConvertToOpenGlFormatElement(TextureFormat::RGB8, internalFormat, pixelType);

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, uploadFormat, pixelType, ((byte*)NULL));
  
  Unbind();
  mBuffer[mWriteIndex % 2].Unbind();

  //Upload new data to second buffer
  mBuffer[++mWriteIndex % 2].Upload(mWidth*mHeight*channelCount, imageData);
}

}//namespace Zero
