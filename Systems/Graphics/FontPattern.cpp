#include "Precompiled.hpp"

namespace Zero
{

FontProcessor::FontProcessor(RenderQueues* renderQueues, ViewNode* viewNode, Vec4 vertexColor)
  : mRenderQueues(renderQueues)
  , mViewNode(viewNode)
  , mVertexColor(vertexColor)
{
}

void FontProcessor::ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale)
{
  Vec2 pos0 = position + rune.Offset * pixelScale;
  Vec2 pos1 = pos0 + rune.Size * pixelScale;
  Vec2 uv0 = rune.Rect.TopLeft;
  Vec2 uv1 = rune.Rect.BotRight;

  mRenderQueues->AddStreamedQuad(*mViewNode, Vec3(pos0, 0), Vec3(pos1, 0), uv0, uv1, mVertexColor);
}

FontProcessorVertexArray::FontProcessorVertexArray(Vec4 vertexColor)
  : mVertexColor(vertexColor)
{
}

void FontProcessorVertexArray::ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale)
{
  Vec2 pos0 = position + rune.Offset * pixelScale;
  Vec2 pos1 = pos0 + rune.Size * pixelScale;
  Vec2 uv0 = rune.Rect.TopLeft;
  Vec2 uv1 = rune.Rect.BotRight;

  StreamedVertex v0(Vec3(pos0),              uv0,                mVertexColor);
  StreamedVertex v1(Vec3(pos0.x, pos1.y, 0), Vec2(uv0.x, uv1.y), mVertexColor);
  StreamedVertex v2(Vec3(pos1),              uv1,                mVertexColor);
  StreamedVertex v3(Vec3(pos1.x, pos0.y, 0), Vec2(uv1.x, uv0.y), mVertexColor);

  mVertices.PushBack(v0);
  mVertices.PushBack(v1);
  mVertices.PushBack(v2);
  mVertices.PushBack(v2);
  mVertices.PushBack(v3);
  mVertices.PushBack(v0);
}

FontProcessorFindCharPosition::FontProcessorFindCharPosition(int charIndex, Vec2 startPositon)
  : mFindIndex(charIndex)
  , mCurrentIndex(0)
  , mCharPosition(startPositon)
{
}

void FontProcessorFindCharPosition::ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale)
{
  // If we never make it to the requested index then we want the position
  // to be after the last character we encountered
  if (mCurrentIndex < mFindIndex)
    mCharPosition = position + Vec2(rune.Advance, 0) * pixelScale;
  else if (mCurrentIndex == mFindIndex)
    mCharPosition = position;

  ++mCurrentIndex;
}

}
