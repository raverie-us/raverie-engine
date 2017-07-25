///////////////////////////////////////////////////////////////////////////////
///
/// \file Sprite.cpp
/// Implementation of the Sprite component class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------ Base Sprite
ZilchDefineType(BaseSprite, builder, type)
{
  ZeroBindDocumented();
  ZeroBindInterface(Graphical);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindFieldProperty(mVertexColor);
  ZilchBindFieldProperty(mGeometryMode);
}

void BaseSprite::Serialize(Serializer& stream)
{
  Graphical::Serialize(stream);
  SerializeNameDefault(mVertexColor, Vec4(1.0f));
  SerializeEnumNameDefault(SpriteGeometryMode, mGeometryMode, SpriteGeometryMode::ZPlane);
}

void BaseSprite::Initialize(CogInitializer& initializer)
{
  Graphical::Initialize(initializer);
}

void BaseSprite::ComponentAdded(BoundType* typeId, Component* component)
{
  Graphical::ComponentAdded(typeId, component);
  if (typeId == ZilchTypeId(Area))
    UpdateBroadPhaseAabb();
}

void BaseSprite::ComponentRemoved(BoundType* typeId, Component* component)
{
  Graphical::ComponentRemoved(typeId, component);
  if (typeId == ZilchTypeId(Area))
    UpdateBroadPhaseAabb();
}

String BaseSprite::GetDefaultMaterialName()
{
  return "AlphaSprite";
}

Aabb BaseSprite::GetViewPlaneAabb(Aabb localAabb)
{
  Vec3 center = localAabb.GetCenter();
  Vec3 widths = localAabb.GetHalfExtents();
  Vec3 worldSize = mTransform->GetScale();
  float maxX = Math::Max(Math::Abs(center.x - widths.x), Math::Abs(center.x + widths.x)) * worldSize.x;
  float maxY = Math::Max(Math::Abs(center.y - widths.y), Math::Abs(center.y + widths.y)) * worldSize.y;
  float radius = Math::Sqrt(maxX * maxX + maxY * maxY);
  return Aabb(Vec3(0.0f), Vec3(1.0f) * radius / worldSize);
}

void BaseSprite::ComputeLocalToViewMatrix(Mat4& localToView, Mat4& localToWorld, Mat4& worldToView)
{
  if (mGeometryMode == SpriteGeometryMode::ViewPlane)
    MakeLocalToViewAligned(localToView, localToWorld, worldToView, mTransform->GetWorldTranslation());
  else
    localToView = worldToView * localToWorld;
}

//----------------------------------------------------------------------- Sprite
ZilchDefineType(Sprite, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterSetterProperty(SpriteSource);
  ZilchBindFieldProperty(mFlipX);
  ZilchBindFieldProperty(mFlipY);
  ZilchBindFieldProperty(mAnimationActive);
  ZilchBindFieldProperty(mAnimationSpeed);
  ZilchBindGetterSetterProperty(StartFrame);
  ZilchBindGetterSetterProperty(CurrentFrame);
}

void Sprite::Serialize(Serializer& stream)
{
  BaseSprite::Serialize(stream);
  SerializeResourceNameManagerDefault(mSpriteSource, SpriteSourceManager);
  SerializeNameDefault(mFlipX, false);
  SerializeNameDefault(mFlipY, false);
  SerializeNameDefault(mAnimationActive, true);
  SerializeNameDefault(mAnimationSpeed, 1.0f);
  SerializeNameDefault(mStartFrame, 0u);
}

void Sprite::Initialize(CogInitializer& initializer)
{
  BaseSprite::Initialize(initializer);
  mCurrentFrame = mStartFrame;
  mFrameTime = 0.0f;

  ConnectThisTo(GetSpace(), Events::LogicUpdate, OnLogicUpdate);
}

void Sprite::DebugDraw()
{
  if (mAnimationActive)
  {
    TimeSpace* timeSpace = GetSpace()->has(TimeSpace);

    // Don't update twice if time space is unpaused
    if (!timeSpace->mPaused)
      return;

    UpdateAnimation(timeSpace->mScaledClampedDt);
  }
}

Aabb Sprite::GetLocalAabb()
{
  Aabb localAabb = Aabb(Vec3(GetLocalCenter(), 0.0f), Vec3(GetLocalWidths(), 0.0f));

  if (mGeometryMode == SpriteGeometryMode::ViewPlane)
    return GetViewPlaneAabb(localAabb);

  return localAabb;
}

void Sprite::ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock)
{
  frameNode.mBorderThickness = 1.0f;
  frameNode.mRenderingType = RenderingType::Streamed;
  frameNode.mCoreVertexType = CoreVertexType::Streamed;

  frameNode.mMaterialRenderData = mMaterial->mRenderData;
  frameNode.mMeshRenderData = nullptr;
  frameNode.mTextureRenderData = mSpriteSource->mTexture->mRenderData;

  frameNode.mLocalToWorld = mTransform->GetWorldMatrix();
  frameNode.mLocalToWorldNormal = Mat3::cIdentity;

  frameNode.mObjectWorldPosition = Vec3::cZero;

  frameNode.mBoneMatrixRange = IndexRange(0, 0);
}

void Sprite::ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  FrameNode& frameNode = frameBlock.mFrameNodes[viewNode.mFrameNodeIndex];

  ComputeLocalToViewMatrix(viewNode.mLocalToView, frameNode.mLocalToWorld, viewBlock.mWorldToView);

  Vec2 center = GetLocalCenter();
  Vec2 widths = GetLocalWidths();

  Vec3 pos0 = Vec3(center, 0.0f) + Vec3(-widths.x, widths.y, 0.0f);
  Vec3 pos1 = Vec3(center, 0.0f) + Vec3(widths.x, -widths.y, 0.0f);

  UvRect rect = mSpriteSource->GetUvRect(mCurrentFrame);

  Vec2 uv0 = rect.TopLeft;
  Vec2 uv1 = rect.BotRight;

  Vec2 uvAux0 = Vec2(0.0f);
  Vec2 uvAux1 = Vec2(1.0f);

  if (mFlipX)
  {
    Math::Swap(uv0.x, uv1.x);
    Math::Swap(uvAux0.x, uvAux1.x);
  }
  if (mFlipY)
  {
    Math::Swap(uv0.y, uv1.y);
    Math::Swap(uvAux0.y, uvAux1.y);
  }

  viewNode.mStreamedVertexType = PrimitiveType::Triangles;
  viewNode.mStreamedVertexStart = frameBlock.mRenderQueues->mStreamedVertices.Size();
  viewNode.mStreamedVertexCount = 0;

  bool hasArea = mOwner->has(Area) != nullptr;

  if (hasArea && mSpriteSource->Fill == SpriteFill::Tiled)
  {
    Vec2 tileSize = mSpriteSource->GetSize() / mSpriteSource->PixelSize;
    frameBlock.mRenderQueues->AddStreamedQuadTiled(viewNode, pos0, pos1, uv0, uv1, mVertexColor, tileSize, uvAux0, uvAux1);
  }
  else if (hasArea && mSpriteSource->Fill == SpriteFill::NineSlice)
  {
    Vec2 size = mSpriteSource->GetSize();
    Vec4 posSlices = mSpriteSource->Slices / mSpriteSource->PixelsPerUnit;
    Vec4 uvSlices = mSpriteSource->Slices / Vec4(size.x, size.y, size.x, size.y);
    Vec2 uvSignedSize = uv1 - uv0;
    uvSlices *= Vec4(uvSignedSize.x, uvSignedSize.y, uvSignedSize.x, uvSignedSize.y);
    frameBlock.mRenderQueues->AddStreamedQuadNineSliced(viewNode, pos0, pos1, uv0, uv1, mVertexColor, posSlices, uvSlices, uvAux0, uvAux1);
  }
  else
  {
    frameBlock.mRenderQueues->AddStreamedQuad(viewNode, pos0, pos1, uv0, uv1, mVertexColor, uvAux0, uvAux1);
  }
}

SpriteSource* Sprite::GetSpriteSource()
{
  return mSpriteSource;
}

void Sprite::SetSpriteSource(SpriteSource* spriteSource)
{
  if (spriteSource == nullptr)
    return;

  mSpriteSource = spriteSource;
  mStartFrame = WrapIndex(mStartFrame);
  mCurrentFrame = mStartFrame;
  mFrameTime = 0.0f;
  UpdateBroadPhaseAabb();
}

uint Sprite::GetStartFrame()
{
  return mStartFrame;
}

void Sprite::SetStartFrame(uint frameIndex)
{
  mStartFrame = WrapIndex(frameIndex);
}

uint Sprite::GetCurrentFrame()
{
  return mCurrentFrame;
}

void Sprite::SetCurrentFrame(uint frameIndex)
{
  mCurrentFrame = WrapIndex(frameIndex);
}

Vec2 Sprite::GetLocalCenter()
{
  if (Area* area = mOwner->has(Area))
  {
    Vec2 size = area->GetSize();
    Vec2 offset = ToOffset(area->GetOrigin());

    return offset * size;
  }
  else
  {
    Vec2 size = mSpriteSource->GetSize();
    Vec2 origin = mSpriteSource->GetOrigin();
    // Origin is in uv coordinate direction, so y needs to be flipped for position
    Vec2 offset = size * 0.5f - origin;
    offset.y = -offset.y;

    return offset / mSpriteSource->PixelsPerUnit;
  }
}

Vec2 Sprite::GetLocalWidths()
{
  if (Area* area = mOwner->has(Area))
    return area->GetSize() * 0.5;
  else
    return mSpriteSource->GetSize() * 0.5f / mSpriteSource->PixelsPerUnit;
}

void Sprite::OnLogicUpdate(UpdateEvent* event)
{
  UpdateAnimation(event->Dt);
}

void Sprite::UpdateAnimation(float time)
{
  if (mAnimationActive && mSpriteSource->FrameCount > 1)
  {
    mFrameTime += time * mAnimationSpeed;
    uint frame = mCurrentFrame;

    // Advanced Frames, mAnimationSpeed or small delay
    // may cause  multiple frames to pass in one update
    while (mFrameTime > mSpriteSource->FrameDelay)
    {
      ++frame;
      mFrameTime -= mSpriteSource->FrameDelay;
    }

    if (frame >= mSpriteSource->FrameCount)
    {
      // Looping wrap around sprite index
      if (mSpriteSource->Looping)
        frame = frame % mSpriteSource->FrameCount;
      // Not looping stop at end frame
      else
        frame = mSpriteSource->FrameCount - 1;

      // The animation reached the end, send out an event to notify the user
      //ObjectEvent toSend(GetOwner());
      //GetOwner()->DispatchEvent(Events::SpriteAnimationEnded, &toSend);
    }

    mCurrentFrame = frame;
  }
}

uint Sprite::WrapIndex(uint index)
{
  if (mSpriteSource->FrameCount == 0)
  {
    String msg = String::Format("Sprite source %s has no frames.", mSpriteSource->Name.c_str());
    DoNotifyError("Invalid Sprite", msg);
    return 0;
  }
  return index % mSpriteSource->FrameCount;
}

//------------------------------------------------------------------ Sprite Text
ZilchDefineType(SpriteText, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterSetterProperty(Text);
  ZilchBindGetterSetterProperty(Font);
  ZilchBindGetterSetterProperty(FontSize);
  ZilchBindGetterSetterProperty(PixelsPerUnit);
  ZilchBindGetterSetterProperty(TextAlign);

  ZilchBindMethod(MeasureText);
  ZilchBindMethod(MeasureGivenText);
  ZilchBindMethod(GetCharacterPosition);
}

void SpriteText::Serialize(Serializer& stream)
{
  BaseSprite::Serialize(stream);
  SerializeNameDefault(mText, String("Sprite Text..."));
  SerializeResourceName(mFont, FontManager);
  SerializeNameDefault(mFontSize, 32u);
  SerializeNameDefault(mPixelsPerUnit, 128.0f);
  SerializeEnumNameDefault(TextAlign, mTextAlign, TextAlign::Center);
}

void SpriteText::Initialize(CogInitializer& initializer)
{
  BaseSprite::Initialize(initializer);
}

Aabb SpriteText::GetLocalAabb()
{
  Aabb localAabb = Aabb(Vec3(GetLocalCenter(), 0.0f), Vec3(GetLocalWidths(), 0.0f));

  if (mGeometryMode == SpriteGeometryMode::ViewPlane)
    return GetViewPlaneAabb(localAabb);

  return localAabb;
}

void SpriteText::ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock)
{
  frameNode.mRenderingType = RenderingType::Streamed;
  frameNode.mCoreVertexType = CoreVertexType::Streamed;

  frameNode.mMeshRenderData = nullptr;
  frameNode.mMaterialRenderData = mMaterial->mRenderData;
  frameNode.mTextureRenderData = mFont->GetRenderFont(mFontSize)->mTexture->mRenderData;

  // Only data needed for ExtractViewData
  frameNode.mLocalToWorld = mTransform->GetWorldMatrix();

  frameNode.mBoneMatrixRange = IndexRange(0, 0);
}

void SpriteText::ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  FrameNode& frameNode = frameBlock.mFrameNodes[viewNode.mFrameNodeIndex];

  ComputeLocalToViewMatrix(viewNode.mLocalToView, frameNode.mLocalToWorld, viewBlock.mWorldToView);

  RenderFont* font = mFont->GetRenderFont(mFontSize);
  float pixelScale = 1.0f / mPixelsPerUnit;

  Vec2 center = GetLocalCenter();
  Vec2 widths = GetLocalWidths();
  Vec2 startLocation = center + Vec2(-widths.x, widths.y);

  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, mVertexColor);
  viewNode.mStreamedVertexType = PrimitiveType::Triangles;
  viewNode.mStreamedVertexStart = frameBlock.mRenderQueues->mStreamedVertices.Size();
  viewNode.mStreamedVertexCount = 0;

  ProcessTextRange(fontProcessor, font, mText, startLocation, mTextAlign, Vec2(1.0f, -1.0f) * pixelScale, widths * 2.0f);
}

String SpriteText::GetText()
{
  return mText;
}

void SpriteText::SetText(StringParam text)
{
  mText = text;
  UpdateBroadPhaseAabb();
}

Font* SpriteText::GetFont()
{
  return mFont;
}

void SpriteText::SetFont(Font* font)
{
  if (font == nullptr || font == mFont)
    return;

  mFont = font;
  UpdateBroadPhaseAabb();
}

uint SpriteText::GetFontSize()
{
  return mFontSize;
}

void SpriteText::SetFontSize(uint size)
{
  size = Math::Min(size, 128u);
  if (size == mFontSize)
    return;

  mFontSize = size;
  UpdateBroadPhaseAabb();
}

float SpriteText::GetPixelsPerUnit()
{
  return mPixelsPerUnit;
}

void SpriteText::SetPixelsPerUnit(float pixelsPerUnit)
{
  mPixelsPerUnit = pixelsPerUnit;
  UpdateBroadPhaseAabb();
}

TextAlign::Enum SpriteText::GetTextAlign()
{
  return mTextAlign;
}

void SpriteText::SetTextAlign(TextAlign::Enum textAlign)
{
  mTextAlign = textAlign;
  UpdateBroadPhaseAabb();
}

Vec2 SpriteText::MeasureText()
{
  return MeasureGivenText(mText);
}

Vec2 SpriteText::MeasureGivenText(StringParam text)
{
  if(Area* area = GetOwner()->has(Area))
  {
    RenderFont* font = mFont->GetRenderFont(mFontSize);
    FontProcessorNoRender noRender;
    return ProcessTextRange(noRender, font, text, Vec2(0.0f), mTextAlign, Vec2(1.0f, -1.0f) / mPixelsPerUnit, area->GetSize());
  }
  else
  {
    RenderFont* font = mFont->GetRenderFont(mFontSize);
    return font->MeasureText(text, 1.0f / mPixelsPerUnit);
  }
}

Vec3 SpriteText::GetCharacterPosition(int characterIndex)
{
  Vec2 center = GetLocalCenter();
  Vec2 widths = GetLocalWidths();
  Vec2 textStart = center + Vec2(-widths.x, widths.y);

  characterIndex = Math::Clamp(characterIndex, 0, (int)mText.SizeInBytes());

  Vec2 size;
  if (Area* area = GetOwner()->has(Area))
    size = area->GetSize();
  else
    size = MeasureText();

  RenderFont* font = mFont->GetRenderFont(mFontSize);

  FontProcessorFindCharPosition findPosition(characterIndex, textStart);
  ProcessTextRange(findPosition, font, mText, textStart, mTextAlign, Vec2(1.0f, -1.0f) / mPixelsPerUnit, size);

  return mTransform->TransformPoint(Vec3(findPosition.mCharPosition, 0.0f));
}

Vec2 SpriteText::GetLocalCenter()
{
  if (Area* area = mOwner->has(Area))
  {
    Vec2 size = area->GetSize();
    // Area offset is a normalized value
    Vec2 offset = ToOffset(area->GetOrigin());
    return offset * size;
  }
  else
  {
    Location::Enum origin;
    switch (mTextAlign)
    {
      case TextAlign::Left: origin = Location::CenterLeft; break;
      case TextAlign::Center: origin = Location::Center; break;
      case TextAlign::Right: origin = Location::CenterRight; break;
    }

    Vec2 size = MeasureText();
    Vec2 offset = OffsetOfOffset(origin, Location::Center);
    return offset * size;
  }
}

Vec2 SpriteText::GetLocalWidths( )
{
  if(Area* area = mOwner->has(Area))
    return area->GetSize( ) * 0.5f;
  else
    return MeasureText( ) * 0.5f;
}

ZilchDefineType(MultiSpriteEntry, builder, type)
{
  ZilchBindCopyConstructor();
  ZilchBindDestructor();

  ZilchBindGetterProperty(Index);
  ZilchBindGetterProperty(SpriteSource);
}

IntVec2 MultiSpriteEntry::GetIndex()
{
  return mIndex;
}

SpriteSource* MultiSpriteEntry::GetSpriteSource()
{
  return mSource;
}

//------------------------------------------------------------MultiSprite
ZilchDefineType(MultiSprite, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindFieldProperty(mAnimationActive);
  ZilchBindFieldProperty(mAnimationSpeed);

  ZilchBindMethod(Get);
  ZilchBindMethod(Set);
  ZilchBindMethod(Clear);
  ZilchBindMethod(All);
}

void MultiSprite::Serialize(Serializer& stream)
{
  BaseSprite::Serialize(stream);

  SerializeNameDefault(mAnimationActive, true);
  SerializeNameDefault(mAnimationSpeed, 1.0f);
}

void MultiSprite::Initialize(CogInitializer& initializer)
{
  BaseSprite::Initialize(initializer);
  mLocalAabb.SetCenterAndHalfExtents(Vec3::cZero, Vec3(0.5f));
  mFrameTime = 0;

  ConnectThisTo(GetSpace(), Events::LogicUpdate, OnLogicUpdate);
}

Aabb MultiSprite::GetLocalAabb()
{
  if (mGeometryMode == SpriteGeometryMode::ViewPlane)
    return GetViewPlaneAabb(mLocalAabb);

  return mLocalAabb;
}

void MultiSprite::ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock)
{
  GraphicalEntryData* entryData = frameNode.mGraphicalEntry->mData;

  Texture* atlas = (Texture*)entryData->mUtility;

  frameNode.mBorderThickness = 1.0f;
  frameNode.mRenderingType = RenderingType::Streamed;
  frameNode.mCoreVertexType = CoreVertexType::Streamed;

  frameNode.mMaterialRenderData = mMaterial->mRenderData;
  frameNode.mMeshRenderData = nullptr;
  frameNode.mTextureRenderData = atlas->mRenderData;

  frameNode.mLocalToWorld = mTransform->GetWorldMatrix();
  frameNode.mLocalToWorldNormal = Mat3::cIdentity;

  frameNode.mObjectWorldPosition = Vec3::cZero;

  frameNode.mBoneMatrixRange = IndexRange(0, 0);
}

void MultiSprite::ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock)
{
  GraphicalEntryData* entryData = viewNode.mGraphicalEntry->mData;

  Texture* atlas = (Texture*)entryData->mUtility;
  // Should never get extract calls on missing data
  GroupMap& groupMap = mGroupMaps[viewBlock.mCameraId];
  MultiSpriteTextureGroup& group = groupMap[atlas];

  FrameNode& frameNode = frameBlock.mFrameNodes[viewNode.mFrameNodeIndex];

  ComputeLocalToViewMatrix(viewNode.mLocalToView, frameNode.mLocalToWorld, viewBlock.mWorldToView);

  viewNode.mStreamedVertexType = PrimitiveType::Triangles;
  viewNode.mStreamedVertexStart = frameBlock.mRenderQueues->mStreamedVertices.Size();
  viewNode.mStreamedVertexCount = 0;

  forRange (MultiSpriteEntry& spriteEntry, group.mSpriteEntries.All())
  {
    IntVec2 index = spriteEntry.mIndex;
    SpriteSource* source = spriteEntry.mSource;

    Vec2 center = ToVec2(index) + Vec2(0.5, 0.5);
    Vec2 widths = Vec2(0.5, 0.5);

    Vec3 pos0 = Vec3(center, 0) + Vec3(-widths.x, widths.y, 0);
    Vec3 pos1 = Vec3(center, 0) + Vec3(widths.x, -widths.y, 0);

    uint frame = uint(mFrameTime / source->FrameDelay) % source->FrameCount;
    UvRect rect = source->GetUvRect(frame);

    Vec2 uv0 = rect.TopLeft;
    Vec2 uv1 = rect.BotRight;

    Vec2 uvAux0 = Vec2(0, 0);
    Vec2 uvAux1 = Vec2(1, 1);

    frameBlock.mRenderQueues->AddStreamedQuad(viewNode, pos0, pos1, uv0, uv1, mVertexColor, uvAux0, uvAux1);
  }
}

void MultiSprite::MidPhaseQuery(Array<GraphicalEntry>& entries, Camera& camera, Frustum* frustum)
{
  CogId cameraId = camera.GetOwner()->GetId();

  GroupMap& groupMap = mGroupMaps[cameraId];
  groupMap.Clear();

  if (frustum == nullptr)
  {
    forRange (MultiSpriteCell& cell, mCells.Values())
      AddCellEntries(cell, groupMap);
  }
  else
  {
    // We're going to get a world quad representing the entire multisprite area in order to clip it against the view frustum
    // The clipped region will contain the exact multisprite area visible to the view frustum
    Mat4 worldMatrix = mTransform->GetWorldMatrix();
    Vec3 localAabbCenter;
    Vec3 localAabbHalfExtents;
    mLocalAabb.GetCenterAndHalfExtents(localAabbCenter, localAabbHalfExtents);

    Vec3 worldQuadPoints[4];
    worldQuadPoints[0] = Math::TransformPoint(worldMatrix, localAabbCenter + (localAabbHalfExtents * Vec3(-1,  1, 0))); // "Top Left"
    worldQuadPoints[1] = Math::TransformPoint(worldMatrix, localAabbCenter + (localAabbHalfExtents * Vec3( 1,  1, 0))); // "Top Right"
    worldQuadPoints[2] = Math::TransformPoint(worldMatrix, localAabbCenter + (localAabbHalfExtents * Vec3( 1, -1, 0))); // "Bottom Right"
    worldQuadPoints[3] = Math::TransformPoint(worldMatrix, localAabbCenter + (localAabbHalfExtents * Vec3(-1, -1, 0))); // "Bottom Left"

    // Negate the frustum planes because ClipPolygonWithPlanes expects outward facing planes
    Vec4 frustumPlanes[6];
    frustumPlanes[0] = -frustum->Planes[0].mData;
    frustumPlanes[1] = -frustum->Planes[1].mData;
    frustumPlanes[2] = -frustum->Planes[2].mData;
    frustumPlanes[3] = -frustum->Planes[3].mData;
    frustumPlanes[4] = -frustum->Planes[4].mData;
    frustumPlanes[5] = -frustum->Planes[5].mData;

    // Clip the world quad against the frustum planes
    // This will produce a maximum of 8 clipped points
    Vec3 clippedPoints[Geometry::cMaxSupportPoints];
    uint clippedPointCount = Geometry::ClipPolygonWithPlanes(frustumPlanes, 6, worldQuadPoints, 4, clippedPoints);
    if (clippedPointCount == 0) // Outside frustum?
      return;

    // From the clipped region we create a surrounding AABB so that we can easily loop over all cells that are potentially within the frustum
    Aabb clippedAabb;
    clippedAabb.Compute(clippedPoints, clippedPointCount);

    Aabb localClippedAabb = Aabb::InverseTransform(clippedAabb, worldMatrix);
    Vec3 localClippedAabbMin = localClippedAabb.mMin;
    Vec3 localClippedAabbMax = localClippedAabb.mMax;

    IntVec2 entryLocationMin = IntVec2((int)Math::Floor(localClippedAabbMin.x), (int)Math::Floor(localClippedAabbMin.y));
    IntVec2 entryLocationMax = IntVec2((int)Math::Floor(localClippedAabbMax.x), (int)Math::Floor(localClippedAabbMax.y));

    IntVec2 cellIndexMin = LocationToCellIndex(entryLocationMin);
    IntVec2 cellIndexMax = LocationToCellIndex(entryLocationMax);

    IntVec2 cellIndexCount = (cellIndexMax - cellIndexMin);

    // Add the entries of all cells overlapping with the frustum
    for (int i = 0; i <= cellIndexCount.x ; ++i)
      for (int j = 0; j <= cellIndexCount.y; ++j)
        if (MultiSpriteCell* cell = mCells.FindPointer(cellIndexMin + IntVec2(i, j)))
        {
          Aabb aabb = cell->mLocalAabb.TransformAabb(worldMatrix);
          if (Overlap(aabb, *frustum))
            AddCellEntries(*cell, groupMap);
        }
  }

  typedef HashMap<Texture*, MultiSpriteTextureGroup>::pair EntryMapPair;
  forRange (EntryMapPair& pair, groupMap.All())
  {
    Texture* atlas = pair.first;
    GraphicalEntryData& entryData = pair.second.mGraphicalEntryData;

    entryData.mGraphical = this;
    entryData.mFrameNodeIndex = -1;
    entryData.mPosition = mTransform->GetWorldTranslation();
    entryData.mUtility = (u64)atlas;

    GraphicalEntry entry;
    entry.mData = &entryData;
    entry.mSort = 0;
      
    entries.PushBack(entry);
  }
}

bool MultiSprite::TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo)
{
  Ray localRay = rayCast.mRay.TransformInverse(mTransform->GetWorldMatrix());

  Intersection::IntersectionPoint point;
  Intersection::Type result = Intersection::RayPlane(localRay.Start, localRay.Direction, Vec3::cZAxis, 0.0f, &point);
  if (result == Intersection::None)
    return false;

  Vec3 pos = point.Points[0];
  IntVec2 location = IntVec2((int)Math::Floor(pos.x), (int)Math::Floor(pos.y));
  IntVec2 cellIndex = LocationToCellIndex(location);

  if (!mCells.ContainsKey(cellIndex))
    return false;

  if (!mCells[cellIndex].mEntries.ContainsKey(location))
    return false;

  rayCast.mObject = GetOwner();
  rayCast.mT = point.T;
  return true;
}

MultiSpriteEntry MultiSprite::Get(IntVec2 index)
{
  MultiSpriteEntry entry;
  entry.mIndex = index;
  
  IntVec2 cellIndex = LocationToCellIndex(index);
  if (mCells.ContainsKey(cellIndex))
    if (mCells[cellIndex].mEntries.ContainsKey(index))
      entry = mCells[cellIndex].mEntries[index];

  return entry;
}

void MultiSprite::Set(IntVec2 index, SpriteSource* spriteSource)
{
  if (spriteSource == nullptr)
  {
    Remove(index);
    return;
  }

  IntVec2 cellIndex = LocationToCellIndex(index);

  MultiSpriteEntry spriteEntry;
  spriteEntry.mIndex = index;
  spriteEntry.mSource = spriteSource;

  if (!mCells.ContainsKey(cellIndex))
  {
    if (mCells.Empty())
      mLocalAabb.SetInvalid();

    MultiSpriteCell& cell = mCells[cellIndex];
    Vec3 center = ToVector3(ToVec2(cellIndex) * (float)cMultiSpriteCellSize);
    Vec2 extents = Vec2(0.5f, 0.5f) * (float)cMultiSpriteCellSize;
    cell.mLocalAabb.SetCenterAndHalfExtents(center, ToVector3(extents, 0.5f));
    mLocalAabb.Combine(cell.mLocalAabb);
    UpdateBroadPhaseAabb();
  }

  mCells[cellIndex].mEntries[index] = spriteEntry;
}

void MultiSprite::Clear()
{
  mCells.Clear();
  mLocalAabb.SetCenterAndHalfExtents(Vec3::cZero, Vec3(0.5f));
  UpdateBroadPhaseAabb();
}

MultiSpriteEntryRange MultiSprite::All()
{
  MultiSpriteEntryRange range;
  forRange (MultiSpriteCell& cell, mCells.Values())
  {
    forRange (MultiSpriteEntry& entry, cell.mEntries.Values())
    {
      range.mEntries.PushBack(entry);
    }
  }

  return range;
}

void MultiSprite::Remove(IntVec2 index)
{
  IntVec2 cellIndex = LocationToCellIndex(index);
  if (mCells.ContainsKey(cellIndex))
  {
    mCells[cellIndex].mEntries.Erase(index);
    if (mCells[cellIndex].mEntries.Empty())
    {
      mCells.Erase(cellIndex);
      if (mCells.Empty())
      {
        mLocalAabb.SetCenterAndHalfExtents(Vec3::cZero, Vec3(0.5f));
      }
      else
      {
        mLocalAabb.SetInvalid();
        forRange (MultiSpriteCell& cell, mCells.Values())
          mLocalAabb.Combine(cell.mLocalAabb);
      }
      UpdateBroadPhaseAabb();
    }
  }
}

void MultiSprite::OnLogicUpdate(UpdateEvent* event)
{
  UpdateAnimation(event->Dt);
}

void MultiSprite::UpdateAnimation(float dt)
{
  if (mAnimationActive)
    mFrameTime += dt * mAnimationSpeed;
}

IntVec2 MultiSprite::LocationToCellIndex(IntVec2 location)
{
  IntVec2 cellIndex;
  cellIndex.x = (int)Math::Round(location.x / (float)cMultiSpriteCellSize);
  cellIndex.y = (int)Math::Round(location.y / (float)cMultiSpriteCellSize);
  return cellIndex;
}

void MultiSprite::AddCellEntries(MultiSpriteCell& cell, GroupMap& groupMap)
{
  typedef HashMap<IntVec2, MultiSpriteEntry>::pair EntryPair;
  forRange (EntryPair& pair, cell.mEntries.All())
  {
    IntVec2 entryIndex = pair.first;
    MultiSpriteEntry& spriteEntry = pair.second;

    Texture* atlas = spriteEntry.mSource->mTexture;
    groupMap[atlas].mSpriteEntries.PushBack(spriteEntry);
  }
}

} // namespace Zero
