// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class VertexDescriptionBuilder
{
public:
  VertexDescriptionBuilder();
  ~VertexDescriptionBuilder();

  FixedVertexDescription& SetupDescriptionFromMesh(aiMesh* mesh);
  void AddAttribute(VertexSemantic::Enum semantic, VertexElementType::Enum type, byte count);
  byte GetElementSize(VertexElementType::Type type);
  FixedVertexDescription GetDescription();

private:
  byte mCurrentOffset;
  size_t mIndex;

  FixedVertexDescription mVertexDescription;
};

} // namespace Raverie
