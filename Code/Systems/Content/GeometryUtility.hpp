// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

IndexElementType::Enum DetermineIndexType(uint numIndices);
Mat4 AiMat4ToZeroMat4(aiMatrix4x4& aiMatrix4);

// Assimp stores UVs in Vec3s so this looks odd, but it is as intended and
// needed
void ConvertAndFillArrayVec2(aiVector3D* aiArray, uint numElements, Array<Vec2>& raverieArray);
void ConvertAndFillArrayVec3(aiVector3D* aiArray, uint numElements, Array<Vec3>& raverieArray);
void ConvertAndFillArrayVec4(aiColor4D* aiArray, uint numElements, Array<Vec4>& raverieArray);
String CleanAssetName(String nodeName);
bool IsPivot(String nodeName);

// Helper functions for converting Assimp animation keys to zero track types
PositionKey AssimpToZeroPositionKey(aiVectorKey positionKey);
RotationKey AssimpToZeroRotationKey(aiQuatKey rotationKey);
ScalingKey AssimpToZeroScalingKey(aiVectorKey scalingKey);

template <typename T>
void WriteIndexData(Array<uint>& indexBuffer, ChunkFileWriter& writer)
{
  for (size_t i = 0; i < indexBuffer.Size(); ++i)
    writer.Write((T)indexBuffer[i]);
}

} // namespace Raverie
