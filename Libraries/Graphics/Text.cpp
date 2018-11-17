///////////////////////////////////////////////////////////////////////////////
///
/// \file Text.cpp
/// Implementation of the display object text class.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(TextDefinition, builder, type)
{
}

void TextDefinition::Initialize()
{
  mFont = FontManager::GetInstance()->GetRenderFont(FontName, (uint)FontSize, 0);
}

void TextDefinition::Serialize(Serializer& stream)
{
  SerializeName(FontName);
  SerializeName(FontSize);
  SerializeName(FontColor);
}

}//namespace Zero
