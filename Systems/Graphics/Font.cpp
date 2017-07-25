///////////////////////////////////////////////////////////////////////////////
///
/// \file Font.cpp
/// Implementation of the font resource class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#pragma comment(lib, "freetype28.lib")

#include "ft2build.h"
#include FT_FREETYPE_H

namespace Zero
{

bool AnyNewLine(Rune rune)
{
  return rune == '\n' || rune == '\r';
}

//character > 255 quick fix for UTF8 needs to be improved to be accurate
bool IsPrintable(Rune rune)
{
  return (rune >= 32 && rune < 127) || rune == 169 || rune == 149 || rune > 255;
}

//------------------------------------------------------------ Render Font 

RenderFont::RenderFont(Font* fontObject, int fontHeight)
  : mFont(fontObject),
    mFontHeight(fontHeight),
    mTextureSize(cDefaultFontTextureSize),
    mTexPosX(cFontSpacing),
    mTexPosY(cFontSpacing),
    mMaxWidthInPixels(1),
    mMaxHeightInPixels(1)
{

}

RenderFont::~RenderFont()
{

}

Vec2 RenderFont::MeasureText(StringRange text, uint runesToCount, float unitsPerPixel)
{
  Vec2 size = Vec2(0.0f, mLineHeight * unitsPerPixel);
  uint count = Math::Min(runesToCount, text.ComputeRuneCount());
  float lineSize = 0.0f;
  float maxLineSize = 0.0f;

  StringRange textTemp = text;
  for (; count > 0; --count, textTemp.PopFront())
  {
    Rune rune = textTemp.Front();

    if (AnyNewLine(rune))
    {
      maxLineSize = Math::Max(maxLineSize, lineSize);
      lineSize = 0.0f;
      size.y += mLineHeight * unitsPerPixel;
      continue;
    }

    if (IsPrintable(rune))
    {
      RenderRune& r = GetRenderRune(rune);
      lineSize += r.Advance * unitsPerPixel;
    }
  }

  size.x = Math::Max(maxLineSize, lineSize);
  return size;
}

Vec2 RenderFont::MeasureText(StringRange text, float unitsPerPixel)
{
  return MeasureText(text, text.SizeInBytes(), unitsPerPixel);
}

uint RenderFont::GetPosition(StringRange text, float offset, float unitsPerPixel, TextRounding::Enum rounding)
{
  float width = 0.0f;
  float lastWidth = 0.0f;
  uint runePositionIndex = 0;

  StringRange textTemp = text;
  for (; !textTemp.Empty(); textTemp.PopFront(), ++runePositionIndex)
  {
    Rune runeCode = textTemp.Front();

    //check for line terminator 
    if (AnyNewLine(runeCode))
      return runePositionIndex;

    //skip unprintable (is printable does not check for UTF8 ranges and codes)
    if (!IsPrintable(runeCode))
      continue;

    RenderRune& r = GetRenderRune(runeCode);
    float characterWidth = r.Advance * unitsPerPixel;

    // Does the offset occur in this character?
    // Small epsilon to prevent clipping last character for exact sized text
    if (width + characterWidth > offset + 0.001f)
    {
      if (rounding == TextRounding::LastCharacter)
        return runePositionIndex;

      //Round the character selection
      //Is the position more than halfway through this character
      float overflow = offset - width;
      if (overflow >  characterWidth * 0.5f)
        return runePositionIndex + 1;
      else
        return runePositionIndex;
    }
    width += characterWidth;
  }
  return runePositionIndex;
}

RenderRune& RenderFont::GetRenderRune(Rune rune)
{
  if (rune == Rune::Invalid)
  {
    ErrorIf(true, "Attempting to Render Invalid Rune");
  }

  // attempt to retrieve the rune from the render font
  RenderRune* renderRune = mRunes.FindPointer(rune.value);

  if (renderRune)
    return *renderRune;
  // if the rune isn't rasterized yet, attempt to load it
  else
  {
    FontRasterizer fontRaster(mFont);
    fontRaster.UpdateRasteredFont(this, rune);
    renderRune = mRunes.FindPointer(rune.value);
    // this should probably be changed to a notification error (its for debug at the moment)
    // as not all fonts have all runes available
    //ErrorIf(renderRune == nullptr, "New Rune Failed To Render");
    
    // quick fix for andy to keep working
    if (renderRune == nullptr)
      renderRune = mRunes.FindPointer('?');
    
    return *renderRune;
  }
}

//------------------------------------------------------------ Font Rendering 

struct FontRenderOptions
{
  int PixelSize;
  int BorderSize;
  int GlyphBorderSize;
};

void RasterGlyph(Image* dest, FT_Bitmap* bitmap, int x, int y)
{
  int destX, destY, srcX, srcY;
  int destEndX = x + bitmap->width;
  int destEndY = y + bitmap->rows;
  destEndX = Math::Min( destEndX, dest->Height );
  destEndY = Math::Min( destEndY, dest->Width );
  for (destX = x, srcX = 0; destX < destEndX; destX++, srcX++)
  {
    for (destY = y, srcY = 0; destY < destEndY; destY++, srcY++)
    {
      ImagePixel& a = dest->GetPixel(destX, destY);
      ImagePixel color = bitmap->buffer[srcY * bitmap->width + srcX] << 24;
      //mix with white
      color |= 0x00FFFFFF;
      a = color;
    }
  }
}

int FtToPixels(int a){return a >> 6;}

//------------------------------------------------------------ Font
ZilchDefineType(Font, builder, type)
{
  ZeroBindDocumented();
}

Font::Font()
{
}

Font::~Font()
{
  DeleteObjectsInContainer(mRendered);

  if(FontBlock)
    FreeBlock(FontBlock);
}

RenderFont* Font::GetRenderFont(uint size)
{
  RenderFont* rfont = mRendered.FindValue(size, nullptr);
  if(rfont)
    return rfont;
  else
  {
    FontRasterizer fontRaster(this);
    RenderFont* newRenderFont = fontRaster.RasterNewFont(size);
    return newRenderFont;
  }
}


//------------------------------------------------------------ FontLoaderTtf
class FontLoaderTtf : public ResourceLoader
{
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry)
  {
    Font* newFont = new Font();
    newFont->LoadPath = entry.FullPath;
    FontManager::GetInstance()->AddResource(entry, newFont);
    return newFont;
  }

  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry)
  {
    Font* newFont = new Font();
    //Store in memory
    CloneBlock(newFont->FontBlock, entry.Block);
    FontManager::GetInstance()->AddResource(entry, newFont);
    return newFont;
  }
};


//------------------------------------------------------------ Font Manager
ImplementResourceManager(FontManager, Font);

FontManager::~FontManager()
{
}

FontManager::FontManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("Font", new FontLoaderTtf());
  mCategory = "Graphics";
  DefaultResourceName = "NotoSans-Regular";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("All Fonts", "*.ttf;*.ttc;*.otf"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.ttf"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.ttc"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.otf"));
}

RenderFont* FontManager::GetRenderFont(StringParam face, uint size, uint flags)
{
  Font* font = FontManager::Find(face);
  return font->GetRenderFont(size);
}

//------------------------------------------------------------ FontRasterizer
struct FontRasterizerData
{
  FT_Library Library;
  FT_Face FontFace;
};

FontRasterizer::FontRasterizer(Font* fontObject)
  : mFontObject(fontObject)
{
  mData = new FontRasterizerData();
  int errorCode = FT_Init_FreeType(&mData->Library);
  ErrorIf(errorCode != 0, nullptr, "Failed to load freetype.");
}

FontRasterizer::~FontRasterizer()
{
  //clean up all used freetype resources
  FT_Done_Face(mData->FontFace);
  FT_Done_FreeType(mData->Library);
  SafeDelete(mData);
}

RenderFont* FontRasterizer::RasterNewFont(int fontHeight)
{
  RenderFont* renderFont = new RenderFont(mFontObject, fontHeight);
  SetRenderFont(renderFont);

  LoadFontFace(fontHeight);

  Array<int> runeCodes;
  if (mFontObject->mRendered.Empty())
  {
    // setup default rune codes to load and rasterize
    for (int runeIndex = 0; runeIndex < cAnsiRunes; ++runeIndex)
      runeCodes.PushBack(runeIndex);
  }
  else
  {
    // get all existing rune codes rasterized for this font so far
    RenderFont* existingRenderFont = mFontObject->mRendered.All().Front().second;
    HashMap<int, RenderRune>::keyrange runeCodeRange = existingRenderFont->mRunes.Keys();

    while (!runeCodeRange.Empty())
    {
      runeCodes.PushBack(runeCodeRange.Front());
      runeCodeRange.PopFront();
    }
  }

  CollectRenderGlyphInfo(runeCodes);
  PrepareFontImage();
  // this is always a new texture
  LoadGlyphsOntoTexture(false);

  return renderFont;
}

RenderFont* FontRasterizer::UpdateRasteredFont(RenderFont* existingFont, Array<int>& newRuneCodes)
{
  SetRenderFont(existingFont);
  LoadFontFace(existingFont->mFontHeight);

  CollectRenderGlyphInfo(newRuneCodes);
  // We are checking whether or not we have to collect and re-rasterize the existing runes onto a new texture
  bool isCurrentTexture = PrepareFontImage();
  // subtex2d is broken in the old graphics engine, just always re-rasterize the whole font set
  if (isCurrentTexture == false)
  {
    // Since we are treating our render font as new, reset tracked values
    ResetRenderFont(mRenderFont->mFontHeight);
    CollectExisitingRuneCodes(newRuneCodes);
    CollectRenderGlyphInfo(newRuneCodes);
    // After recollecting the existing runes we may need to increase the texture size again
    // so re-run just in case, it deallocates/allocates again, investigate
    // better solutions
    PrepareFontImage();
  }

  LoadGlyphsOntoTexture(isCurrentTexture);

  return existingFont;
}

// For ease of use when adding just one new rune code
Zero::RenderFont* FontRasterizer::UpdateRasteredFont(RenderFont* existingFont, Rune newRuneCode)
{
  Array<int> runeCodes;
  runeCodes.PushBack(newRuneCode.value);
  UpdateRasteredFont(existingFont, runeCodes);
  
  return existingFont;
}

void FontRasterizer::SetRenderFont(RenderFont* renderFont)
{
  mRenderFont = renderFont;
}

void FontRasterizer::ResetRenderFont(int fontHeight)
{
  mRenderFont->mTextureSize = cDefaultFontTextureSize;
  mRenderFont->mTexPosX = cFontSpacing;
  mRenderFont->mTexPosY = cFontSpacing;
  mRenderFont->mMaxWidthInPixels = 0;
  mRenderFont->mMaxHeightInPixels = 0;
}

void FontRasterizer::LoadFontFace(int fontHeight)
{
  //Always face index zero for now.
  uint faceIndex = 0;

  //Load the font from the font file
  int errorCode = FT_New_Face(mData->Library, mFontObject->LoadPath.c_str(), faceIndex, &mData->FontFace);

  ErrorIf(errorCode == FT_Err_Unknown_File_Format, nullptr, "File is not a valid font file.");
  ErrorIf(errorCode != 0, nullptr, "Bad file or path.");

  //mGlyphSlot = mFontFace->glyph;

  errorCode = FT_Set_Pixel_Sizes(mData->FontFace, 0, fontHeight);
  ErrorIf(errorCode != 0, nullptr, "Pixel size failed for some reason.");

  FT_Select_Charmap(mData->FontFace, FT_ENCODING_UNICODE);
  ErrorIf(errorCode != 0, nullptr, "Failed to set unicode charmap for %s.", mFontObject->LoadPath.c_str());
}

void FontRasterizer::CollectRenderGlyphInfo(Array<int>& runeCodes)
{
  int errorCode = 0;
  mGlyphInfo.Clear();
  mRunesToUseFallback.Clear();

  int& maxWidth = mRenderFont->mMaxWidthInPixels;
  int& maxHeight = mRenderFont->mMaxHeightInPixels;

  for (uint i = 0; i < runeCodes.Size(); ++i)
  {
    Rune rune = Rune(runeCodes[i]);
    if (IsPrintable(rune))
    {
      RenderGlyph curGlyph;
      curGlyph.RuneCode = rune.value;

      FT_UInt glyphIndex = FT_Get_Char_Index(mData->FontFace, UTF8::Utf8ToUtf32(rune));

      errorCode = FT_Load_Glyph(mData->FontFace, glyphIndex, FT_LOAD_DEFAULT);

      if (errorCode) continue;

      FT_GlyphSlot glyphSlot = mData->FontFace->glyph;
      curGlyph.Width = FtToPixels(glyphSlot->metrics.width);
      curGlyph.Height = FtToPixels(glyphSlot->metrics.height);

      //these will determine our fonts standard render box size
      maxWidth = Math::Max(maxWidth, curGlyph.Width + cFontSpacing);
      maxHeight = Math::Max(maxHeight, curGlyph.Height + cFontSpacing);

      mGlyphInfo.PushBack(curGlyph);
    }
  }
}

void FontRasterizer::LoadGlyphsOntoTexture(bool isOriginalTexture)
{
  int errorCode = 0;
 
  // we want to track the starting texPos(X,Y) for where to start rendering glyphs
  // after we have placed them in our image font
  uint texPosX = mRenderFont->mTexPosX;
  uint texPosY = mRenderFont->mTexPosY;

  int fontHeight = mRenderFont->mFontHeight;
  int slotWidth = mRenderFont->mMaxWidthInPixels;
  int slotHeight = mRenderFont->mMaxHeightInPixels;

  // Gather glyph info and rasterize them to the image
  // Updates render font TexPos(X,Y) tracking based on glyph positions
  ComputeAndRasterizeGlyphs();

  HandleOf<Texture> texture;
  // We have to special case if this is a new texture entirely, or if we are adding
  // these glyphs to an existing texture
  if (isOriginalTexture)
  {
    texture = mRenderFont->mTexture;
    
    // We must render the new glyphs in rectangles so figure out how many
    // are left on the current line
    uint stripStartX = texPosX;
    uint stripStartY = texPosY;
    int glyphsInStrip = 0;
    bool timeToRasterStrip;

    for (uint i = 0; i < mGlyphInfo.Size(); ++i)
    {
      // increment how many glyphs we are rasterizing in this pass
      ++glyphsInStrip;
      timeToRasterStrip = false;
      int pixelsLeft = mFontImage.Width - texPosX;
      // have we reached the end of the strip, rasterize what we have
      if ((slotWidth * glyphsInStrip) > pixelsLeft)
      {
        texPosY += slotHeight;
        texPosX = cFontSpacing;
        timeToRasterStrip = true;
        // since we 'counted' one glyph past the end we need to move our count back one to get all the glyphs
        // should their be more to render
        glyphsInStrip -= 1;
        i -= 1;
      }
      else if (i == (mGlyphInfo.Size() - 1))
      {
        // there are no more glyphs left, rasterize the strip
        texPosX += slotWidth * glyphsInStrip;
        timeToRasterStrip = true;
      }
      
      if (timeToRasterStrip)
      {
        // figure out the size of the sub image we will be copying and uploading to the
        // existing rasterized font texture
        int stripWidth = slotWidth * glyphsInStrip;
        int stripHeight = slotHeight;

        // prepare the sub image
        Image subImage;
        subImage.Allocate(stripWidth, stripHeight);
        int stripX = stripStartX;
        int stripY = stripStartY;

        // copy the data into an image to upload to the existing texture
        for (int x = 0; x < stripWidth; ++x)
          for (int y = 0; y < stripHeight; ++y)
            subImage.SetPixel(x, y, mFontImage.GetPixel(stripX + x, stripY + y));

        // upload to the texture
        texture->SubUpload(subImage, stripStartX, stripStartY);

        // set our next strip position offsets and reset the glyph count
        stripStartX = texPosX;
        stripStartY = texPosY;
        glyphsInStrip = 0;
      }
    }
  }
  // This is a new texture altogether so create a texture and load the image data into it
  else
  {
    int textureSize = mRenderFont->mTextureSize;
    texture = Texture::CreateRuntime();
    texture->mFiltering = TextureFiltering::Bilinear;
    texture->Upload(textureSize, textureSize, TextureFormat::RGBA8, (byte*)mFontImage.Data, mFontImage.SizeInBytes);
    
    // None of these steps should be repeated for new textures altogether
    // Load all the data into the Rendered Font
    mRenderFont->mDescent = -(float)FtToPixels(mData->FontFace->size->metrics.descender);
    mRenderFont->mFontHeight = fontHeight;
    mRenderFont->mLineHeight = (float)FtToPixels(mData->FontFace->size->metrics.height);

    const int empty = ' ';

    // Clear all default characters
    mRenderFont->mRunes[empty].Rect.BotRight = Vec2(1, 1);
    mRenderFont->mRunes[empty].Rect.TopLeft = Vec2(1, 1);
    mRenderFont->mRunes[empty].Offset = Vec2(0, 0);
    mRenderFont->mRunes[empty].Size = Vec2(0, 0);
    mRenderFont->mRunes[empty].Advance = 0;

    // Get empty character size
    FT_UInt glyph_index = FT_Get_Char_Index(mData->FontFace, empty);
    errorCode = FT_Load_Glyph(mData->FontFace, glyph_index, FT_LOAD_DEFAULT);
    FT_GlyphSlot glyphSlot = mData->FontFace->glyph;
    mRenderFont->mRunes[empty].Advance = (float)FtToPixels(glyphSlot->advance.x);

    // If the hashmap resizes during an assignment the returned reference will be invalid so we need
    // a copy here for assignment below.
    RenderRune emptyRune = mRenderFont->mRunes[empty];
    // Copy to other characters
    for (int i = 1; i <= empty; ++i)
    {
      mRenderFont->mRunes[i] = emptyRune;
    }
  }

  ComputeGlyphTextureCoordinates();

  mRenderFont->mTexture = texture;
  mFontObject->mRendered[fontHeight] = mRenderFont;
}

void FontRasterizer::ComputeAndRasterizeGlyphs()
{
  uint& texPosX = mRenderFont->mTexPosX;
  uint& texPosY = mRenderFont->mTexPosY;

  int fontHeight = mRenderFont->mFontHeight;
  int slotWidth = mRenderFont->mMaxWidthInPixels;
  int slotHeight = mRenderFont->mMaxHeightInPixels;

  int errorCode = 0;

  // even when adding new glyphs to an existing font texture this places them correctly
  for (uint n = 0; n < mGlyphInfo.Size(); n++)
  {
    RenderGlyph& curGlyph = mGlyphInfo[n];

    int pixelsLeft = mFontImage.Width - texPosX;

    //Is there enough room on this line?
    if (slotWidth > pixelsLeft)
    {
      //start a new line
      texPosY += slotHeight;
      texPosX = cFontSpacing;
    }

    // retrieve glyph index from character code
    FT_UInt glyph_index;
    glyph_index = FT_Get_Char_Index(mData->FontFace, UTF8::Utf8ToUtf32(Rune(curGlyph.RuneCode)));

    // load glyph image into the slot
    errorCode = FT_Load_Glyph(mData->FontFace, glyph_index, FT_LOAD_DEFAULT);
    if (errorCode) continue;

    FT_GlyphSlot glyphSlot = mData->FontFace->glyph;

    // convert to an anti-aliased bitmap
    errorCode = FT_Render_Glyph(glyphSlot, FT_RENDER_MODE_NORMAL);
    if (errorCode) continue; /* now, draw to our target surface */

    // Store data needed to draw text
    curGlyph.X = texPosX;
    curGlyph.Y = texPosY;
    curGlyph.DrawOffsetX = FtToPixels(glyphSlot->metrics.horiBearingX);
    curGlyph.DrawOffsetY = FtToPixels(-glyphSlot->metrics.horiBearingY) + fontHeight;
    curGlyph.AdvanceX = FtToPixels(glyphSlot->advance.x);

    // Raster the glyph to the image
    RasterGlyph(&mFontImage, &glyphSlot->bitmap, texPosX, texPosY);

    // increment pen position on image
    texPosX += slotWidth;
  }
}

void FontRasterizer::ComputeGlyphTextureCoordinates()
{
  const float texSize = (float)mRenderFont->mTextureSize;

  // Compute texture coordinates, since we place the glyphs in the respective positions
  // on the font image this step doesn't change between new fonts or adding to existing sets
  Array<RenderGlyph>::range r = mGlyphInfo.All();
  for (; !r.Empty(); r.PopFront())
  {
    RenderGlyph curGlyph = r.Front();

    RenderRune* r;
    r = &mRenderFont->mRunes[curGlyph.RuneCode];

    r->Size = Vec2(float(curGlyph.Width), float(curGlyph.Height));
    r->Offset = Vec2(float(curGlyph.DrawOffsetX), float(curGlyph.DrawOffsetY));
    r->Advance = float(curGlyph.AdvanceX);

    r->Rect.TopLeft = Vec2(float(curGlyph.X) / texSize, float(curGlyph.Y) / texSize);
    r->Rect.BotRight = Vec2(float(curGlyph.X + curGlyph.Width) / texSize, float(curGlyph.Y + curGlyph.Height) / texSize);
  }
}

void FontRasterizer::CollectExisitingRuneCodes(Array<int>& runeCodes)
{
  HashMap<int, RenderRune>::keyrange runeCodeRange = mRenderFont->mRunes.Keys();

  while (!runeCodeRange.Empty())
  {
    runeCodes.PushBack(runeCodeRange.Front());
    runeCodeRange.PopFront();
  }
}

bool FontRasterizer::PrepareFontImage()
{
  int& textureSize = mRenderFont->mTextureSize;
  int prevTextureSize = textureSize;

  // then we need to see if we have a texture space available for the number of glyphs
  while (!RoomOnTextureForRunes(mRenderFont->mTexPosX, mRenderFont->mTexPosY))
  {
    // increase texture size, there is no space
    textureSize *= 2;
  }
  // now that we have a large enough texture create it
  mFontImage.Allocate(textureSize, textureSize);
  mFontImage.ClearColorTo(0x00FFFFFF);
  
  return prevTextureSize == textureSize;
}

bool FontRasterizer::RoomOnTextureForRunes(int xPos, int yPos)
{
  //int for round down division, we can't rasterize 1/2 a rune
  int totalColumns = mRenderFont->mTextureSize / mRenderFont->mMaxWidthInPixels;
  int totalRows = mRenderFont->mTextureSize / mRenderFont->mMaxHeightInPixels;

  int currentColumn = (xPos / mRenderFont->mMaxWidthInPixels) + 1;
  int currentRow = (yPos / mRenderFont->mMaxHeightInPixels) + 1;

  uint runeSlotsLeft = (totalColumns * (totalRows - currentRow)) + (totalColumns - currentColumn);

  return runeSlotsLeft >= mGlyphInfo.Size();
}

}//namespace Zero
