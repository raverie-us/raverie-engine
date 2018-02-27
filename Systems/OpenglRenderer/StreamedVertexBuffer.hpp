// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#pragma once

namespace Zero
{

class StreamedVertexBuffer
{
public:
  void Initialize();
  void Destroy();

  void AddVertices(StreamedVertex* vertices, uint count, PrimitiveType::Enum primitiveType);
  void FlushBuffer(bool deactivate);

  uint mBufferSize;
  GLuint mVertexArray;
  GLuint mVertexBuffer;

  uint mCurrentBufferOffset;

  PrimitiveType::Enum mPrimitiveType;
  bool mActive;
};

} // namespace Zero
