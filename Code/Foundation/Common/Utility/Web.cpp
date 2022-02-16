// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

String UrlParamEncode(StringParam string)
{
  StringBuilder builder;
  StringIterator it = string.Begin();
  StringIterator end = string.End();
  for (; it < end; ++it)
  {
    Rune rune = *it;

    if (IsAlphaNumeric(rune))
      builder << rune;
    else
      builder << String::Format("%%%02X", rune);
  }
  return builder.ToString();
}

Zero::String UrlParamDecode(StringParam string)
{
  StringBuilder builder;
  StringIterator it = string.Begin();
  StringIterator end = string.End();

  for (; it < end; ++it)
  {
    Rune rune = *it;
    // if the current rune is a % check for two following numbers
    if (rune == '%')
    {
      // make sure moving forward 2 characters is within the string
      if ((it + 3) < end)
      {
        // create a substring of the next two characters should they both be hex
        // numbers
        Rune nextRune1 = *(it + 1);
        Rune nextRune2 = *(it + 2);
        if (IsHex(nextRune1) && IsHex(nextRune2))
        {
          String encodedValue = string.SubString(it + 1, it + 3);
          uint value;
          ToValue(encodedValue, value, 16);
          // if the substring was a valid number take the encoded value
          // otherwise take the % as is
          if (value)
          {
            builder << Rune(value);
            it += 2;
            continue;
          }
        }
      }
    }
    // falls back on the runes value in all cases of it not being an encoded
    // value
    builder << rune;
  }

  return builder.ToString();
}

} // namespace Zero
