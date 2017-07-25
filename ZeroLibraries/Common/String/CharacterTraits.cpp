///////////////////////////////////////////////////////////////////////////////
///
/// \file CharacterTraits.cpp
/// Used to convert strings into values.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const uint CharacterTableSize = 128;
typedef int (*ControlFunction)(int c);

class CharacterTable
{
public:
  CharacterTable()
  {
    // clear all states to zero
    for(uint i=0;i<CharacterTableSize;++i)
      CharacterTrait[i] = 0;

    // fill out the state bits
    Annotate(isgraph, CharacterBits::Graphical);
    Annotate(isalpha, CharacterBits::Alpha);
    Annotate(isdigit, CharacterBits::Number);
    Annotate(isspace, CharacterBits::WhiteSpace);
    Annotate(islower, CharacterBits::Lower);
    Annotate(ispunct, CharacterBits::Symbol);
    Annotate(iscntrl, CharacterBits::Control);
  }

  // Get the character state
  int GetTraits(int c)
  {
    if(c >= 0 && c <128)
      return CharacterTrait[c];
    return 0;
  }

private:
  void Annotate(ControlFunction c, u32 bit)
  { 
    for(uint i=0;i<CharacterTableSize;++i)
    {
      if((*c)((int)i))
      {
        CharacterTrait[i] |= bit;
      }
    }
  }

  char CharacterTrait[CharacterTableSize];
};

CharacterTable t;
int IsSpace(Rune r){ return t.GetTraits(r.value) & CharacterBits::WhiteSpace; }
int IsGraph(Rune r){ return t.GetTraits(r.value) & CharacterBits::Graphical; }
int IsGraphOrSpace(Rune r){return t.GetTraits(r.value) & (CharacterBits::Graphical | CharacterBits::WhiteSpace);}
int IsAlpha(Rune r){ return t.GetTraits(r.value) & CharacterBits::Alpha; }
int IsDigit(Rune r){ return t.GetTraits(r.value) & CharacterBits::Number; }
int IsNumber(Rune r){ return t.GetTraits(r.value) & CharacterBits::Number; }
int IsAlphaNumeric(Rune r){ return t.GetTraits(r.value) & (CharacterBits::Alpha | CharacterBits::Number); }
int IsLower(Rune r){ return t.GetTraits(r.value) & CharacterBits::Lower; }
int IsUpper(Rune r){ return !(t.GetTraits(r.value) & CharacterBits::Lower); }
int IsSymbol(Rune r){ return t.GetTraits(r.value) & CharacterBits::Symbol; }
int IsControl(Rune r){ return t.GetTraits(r.value) & CharacterBits::Control; }
int GetTrait(Rune r){ return t.GetTraits(r.value); }
int ToLower(Rune r){ return tolower(r.value); }
int ToUpper(Rune r){ return toupper(r.value); }
bool IsHex(Rune r) { return IsNumber(r) || (r.value >= 'a' && r.value <= 'f') || (r.value >= 'A' && r.value <= 'F'); };
};
