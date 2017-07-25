///////////////////////////////////////////////////////////////////////////////
///
/// \file TilePaletteView.hpp
///
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

static const String DefaultTileArchetype = "DefaultTile";
static const String DefaultTileSpriteSource = "SquareBordered";
static const String DefaultTilePhysicsMesh = "Box";

//------------------------------------------------------------------------ Tile Sprite
ZilchDefineType(TilePaletteSprite, builder, type)
{
}

TilePaletteSprite::TilePaletteSprite(Composite* parent)
 : Composite(parent)
{
  static const String className = "TileSelect";
  mDefSet = mDefSet->GetDefinitionSet(className);
  mBackground = new TextureView(this);
  mBorder = CreateAttached<Element>(cBackground);
  mFrameDisplay = new TextureView(this);
  mBackground->SetTexture(TextureManager::Find("AlphaBackground"));
  mFrameOverlay = NULL;
  mBorderEnabled = true;
  mMinSize = Vec2(128, 128);
  mFrameOverlay = NULL;
}

TilePaletteSprite::~TilePaletteSprite()
{

}

void TilePaletteSprite::SetBackground(bool background)
{
  mBorderEnabled = background;
  mBackground->SetActive(background);
  mBorder->SetActive(background);
}

void TilePaletteSprite::SetFrame(uint frameIndex, SpriteSource* sprite)
{
  UvRect rect = sprite->GetUvRect(frameIndex);
  mFrameDisplay->SetTexture(sprite->mTexture);
  mFrameDisplay->SetUv(rect.TopLeft, rect.BotRight);
}

void TilePaletteSprite::SetFrameOverlay(StringParam textureName, UvRect uv)
{
  if (!mFrameOverlay)
    mFrameOverlay = new TextureView(this);

  mFrameOverlay->SetTexture(TextureManager::Find(textureName));
  mFrameOverlay->SetUv(uv.TopLeft, uv.BotRight);
}

void TilePaletteSprite::RemoveFrameOverlay()
{
  if (mFrameOverlay)
  {
    mFrameOverlay->Destroy();
    mFrameOverlay = NULL;
  }
}

Vec2 TilePaletteSprite::GetSizeNeeded(Vec2 frameSize)
{
  return ExpandSizeByThickness(mBorder->GetBorderThickness(), frameSize);
}

void TilePaletteSprite::UpdateTransform()
{
  if (mBorderEnabled)
  {
    LayoutResult lr = RemoveThickness(mBorder->GetBorderThickness(), mSize);
    mBackground->SetUv(Vec2(0, 0), (mSize / 2.0f) / 8.0f);
    mBorder->SetSize(mSize);
    mFrameDisplay->SetTranslationAndSize(lr.Translation, lr.Size);
    mBackground->SetTranslationAndSize(lr.Translation, lr.Size);
  }
  else
  {
    mFrameDisplay->SetTranslationAndSize(Vec3(0, 0, 0), mSize);
  }

  if (mFrameOverlay)
    mFrameOverlay->SetTranslationAndSize(Vec3(0, 0, 0), mSize);
  
  Composite::UpdateTransform();
}

//------------------------------------------------------------------------ Tile Palette Operation
void TilePaletteOperation::Undo()
{
  TilePaletteSource* palette = mTilePaletteSource;
  if (!palette) return;

  forRange (TilePaletteChange change, mChangeList.All())
  {
    if (change.mChangeType == TilePaletteChange::Edited)
      palette->mData[change.mLocation] = change.mOldTile;
    else if (change.mChangeType == TilePaletteChange::Created)
      palette->mData.Erase(change.mLocation);
    else if (change.mChangeType == TilePaletteChange::Removed)
      palette->mData[change.mLocation] = change.mOldTile;
  }

  MetaOperations::NotifyObjectModified(palette);

  TilePaletteView* paletteView = mTilePaletteView;
  if (!paletteView) return;

  if (paletteView->GetTilePalette() == palette)
    paletteView->LoadPalette();
}

void TilePaletteOperation::Redo()
{
  TilePaletteSource* palette = mTilePaletteSource;
  if (!palette) return;

  forRange (TilePaletteChange change, mChangeList.All())
  {
    if (change.mChangeType == TilePaletteChange::Edited)
      palette->mData[change.mLocation] = change.mNewTile;
    else if (change.mChangeType == TilePaletteChange::Created)
      palette->mData[change.mLocation] = change.mNewTile;
    else if (change.mChangeType == TilePaletteChange::Removed)
      palette->mData.Erase(change.mLocation);
  }

  MetaOperations::NotifyObjectModified(palette);

  TilePaletteView* paletteView = mTilePaletteView.Get<TilePaletteView*>();
  if (!paletteView) return;

  if (paletteView->GetTilePalette() == palette)
    paletteView->LoadPalette();
}

class PaletteText : public Text
{
public:
  PaletteText(Composite* parent) : Text(parent, "Text")
  {
    mFont = FontManager::GetDefault()->GetRenderFont(18);
  }
};

const String TilePaletteView::mCollisionTextureName = "TileCollisionSheet";

ZilchDefineType(TilePaletteView, builder, type)
{
  // METAREFACTOR - We don't want to inherit base class meta..
  // Can't bind base or composite properties will be exposed
  //BindBase(EventObject);

  ZilchBindGetterSetterProperty(TilePalette)->Add(new EditorResource(true, false));

  ZilchBindGetterSetterProperty(Archetype);

  ZilchBindGetterSetterProperty(Sprite)->Add(new EditorResource(true, true));

  ZilchBindGetterSetterProperty(Collision)->Add(new EditorResource(true, true, "TileMesh"));

  ZilchBindGetterSetterProperty(Mergeable);
}

TilePaletteView::TilePaletteView(Composite* parent, TileEditor2D* editor)
  :Composite(parent), mEditor(editor), mShowCollision(true)
{
  mTileSize = 64;
  mSelectionStart = IntVec2(0, 0);
  mSelectionEnd = IntVec2(0, 0);
  mMouseDown = false;

  mScrollArea = new ScrollArea(this);
  mPaletteArea = mScrollArea->GetClientWidget();

  static const String className = "TileSelect";
  mPaletteArea->mDefSet = mDefSet->GetDefinitionSet(className);

  mActiveBorder = mPaletteArea->mDefSet->GetDefinitionSet("BackgroundFocus");
  mInactiveBorder = mPaletteArea->mDefSet->GetDefinitionSet("Background");

  mSelectionBorder = mPaletteArea->CreateAttached<Element>("Background");
  mSelectionBorder->SetInteractive(false);
  mSelectionBorder->SetSize(Vec2(1, 1) * real(mTileSize));
  mSelectionBorder->SetVisible(false);

  mPaletteText = new PaletteText(mPaletteArea);
  mPaletteText->SetText("No TilePalette Selected");
  mPaletteText->SizeToContents();

  SetDefaultResources();

  ConnectThisTo(Z::gResources, Events::ResourceRemoved, OnResourceRemoved);
}

TilePaletteView::~TilePaletteView()
{
  Z::gResources->GetDispatcher()->Disconnect(this);
}

void TilePaletteView::ActivatePaletteUi()
{
  ConnectThisTo(mPaletteArea, Events::LeftMouseDown, OnMouseDownArea);
  ConnectThisTo(mPaletteArea, Events::LeftMouseUp, OnMouseUpArea);
  ConnectThisTo(mPaletteArea, Events::MouseMove, OnMouseMoveArea);
  ConnectThisTo(mPaletteArea, Events::RightMouseDown, OnRightMouseDownArea);
  ConnectThisTo(mPaletteArea, Events::MouseScroll, OnMouseScroll);

  mSelectionBorder->SetVisible(true);
  SetSelectionBorderActive(true);

  mPaletteText->SetVisible(false);
  mPaletteText->SetInteractive(false);
}

void TilePaletteView::DeactivatePaletteUi()
{
  mPaletteArea->GetDispatcher()->Disconnect(this);
  mSelectionBorder->SetVisible(false);

  mPaletteText->SetVisible(true);
  mPaletteText->SetInteractive(true);

  ClearPaletteTiles();
  SetDefaultResources();
}

void TilePaletteView::ClearPaletteTiles()
{
  forRange (auto pair, mPaletteTiles.All())
  {
    pair.second.frame->Destroy();
  }
  mPaletteTiles.Clear();
}

TilePaletteSource* TilePaletteView::GetTilePalette()
{
  return mPaletteSource;
}

void TilePaletteView::SetTilePalette(TilePaletteSource* paletteSource)
{
  mSelectionStart = IntVec2(0, 0);
  mSelectionEnd = IntVec2(0, 0);

  if (paletteSource && !mPaletteSource)
    ActivatePaletteUi();
  else if (!paletteSource)
    DeactivatePaletteUi();

  mPaletteSource = paletteSource;
  if(mEditor)
    mEditor->SetTileMapPalette(mPaletteSource);
  LoadPalette();
}

void TilePaletteView::SetShowCollision(bool showCollision)
{
  if (showCollision != mShowCollision)
  {
    if (showCollision)
      AddCollisionOverlays();
    else
      RemoveCollisionOverlays();
  }

  mShowCollision = showCollision;
}

void TilePaletteView::AddCollisionOverlays()
{
  forRange (auto pair, mPaletteTiles.All())
  {
    TilePaletteEntry& entry = pair.second;
    if (PhysicsMesh* collision = entry.tile.GetCollisionResource())
      entry.frame->SetFrameOverlay(mCollisionTextureName, mEditor->GetCollisionTextureUv(collision->Name));
  }
}

void TilePaletteView::RemoveCollisionOverlays()
{
  forRange (auto pair, mPaletteTiles.All())
  {
    pair.second.frame->RemoveFrameOverlay();
  }
}

TilePaletteSprite* TilePaletteView::CreateTilePaletteSprite(SpriteSource* sprite, PhysicsMesh* collision)
{
  TilePaletteSprite* frame = new TilePaletteSprite(mPaletteArea);

  if (sprite)
      frame->SetFrame(0, sprite);
    else
      frame->SetFrame(0, SpriteSourceManager::GetDefault());
  
  frame->SetBackground(false);

  if (collision && mShowCollision)
  {
    if(mEditor)
      frame->SetFrameOverlay(mCollisionTextureName, mEditor->GetCollisionTextureUv(collision->Name));
    else
      frame->SetFrameOverlay(mCollisionTextureName, UvRect{ Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f) });
  }

  return frame;
}

void TilePaletteView::LoadPalette()
{
  if (mPaletteSource)
  {
    ClearPaletteTiles();

    forRange (auto pair, mPaletteSource->mData.All())
    {
      TilePaletteEntry entry;
      entry.frame = CreateTilePaletteSprite(pair.second.GetSpriteResource(), pair.second.GetCollisionResource());
      entry.tile = pair.second;
      // Use defaults if a resource was removed
      if (entry.tile.GetArchetypeResource())
        entry.tile.ArchetypeResource = ArchetypeManager::Find(DefaultTileArchetype)->mResourceId;
      /* METAREFACTOR - The handle has "valid" data, but cannot be resolved (resource was removed)
      if (entry.tile.CollisionResource.IsNotNullAndCantResolve())
        entry.tile.CollisionResource = PhysicsMeshManager::Find(DefaultTilePhysicsMesh);
      if(entry.tile.SpriteResource.IsNotNullAndCantResolve())
        entry.tile.SpriteResource = SpriteSourceManager::Find(DefaultTileSpriteSource);
      */
      mPaletteTiles.Insert(pair.first, entry);
    }

    if (mPaletteTiles.Empty())
      CreateNewEntry(IntVec2(0, 0), false);
  }

  RefreshSelection();
}

PropertyState TilePaletteView::GetTilePaletteState()
{
  return PropertyState((TilePaletteSource*)mPaletteSource);
}

PropertyState TilePaletteView::GetArchetypeState()
{
  if (!mPaletteSource)
    return PropertyState((Archetype*)mArchetype);

  SelectionRange range(&mPaletteTiles, mSelectionStart, mSelectionEnd);
  if (range.Empty())
    return PropertyState();

  Archetype* firstValue = range.Front().mEntry->tile.GetArchetypeResource();

  forRange (SelectionRange::Selection selection, range)
  {
    TilePaletteEntry* entry = selection.mEntry;

    if ((Archetype*)entry->tile.GetArchetypeResource() != firstValue)
      return PropertyState();
  }

  return PropertyState(firstValue);
}

PropertyState TilePaletteView::GetSpriteState()
{
  if (!mPaletteSource)
    return PropertyState((SpriteSource*)mSprite);

  SelectionRange range(&mPaletteTiles, mSelectionStart, mSelectionEnd);
  if (range.Empty())
    return PropertyState();

  SpriteSource* firstValue = range.Front().mEntry->tile.GetSpriteResource();

  forRange (SelectionRange::Selection selection, range)
  {
    TilePaletteEntry* entry = selection.mEntry;

    if ((SpriteSource*)entry->tile.GetSpriteResource() != firstValue)
      return PropertyState();
  }

  return PropertyState(firstValue);
}

PropertyState TilePaletteView::GetCollisionState()
{
  if (!mPaletteSource)
    return PropertyState((PhysicsMesh*)mCollision);

  SelectionRange range(&mPaletteTiles, mSelectionStart, mSelectionEnd);
  if (range.Empty())
    return PropertyState();

  PhysicsMesh* firstValue = range.Front().mEntry->tile.GetCollisionResource();

  forRange (SelectionRange::Selection selection, range)
  {
    TilePaletteEntry* entry = selection.mEntry;

    if ((PhysicsMesh*)entry->tile.GetCollisionResource() != firstValue)
      return PropertyState();
  }

  return PropertyState(firstValue);
}

PropertyState TilePaletteView::GetMergeableState()
{
  if (!mPaletteSource)
    return PropertyState(mMergeable);

  SelectionRange range(&mPaletteTiles, mSelectionStart, mSelectionEnd);
  if (range.Empty())
    return PropertyState();

  bool firstValue = range.Front().mEntry->tile.Merge;

  forRange (SelectionRange::Selection selection, range)
  {
    TilePaletteEntry* entry = selection.mEntry;

    if (entry->tile.Merge != firstValue)
      return PropertyState();
  }

  return PropertyState(firstValue);
}

Archetype* TilePaletteView::GetArchetype()
{
  return mArchetype;
}

SpriteSource* TilePaletteView::GetSprite()
{
  return mSprite;
}

PhysicsMesh* TilePaletteView::GetCollision()
{
  return mCollision;
}

bool TilePaletteView::GetMergeable()
{
  return mMergeable;
}

void TilePaletteView::SetArchetype(Archetype* newResource)
{
  mArchetype = newResource;

  if (mSelectionStart == mSelectionEnd && mPaletteTiles.Find(mSelectionStart).Empty())
  {
    CreateNewEntry(mSelectionStart, false);
  }
  else
  {
    forRange (SelectionRange::Selection selection, SelectionRange(&mPaletteTiles, mSelectionStart, mSelectionEnd))
    {
      TilePaletteEntry* entry = selection.mEntry;

      entry->tile.ArchetypeResource = mArchetype->mResourceId;
      AddToPaletteSource(selection.mLocation, entry->tile);
    }
  }

  RefreshSelection();
}

void TilePaletteView::SetSprite(SpriteSource* newResource)
{
  mSprite = newResource;

  if (mSelectionStart == mSelectionEnd && mPaletteTiles.Find(mSelectionStart).Empty())
  {
    CreateNewEntry(mSelectionStart, false);
  }
  else
  {
    forRange (SelectionRange::Selection selection, SelectionRange(&mPaletteTiles, mSelectionStart, mSelectionEnd))
    {
      TilePaletteEntry* entry = selection.mEntry;

      entry->tile.SpriteResource = mSprite->mResourceId;
      AddToPaletteSource(selection.mLocation, entry->tile);

      if (mSprite)
        entry->frame->SetFrame(0, mSprite);
      else
        entry->frame->SetFrame(0, SpriteSourceManager::GetDefault());
    }
  }

  RefreshSelection();
}

void TilePaletteView::SetCollision(PhysicsMesh* newResource)
{
  mCollision = newResource;

  if (mSelectionStart == mSelectionEnd && mPaletteTiles.Find(mSelectionStart).Empty())
  {
    CreateNewEntry(mSelectionStart, false);
  }
  else
  {
    forRange (SelectionRange::Selection selection, SelectionRange(&mPaletteTiles, mSelectionStart, mSelectionEnd))
    {
      TilePaletteEntry* entry = selection.mEntry;

      entry->tile.CollisionResource = mCollision->mResourceId;
      AddToPaletteSource(selection.mLocation, entry->tile);

      if (mCollision && mShowCollision)
      {
        PhysicsMesh* collision = mCollision;
        entry->frame->SetFrameOverlay(mCollisionTextureName, mEditor->GetCollisionTextureUv(collision->Name));
      }
      else
      {
        entry->frame->RemoveFrameOverlay();
      }
    }
  }

  RefreshSelection();
}

void TilePaletteView::SetMergeable(bool mergeable)
{
  mMergeable = mergeable;

  if (mSelectionStart == mSelectionEnd && mPaletteTiles.Find(mSelectionStart).Empty())
  {
    CreateNewEntry(mSelectionStart, false);
  }
  else
  {
    forRange (SelectionRange::Selection selection, SelectionRange(&mPaletteTiles, mSelectionStart, mSelectionEnd))
    {
      TilePaletteEntry* entry = selection.mEntry;

      entry->tile.Merge = mMergeable;
      AddToPaletteSource(selection.mLocation, entry->tile);
    }
  }

  RefreshSelection();
}

Vec2 TilePaletteView::Measure(LayoutArea& data)
{
  // At least 3 tiles in size
  return Vec2(1, 1) * (mTileSize * 3.0f);
}

void TilePaletteView::UpdateTransform()
{
  IntVec2 max = IntVec2(0, 0);
  float tileSize = float(mTileSize);

  //Update all SpriteFrame / PaletteTiles translations
  forRange (PalettePair pair, mPaletteTiles.All())
  {
    IntVec2 location = pair.first;

    // Compute max tile location
    max.x = Math::Max(location.x, max.x);
    max.y = Math::Max(location.y, max.y);

    // Update SpriteFrame
    TilePaletteSprite* frame = pair.second.frame;
    pair.second.frame->SetSize(Pixels(tileSize, tileSize));
    pair.second.frame->SetTranslation(Vec3(location.x * tileSize, location.y * tileSize, 0));
  }

  max += IntVec2(1, 1);

  mScrollArea->SetSize(mSize);
  mScrollArea->SetClientSize(Vec2(real(max.x + 0.5), real(max.y + 0.5)) * tileSize);

  Vec2 minSelect = Vec2(real(Math::Min(mSelectionStart.x, mSelectionEnd.x)), real(Math::Min(mSelectionStart.y, mSelectionEnd.y)));
  Vec2 maxSelect = Vec2(real(Math::Max(mSelectionStart.x, mSelectionEnd.x)), real(Math::Max(mSelectionStart.y, mSelectionEnd.y)));

  mSelectionBorder->SetTranslation(Math::ToVector3(minSelect, 0) * tileSize);
  mSelectionBorder->SetSize((maxSelect - minSelect + Vec2(1, 1)) * real(mTileSize));

  mSelectionBorder->MoveToFront();

  Composite::UpdateTransform();
}

void TilePaletteView::SetSelectionBorderActive(bool active)
{
  if (active)
    mSelectionBorder->ChangeDefinition(mActiveBorder);
  else
    mSelectionBorder->ChangeDefinition(mInactiveBorder);
}

void TilePaletteView::RefreshSelection()
{
  if (!mPaletteSource)
  {
    TileMapSelection selection;
    selection.Offset = IntVec2(0, 0);
    selection.Width = 1;
    ResourceId archetypeId = mArchetype.IsNotNull() ? mArchetype->mResourceId : 0;
    ResourceId spriteId = mSprite.IsNotNull() ? mSprite->mResourceId : 0;
    ResourceId collisionId = mCollision.IsNotNull() ? mCollision->mResourceId : 0;
    Tile tile(archetypeId, spriteId, collisionId, mMergeable);
    selection.Tiles.PushBack(tile);
    if (mEditor)
      mEditor->SetSelection(selection);
    return;
  }

  IntVec2 minSelect = IntVec2(Math::Min(mSelectionStart.x, mSelectionEnd.x), Math::Min(mSelectionStart.y, mSelectionEnd.y));
  IntVec2 maxSelect = IntVec2(Math::Max(mSelectionStart.x, mSelectionEnd.x), Math::Max(mSelectionStart.y, mSelectionEnd.y));

  TileMapSelection selection;
  selection.Offset = IntVec2(0, 0);
  selection.Width = maxSelect.x - minSelect.x + 1;
  for (int y = minSelect.y; y <= maxSelect.y; ++y)
  {
    for (int x = minSelect.x; x <= maxSelect.x; ++x)
    {
      TilePaletteEntry* entry = mPaletteTiles.FindPointer(IntVec2(x, y));
      if (entry)
        selection.Tiles.PushBack(entry->tile);
      else
        selection.Tiles.PushBack(Tile());
    }
  }

  SetResourcesFromEntry(mPaletteTiles.FindPointer(minSelect));
  if (mEditor)
    mEditor->SetSelection(selection);

  SetSelectionBorderActive(true);

  CreateChangeOperation();
  MarkAsNeedsUpdate();
}

void TilePaletteView::SetDefaultResources()
{
  mArchetype = (Archetype*)ArchetypeManager::Find(DefaultTileArchetype);
  mSprite = (SpriteSource*)SpriteSourceManager::Find(DefaultTileSpriteSource);
  mCollision = (PhysicsMesh*)PhysicsMeshManager::Find(DefaultTilePhysicsMesh);
  mMergeable = true;
}

void TilePaletteView::SetResourcesFromEntry(TilePaletteEntry* entry)
{
  if (entry)
  {
    mArchetype = entry->tile.GetArchetypeResource();
    mSprite = entry->tile.GetSpriteResource();
    mCollision = entry->tile.GetCollisionResource();
    mMergeable = entry->tile.Merge;
  }
}

void TilePaletteView::CreateNewEntry(IntVec2 location, bool defaults)
{
  if (!mPaletteSource)
    return;

  if (defaults)
    SetDefaultResources();

  if (mSprite == nullptr)
    return;

  TilePaletteEntry entry;
  entry.frame = CreateTilePaletteSprite(mSprite, mCollision);
  entry.tile.ArchetypeResource = mArchetype->mResourceId;
  entry.tile.SpriteResource = mSprite->mResourceId;
  entry.tile.CollisionResource = mCollision->mResourceId;
  entry.tile.Merge = mMergeable;
  mPaletteTiles[location] = entry;

  AddToPaletteSource(location, entry.tile);
}

void TilePaletteView::DeleteEntry(IntVec2 location)
{
  TilePaletteEntry* entry = mPaletteTiles.FindPointer(location);
  if (entry)
  {
    entry->frame->Destroy();
    mPaletteTiles.Erase(location);

    RemoveFromPaletteSource(location);
  }
}

void TilePaletteView::AddToPaletteSource(IntVec2 location, Tile tile)
{
  if (mPaletteSource)
  {
    TilePaletteChange change(location);
    change.mNewTile = tile;
    if (mPaletteSource->mData.Find(location).Empty())
    {
      change.mChangeType = TilePaletteChange::Created;
    }
    else
    {
      change.mChangeType = TilePaletteChange::Edited;
      change.mOldTile = mPaletteSource->mData[location];
    }

    mPaletteSource->mData[location] = tile;

    MetaOperations::NotifyObjectModified(mPaletteSource);

    mPaletteChangeList.PushBack(change);
  }
}

void TilePaletteView::RemoveFromPaletteSource(IntVec2 location)
{
  TilePaletteChange change(location);
  change.mOldTile = mPaletteSource->mData[location];
  change.mChangeType = TilePaletteChange::Removed;

  mPaletteSource->mData.Erase(location);
  MetaOperations::NotifyObjectModified(mPaletteSource);

  mPaletteChangeList.PushBack(change);
}

void TilePaletteView::CreateChangeOperation()
{
  if (!mPaletteChangeList.Empty())
  {
    TilePaletteOperation* op = new TilePaletteOperation;
    op->mTilePaletteView = this;
    op->mTilePaletteSource = mPaletteSource;
    op->mChangeList = mPaletteChangeList;

    OperationQueue* queue = Z::gEditor->GetOperationQueue();
    queue->Queue(op);

    mPaletteChangeList.Clear();
  }
}

IntVec2 TilePaletteView::GetTileLocation(Vec2 localOffset)
{
  float tileSize = float(mTileSize);
  int x = int(localOffset.x / tileSize);
  int y = int(localOffset.y / tileSize);
  return IntVec2(x, y);
}

void TilePaletteView::OnResourceRemoved(ResourceEvent* event)
{
  if (event->EventResource == mPaletteSource)
    SetTilePalette(nullptr);
}

void TilePaletteView::OnMouseDownArea(MouseEvent* event)
{
  //Get local offset in client area
  Vec2 localOffset = mPaletteArea->ToLocal(event->Position);

  //Which tile did we hit?
  IntVec2 location = GetTileLocation(localOffset);

  mMouseDown = true;
  mSelectionStart = location;
  mSelectionEnd = location;

  MarkAsNeedsUpdate();
}

void TilePaletteView::OnMouseUpArea(MouseEvent* event)
{
  if (!mMouseDown)
    return;

  mMouseDown = false;

  if (mSelectionStart == mSelectionEnd)
  {
    Vec2 localOffset = mPaletteArea->ToLocal(event->Position);
    IntVec2 location = GetTileLocation(localOffset);

    TilePaletteEntry* entry = mPaletteTiles.FindPointer(location);
    if(entry)
      SetResourcesFromEntry(entry);
    else
      CreateNewEntry(location);
  }

  RefreshSelection();
}

void TilePaletteView::OnMouseMoveArea(MouseEvent* event)
{
  if (mMouseDown)
  {
    Vec2 localOffset = mPaletteArea->ToLocal(event->Position);
    IntVec2 location = GetTileLocation(localOffset);
    mSelectionEnd = location;

    MarkAsNeedsUpdate();
  }
}

void TilePaletteView::OnMouseScroll(MouseEvent* event)
{
  if(event->CtrlPressed)
  {
    mTileSize = Math::Clamp(int(mTileSize + event->Scroll.y), 20, 100);
    MarkAsNeedsUpdate();
  }
}

void TilePaletteView::OnRightMouseDownArea(MouseEvent* event)
{
  Vec2 localOffset = mPaletteArea->ToLocal(event->Position);
  IntVec2 location = GetTileLocation(localOffset);

  if (event->ShiftPressed)
  {
    const TileMapSelection& selection = mEditor->GetSelection();
    int width = selection.Width;
    int height = selection.Tiles.Size() / width;
    for (uint i = 0; i < selection.Tiles.Size(); ++i)
    {
      IntVec2 pos = IntVec2(location.x + i % width, location.y + i / width);

      if (selection.Tiles[i].GetArchetypeResource() == nullptr)
      {
        DeleteEntry(pos);
        continue;
      }

      SpriteSource* sprite = selection.Tiles[i].GetSpriteResource( );
      PhysicsMesh* collision = selection.Tiles[i].GetCollisionResource( );

      TilePaletteEntry* currentEntry = mPaletteTiles.FindPointer(pos);
      if (currentEntry)
      {
        currentEntry->tile = selection.Tiles[i];

        if (sprite)
          currentEntry->frame->SetFrame(0, sprite);
        else
          currentEntry->frame->SetFrame(0, SpriteSourceManager::GetDefault());

        if (collision && mShowCollision)
          currentEntry->frame->SetFrameOverlay(mCollisionTextureName, mEditor->GetCollisionTextureUv(collision->Name));
        else
          currentEntry->frame->RemoveFrameOverlay();

        AddToPaletteSource(pos, currentEntry->tile);
      }
      else
      {
        TilePaletteEntry newEntry;
        newEntry.frame = CreateTilePaletteSprite(sprite, collision);
        newEntry.tile = selection.Tiles[i];
        mPaletteTiles[pos] = newEntry;
        AddToPaletteSource(pos, newEntry.tile);
      }
    }

    if (selection.Tiles.Size() == 1)
    {
      TilePaletteEntry* entry = mPaletteTiles.FindPointer(location);
      if (entry)
        SetResourcesFromEntry(entry);
    }

    mSelectionStart = location;
    mSelectionEnd = mSelectionStart + IntVec2(width - 1, height - 1);
  }
  else
  {
    DeleteEntry(location);
  }

  RefreshSelection();
}

TilePaletteView::SelectionRange::SelectionRange(TilePaletteView::PaletteEntryMap* paletteTiles, IntVec2 selectionStart, IntVec2 selectionEnd)
  : mPaletteTiles(paletteTiles)
{
  mSelectionMin = IntVec2(Math::Min(selectionStart.x, selectionEnd.x), Math::Min(selectionStart.y, selectionEnd.y));
  mSelectionMax = IntVec2(Math::Max(selectionStart.x, selectionEnd.x), Math::Max(selectionStart.y, selectionEnd.y));

  // -1 for popFront logic to get first valid entry
  mSelection.x = mSelectionMin.x - 1;
  mSelection.y = mSelectionMin.y;

  PopFront();
}

bool TilePaletteView::SelectionRange::Empty()
{
  return mSelection.y > mSelectionMax.y;
}

TilePaletteView::SelectionRange::Selection TilePaletteView::SelectionRange::Front()
{
  return Selection(mCurrentEntry, mSelection);
}

void TilePaletteView::SelectionRange::PopFront()
{
  ++mSelection.x;

  for (; mSelection.y <= mSelectionMax.y; ++mSelection.y)
  {
    for (; mSelection.x <= mSelectionMax.x; ++mSelection.x)
    {
      mCurrentEntry = mPaletteTiles->FindPointer(mSelection);
      if (mCurrentEntry) return;
    }

    mSelection.x = mSelectionMin.x;
  }
}

}
