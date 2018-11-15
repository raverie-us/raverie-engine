#include "Precompiled.hpp"

Real Test01()
{
  Integer i1 = ((5 * 3) / 2);
  Integer i2 = (3 + i1) << (2 + (3 - 1));
  Integer i3 = ((99 % 24) >> 1) | (22 & 33);
  Integer i4 = Zilch::VirtualMachine::IntegralPower(i1 ^ i2, 2);

  Real r1 = ((5.12f * 3.35f) / 2.3f);
  Real r2 = (3.92f + r1) / (2.32f + (3.1f - 16.2f));
  Real r3 = (fmod(99.2f, 24.35f) * 1.12f) / (22.01f - 33.32f);
  Real r4 = (Real)pow((r1 + r2), 2.2f);

  i1 += (Integer)r1;
  i2 -= (Integer)r2;
  i3 *= (Integer)r3;
  i4 /= (Integer)r4;
  i1 %= (Integer)r1;
  i2  = (Integer)Zilch::VirtualMachine::IntegralPower(i2, (Integer)r2);
  i3 |= (Integer)r3;
  i4 &= (Integer)r4;
  i1 ^= (Integer)r1;

  r1 += (Real)i1;
  r2 -= (Real)i2;
  r3 *= (Real)i3;
  r4 /= (Real)i4;
  r1  = (Real)fmod(r1, (Real)i1);
  r2  = (Real)pow(r2, (Real)i2);

  return r1 + r2 + r3 + r4 + ((Real)i1) + ((Real)i2) + ((Real)i3) + ((Real)i4);
}