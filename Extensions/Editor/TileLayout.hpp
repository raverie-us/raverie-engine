///////////////////////////////////////////////////////////////////////////////
///
/// \file TileLayout.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Helper for computed tiled Layouts
// As many tiles will be place on a row that  will fit and then as many rows 
// are used to fit all items. Tiles will also be spaced on each row.
struct TileLayout
{
  int TilesX;
  int TilesY;

  Vec2 Spacing;
  Vec2 TileSize;

  // First and last visible index of tiles
  IntVec2 GetFirstAndLastVisible(float minY, float maxY)
  {
    int firstVisibleRow = int(minY / (TileSize.y + Spacing.y));
    int lastVisibleRow = int(maxY / (TileSize.y + Spacing.y)) + 1;
    return IntVec2((firstVisibleRow)*TilesX, (lastVisibleRow)*TilesX -1);
  }

  TileLayout()
  {
    TilesX = 0;
    TilesY = 0;
    Spacing = Vec2::cZero;
    TileSize = Vec2::cZero;
  }

  // tileSize is size for each tile
  // areaSize is the size of the area the tiles will be placed in (y ignored)
  // tileCount number of tiles used for single line case
  // tilePadding will be used to pad each row and as min padding on x
  TileLayout(Vec2 tileSize, Vec2 areaSize, int tileCount, float tilePadding)
  {
    TileSize = tileSize;

    // Compute the max number of tiles that can fit on a row
    TilesX = Math::Max(1, int(areaSize.x / (tileSize.x + tilePadding)));

    // Need max number of tiles for spacing
    float maxTilesX = (float)TilesX;

    // Number of tiles on Y based on X 
    // Adding TilesX-1 will round up
    TilesY = (tileCount + TilesX-1) / TilesX;

    // All items fit one row
    if(TilesX > tileCount)
    {
      TilesX = tileCount;
      TilesY = 1;
    }

    // Compute how much space is left over
    float tilesSizeX = maxTilesX * tileSize.x;
    float spacingAreaX = areaSize.x - tilesSizeX;

    // Divide the space between all the tiles
    // Add 1 for padding on the sides
    float spacingX = spacingAreaX / (maxTilesX + 1.0f);

    // Store spacing
    Spacing.x = SnapToPixels(spacingX);

    Spacing.y = tilePadding;
  }

  Vec2 GetSizeNeeded()
  {
    return Vec2(TilesX * (Spacing.x + TileSize.x), TilesY * (Spacing.y + TileSize.y));
  }

  int GetTileInDirection(int tileIndex, IntVec2Param direction)
  {
    // Compute tile x and y from index
    int tileX = tileIndex % TilesX;
    int tileY = tileIndex / TilesX;

    tileX += direction.x;
    tileY += direction.y;

    return tileX + (TilesX * tileY);
  }

  void GetOverlappingTiles(Rect& rect, Array<int>& overlappingTiles)
  {
    // This could be optimized to not loop over all tiles
    for(int i = 0; i < TilesX * TilesY; ++i)
    {
      Rect tile = ComputeRect(i);
      if(rect.Overlap(tile))
        overlappingTiles.PushBack(i);

      // We've gone passed the bottom of the given rect
      if(tile.Y > (rect.Y + rect.SizeY))
        break;
    }
  }

  Rect ComputeRect(int tileIndex)
  {
    LayoutResult result = ComputeTileLayout(tileIndex);
    return Rect::PointAndSize(ToVector2(result.Translation), result.Size);
  }

  LayoutResult ComputeTileLayout(int tileIndex)
  {
    // Compute tile x and y from index
    int tileX = tileIndex % TilesX;
    int tileY = tileIndex / TilesX;

    LayoutResult result;
    Vec3 translation = Vec3(Spacing.x + float(tileX) * (Spacing.x + TileSize.x),
                            Spacing.y + float(tileY) * (Spacing.y + TileSize.y),
                            0);

    result.Translation = SnapToPixels(translation);
    result.Size = TileSize;

    return result;
  }
};

}
