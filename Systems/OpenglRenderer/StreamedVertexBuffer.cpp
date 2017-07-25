#include "Precompiled.hpp"

namespace Zero
{

void StreamedVertexBuffer::Initialize()
{
  mBufferSize = 1 << 18; // 256Kb, 1213 sprites at 216 bytes per sprite
  mCurrentBufferOffset = 0;

  glGenVertexArrays(1, &mVertexArray);
  glBindVertexArray(mVertexArray);

  glGenBuffers(1, &mVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, mBufferSize, nullptr, GL_STREAM_DRAW);

  glEnableVertexAttribArray(VertexSemantic::Position);
  glVertexAttribPointer(VertexSemantic::Position, 3, GL_FLOAT, GL_FALSE, sizeof(StreamedVertex), (void*)offsetof(StreamedVertex, mPosition));
  glEnableVertexAttribArray(VertexSemantic::Uv);
  glVertexAttribPointer(VertexSemantic::Uv, 2, GL_FLOAT, GL_FALSE, sizeof(StreamedVertex), (void*)offsetof(StreamedVertex, mUv));
  glEnableVertexAttribArray(VertexSemantic::Color);
  glVertexAttribPointer(VertexSemantic::Color, 4, GL_FLOAT, GL_FALSE, sizeof(StreamedVertex), (void*)offsetof(StreamedVertex, mColor));
  glEnableVertexAttribArray(VertexSemantic::UvAux);
  glVertexAttribPointer(VertexSemantic::UvAux, 2, GL_FLOAT, GL_FALSE, sizeof(StreamedVertex), (void*)offsetof(StreamedVertex, mUvAux));

  glBindVertexArray(0);

  mPrimitiveType = PrimitiveType::Triangles;
  mActive = false;
}

void StreamedVertexBuffer::Destroy()
{
  glDeleteBuffers(1, &mVertexBuffer);
  glDeleteVertexArrays(1, &mVertexArray);
}

void StreamedVertexBuffer::AddVertices(StreamedVertex* vertices, uint count, PrimitiveType::Enum primitiveType)
{
  if (!mActive)
  {
    glBindVertexArray(mVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    mActive = true;
  }

  if (primitiveType != mPrimitiveType)
  {
    FlushBuffer(false);
    mPrimitiveType = primitiveType;
  }

  uint uploadSize = sizeof(StreamedVertex) * count;
  if (mCurrentBufferOffset + uploadSize > mBufferSize)
  {
    FlushBuffer(false);
    // If upload size is larger than the entire buffer then break it into multiple draws
    while (uploadSize > mBufferSize)
    {
      uint verticesPerPrimitive = primitiveType == PrimitiveType::Lines ? 2 : 3;
      uint primitiveSize = sizeof(StreamedVertex) * verticesPerPrimitive;
      uint maxPrimitiveCount = mBufferSize / primitiveSize;
      uint maxByteCount = maxPrimitiveCount * primitiveSize;

      mCurrentBufferOffset = maxByteCount;
      glBufferSubData(GL_ARRAY_BUFFER, 0, mCurrentBufferOffset, vertices);
      FlushBuffer(false);
      // Move pointer forward by byte count, below condition will grab this new value
      vertices = (StreamedVertex*)((char*)vertices + maxByteCount);
      uploadSize -= maxByteCount;
    }
  }

  glBufferSubData(GL_ARRAY_BUFFER, mCurrentBufferOffset, uploadSize, vertices);
  mCurrentBufferOffset += uploadSize;
}

void StreamedVertexBuffer::FlushBuffer(bool deactivate)
{
  if (mCurrentBufferOffset > 0)
  {
    if (mPrimitiveType == PrimitiveType::Triangles)
      glDrawArrays(GL_TRIANGLES, 0, mCurrentBufferOffset / sizeof(StreamedVertex));
    else if (mPrimitiveType == PrimitiveType::Lines)
      glDrawArrays(GL_LINES, 0, mCurrentBufferOffset / sizeof(StreamedVertex));
    mCurrentBufferOffset = 0;
  }

  if (deactivate && mActive)
  {
    glBindVertexArray(0);
    glLineWidth(1.0f);
    mActive = false;
  }
}

} // namespace Zero
