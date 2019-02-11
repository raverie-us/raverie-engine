// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Rune.hpp"

namespace Zero
{

Rune::Rune() : value(Invalid)
{
}

Rune::Rune(uint runeCode) : value(runeCode)
{
}

bool Rune::operator==(RuneParam rhs) const
{
  return value == rhs.value;
}

bool Rune::operator!=(RuneParam rhs) const
{
  return value != rhs.value;
}

bool Rune::operator>(RuneParam rhs) const
{
  return value > rhs.value;
}

bool Rune::operator<(RuneParam rhs) const
{
  return value < rhs.value;
}

bool Rune::operator>=(RuneParam rhs) const
{
  return value >= rhs.value;
}

bool Rune::operator<=(RuneParam rhs) const
{
  return value <= rhs.value;
}

bool Rune::operator==(uint rhs) const
{
  return value == rhs;
}

bool Rune::operator!=(uint rhs) const
{
  return value != rhs;
}

bool Rune::operator>(uint rhs) const
{
  return value > rhs;
}

bool Rune::operator<(uint rhs) const
{
  return value < rhs;
}

bool Rune::operator>=(uint rhs) const
{
  return value >= rhs;
}

bool Rune::operator<=(uint rhs) const
{
  return value <= rhs;
}

size_t Rune::Hash() const
{
  return HashUint(value);
}

} // namespace Zero
