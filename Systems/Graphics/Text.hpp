///////////////////////////////////////////////////////////////////////////////
///
/// \file Text.hpp
/// Declaration of the display object text class.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class RenderFont;

class TextDefinition : public BaseDefinition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Vec4 FontColor;
  String FontName;
  float FontSize;
  RenderFont* mFont;

  //BaseDefinition Interface
  void Initialize() override;
  void Serialize(Serializer& stream) override;
};

}//namespace Zero
