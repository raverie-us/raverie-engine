// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class RenderFont;

class TextDefinition : public BaseDefinition
{
public:
  RaverieDeclareType(TextDefinition, TypeCopyMode::ReferenceType);

  Vec4 FontColor;
  String FontName;
  float FontSize;
  RenderFont* mFont;

  // BaseDefinition Interface
  void Initialize() override;
  void Serialize(Serializer& stream) override;
};

} // namespace Raverie
