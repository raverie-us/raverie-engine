// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

class RenderFont;

class TextDefinition : public BaseDefinition
{
public:
  ZilchDeclareType(TextDefinition, TypeCopyMode::ReferenceType);

  Vec4 FontColor;
  String FontName;
  float FontSize;
  RenderFont* mFont;

  // BaseDefinition Interface
  void Initialize() override;
  void Serialize(Serializer& stream) override;
};

} // namespace Zero
