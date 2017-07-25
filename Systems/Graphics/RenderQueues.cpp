#include "Precompiled.hpp"

namespace Zero
{

void FrameNode::Extract(FrameBlock& frameBlock)
{
  mGraphicalEntry->mData->mGraphical->ExtractFrameData(*this, frameBlock);
}

void ViewNode::Extract(ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  mGraphicalEntry->mData->mGraphical->ExtractViewData(*this, viewBlock, frameBlock);
}

void RenderQueues::Clear()
{
  mFrameBlocks.Clear();
  mViewBlocks.Clear();

  mStreamedVertices.Clear();

  mSkinningBuffer.Clear();
  mIndexRemapBuffer.Clear();
}

void RenderQueues::AddStreamedLineRect(ViewNode& viewNode, Vec3 pos0, Vec3 pos1, Vec2 uv0, Vec2 uv1, Vec4 color, Vec2 uvAux0, Vec2 uvAux1)
{
  StreamedVertex v0(Math::TransformPoint(viewNode.mLocalToView, pos0),                    uv0,                color, uvAux0);
  StreamedVertex v1(Math::TransformPoint(viewNode.mLocalToView, Vec3(pos0.x, pos1.y, 0)), Vec2(uv0.x, uv1.y), color, Vec2(uvAux0.x, uvAux1.y));
  StreamedVertex v2(Math::TransformPoint(viewNode.mLocalToView, pos1),                    uv1,                color, uvAux1);
  StreamedVertex v3(Math::TransformPoint(viewNode.mLocalToView, Vec3(pos1.x, pos0.y, 0)), Vec2(uv1.x, uv0.y), color, Vec2(uvAux1.x, uvAux0.y));

  mStreamedVertices.PushBack(v0);
  mStreamedVertices.PushBack(v1);
  mStreamedVertices.PushBack(v1);
  mStreamedVertices.PushBack(v2);
  mStreamedVertices.PushBack(v2);
  mStreamedVertices.PushBack(v3);
  mStreamedVertices.PushBack(v3);
  mStreamedVertices.PushBack(v0);

  viewNode.mStreamedVertexCount = mStreamedVertices.Size() - viewNode.mStreamedVertexStart;
  viewNode.mStreamedVertexType = PrimitiveType::Lines;
}

void RenderQueues::AddStreamedQuad(ViewNode& viewNode, Vec3 pos0, Vec3 pos1, Vec2 uv0, Vec2 uv1, Vec4 color, Vec2 uvAux0, Vec2 uvAux1)
{
  StreamedVertex v0(Math::TransformPoint(viewNode.mLocalToView, pos0),                    uv0,                color, uvAux0);
  StreamedVertex v1(Math::TransformPoint(viewNode.mLocalToView, Vec3(pos0.x, pos1.y, 0)), Vec2(uv0.x, uv1.y), color, Vec2(uvAux0.x, uvAux1.y));
  StreamedVertex v2(Math::TransformPoint(viewNode.mLocalToView, pos1),                    uv1,                color, uvAux1);
  StreamedVertex v3(Math::TransformPoint(viewNode.mLocalToView, Vec3(pos1.x, pos0.y, 0)), Vec2(uv1.x, uv0.y), color, Vec2(uvAux1.x, uvAux0.y));

  mStreamedVertices.PushBack(v0);
  mStreamedVertices.PushBack(v1);
  mStreamedVertices.PushBack(v2);
  mStreamedVertices.PushBack(v2);
  mStreamedVertices.PushBack(v3);
  mStreamedVertices.PushBack(v0);

  viewNode.mStreamedVertexCount = mStreamedVertices.Size() - viewNode.mStreamedVertexStart;
}

void RenderQueues::AddStreamedQuadTiled(ViewNode& viewNode, Vec3 pos0, Vec3 pos1, Vec2 uv0, Vec2 uv1, Vec4 color, Vec2 tileSize, Vec2 uvAux0, Vec2 uvAux1)
{
  Vec2 size = Vec2(pos1.x - pos0.x, pos0.y - pos1.y);
  Vec2 tiles = size / tileSize;
  float remainderX = Math::FMod(tiles.x, tileSize.x);
  float remainderY = Math::FMod(tiles.y, tileSize.y);

  float uvDirX = uv0.x < uv1.x ? 1.0f : -1.0f;
  float uvDirY = uv0.y < uv1.y ? 1.0f : -1.0f;
  Vec2 uvSignedSize = uv1 - uv0;
  Vec2 uvAuxSignedSize = uvAux1 - uvAux0;

  // Full tiles
  for (uint x = 0; x < (uint)tiles.x; ++x)
  {
    for (uint y = 0; y < (uint)tiles.y; ++y)
    {
      Vec3 posOffset = Vec3(tileSize * Vec2((float)x, -(float)y), 0);
      Vec3 newPos0 = pos0 + posOffset;
      Vec3 newPos1 = newPos0 + Vec3(tileSize.x, -tileSize.y, 0);

      AddStreamedQuad(viewNode, newPos0, newPos1, uv0, uv1, color, uvAux0, uvAux1);
    }
  }
  // Bottom edge
  for (uint x = 0; x < (uint)tiles.x; ++x)
  {
    Vec3 posOffset = Vec3(tileSize * Vec2((float)x, -(float)(uint)tiles.y), 0);
    Vec3 newPos0 = pos0 + posOffset;
    Vec3 newPos1 = newPos0 + Vec3(tileSize.x, -remainderY, 0);
    Vec2 newUv1 = uv0 + Vec2(1, remainderY / tileSize.y) * uvSignedSize;
    Vec2 newUvAux1 = uvAux0 + Vec2(1, remainderY / tileSize.y) * uvAuxSignedSize;

    AddStreamedQuad(viewNode, newPos0, newPos1, uv0, newUv1, color, uvAux0, newUvAux1);
  }
  // Right edge
  for (uint y = 0; y < (uint)tiles.y; ++y)
  {
    Vec3 posOffset = Vec3(tileSize * Vec2((float)(uint)tiles.x, -(float)y), 0);
    Vec3 newPos0 = pos0 + posOffset;
    Vec3 newPos1 = newPos0 + Vec3(remainderX, -tileSize.y, 0);
    Vec2 newUv1 = uv0 + Vec2(remainderX / tileSize.x, 1) * uvSignedSize;
    Vec2 newUvAux1 = uvAux0 + Vec2(remainderX / tileSize.x, 1) * uvAuxSignedSize;

    AddStreamedQuad(viewNode, newPos0, newPos1, uv0, newUv1, color, uvAux0, newUvAux1);
  }
  // Corner
  if (remainderX > 0.0f && remainderY > 0.0f)
  {
    Vec3 posOffset = Vec3(tileSize * Vec2((float)(uint)tiles.x, -(float)(uint)tiles.y), 0);
    Vec3 newPos0 = pos0 + posOffset;
    Vec3 newPos1 = newPos0 + Vec3(remainderX, -remainderY, 0);
    Vec2 newUv1 = uv0 + Vec2(remainderX, remainderY) / tileSize * uvSignedSize;
    Vec2 newUvAux1 = uvAux0 + Vec2(remainderX, remainderY) / tileSize * uvAuxSignedSize;

    AddStreamedQuad(viewNode, newPos0, newPos1, uv0, newUv1, color, uvAux0, newUvAux1);
  }
}

void RenderQueues::AddStreamedQuadNineSliced(ViewNode& viewNode, Vec3 pos0, Vec3 pos1, Vec2 uv0, Vec2 uv1, Vec4 color, Vec4 posSlices, Vec4 uvSlices, Vec2 uvAux0, Vec2 uvAux1)
{
  Vec4 posX = Vec4(pos0.x, pos0.x + posSlices[NineSlices::Left], pos1.x - posSlices[NineSlices::Right], pos1.x);
  Vec4 posY = Vec4(pos0.y, pos0.y - posSlices[NineSlices::Top], pos1.y + posSlices[NineSlices::Bottom], pos1.y);

  Vec4 uvX = Vec4(uv0.x, uv0.x + uvSlices[NineSlices::Left], uv1.x - uvSlices[NineSlices::Right], uv1.x);
  Vec4 uvY = Vec4(uv0.y, uv0.y + uvSlices[NineSlices::Top], uv1.y - uvSlices[NineSlices::Bottom], uv1.y);

  float denomX = posX.w - posX.x;
  float denomY = posY.w - posY.x;
  // Safeguard against zero sized sprites
  if(denomX == 0.0f || denomY == 0.0f)
    return;

  // If inner slice positions overlap then need to clamp them to a midpoint.
  if (posX.y > posX.z)
  {
    float middle = (posX.x + posX.w) * 0.5f;
    middle = Math::Clamp(middle, posX.z, posX.y);

    // Interpolant values to compute new uv coordinates.
    float t1 = (middle - posX.x) / (posX.y - posX.x);
    float t2 = (middle - posX.z) / (posX.w - posX.z);

    posX.y = posX.z = middle;
    uvX.y = Math::Lerp(uvX.x, uvX.y, t1);
    uvX.z = Math::Lerp(uvX.z, uvX.w, t2);
  }

  // Because current ui also uses this method except with opposite y-axis direction,
  // have to check slice clipping for either direction.
  if (posY.z > posY.y && posY.x > posY.y || posY.y > posY.z && posY.y > posY.x)
  {
    float middle = (posY.w + posY.x) * 0.5f;
    // Axis direction can be either way
    middle = Math::Clamp(middle, Math::Min(posY.y, posY.z), Math::Max(posY.y, posY.z));

    // Interpolant values to compute new uv coordinates.
    float t1 = (middle - posY.x) / (posY.y - posY.x);
    float t2 = (middle - posY.z) / (posY.w - posY.z);

    posY.z = posY.y = middle;
    uvY.y = Math::Lerp(uvY.x, uvY.y, t1);
    uvY.z = Math::Lerp(uvY.z, uvY.w, t2);
  }

  // Compute lerp values where the position slices are
  Vec2 sliceLerpX = Vec2(posX.y - posX.x, posX.z - posX.x) / denomX;
  Vec2 sliceLerpY = Vec2(posY.y - posY.x, posY.z - posY.x) / denomY;

  // Want normalized uv's evenly mapped over the positional area
  Vec4 uvAuxX = Vec4(uvAux0.x, uvAux0.x + (uvAux1.x - uvAux0.x) * sliceLerpX.x, uvAux0.x + (uvAux1.x - uvAux0.x) * sliceLerpX.y, uvAux1.x);
  Vec4 uvAuxY = Vec4(uvAux0.y, uvAux0.y + (uvAux1.y - uvAux0.y) * sliceLerpY.x, uvAux0.y + (uvAux1.y - uvAux0.y) * sliceLerpY.y, uvAux1.y);

  StreamedVertex vertices[16];
  for (uint y = 0; y < 4; ++y)
  {
    for (uint x = 0; x < 4; ++x)
    {
      StreamedVertex vertex(Math::TransformPoint(viewNode.mLocalToView, Vec3(posX[x], posY[y], 0)), Vec2(uvX[x], uvY[y]), color, Vec2(uvAuxX[x], uvAuxY[y]));
      vertices[x + y * 4] = vertex;
    }
  }

  for (uint y = 0; y < 3; ++y)
  {
    for (uint x = 0; x < 3; ++x)
    {
      uint i = x + y * 4;
      mStreamedVertices.PushBack(vertices[i]);
      mStreamedVertices.PushBack(vertices[i + 4]);
      mStreamedVertices.PushBack(vertices[i + 5]);
      mStreamedVertices.PushBack(vertices[i + 5]);
      mStreamedVertices.PushBack(vertices[i + 1]);
      mStreamedVertices.PushBack(vertices[i]);
    }
  }

  viewNode.mStreamedVertexCount = mStreamedVertices.Size() - viewNode.mStreamedVertexStart;
}

void RenderQueues::AddStreamedQuadView(ViewNode& viewNode, Vec3 pos[4], Vec2 uv0, Vec2 uv1, Vec4 color)
{
  StreamedVertex v0(pos[0], uv0,                color, Vec2(0, 0));
  StreamedVertex v1(pos[1], Vec2(uv0.x, uv1.y), color, Vec2(0, 1));
  StreamedVertex v2(pos[2], uv1,                color, Vec2(1, 1));
  StreamedVertex v3(pos[3], Vec2(uv1.x, uv0.y), color, Vec2(1, 0));

  mStreamedVertices.PushBack(v0);
  mStreamedVertices.PushBack(v1);
  mStreamedVertices.PushBack(v2);
  mStreamedVertices.PushBack(v2);
  mStreamedVertices.PushBack(v3);
  mStreamedVertices.PushBack(v0);

  viewNode.mStreamedVertexCount = mStreamedVertices.Size() - viewNode.mStreamedVertexStart;
}

} // namespace Zero
