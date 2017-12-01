///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

String mFragmentExtension = "zilchfrag";

String RenderResults::mZilchKey = "Zilch";

//-------------------------------------------------------------------FragmentInfo
FragmentInfo::FragmentInfo()
{

}

FragmentInfo::FragmentInfo(StringParam filePath)
{
  mFilePath = filePath;
  mFragmentCode = ReadFileIntoString(filePath);
}

FragmentInfo::FragmentInfo(StringParam filePath, StringParam fragmentCode)
{
  mFilePath = filePath;
  mFragmentCode = fragmentCode;
}

//-------------------------------------------------------------------BaseRenderer
UniformByteBuffer::UniformByteBuffer()
{
  mData = nullptr;
  mSize = 0;
}

UniformByteBuffer::UniformByteBuffer(const UniformByteBuffer& rhs)
{
  CopyFrom(rhs);
}

UniformByteBuffer::~UniformByteBuffer()
{
  Clear();
}

UniformByteBuffer& UniformByteBuffer::operator=(const UniformByteBuffer& rhs)
{
  if(&rhs == this)
    return *this;

  CopyFrom(rhs);
  return *this;
}

void UniformByteBuffer::Set(byte* data, size_t size)
{
  Clear();
  mSize = size;
  mData = new byte[mSize]();
  memcpy(mData, data, mSize);
}

void UniformByteBuffer::CopyFrom(const UniformByteBuffer& rhs)
{
  Set(rhs.mData, rhs.mSize);
}

void UniformByteBuffer::Clear()
{
  delete mData;
  mData = nullptr;
  mSize = 0;
}

//-------------------------------------------------------------------BaseRenderer
BaseRenderer::BaseRenderer()
{
  // buffer for fullscreen triangle
  mFullScreenTriangleVerts[0] = {Vec3(-1, 3, 0), Vec2(0, -1), Vec4()};
  mFullScreenTriangleVerts[1] = {Vec3(-1, -1, 0), Vec2(0, 1), Vec4()};
  mFullScreenTriangleVerts[2] = {Vec3(3, -1, 0), Vec2(2, 1), Vec4()};
}

//-------------------------------------------------------------------RendererPackage
RendererPackage::RendererPackage()
{
  mRenderer = nullptr;
  mErrorReporter = nullptr;
  mBackend = nullptr;
}
