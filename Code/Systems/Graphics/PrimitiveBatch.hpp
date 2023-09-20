// MIT Licensed (see LICENSE.md).
#pragma once
#include "GraphicsApi/BatchBuffer.hpp"

namespace Raverie
{

/// Templated wrapper around batch buffer.
template <typename VertexType>
class PrimitiveBatcher
{
public:
  void Begin(DrawContext* dc,
             BatchBuffer* buffer,
             const VertexLayouts::Element* layout,
             uint vertexSize,
             PrimitiveType::Enum type)
  {
    mPrimtiveType = type;
    mBuffer = buffer;
    mBuffer->Begin(dc, layout, vertexSize);
    GetBuffer();
  }

  VertexType* CheckFlush(uint count)
  {
    if (mCurrent + count >= mEnd)
      Flush();
    return mCurrent;
  }

  void Move(uint count)
  {
    mCurrent += count;
  }

  void Flush()
  {
    uint vertexCount = mCurrent - mBegin;
    mBuffer->Flush(vertexCount, mPrimtiveType, IndexType::None);
    GetBuffer();
  }

  void End()
  {
    uint vertexCount = mCurrent - mBegin;
    mBuffer->Flush(vertexCount, mPrimtiveType, IndexType::None);
    mBuffer->End();
  }

  void GetBuffer()
  {
    DataBlock block = mBuffer->GetVertexBuffer();
    mBegin = (VertexType*)block.Data;
    mEnd = mBegin + (block.Size / sizeof(VertexType));
    mCurrent = mBegin;
  }

  VertexType* mCurrent;
  VertexType* mBegin;
  VertexType* mEnd;
  BatchBuffer* mBuffer;
  PrimitiveType::Enum mPrimtiveType;
};

} // namespace Raverie
