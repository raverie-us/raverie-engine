///////////////////////////////////////////////////////////////////////////////
///
/// \file SpriteDefinition.cpp
/// Implementation of the SpriteDefinition class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const String DefaultGroup = "Default";

//--------------------------------------------------------- Sprite Source Loader
class SpriteSourceLoader : public ResourceLoader
{
public:

  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override
  {
    SpriteSource* source = new SpriteSource();
    DataBlock dataBlock = ReadFileIntoDataBlock(entry.FullPath.c_str());
    LoadSprite(source, entry, dataBlock);
    SpriteSourceManager::GetInstance()->AddResource(entry, source);
    FreeBlock(dataBlock);
    return source;
  }

  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override
  {
    SpriteSource* source = new SpriteSource();
    DataBlock dataBlock = entry.Block;
    LoadSprite(source, entry, dataBlock);
    SpriteSourceManager::GetInstance()->AddResource(entry, source);
    return source;
  }

  void ReloadFromFile(Resource* resource, ResourceEntry& entry)
  {
    SpriteSource* source = (SpriteSource*)resource;
    source->Unload();
    DataBlock dataBlock = ReadFileIntoDataBlock(entry.FullPath.c_str());
    LoadSprite(source, entry, dataBlock);    
    FreeBlock(dataBlock);
  }

  void AddSpriteToGroup(SpriteSource* source)
  {
    SpriteSourceManager* manager = SpriteSourceManager::GetInstance();
    Atlas* group = AtlasManager::Find(source->AtlasId);
    if(group == nullptr)
      group = AtlasManager::GetDefault();
    group->AddSpriteSource(source);
  }

  void LoadSprite(SpriteSource* source, ResourceEntry& entry, DataBlock block)
  {
    source->mUvRect.TopLeft = Vec2(0,0);
    source->mUvRect.BotRight = Vec2(0,0);
    source->FramesX = 0;
    source->FramesY = 0;
    source->PixelSize = Vec2(1,1);
    source->FrameTexSize = Vec2(1,1);

    byte* imageData = 0;
    unsigned width, height, bitDepth;
    Status status;
    LoadFromPng(status, &imageData, &width, &height, &bitDepth, block.Data, block.Size, true);

    source->mTexture = nullptr;

    if(imageData == nullptr)
    {
      String message = String::Format("Failed to load sprite %s", entry.Name.c_str());
      DoNotifyWarning("Sprite Error", message);

      //Create a dummy source
      width = 2;
      height = 2;
      int imageSize = sizeof(ImagePixel)*width*height;
      imageData = (byte*)zAllocate(imageSize);
      memset(imageData, 0xFF, imageSize);
    }

    source->SourceImage.Data = (ImagePixel*)imageData;
    source->SourceImage.Width = width;
    source->SourceImage.Height = height;
    source->SourceImage.UserIndex = -1;
    source->SourceImage.SizeInBytes = width * height * sizeof(ImagePixel);

    //If there is a builder the sprite is being load in the editor
    SpriteSourceBuilder* builder = (SpriteSourceBuilder*)entry.mBuilder;
    source->mBuilder = builder;

    if(builder)
    {
      //Load sprite frame data directly from builder
      source->GetSpriteData() = builder->GetSpriteData();
    }
    else
    {
      //Get extra data on the end of the sprite file
      SpriteData* spriteData = (SpriteData*)(block.Data + block.Size - sizeof(SpriteData));
      memcpy(&source->GetSpriteData(), spriteData, sizeof(SpriteData));
    }

    FixAlphaHalo(&source->SourceImage);

    if(source->mAtlas == nullptr)
    {
      AddSpriteToGroup(source);
    }
    else
    {
      //Need to rebuild sprite sheet
      source->mAtlas->NeedsBuilding = true;
    }
  }
};

//---------------------------------------------------------------- Sprite Source
ZilchDefineType(SpriteSource, builder, type)
{
  ZeroBindDocumented();

  ZilchBindField(FrameCount);

  ZilchBindMethod(GetSize);
  ZilchBindMethod(GetOrigin);
}

SpriteSource::SpriteSource()
{
  mAtlas = nullptr;
}

void SpriteSource::Save(StringParam filename)
{
  if(mBuilder)
  {
    mBuilder->FrameCount = FrameCount;
    mBuilder->PixelsPerUnit = PixelsPerUnit;
    mBuilder->FrameSizeX = FrameSizeX;
    mBuilder->FrameSizeY = FrameSizeY;
    mBuilder->OriginX = OriginX;
    mBuilder->OriginY = OriginY;
    mBuilder->FrameDelay = FrameDelay;
    mBuilder->Sampling = Sampling;
    mBuilder->Slices = Slices;
    mBuilder->AtlasId = mAtlas->mResourceId;
    mBuilder->Looping = Looping;
    mBuilder->Fill = Fill;
    mContentItem->SaveMetaFile();
  }
}

void SpriteSource::SetAtlas(Atlas* atlas)
{
  if(mAtlas)
  {
    mAtlas->Sources.Erase(this);
    mAtlas = nullptr;
  }

  atlas->AddSpriteSource(this);
}

//TextureRegionInfo SpriteSource::GetData()
//{
//  if(mUvRects.Empty())
//  {
//    for(uint i=0;i<FrameCount;++i)
//      mUvRects.PushBack(GetUvRect(i));
//  }
//
//  TextureRegionInfo data = {mTexture, FrameCount, FrameDelay, mUvRects.Data() };
//  return data;
//}

Vec2 SpriteSource::GetSize()
{
  return PixelSize;
}

Vec2 SpriteSource::GetOrigin()
{
  return Vec2(OriginX, OriginY);
}

float SpriteSource::GetFrameRate()
{
  return 1.0f / FrameDelay;
}

void SpriteSource::SetFrameRate(float newFrameRate)
{
  // avoid division by zero frame rates resulting in invalid data/infinite loops when playing the animation
  newFrameRate = Math::Clamp(newFrameRate, cMinFrameRate, cMaxFrameRate);
  FrameDelay = 1.0f / newFrameRate;
}

SpriteSource::~SpriteSource()
{
  if(mAtlas)
    mAtlas->Sources.Erase(this);
}

void SpriteSource::FrameSetup()
{
  // Fix old projects that had a frame count of 0 set
  if(FrameCount == 0)
    FrameCount = 1;

  uint imageSizeX = SourceImage.Width;
  uint imageSizeY = SourceImage.Height;
  if(FrameSizeX != 0 )
  {
    // Check for invalid values for FrameSize
    // FrameSize can be 0 or too large
    if(FrameSizeX == 0 || FrameSizeX > imageSizeX)
      FrameSizeX = imageSizeX;

    if(FrameSizeY == 0 || FrameSizeY > imageSizeY)
      FrameSizeY = imageSizeY;

    FramesX = imageSizeX / FrameSizeX;
    FramesY = imageSizeY / FrameSizeY;

    Vec2 size = Math::ToVec2(mTexture->GetSize());

    float texFrameSizeX = FrameSizeX / size.x;
    float texFrameSizeY = FrameSizeY / size.y;

    PixelSize = Vec2((float)FrameSizeX, (float)FrameSizeY);
    FrameTexSize = Vec2(texFrameSizeX, texFrameSizeY);
  }
  else
  {
    FramesX = 1;
    FramesY = 1;
    FrameCount = 1;

    FrameSizeX = imageSizeX;
    FrameSizeY = imageSizeY;

    PixelSize = Vec2((float)imageSizeX, (float)imageSizeY);
    FrameTexSize = Vec2(PixelSize.x, PixelSize.y);
  }
}

UvRect SpriteSource::GetUvRect(uint currentFrame)
{
  SpriteSource* source = this;
  if(source->FrameCount > 1)
  {
    int spriteExpand = Atlas::sBorderWidth * 2;
    float uvExpandX = spriteExpand / (float)mTexture->GetSize().x;
    float uvExpandY = spriteExpand / (float)mTexture->GetSize().y;

    UvRect rect;
    uint frameX = currentFrame % source->FramesX;
    uint frameY = currentFrame / source->FramesX;
    rect.TopLeft = Vec2(float(frameX) * (source->FrameTexSize.x + uvExpandX) + mUvRect.TopLeft.x, 
                        float(frameY) * (source->FrameTexSize.y + uvExpandY) + mUvRect.TopLeft.y);
    rect.BotRight = rect.TopLeft + source->FrameTexSize;

    return rect;
  }
  else
  {
    return source->mUvRect;
  }
}


//ByteColor SpriteSource::Sample(SoftwareSampleMode::Enum mode, real mip, 
//                               uint currFrame, Vec2Param uv)
//{
//  //UvRect rect = GetUvRect(currFrame);
//
//  //// Convert the uv from local (0-1) to the world coordinates in the shared texture
//  //Vec2 atlasUV = rect.TopLeft + (rect.BotRight - rect.TopLeft) * uv;
//
//  //// Sample the texture
//  //return mTexture->Sample(mode, mip, atlasUV);
//  return ByteColor();
//}

//-------------------------------------------------------- Sprite Source Manager
ImplementResourceManager(SpriteSourceManager, SpriteSource);

SpriteSourceManager::SpriteSourceManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("SpriteSource", new SpriteSourceLoader());
  mCategory = "Graphics";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.png"));
  DefaultResourceName = "SquareBordered";
  mPreview = true;
  mCanReload = true;
  mExtension = "png";
  ConnectThisTo(Z::gResources, Events::ResourcesLoaded, OnResourcesLoaded);
  //mSharedManager = TextureRegionManager::GetInstance();
}

SpriteSourceManager::~SpriteSourceManager()
{
}

void SpriteSourceManager::OnResourcesLoaded(ResourceEvent* event)
{
  //Rebuild all sprite sheets
  RebuildSpriteSheets();
}

void SpriteSourceManager::RebuildSpriteSheets()
{
  // For even atlas
  AtlasManager* instance = AtlasManager::GetInstance();
  forRange(Resource* groupPtr, instance->ResourceIdMap.Values())
  {
    Atlas& group = * (Atlas*)groupPtr;
    if(!group.NeedsBuilding)
    {
      // Just update frame locations
      forRange(SpriteSource& source, group.Sources.All())
        source.FrameSetup();
      continue;
    }

    
    ZPrint("Building Atlas '%s'\n", group.Name.c_str());
    group.ClearTextures();
    group.NeedsBuilding = false;

    Array<Image*> Inputs[2];
    Array<Image*> Outputs;
    Array<Image*> Generated;
    Array<SpriteSource*> Sources[2];
    Array<PlacedSprite> Placed;
    Array<Image*> Padded;

    forRange(SpriteSource& source, group.Sources.All())
    {
      // generate different atlases for sampling modes
      uint inputIndex = source.Sampling == SpriteSampling::Nearest ? 0 : 1;

      source.mAtlas = &group;
      Sources[inputIndex].PushBack(&source);
      if (source.FrameCount > 1)
      {
        Image* image = new Image;
        Padded.PushBack(image);
        AddPixelBorders(&source.SourceImage, image, source.FrameSizeX, source.FrameSizeY, Atlas::sBorderWidth);
        Inputs[inputIndex].PushBack(image);
      }
      else
        Inputs[inputIndex].PushBack(&source.SourceImage);
    }

    for (uint inputIndex = 0; inputIndex < 2; ++inputIndex)
    {
      GenerateSpriteSheets(Outputs, Inputs[inputIndex], Placed, Generated, group.Textures.Size());

      for(uint i = 0; i < Outputs.Size(); ++i)
      {
        // If the image loaded successfully load the texture.
        Image& output = *Outputs[i];

        HandleOf<Texture> textureHandle = Texture::CreateRuntime();

        Texture* texture = textureHandle;
        texture->mAddressingX = TextureAddressing::Clamp;
        texture->mAddressingY = TextureAddressing::Clamp;
        texture->mFiltering = inputIndex == 0 ? TextureFiltering::Nearest : TextureFiltering::Bilinear;
        texture->Upload(output);

        group.Textures.PushBack(textureHandle);
      }

      for(uint i = 0; i < Placed.Size(); ++i)
      {
        PlacedSprite& placed = Placed[i];
        SpriteSource* source = Sources[inputIndex][placed.Source->UserIndex];
        Texture* texture = group.Textures[placed.OutputSheet];
        IntVec2 textureSize = texture->GetSize();
        float invWidth = 1.0f / textureSize.x;
        float invHeight = 1.0f / textureSize.y;
        source->mTexture = texture;


        source->mUvRect.TopLeft = Vec2(float(placed.Rect.X), float(placed.Rect.Y)) * invWidth;
        source->mUvRect.BotRight = Vec2(float(placed.Rect.X + placed.Rect.SizeX) * invWidth, 
                                        float(placed.Rect.Y + placed.Rect.SizeY) * invHeight);
        source->FrameSetup();
      }

      Outputs.Clear();
      Placed.Clear();
    }

    //Clean up generated scratch sprite sheets
    DeleteObjectsIn(Generated.All());
    DeleteObjectsIn(Padded.All());
  }
}

bool BufferSorter(Image* left, Image* right)
{
  int leftSize = left->Height  + left->Width * 2048;
  int rightSize = right->Height + right->Width* 2048;
  return leftSize > rightSize;
}

template<typename type>
bool CheckOverlap(type& a, type& b)
{
  if(a.X >= b.X + b.SizeX )
    return false;

  if(a.X + a.SizeX  <= b.X)
    return false;

  if(a.Y >= b.Y + b.SizeY )
    return false;

  if(a.Y + a.SizeY  <= b.Y)
    return false;
  return true;
}

int FindOverlappingSprite(PlacedSprite& toPlace, Array<PlacedSprite>& placed)
{
  for(uint i = 0; i < placed.Size(); i++)
  {
    if(CheckOverlap(placed[i].Rect, toPlace.Rect))
      return i;
  }
  return -1;
}


bool PlaceSprite(PlacedSprite& toPlace, Array<PlacedSprite>& placed,
                 int sheetIndex, int outputHeight, int outputWidth)
{
  int x = 0;
  int y = 0;

  int lastValidLine = outputHeight - toPlace.Rect.SizeY;
  int smallestInLine = outputHeight;

  while(y <= lastValidLine)
  {
    toPlace.Rect.X = x;
    toPlace.Rect.Y = y;

    int overlapIndex = FindOverlappingSprite(toPlace, placed);
    if(overlapIndex < 0)
    {
      //Found free spot place the sprite.
      toPlace.OutputSheet = sheetIndex;
      placed.PushBack(toPlace);
      return true;
    }

    smallestInLine = Math::Min(smallestInLine, placed[overlapIndex].Rect.SizeY);

    //Move past this sprite.
    x = placed[overlapIndex].Rect.X + placed[overlapIndex].Rect.SizeX;
    if(x + toPlace.Rect.SizeX > outputWidth)
    {
      x = 0;
      y++;
    }
  }

  return false;

}

void GenerateSpriteSheets(Array<Image*>& outputs, 
                          Array<Image*>& inputs, 
                          Array<PlacedSprite>& placedSprites, 
                          Array<Image*>& generated,
                          int outputSheetOffset)
{
  int spriteExpand = Atlas::sBorderWidth * 2;

  for(uint i = 0; i < inputs.Size(); ++i)
    inputs[i]->UserIndex = i;

  Sort(inputs.All(), BufferSorter);

  Array< Array<PlacedSprite> > SpriteSheets;
  SpriteSheets.Reserve(20);

  const uint maxSpriteSheetSize = 1024;
  Array<Image*> largeImages;

  for(uint i = 0; i < inputs.Size(); ++i)
  {
    bool inserted = false;
    int sheet = 0;
    
    Image* input = inputs[i];
    if(input->Width + spriteExpand >= maxSpriteSheetSize || input->Height + spriteExpand >= maxSpriteSheetSize)
    {
      largeImages.PushBack(input);
      continue;
    }

    while(!inserted)
    {
      if(SpriteSheets.Size() <= uint(sheet))
        SpriteSheets.PushBack();

      PlacedSprite p;
      p.Index = SpriteSheets[sheet].Size();
      p.Source = input;
      p.Rect.SizeX = input->Width + spriteExpand;
      p.Rect.SizeY = input->Height + spriteExpand;
      inserted = PlaceSprite(p, SpriteSheets[sheet], sheet + outputSheetOffset,
                             maxSpriteSheetSize, maxSpriteSheetSize);
      ++sheet;
    }

  }

  //Generate image buffers
  for(uint i = 0; i < SpriteSheets.Size(); ++i)
  {
    if(SpriteSheets[i].Size() > 0)
    {
      Image* buffer = new Image();
      buffer->Allocate(maxSpriteSheetSize, maxSpriteSheetSize);

      outputs.PushBack(buffer);
      generated.PushBack(buffer);

      for(uint b = 0; b < SpriteSheets[i].Size(); ++b)
      {
        PlacedSprite& s = SpriteSheets[i][b];

        //Fix the sprite expansion
        s.Rect.X += Atlas::sBorderWidth;
        s.Rect.Y += Atlas::sBorderWidth;
        s.Rect.SizeX = s.Source->Width;
        s.Rect.SizeY =  s.Source->Height;

        CopyImage(buffer, s.Source, s.Rect.X, s.Rect.Y);

        // Fill a pixel border around the sprite to prevent issues with mip mapping
        FillPixelBorders(buffer, s.Rect.TopLeft(), s.Rect.TopLeft() + s.Rect.Size() - IntVec2(1, 1), Atlas::sBorderWidth);

        placedSprites.PushBack(s);
      }
    }
  }

  //Pass thru large sprites
  for(uint i = 0; i < largeImages.Size(); ++i)
  {
    //Image is very large place on single sprite sheet
    Image* input = largeImages[i];

    PlacedSprite s;
    s.OutputSheet = outputs.Size() + outputSheetOffset;
    s.Source = input;
    s.Index = input->UserIndex;
    s.Rect.SizeX = input->Width;
    s.Rect.SizeY = input->Height;
    s.Rect.X = 0;
    s.Rect.Y = 0;
    outputs.PushBack(input);

    placedSprites.PushBack(s);
  }
}

}// namespace Zero
