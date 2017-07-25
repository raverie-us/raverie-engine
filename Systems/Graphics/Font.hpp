///////////////////////////////////////////////////////////////////////////////
///
/// \file Font.hpp
/// Declaration of the font resource class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
// Forward declaration
struct FontRasterizerData;
class Font;

///Text format constants and helpers
const Rune NewLine = Rune('\n');
bool AnyNewLine(Rune rune);
bool IsPrintable(Rune rune);
const int cAnsiRunes = 256;
const int cDefaultFontTextureSize = 512;
const int cFontSpacing = 4;

struct RenderGlyph
{
  int RuneCode;

  int Width;
  int Height;

  //After Raster
  int X;
  int Y;

  int DrawOffsetX;
  int DrawOffsetY;

  int AdvanceX;
};

struct RenderRune
{
  UvRect Rect;
  //Size of character in pixels
  Vec2 Size;
  //Offset of character from the baseline in pixels
  Vec2 Offset;
  //How much to advance the text position horizontally
  float Advance;
};

///RenderFont resource class.
class RenderFont : public Resource
{
public:
  RenderFont(Font* fontObject, int fontHeight);
  ~RenderFont();

  /// How wide is the given string
  Vec2 MeasureText(StringRange text, float unitsPerPixel = 1.0f);
  /// How wide this the given string up to count
  Vec2 MeasureText(StringRange text, uint charsToCount, float unitsPerPixel = 1.0f);
  /// How far in the string is the given position?
  /// The character found can be rounded up to nearest or last valid.
  uint GetPosition(StringRange text, float offset, float unitsPerPixel,
                   TextRounding::Enum rounding);

  // We need to keep track of what font is being used for at use loading of symbols
  Font* mFont;
  //Font size in pixels
  int mFontHeight;
  //Number of pixels below the base line the font
  //can descend
  float mDescent;
  //The size of a line in this font in pixels
  float mLineHeight;

  RenderRune& GetRenderRune(Rune runeCode);

  HashMap<int, RenderRune> mRunes;
  HandleOf<Texture> mTexture;
  // Data for texture to keep track of current status
  // Current texture size
  int mTextureSize;
  // Current X and Y position for next font glyph placement
  uint mTexPosX;
  uint mTexPosY;
  // used to check if there is enough texture space left
  // when adding new glyphs to a default rasterized font
  int mMaxWidthInPixels;
  int mMaxHeightInPixels;
};

//Font Class
class Font : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Font();
  ~Font();

  String LoadPath;
  HashMap<int, RenderFont*> mRendered;
  RenderFont* GetRenderFont(uint size);
  DataBlock FontBlock;
};

class FontManager : public ResourceManager
{
public:
  DeclareResourceManager(FontManager, Font);

  FontManager(BoundType* resourceType);
  ~FontManager();
  RenderFont* GetRenderFont(StringParam face, uint size, uint flags);
};

//Font Rasterizer helper class, creates new Render Fonts or updates existing ones
class FontRasterizer
{
public:
  FontRasterizer(Font* fontObject);
  ~FontRasterizer();

  RenderFont* RasterNewFont(int fontHeight);
  RenderFont* UpdateRasteredFont(RenderFont* existingFont, Rune newRuneCode);
  RenderFont* UpdateRasteredFont(RenderFont* existingFont, Array<int>& newRuneCodes);

  // Must be called within RasterNewFont or UpdateReasteredFont before additional calls
  // this is better than constantly passing the RenderFont into each function herein
  void SetRenderFont(RenderFont* renderFont);
  void ResetRenderFont(int fontHeight);

  /// FT_Face is a pointer to the freetype face object and must be released later
  void LoadFontFace(int fontHeight);

  void CollectRenderGlyphInfo(Array<int>& runeCodes);
  void CollectExisitingRuneCodes(Array<int>& runeCodes);
  /// Returns whether or not the current texture is being used
  /// as to determine whether we need to re-rasterize everything or add
  /// onto an existing texture
  bool PrepareFontImage();
  bool RoomOnTextureForRunes(int xPos, int yPos);
  
  void LoadGlyphsOntoTexture(bool isCurrentTexture);
  void ComputeAndRasterizeGlyphs();
  void ComputeGlyphTextureCoordinates();


  Font* mFontObject;
  RenderFont* mRenderFont;
  Array<RenderGlyph> mGlyphInfo;
  Image mFontImage;
  Array<Rune> mRunesToUseFallback;

  // Font Rendering object data
  FontRasterizerData* mData;
};

}//namespace Zero

