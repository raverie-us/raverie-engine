///////////////////////////////////////////////////////////////////////////////
///
/// \file Sprite.hpp
/// Declaration of the Sprite component class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Common interface for 2D Sprite based graphicals.
class BaseSprite : public Graphical
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void ComponentAdded(BoundType* typeId, Component* component) override;
  void ComponentRemoved(BoundType* typeId, Component* component) override;

  // Graphical Interface

  String GetDefaultMaterialName() override;

  // Properties

  /// Color attribute of the generated vertices accessible in the vertex shader.
  Vec4 mVertexColor;

  /// How the Sprite should be oriented in 3D space.
  SpriteGeometryMode::Enum mGeometryMode;

  // Internal

  Aabb GetViewPlaneAabb(Aabb localAabb);
  void ComputeLocalToViewMatrix(Mat4& localToView, Mat4& localToWorld, Mat4& worldToView);
};

/// A generated quad that addresses atlased image data for efficient frame-based animations and batched rendering.
class Sprite : public BaseSprite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void DebugDraw() override;

  // Graphical Interface

  Aabb GetLocalAabb() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;

  // Properties

  /// The resource defining one or more image sequences used for frame-based animation.
  SpriteSource* GetSpriteSource();
  void SetSpriteSource(SpriteSource* spriteSource);
  HandleOf<SpriteSource> mSpriteSource;

  /// Flips the X axis of the Sprite's image (left/right).
  bool mFlipX;

  /// Flips the Y axis of the Sprite's image (top/bottom).
  bool mFlipY;

  /// If the Sprite animation should be playing on logic update, paused if false.
  bool mAnimationActive;

  /// Scalar to the amount of time passed used to advance frames of animation.
  float mAnimationSpeed;

  /// Index of the frame to start the animation on when the object is initialized, 0-based.
  uint GetStartFrame();
  void SetStartFrame(uint frameIndex);
  uint mStartFrame;

  /// Index of the frame the animation is currently on.
  uint GetCurrentFrame();
  void SetCurrentFrame(uint frameIndex);
  uint mCurrentFrame;

  // Internal

  Vec2 GetLocalCenter();
  Vec2 GetLocalWidths();
  void OnLogicUpdate(UpdateEvent* event);
  void UpdateAnimation(float dt);
  uint WrapIndex(uint index);

  float mFrameTime;
};

/// Text that is rendered from a texture atlas in the same way that Sprites are.
class SpriteText : public BaseSprite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  // Graphical Interface

  Aabb GetLocalAabb() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;

  // Properties

  /// Text to display.
  String GetText();
  void SetText(StringParam newText);
  String mText;

  /// Font used to display the text.
  Font* GetFont();
  void SetFont(Font* font);
  HandleOf<Font> mFont;

  /// Size that the font will be rastered at to a texture atlas.
  uint GetFontSize();
  void SetFontSize(uint size);
  uint mFontSize;

  /// Number of pixels of the font size that map to one world space unit.
  float GetPixelsPerUnit();
  void SetPixelsPerUnit(float pixelsPerUnit);
  float mPixelsPerUnit;

  /// How to position the text about the objects origin.
  TextAlign::Enum GetTextAlign();
  void SetTextAlign(TextAlign::Enum textAlign);
  TextAlign::Enum mTextAlign;

  /// Get the effective size in world space of the current text.
  Vec2 MeasureText();

  /// Get the effective size in world space that the SpriteText would be if this was its text.
  Vec2 MeasureGivenText(StringParam text);

  /// Get the position in world space of a character by index.
  Vec3 GetCharacterPosition(int characterIndex);

  // Internal

  Vec2 GetLocalCenter();
  Vec2 GetLocalWidths();
};

const uint cMultiSpriteCellSize = 8;

class MultiSpriteEntry
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  IntVec2 GetIndex();
  SpriteSource* GetSpriteSource();

  IntVec2 mIndex;
  HandleOf<SpriteSource> mSource;
};

class MultiSpriteEntryRange
{
public:
  typedef MultiSpriteEntry value_type;
  typedef MultiSpriteEntry& FrontResult;

  MultiSpriteEntryRange() : mIndex(0) {}

  FrontResult Front() {return mEntries[mIndex];}
  void PopFront() {++mIndex;}
  bool Empty() {return mIndex >= mEntries.Size();}

  uint mIndex;
  Array<MultiSpriteEntry> mEntries;
};

class MultiSpriteCell
{
public:
  Aabb mLocalAabb;
  HashMap<IntVec2, MultiSpriteEntry> mEntries;
};

class MultiSpriteTextureGroup
{
public:
  GraphicalEntryData mGraphicalEntryData;
  Array<MultiSpriteEntry> mSpriteEntries;
};

/// A grid of sprites that can be efficiently culled and rendered.
class MultiSprite : public BaseSprite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Component Interface

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  // Graphical Interface

  Aabb GetLocalAabb() override;
  void ExtractFrameData(FrameNode& frameNode, FrameBlock& frameBlock) override;
  void ExtractViewData(ViewNode& viewNode, ViewBlock& viewBlock, FrameBlock& frameBlock) override;
  void MidPhaseQuery(Array<GraphicalEntry>& entries, Camera& camera, Frustum* frustum) override;
  bool TestRay(GraphicsRayCast& rayCast, CastInfo& castInfo) override;

  // Properties

  /// If the Sprite animation should be playing on logic update, paused if false.
  bool mAnimationActive;

  /// Scalar to the amount of time passed used to advance frames of animation.
  float mAnimationSpeed;

  /// Gets an entry containing which SpriteSource is stored at the given index.
  MultiSpriteEntry Get(IntVec2 index);

  /// Set the SpriteSource to be used at the given index, passing null removes entry.
  void Set(IntVec2 index, SpriteSource* spriteSource);

  /// Removes all SpriteSource entries.
  void Clear();

  /// Returns a range containing all non-null entries.
  MultiSpriteEntryRange All();

  // Internal

  typedef HashMap<Texture*, MultiSpriteTextureGroup> GroupMap;

  void Remove(IntVec2 index);
  void OnLogicUpdate(UpdateEvent* event);
  void UpdateAnimation(float dt);
  IntVec2 LocationToCellIndex(IntVec2 location);
  void AddCellEntries(MultiSpriteCell& cell, GroupMap& groupMap);

  float mFrameTime;
  Aabb mLocalAabb;
  HashMap<IntVec2, MultiSpriteCell> mCells;
  HashMap<CogId, GroupMap> mGroupMaps;
};

} // namespace Zero
