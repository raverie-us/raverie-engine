#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------- Character Index
ZilchDefineType(CharacterIndex, builder, type)
{
  ZeroBindDocumented();
  ZilchBindDefaultCopyDestructor();
  ZilchBindConstructor(int);
  ZilchBindConstructor(int, int, bool);
  ZilchBindMember(mIndex);
  ZilchBindMember(mLine);
  ZilchBindMember(mNextLine);
  type->CreatableInScript = true;
}

CharacterIndex::CharacterIndex()
  : mIndex(0)
  , mLine(0)
  , mNextLine(true)
{
}

CharacterIndex::CharacterIndex(int index)
  : mIndex(index)
  , mLine(0)
  , mNextLine(true)
{
}

CharacterIndex::CharacterIndex(int index, int line, bool nextLine)
  : mIndex(index)
  , mLine(line)
  , mNextLine(nextLine)
{
}

//--------------------------------------------------------------- Font Processor
FontProcessor::FontProcessor(RenderQueues* renderQueues, ViewNode* viewNode, Vec4 vertexColor)
  : mRenderQueues(renderQueues)
  , mViewNode(viewNode)
  , mVertexColor(vertexColor)
{
}

void FontProcessor::ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale, int line)
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

void FontProcessorVertexArray::ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale, int line)
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

FontProcessorFindCharPosition::FontProcessorFindCharPosition(int charIndex, bool nextLine, Vec2 startPositon)
  : mFindIndex(charIndex)
  , mNextLine(nextLine)
  , mCurrentIndex(0)
  , mCharPosition(startPositon)
  , mLastLine(0)
{
}

void FontProcessorFindCharPosition::ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale, int line)
{
  // If we never make it to the requested index then we want the position
  // to be after the last character we encountered
  if (mCurrentIndex < mFindIndex)
  {
    mCharPosition = position + Vec2(rune.Advance, 0) * pixelScale;
  }
  else if (mCurrentIndex == mFindIndex)
  {
    if (line == mLastLine || mNextLine)
      mCharPosition = position;
  }

  mLastLine = line;
  ++mCurrentIndex;
}

FontProcessorOutputPositions::FontProcessorOutputPositions(Array<Vec3>* worldPositions, Transform* transform)
  : mWorldPositions(worldPositions)
  , mTransform(transform)
{
}

void FontProcessorOutputPositions::ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale, int line)
{
  Vec3 worldPos = mTransform->TransformPoint(Vec3(position, 0.0f));
  mWorldPositions->PushBack(worldPos);
}

FontProcessorFindCharIndex::FontProcessorFindCharIndex(Vec2Param localPosition, float lineHeight)
  : mLocalPosition(localPosition)
  , mLineHeight(lineHeight)
  , mClosestDistance(Math::PositiveMax())
  , mCurrentIndex(0)
  , mLastLine(0)
{
}

void FontProcessorFindCharIndex::ProcessRenderRune(RenderRune& rune, Vec2 position, Vec2 pixelScale, int line)
{
  Vec2 center = position + Vec2(rune.Advance, mLineHeight) * 0.5f * pixelScale;

  Vec2 offset = mLocalPosition - center;
  float manhattenDistance = Math::Abs(offset.x) + Math::Abs(offset.y);

  if (manhattenDistance < mClosestDistance)
  {
    mClosestDistance = manhattenDistance;
    mResult.mLine = line;
    mResult.mNextLine = false;

    // We divide the character in half and add 1 if it's on the right side
    if (offset.x < 0)
    {
      mResult.mIndex = mCurrentIndex;

      // Are we just entering the next line and on the left hand side?
      if (line != mLastLine)
        mResult.mNextLine = true;
    }
    else
    {
      mResult.mIndex = mCurrentIndex + 1;
    }
  }

  mLastLine = line;
  ++mCurrentIndex;
}

}
