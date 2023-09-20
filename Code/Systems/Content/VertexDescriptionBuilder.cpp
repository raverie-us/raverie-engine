// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

VertexDescriptionBuilder::VertexDescriptionBuilder() : mCurrentOffset(0), mIndex(0)
{
}

VertexDescriptionBuilder::~VertexDescriptionBuilder()
{
}

FixedVertexDescription& VertexDescriptionBuilder::SetupDescriptionFromMesh(aiMesh* mesh)
{
  if (mesh->HasPositions())
    AddAttribute(VertexSemantic::Position, VertexElementType::Real, 3);

  if (mesh->HasNormals())
    AddAttribute(VertexSemantic::Normal, VertexElementType::Real, 3);

  if (mesh->HasTangentsAndBitangents())
  {
    AddAttribute(VertexSemantic::Tangent, VertexElementType::Real, 3);
    AddAttribute(VertexSemantic::Bitangent, VertexElementType::Real, 3);
  }

  if (mesh->HasTextureCoords(0))
    AddAttribute(VertexSemantic::Uv, VertexElementType::Real, 2);

  if (mesh->HasTextureCoords(1))
    AddAttribute(VertexSemantic::UvAux, VertexElementType::Real, 2);

  if (mesh->HasVertexColors(0))
    AddAttribute(VertexSemantic::Color, VertexElementType::Real, 4);

  if (mesh->HasVertexColors(1))
    AddAttribute(VertexSemantic::ColorAux, VertexElementType::Real, 4);

  if (mesh->HasBones())
  {
    AddAttribute(VertexSemantic::BoneWeights, VertexElementType::Real, cMaxBonesWeights);
    AddAttribute(VertexSemantic::BoneIndices, VertexElementType::Byte, cMaxBonesWeights);
  }

  mVertexDescription.mVertexSize = mCurrentOffset;
  // if we have filled up and used every vertex description slot we don't need
  // to mark none to signify the end
  if (mIndex != (VertexSemantic::Size - 1))
    AddAttribute(VertexSemantic::None, VertexElementType::Byte, 0);

  return mVertexDescription;
}

void VertexDescriptionBuilder::AddAttribute(VertexSemantic::Enum semantic, VertexElementType::Enum type, byte count)
{
  mVertexDescription.mAttributes[mIndex++] = VertexAttribute(semantic, type, count, mCurrentOffset);
  mCurrentOffset += count * GetElementSize(type);
}

byte VertexDescriptionBuilder::GetElementSize(VertexElementType::Type type)
{
  switch (type)
  {
  case VertexElementType::Byte:
  case VertexElementType::NormByte:
    return 1;
  case VertexElementType::Short:
  case VertexElementType::NormShort:
  case VertexElementType::Half:
    return 2;
  case VertexElementType::Real:
    return 4;
  }
  return 0;
}

FixedVertexDescription VertexDescriptionBuilder::GetDescription()
{
  return mVertexDescription;
}

} // namespace Raverie
