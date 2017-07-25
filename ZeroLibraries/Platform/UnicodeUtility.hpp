///////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace UTF8
{
  // en_US.UTF-8 includes the following Unicode Standards, perhaps windows will support this someday.
  // ISO 8859-1 (most Western European languages, such as English, French, Spanish, and German)
  // ISO 8859-2 (most Central European languages, such as Czech, Polish, and Hungarian)
  // ISO 8859-4 (Scandinavian and Baltic languages)
  // ISO 8859-5 (Russian)
  // ISO 8859-6 (Arabic, including many more presentation form character glyphs)
  // ISO 8859-7 (Greek)
  // ISO 8859-8 (Hebrew)
  // ISO 8859-9 (Turkish)
  // TIS 620.2533 (Thai, including many more presentation form character glyphs)
  // ISO 8859-15 (most Western European languages with euro sign)
  // GB 2312-1980 (Simplified Chinese)
  // Big5(Traditional Chinese)
  // JIS X0201-1976, JIS X0208-1983 (Japanese)
  // KS C 5601-1992 Annex 3 (Korean)

  bool IsLower(Rune rune);
  bool IsUpper(Rune rune);
  bool IsWhiteSpace(Rune rune);
  bool IsAlphaNumeric(Rune rune);
  Rune  ToLower(Rune rune);
  Rune  ToUpper(Rune rune);

  uint Utf8ToUtf32(Rune utf8);
  size_t UnpackUtf8RuneIntoBuffer(Rune uft8Rune, byte(&utf8Bytes)[4]);
  Rune ReadUtf8Rune(byte* firstByte);
  size_t EncodedCodepointLength(byte utf8FirstByte);
}

}// namespace Zero
