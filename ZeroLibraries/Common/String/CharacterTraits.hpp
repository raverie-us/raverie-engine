///////////////////////////////////////////////////////////////////////////////
///
/// \file CharacterTraits.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Utility/EnumDeclaration.hpp"
#include "Utility/Typedefs.hpp"

namespace Zero
{

DeclareBitField7(CharacterBits, Graphical, Alpha, Number, WhiteSpace, Lower, Symbol, Control);

int IsSpace(Rune r);
int IsGraph(Rune r);
int IsGraphOrSpace(Rune r);
int IsAlpha(Rune r);
int IsDigit(Rune r);
int IsNumber(Rune r);
int IsAlphaNumeric(Rune r);
int IsLower(Rune r);
int IsUpper(Rune r);
int IsSymbol(Rune r);
int IsControl(Rune r);

int GetTraits(Rune r);

int ToLower(Rune r);
int ToUpper(Rune r);

bool IsHex(Rune r);

}//namespace Zero
