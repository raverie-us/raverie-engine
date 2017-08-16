//////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
//////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

IndexElementType::Enum DetermineIndexType(uint numIndices);
Mat4 AiMat4ToZeroMat4(aiMatrix4x4& aiMatrix4);
// Assimp stores UVs in Vec3s so this looks odd, but it is as intended and needed
void ConvertAndFillArrayVec2(aiVector3D* aiArray, uint numElements, Array<Vec2>& zeroArray);
void ConvertAndFillArrayVec3(aiVector3D* aiArray, uint numElements, Array<Vec3>& zeroArray);
void ConvertAndFillArrayVec4(aiColor4D* aiArray, uint numElements, Array<Vec4>& zeroArray);
String CleanAssetName(String nodeName);

// Templates below here

template<typename T>
void WriteIndexData(Array<uint>& indexBuffer, ChunkFileWriter& writer)
{
  for (size_t i = 0; i < indexBuffer.Size(); ++i)
    writer.Write((T)indexBuffer[i]);

//   for (size_t i = 0; i < numFaces; ++i)
//   {
//     for (size_t j = 0; j < faces[i].mNumIndices; ++j)
//       writer.Write((T)faces[i].mIndices[j]);
//   }

// template<>
// struct HashPolicy<Vec3>
// {
//   inline size_t operator()(Vec3Param value) const
//   {
//     uint value1 = *(uint*)&value.x;
//     uint value2 = *(uint*)&value.y;
//     uint value3 = *(uint*)&value.z;
//     uint prime1 = 805306457;
//     uint prime2 = 402653189;
//     uint prime3 = 1610612741;
//     return (value1 * prime1) ^ (value2 * prime2) ^ (value3 * prime3);
//   }
//   inline bool Equal(Vec3Param left, Vec3Param right) const
//   {
//     return left.x == right.x &&
//            left.y == right.y &&
//            left.z == right.z;
//   }
// };
}

}// namespace Zero