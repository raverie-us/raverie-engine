// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

IndexElementType::Enum DetermineIndexType(uint numIndices)
{
  if (numIndices < 256)
    return IndexElementType::Byte;
  else if (numIndices < 65535)
    return IndexElementType::Ushort;
  // we have more than 65535 indices, use a full 4 bytes
  return IndexElementType::Uint;
}

Mat4 AiMat4ToZeroMat4(aiMatrix4x4& aiMatrix4)
{
#if ColumnBasis
  return Mat4(aiMatrix4.a1,
              aiMatrix4.a2,
              aiMatrix4.a3,
              aiMatrix4.a4,
              aiMatrix4.b1,
              aiMatrix4.b2,
              aiMatrix4.b3,
              aiMatrix4.b4,
              aiMatrix4.c1,
              aiMatrix4.c2,
              aiMatrix4.c3,
              aiMatrix4.c4,
              aiMatrix4.d1,
              aiMatrix4.d2,
              aiMatrix4.d3,
              aiMatrix4.d4);
#else
  return Mat4(aiMatrix4.a1,
              aiMatrix4.b1,
              aiMatrix4.c1,
              aiMatrix4.d1,
              aiMatrix4.a2,
              aiMatrix4.b2,
              aiMatrix4.c2,
              aiMatrix4.d2,
              aiMatrix4.a3,
              aiMatrix4.b3,
              aiMatrix4.c3,
              aiMatrix4.d3,
              aiMatrix4.a4,
              aiMatrix4.b4,
              aiMatrix4.c4,
              aiMatrix4.d4);
#endif
}

void ConvertAndFillArrayVec2(aiVector3D* aiArray, uint numElements, Array<Vec2>& raverieArray)
{
  raverieArray.Resize(numElements);
  for (uint i = 0; i < numElements; ++i)
  {
    aiVector3D aiVec = aiArray[i];
    raverieArray.PushBack(Vec2(aiVec.x, aiVec.y));
  }
}

void ConvertAndFillArrayVec3(aiVector3D* aiArray, uint numElements, Array<Vec3>& raverieArray)
{
  raverieArray.Resize(numElements);
  for (uint i = 0; i < numElements; ++i)
  {
    aiVector3D aiVec = aiArray[i];
    raverieArray.PushBack(Vec3(aiVec.x, aiVec.y, aiVec.z));
  }
}

void ConvertAndFillArrayVec4(aiColor4D* aiArray, uint numElements, Array<Vec4>& raverieArray)
{
  raverieArray.Resize(numElements);
  for (uint i = 0; i < numElements; ++i)
  {
    aiColor4D aiVec = aiArray[i];
    raverieArray.PushBack(Vec4(aiVec.r, aiVec.g, aiVec.b, aiVec.a));
  }
}

String CleanAssetName(String nodeName)
{
  // Remove Assimp's tag on pivot nodes.
  nodeName = nodeName.Replace("_$AssimpFbx$_", "");
  // Restrict names to identifiers allowed on objects in the engine.
  nodeName = LibraryBuilder::FixIdentifier(nodeName, TokenCheck::None, '\0');
  return nodeName;
}

bool IsPivot(String nodeName)
{
  return nodeName.Contains("_$AssimpFbx$_");
}

PositionKey AssimpToZeroPositionKey(aiVectorKey positionKey)
{
  PositionKey zKey;
  zKey.Keytime = (float)positionKey.mTime;
  zKey.Position = Vec3(positionKey.mValue.x, positionKey.mValue.y, positionKey.mValue.z);
  return zKey;
}

RotationKey AssimpToZeroRotationKey(aiQuatKey rotationKey)
{
  RotationKey zKey;
  zKey.Keytime = (float)rotationKey.mTime;
  zKey.Rotation = Quat(rotationKey.mValue.x, rotationKey.mValue.y, rotationKey.mValue.z, rotationKey.mValue.w);
  return zKey;
}

ScalingKey AssimpToZeroScalingKey(aiVectorKey scalingKey)
{
  ScalingKey zKey;
  zKey.Keytime = (float)scalingKey.mTime;
  zKey.Scale = Vec3(scalingKey.mValue.x, scalingKey.mValue.y, scalingKey.mValue.z);
  return zKey;
}

} // namespace Raverie
