////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2013, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace
{
  void ComputeCorners(IntVec2 pos, IntVec2 scale, Transform* transform, Vec3 quad[4])
  {
    quad[0] = Vec3(real(pos.x),           real(pos.y - scale.y + 1), 0.0);
    quad[1] = Vec3(real(pos.x + scale.x), real(pos.y - scale.y + 1), 0.0);
    quad[2] = Vec3(real(pos.x + scale.x), real(pos.y + 1),           0.0);
    quad[3] = Vec3(real(pos.x),           real(pos.y + 1),           0.0);

    for (uint i = 0; i < 4; ++i)
      quad[i] = transform->TransformPoint(quad[i]);
  }

  void DebugDrawQuad(IntVec2 pos, IntVec2 scale, Transform* transform, ByteColor color)
  {
    Vec3 quad[4];
    ComputeCorners(pos, scale, transform, quad);

    gDebugDraw->Add(Debug::Triangle(quad[0], quad[1], quad[2]).Color(color).Alpha(50));
    gDebugDraw->Add(Debug::Triangle(quad[0], quad[2], quad[3]).Color(color).Alpha(50));
  }

  void DebugDrawOutline(IntVec2 pos, IntVec2 scale, Transform* transform, ByteColor color)
  {
    Vec3 quad[4];
    ComputeCorners(pos, scale, transform, quad);

    for (int i = 0, j = 3; i < 4; j = i++)
      gDebugDraw->Add(Debug::Line(quad[j], quad[i]).Color(color));
  }

  void DebugDrawCross(IntVec2 pos, IntVec2 scale, Transform* transform, ByteColor color)
  {
    Vec3 quad[4];
    ComputeCorners(pos, scale, transform, quad);

    gDebugDraw->Add(Debug::Line(quad[0], quad[2]).Color(color));
    gDebugDraw->Add(Debug::Line(quad[1], quad[3]).Color(color));
  }

  void DebugDrawTriangle(IntVec2 pos, Vec3 points[3], Transform* transform, ByteColor color)
  {
    for (uint i = 0; i < 3; ++i)
      points[i] = transform->TransformPoint(Vec3(real(points[i].x + pos.x + 0.5), real(points[i].y + pos.y + 0.5), 0.0));

    gDebugDraw->Add(Debug::Triangle(points[0], points[1], points[2]).Color(color).Alpha(50));
  }

  void DebugDrawCoordinates(IntVec2 pos, int size, int width, Transform* transform, ByteColor color)
  {
    int height = size / width;
    IntVec2 min(pos.x, pos.y - height + 1);
    IntVec2 max(pos.x + width - 1, pos.y);

    Vec3 textPos = transform->TransformPoint(Vec3(min.x + width / 2.0f, max.y + 1.5f, 0.0f));
    float textSize = 0.25f;
    String text;
    if (size == 1)
      text = String::Format("(%d,%d)", min.x, min.y);
    else
      text = String::Format("[(%d, %d), (%d,%d)]", min.x, min.y, max.x, max.y);

    gDebugDraw->Add(Debug::Text(textPos, textSize, text).Color(color).Centered(true).ViewAligned(false));
  }

  real RainbowColor(real value)
  {
    if (value <= 0.167)
      return 1.0;
    else if (value <= 0.333)
      return real(1.0 - (value - 0.167) / 0.167);
    else if (value <= 0.666)
      return 0.0;
    else if (value <= 0.833)
      return real((value - 0.666) / 0.167);
    else if (value <= 1.167)
      return 1.0;
    else if (value <= 1.333)
      return real(1.0 - (value - 1.167) / 0.167);
    else
      return 0.0;
  }

  void InitializeCollisionUvMap(HashMap<String, UvRect>& collisionTextureUv)
  {
    float textureSize = 256.0f;

    Array<UvRect> texRects;
    for (uint y = 0; y < 5; ++y)
    for (uint x = 0; x < 5; ++x)
    {
      Vec2 topleft = Vec2(51.0f * x, 51.0f * y) / 256.0f;
      Vec2 dim = Vec2(52.0f, 52.0f) / 256.0f;
      UvRect rect = {topleft, topleft + dim};
      texRects.PushBack(rect);
    }

    collisionTextureUv["Box"]                  = texRects[0];
    collisionTextureUv["HalfBoxTop"]           = texRects[1];
    collisionTextureUv["HalfBoxBottom"]        = texRects[2];
    collisionTextureUv["DoubleSlopeLeft2"]     = texRects[3];
    collisionTextureUv["DoubleSlopeRight2"]    = texRects[4];
    collisionTextureUv["HalfBoxLeft"]          = texRects[5];
    collisionTextureUv["SlopeLeft"]            = texRects[6];
    collisionTextureUv["SlopeRight"]           = texRects[7];
    collisionTextureUv["DoubleSlopeLeft1"]     = texRects[8];
    collisionTextureUv["DoubleSlopeRight1"]    = texRects[9];
    collisionTextureUv["HalfBoxRight"]         = texRects[10];
    collisionTextureUv["SlopeLeftInv"]         = texRects[11];
    collisionTextureUv["SlopeRightInv"]        = texRects[12];
    collisionTextureUv["DoubleSlopeLeftInv1"]  = texRects[13];
    collisionTextureUv["DoubleSlopeRightInv1"] = texRects[14];
    collisionTextureUv["HalfSlopeLeft1"]       = texRects[15];
    collisionTextureUv["HalfSlopeLeft2"]       = texRects[16];
    collisionTextureUv["HalfSlopeRight2"]      = texRects[17];
    collisionTextureUv["HalfSlopeRight1"]      = texRects[18];
    collisionTextureUv["DoubleSlopeRightInv2"] = texRects[19];
    collisionTextureUv["HalfSlopeLeftInv1"]    = texRects[20];
    collisionTextureUv["HalfSlopeLeftInv2"]    = texRects[21];
    collisionTextureUv["HalfSlopeRightInv2"]   = texRects[22];
    collisionTextureUv["HalfSlopeRightInv1"]   = texRects[23];
    collisionTextureUv["DoubleSlopeLeftInv2"]  = texRects[24];
  }
}

void TileEditor2DOperation::Undo()
{
  Cog* cog = mTileMapHandle;
  if (cog == NULL)
    return;

  TileMap* map = cog->has(TileMap);
  if (map == NULL)
    return;

  for (auto range = mChanges.All(); range.Empty() == false; range.PopFront())
  {
    TileMapChange& change = range.Front();
    map->SetTile(change.GridPos, change.PrevTile);
  }

  map->GetSpace()->MarkModified();
}

void TileEditor2DOperation::Redo()
{
  Cog* cog = mTileMapHandle;
  if (cog == NULL)
    return;

  TileMap* map = cog->has(TileMap);
  if (map == NULL)
    return;

  for (auto range = mChanges.All(); range.Empty() == false; range.PopFront())
  {
    TileMapChange& change = range.Front();
    map->SetTile(change.GridPos, change.NewTile);
  }

  map->GetSpace()->MarkModified();
}

// --------------------------------------------------------- TileEditor2DSubTool
ZilchDefineType(TileEditor2DSubTool, builder, type)
{
}

TileEditor2DSubTool::TileEditor2DSubTool(TileEditor2D* owner)
  : mOwner(owner)
  , mPrimaryActive(false)
  , mSecondaryActive(false)
{
}

void TileEditor2DSubTool::StartPrimaryAction(TileMap* map)
{
  mPrimaryActive = true;
  PrimaryStart(map);
}

void TileEditor2DSubTool::StartSecondaryAction(TileMap* map)
{
  mSecondaryActive = true;
  SecondaryStart(map);
}

void TileEditor2DSubTool::ContinueAction(TileMap* map)
{
  if (mPrimaryActive)
    PrimaryContinue(map);
  else if (mSecondaryActive)
    SecondaryContinue(map);
}

void TileEditor2DSubTool::EndAction(TileMap* map)
{
  if (mPrimaryActive)
    PrimaryEnd(map);
  else if (mSecondaryActive)
    SecondaryEnd(map);

  mPrimaryActive = false;
  mSecondaryActive = false;
}

void TileEditor2DSubTool::CommitOperation(TileMap* map)
{
  mOwner->CommitOperation(map, mChanges);
  mChanges.Clear();
}

bool TileEditor2DSubTool::HasChange(TileMapChange& change)
{
  for (auto range = mChanges.All(); range.Empty() == false; range.PopFront())
  {
    if (change.GridPos == range.Front().GridPos)
      return true;
  }

  return false;
}

// -------------------------------------------------------- TileEditor2DDrawTool
ZilchDefineType(TileEditor2DDrawTool, builder, type)
{
}

TileEditor2DDrawTool::TileEditor2DDrawTool(TileEditor2D* owner)
  : TileEditor2DSubTool(owner)
{
}

void TileEditor2DDrawTool::Draw(TileMap* map)
{
  Transform* transform = map->GetOwner()->has(Transform);
  const TileMapSelection& selection = mOwner->GetSelection();
  IntVec2 gridPos = mOwner->GetMouseGridPosition(map) + selection.Offset;
  //not sure how this happened, but safeguard against a crash
  if(selection.Width == 0)
    return;
  IntVec2 scale = IntVec2(selection.Width, selection.Tiles.Size() / selection.Width);

  DebugDrawOutline(gridPos, scale, transform, Color::Red);

  int size = selection.Tiles.Size();
  int width = selection.Width;

  if (mSecondaryActive)
  {
    DebugDrawCross(gridPos, scale, transform, Color::Red);
  }
  else
  {
    if (mPrimaryActive)
    {
      for (uint i = 0; i < mChanges.Size(); ++i)
        DebugDrawQuad(mChanges[i].GridPos, IntVec2(1, 1), transform, Color::Red);
    }

    for (int i = 0; i < size; ++i)
    {
      if (selection.Tiles[i].GetArchetypeResource() == nullptr)
        continue;

      DebugDrawQuad(gridPos + IntVec2(i % width, -i / width), IntVec2(1, 1), transform, Color::Red);
    }
  }

  if (mOwner->GetShowCoordinates())
    DebugDrawCoordinates(gridPos, size, width, transform, Color::Red);
}

void TileEditor2DDrawTool::PrimaryStart(TileMap* map)
{
  mGridPosition = mOwner->GetMouseGridPosition(map);
  DrawTile(map);
}

void TileEditor2DDrawTool::PrimaryContinue(TileMap* map)
{
  IntVec2 gridPos = mOwner->GetMouseGridPosition(map);
  if (gridPos == mGridPosition)
    return;

  mGridPosition = gridPos;
  DrawTile(map);
}

void TileEditor2DDrawTool::PrimaryEnd(TileMap* map)
{
  CommitOperation(map);
}

void TileEditor2DDrawTool::SecondaryStart(TileMap* map)
{
  EraseTile(map);
}

void TileEditor2DDrawTool::SecondaryContinue(TileMap* map)
{
  EraseTile(map);
}

void TileEditor2DDrawTool::SecondaryEnd(TileMap* map)
{
  CommitOperation(map);
}

void TileEditor2DDrawTool::DrawTile(TileMap* map)
{
  const TileMapSelection& selection = mOwner->GetSelection();
  if (selection.Tiles.Size())
  {
    Array<TileMapChange> changes;

    int size = selection.Tiles.Size();
    int width = selection.Width;
    for (int i = 0; i < size; ++i)
    {
      if (selection.Tiles[i].GetArchetypeResource() == nullptr)
        continue;

      TileMapChange change;
      change.GridPos = IntVec2(mGridPosition.x + i % width, mGridPosition.y - i / width) + selection.Offset;
      change.NewTile = selection.Tiles[i];
      change.PrevTile = map->GetTile(change.GridPos);

      if (HasChange(change) == false)
        changes.PushBack(change);
      else if (mOwner->TiledDrawing())
        return;
    }

    ApplyChanges(map, changes);
    mChanges.Append(changes.All());
    return;
  }
}

void TileEditor2DDrawTool::EraseTile(TileMap* map)
{
  const TileMapSelection& selection = mOwner->GetSelection();
  if (selection.Tiles.Size())
  {
    Array<TileMapChange> changes;

    IntVec2 gridPos = mOwner->GetMouseGridPosition(map);
    int size = selection.Tiles.Size();
    int width = selection.Width;
    for (int i = 0; i < size; ++i)
    {
      TileMapChange change;
      change.GridPos = IntVec2(gridPos.x + i % width, gridPos.y - i / width) + selection.Offset;
      change.NewTile = Tile();
      change.PrevTile = map->GetTile(change.GridPos);

      if (HasChange(change) == false)
        changes.PushBack(change);
    }

    ApplyChanges(map, changes);
    mChanges.Append(changes.All());
    return;
  }

  TileMapChange change;
  change.GridPos = mOwner->GetMouseGridPosition(map);
  change.NewTile = Tile();
  change.PrevTile = map->GetTile(change.GridPos);

  if (HasChange(change) == false)
  {
    mChanges.PushBack(change);
    ApplyChange(map, change);
  }
}

void TileEditor2DDrawTool::ApplyChange(TileMap* map, TileMapChange& change)
{
  map->SetTile(change.GridPos, change.NewTile);
}

void TileEditor2DDrawTool::ApplyChanges(TileMap* map, Array<TileMapChange>& changes)
{
  for (uint i = 0; i < changes.Size(); ++i)
    map->SetTile(changes[i].GridPos, changes[i].NewTile);
}

// ---------------------------------------------------------------- TileEditor2DSelectTool
ZilchDefineType(TileEditor2DSelectTool, builder, type)
{
}

TileEditor2DSelectTool::TileEditor2DSelectTool(TileEditor2D* owner)
  : TileEditor2DSubTool(owner)
  , mHasSelection(false)
{
}

void TileEditor2DSelectTool::Draw(TileMap* map)
{
  Transform* transform = map->GetOwner()->has(Transform);

  if (mHasSelection)
  {
    IntVec2 min, max;
    min.x = mStart.x < mEnd.x ? mStart.x : mEnd.x;
    min.y = mStart.y < mEnd.y ? mStart.y : mEnd.y;
    max.x = mStart.x > mEnd.x ? mStart.x + 1 : mEnd.x + 1;
    max.y = mStart.y > mEnd.y ? mStart.y + 1 : mEnd.y + 1;

    IntVec2 gridPos = IntVec2(min.x, max.y - 1);
    IntVec2 scale = max - min;
    DebugDrawQuad(gridPos, scale, transform, Color::Blue);

    if (mOwner->GetShowCoordinates())
      DebugDrawCoordinates(gridPos, scale.x * scale.y, scale.x, transform, Color::Blue);
  }
  else
  {
    IntVec2 gridPos = mOwner->GetMouseGridPosition(map);
    DebugDrawQuad(gridPos, IntVec2(1, 1), transform, Color::Blue);

    if (mOwner->GetShowCoordinates())
      DebugDrawCoordinates(gridPos, 1, 1, transform, Color::Blue);
  }
}

void TileEditor2DSelectTool::PrimaryStart(TileMap* map)
{
  mStart = mOwner->GetMouseGridPosition(map);
  mEnd = mStart;
  mHasSelection = true;
}

void TileEditor2DSelectTool::PrimaryContinue(TileMap* map)
{
  mEnd = mOwner->GetMouseGridPosition(map);
}

void TileEditor2DSelectTool::SecondaryStart(TileMap* map)
{
  if (!mHasSelection)
    return;

  mHasSelection = false;

  IntVec2 gridPos = mOwner->GetMouseGridPosition(map);

  IntVec2 min, max;
  min.x = mStart.x < mEnd.x ? mStart.x : mEnd.x;
  min.y = mStart.y < mEnd.y ? mStart.y : mEnd.y;
  max.x = mStart.x > mEnd.x ? mStart.x : mEnd.x;
  max.y = mStart.y > mEnd.y ? mStart.y : mEnd.y;

  if (gridPos.x < min.x || gridPos.x > max.x || gridPos.y < min.y || gridPos.y > max.y)
    return;

  int width = max.x - min.x + 1;

  TileMapSelection selection;
  selection.Offset = IntVec2(min.x - gridPos.x, max.y - gridPos.y);
  selection.Width = width;
  for (int y = max.y; y >= min.y; --y)
    for (int x = min.x; x <= max.x; ++x)
      selection.Tiles.PushBack(map->GetTile(IntVec2(x, y)));

  mOwner->SetSelection(selection);
  mOwner->SetToolType(TileEditor2DSubToolType::DrawTool);
  mOwner->SetCustomSelection();
}

// ---------------------------------------------------------------- TileEditor2D
const char* TileEditor2D::cDefaultName = "TileMap";
const char* TileEditor2D::cDefaultArchetype = "DefaultTileMap";

ZilchDefineType(TileEditor2D, builder, type)
{
  ZeroBindComponent();
  ZeroBindDependency(MouseCapture);
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterSetterProperty(ToolType);
  ZilchBindMethodProperty(CreateTileMap);
  ZeroBindTag(Tags::Tool);

  ZilchBindGetterSetterProperty(ShowCollision);
  ZilchBindGetterSetterProperty(ShowCoordinates);
  ZilchBindFieldProperty(mShowArchetype);
  ZilchBindFieldProperty(mShowInvalid);
  ZilchBindFieldProperty(mShowGrid);
  ZilchBindFieldProperty(mTiledDrawing);
}

TileEditor2D::TileEditor2D()
{
  mAddTileMapWidget = 0;
  mToolType = TileEditor2DSubToolType::DrawTool;
  mShowCollision = true;
  mShowCoordinates = true;
  mShowArchetype = false;
  mShowInvalid = false;
  mShowGrid = false;
  mTiledDrawing = false;
  mTilePaletteProperty = nullptr;
  mMousePos = Vec3(0,0,0);

  mSubTools.PushBack(new TileEditor2DDrawTool(this));
  mSubTools.PushBack(new TileEditor2DSelectTool(this));

  SetToolType(TileEditor2DSubToolType::DrawTool);

  InitializeCollisionUvMap(mCollisionTextureUv);
}

TileEditor2D::~TileEditor2D()
{
  DeleteObjectsInContainer(mSubTools);
  SafeDelete(mTilePaletteProperty);
}

void TileEditor2D::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::ToolActivate,   OnToolActivate);
  ConnectThisTo(GetOwner(), Events::ToolDeactivate, OnToolDeactivate);
  ConnectThisTo(GetOwner(), Events::KeyDown,        OnKeyDown);
  ConnectThisTo(GetOwner(), Events::LeftMouseDown,  OnLeftMouseDown);
  ConnectThisTo(GetOwner(), Events::RightMouseDown, OnRightMouseDown);
  ConnectThisTo(GetOwner(), Events::MouseMove,      OnMouseMove);
  ConnectThisTo(GetOwner(), Events::MouseDragMove,  OnMouseDragMove);
  ConnectThisTo(GetOwner(), Events::MouseDragEnd,   OnMouseDragEnd);
  ConnectThisTo(GetOwner(), Events::ToolDraw,       OnToolDraw);
  ConnectThisTo(GetOwner(), Events::GetToolInfo,    OnGetToolInfo);
}

void TileEditor2D::GeneratePhysicsMeshResource(const Array<Vec3>& originalPoints, StringParam name)
{
  Array<Vec3> vertices = originalPoints;
  Array<uint> indices;

  // Back vertices
  uint size = vertices.Size();
  for (uint i = 0; i < size; ++i)
    vertices.PushBack(vertices[i] + Vec3(0.0, 0.0, -1.0));

  // Front face
  for (uint i = 2; i < size; ++i)
  {
    indices.PushBack(0);
    indices.PushBack(i - 1);
    indices.PushBack(i);
  }

  // Side faces
  for (uint i = 0; i < size; ++i)
  {
    uint j = i + size;
    indices.PushBack(j);
    indices.PushBack((j + 1) % size + size);
    indices.PushBack((i + 1) % size);
    indices.PushBack(j);
    indices.PushBack((i + 1) % size);
    indices.PushBack(i);
  }

  // Back face
  for (uint i = size * 2 - 2; i > size; --i)
  {
    indices.PushBack(size);
    indices.PushBack(i + 1);
    indices.PushBack(i);
  }

  PhysicsMesh* mesh = new PhysicsMesh();
  mesh->Upload(vertices, indices);
  mesh->FilterTag = "TileMesh";

  TileMap* tileMap = GetTileMap();
  ResourceAdd resourceAdd;
  resourceAdd.SourceResource = mesh;
  resourceAdd.Name = name;

  AddNewResource(PhysicsMeshManager::GetInstance(), resourceAdd);
}

void TileEditor2D::OnSelectionFinal(SelectionChangedEvent* event)
{
  MetaSelection* selection = (MetaSelection*)event->Selection;
  Cog* cog = selection->GetPrimaryAs<Cog>();

  if (cog && cog->has(TileMap))
  {
    // This behavior currently relies on MainPropertyView connecting to this event first
    Z::gEditor->ShowWindow("Tools");
    TilePaletteSourceManager* manager = (TilePaletteSourceManager*)TilePaletteSourceManager::GetInstance();
    TilePaletteSource* palette = (TilePaletteSource*)manager->GetResource(cog->has(TileMap)->GetPaletteName(), ResourceNotFound::ReturnNull);
    mTilePalatte->SetTilePalette(palette);
  }
  else
  {
    // Return to select tool if no object has a TileMap
    Z::gEditor->Tools->SelectToolIndex(0);
  }
}

void TileEditor2D::SetTileMapPalette(TilePaletteSource* tilePalette)
{
  TileMap* tileMap = GetTileMap();
  if (tileMap)
  {
    String paletteName = String();
    if (tilePalette)
      paletteName = tilePalette->ResourceIdName;

    if (paletteName != tileMap->GetPaletteName())
    {
      tileMap->SetPaletteName(paletteName);
      tileMap->GetSpace()->MarkModified();
    }
  }
}

void TileEditor2D::OnToolActivate(Event*)
{
  ConnectThisTo(Z::gEditor->GetSelection(), Events::SelectionFinal, OnSelectionFinal);

  TileMap* tileMap = GetTileMap();
  if (tileMap)
  {
    Cog* cog = tileMap->GetOwner();
    Z::gEditor->SelectOnly(cog);
  }
}

void TileEditor2D::OnToolDeactivate(Event*)
{
  mAddTileMapWidget.SafeDestroy();
  Z::gEditor->GetSelection()->GetDispatcher()->Disconnect(this);
}

void TileEditor2D::OnKeyDown(KeyboardEvent* e)
{
  switch (e->Key)
  {
    case Keys::Escape:
      mTilePalatte->RefreshSelection();
      e->Handled = true;
    break;

    case Keys::Num1:
      if (e->ShiftPressed)
      {
        SetToolType(TileEditor2DSubToolType::DrawTool);
        e->Handled = true;
      }
    break;

    case Keys::Num2:
      if (e->ShiftPressed)
      {
        SetToolType(TileEditor2DSubToolType::SelectionTool);
        e->Handled = true;
      }
    break;
  }
}

void TileEditor2D::OnLeftMouseDown(ViewportMouseEvent* e)
{
  Viewport* viewport = e->GetViewport();
  if (viewport->GetTargetSpace() && viewport->GetTargetSpace()->IsEditorMode() == false)
    return;

  TileMap* tileMap = GetTileMap();
  if (tileMap == NULL)
    return;

  if (e->ShiftPressed)
    SetToolType(TileEditor2DSubToolType::SelectionTool);

  MouseCapture* mouseCapture = GetOwner()->has(MouseCapture);
  mouseCapture->Capture(e);
  
  mCurrentTool->StartPrimaryAction(tileMap);
  e->Handled = true;
}

void TileEditor2D::OnRightMouseDown(ViewportMouseEvent* e)
{
  Viewport* viewport = e->GetViewport();
  if (viewport->GetTargetSpace() && viewport->GetTargetSpace()->IsEditorMode() == false)
    return;

  TileMap* tileMap = GetTileMap();
  if (tileMap == NULL)
    return;

  MouseCapture* mouseCapture = GetOwner()->has(MouseCapture);
  mouseCapture->Capture(e);

  mCurrentTool->StartSecondaryAction(tileMap);
  e->Handled = true;
}

void TileEditor2D::OnMouseMove(ViewportMouseEvent* e)
{
  Viewport* viewport = e->GetViewport();
  if (viewport->GetTargetSpace() && viewport->GetTargetSpace()->IsEditorMode() == false)
    return;

  TileMap* tileMap = GetTileMap();
  if (tileMap == NULL)
    return;

  Ray worldRay = viewport->ScreenToWorldRay(e->Position);

  Intersection::IntersectionPoint point;
  Plane plane(tileMap->GetOwner()->has(Transform)->TransformNormal(Vec3(0.0, 0.0, 1.0)), tileMap->GetOwner()->has(Transform)->GetWorldTranslation());

  Intersection::Type type = Intersection::RayPlane(worldRay.Start, worldRay.Direction, plane.GetNormal(), plane.GetDistance(), &point);
  if (type == Intersection::None)
    return;

  mMousePos = point.Points[0];

  mCurrentTool->ContinueAction(tileMap);
}

void TileEditor2D::OnMouseDragMove(ViewportMouseEvent* e)
{
  Viewport* viewport = e->GetViewport();
  if (viewport->GetTargetSpace() && viewport->GetTargetSpace()->IsEditorMode() == false)
    return;

  TileMap* tileMap = GetTileMap();
  if (tileMap == NULL)
    return;

  Ray worldRay = viewport->ScreenToWorldRay(e->Position);

  Intersection::IntersectionPoint point;
  Plane plane(tileMap->GetOwner()->has(Transform)->TransformNormal(Vec3(0.0, 0.0, 1.0)), tileMap->GetOwner()->has(Transform)->GetWorldTranslation());

  Intersection::Type type = Intersection::RayPlane(worldRay.Start, worldRay.Direction, plane.GetNormal(), plane.GetDistance(), &point);
  if (type == Intersection::None)
    return;

  mMousePos = point.Points[0];

  mCurrentTool->ContinueAction(tileMap);

  e->Handled = true;
}

void TileEditor2D::OnMouseDragEnd(Event*)
{
  TileMap* tileMap = GetTileMap();
  if (tileMap == NULL)
    return;

  mCurrentTool->EndAction(tileMap);
}

void TileEditor2D::OnToolDraw(Event*)
{
  TileMap* tileMap = GetTileMap();
  if (tileMap == NULL)
    return;

  mAddTileMapWidget.SafeDestroy();

  Transform* mapTransform = tileMap->GetOwner()->has(Transform);

  if (mShowGrid)
  {
    IntVec2 gridPos = GetMouseGridPosition(tileMap);

    gridPos /= 32;
    gridPos *= 32;

    Array<Vec3> gridLines;
    int gridSize = 64;
    for (int i = -gridSize; i <= gridSize + 1; ++i)
    {
      gridLines.PushBack(mapTransform->TransformPoint(Vec3(real(i + gridPos.x), real(-gridSize + gridPos.y), 0.0f)));
      gridLines.PushBack(mapTransform->TransformPoint(Vec3(real(i + gridPos.x), real( gridSize + gridPos.y + 1), 0.0f)));
      gridLines.PushBack(mapTransform->TransformPoint(Vec3(real(-gridSize + gridPos.x), real(i + gridPos.y), 0.0f)));
      gridLines.PushBack(mapTransform->TransformPoint(Vec3(real( gridSize + gridPos.x + 1), real(i + gridPos.y), 0.0f)));
    }

    for (uint i = 1; i < gridLines.Size(); i += 2)
      Zero::gDebugDraw->Add(Zero::Debug::Line(gridLines[i - 1], gridLines[i]).Color(Color::Gray));
  }

  if (mShowCollision)
  {
    // ** INTENDED TO BE COMMENTED OUT **
    // forRange(auto pair, tileMap->GetTiles())
    // {
    //   IntVec2 gridPos = pair.first;
    //   Tile tile = pair.second;

    //   Vec3 pos = Vec3(real(gridPos.x + 0.5), real(gridPos.y + 0.5), -0.5);

    //   PhysicsMesh* mesh = tile.CollisionResource;
    //   if (mesh)
    //   {
    //     const Array<Vec3>& verts = mesh->GetVertices();
    //     for (uint i = 2; i < verts.Size() / 2; ++i)
    //     {
    //       Vec3 points[3];
    //       points[0] = verts[0];
    //       points[1] = verts[i - 1];
    //       points[2] = verts[i];
    //       DebugDrawTriangle(gridPos, points, mapTransform, Color::Red);
    //     }
    //   }
    // }

    static real colorTime = 0.0;

    Vec4 color;
    color.x = RainbowColor(colorTime);
    color.y = RainbowColor(colorTime + real(0.666f));
    color.z = RainbowColor(colorTime + real(0.333f));
    color.w = 1.0;

    colorTime += 0.002f;
    if (colorTime > 1.0f)
      colorTime -= 1.0f;

    tileMap->DrawContours(ToByteColor(color));
  }

  if (mShowArchetype)
  {
    forRange(auto pair, tileMap->GetTiles())
    {
      IntVec2 gridPos = pair.first;
      Tile tile = pair.second;

      Vec3 pos = Vec3(real(gridPos.x + 0.5f), real(gridPos.y + 0.5f), 0.0);
      pos = mapTransform->TransformPoint(pos);
      pos.z = 0.1f;

      Archetype* archetype = tile.GetArchetypeResource();
      if (archetype)
        Zero::gDebugDraw->Add(Zero::Debug::Text(pos, real(0.15f), archetype->Name).Centered(true).Color(Color::Orange));
    }
  }

  if (mShowInvalid)
  {
    forRange(auto pair, tileMap->GetTiles())
    {
      IntVec2 gridPos = pair.first;
      Tile tile = pair.second;

      if (tileMap->ValidTile(tile) != TileStatus::Valid)
        DebugDrawCross(gridPos, IntVec2(1, 1), mapTransform, Color::Red);
    }
  }

  mCurrentTool->Draw(tileMap);
}

void TileEditor2D::OnGetToolInfo(ToolUiEvent* e)
{
  Composite* ui = new Composite(e->mParent);
  ui->SetLayout(CreateStackLayout());
  ui->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

  PropertyView* propView = new PropertyView(ui);
  propView->mFixedHeight = true;
  propView->ActivateAutoUpdate();

  mTilePalatte = new TilePaletteView(ui, this);
  mTilePalatte->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

  if(mTilePaletteProperty == nullptr)
    mTilePaletteProperty = new TilePaletteProperty();
  propView->SetObject(mTilePalatte, mTilePaletteProperty);
  propView->Rebuild();

  e->mCustomUi = ui;
  e->mNeedsPropertyGrid = true;
}

void TileEditor2D::OnLeftMouseDownAddTileMapWidget(MouseEvent* event)
{
  event->Handled = true;
}

void TileEditor2D::OnLeftClickAddTileMapWidget(MouseEvent* event)
{
  event->Handled = true;
  CreateTileMap();
}

void TileEditor2D::CreateTileMap()
{
  Space* space = Z::gEditor->GetEditSpace();

  if (space)
  {
    Cog* created = space->CreateNamed(cDefaultArchetype);
    if (created)
    {
      created->ClearArchetype();

      OperationQueue* queue = Z::gEditor->GetOperationQueue();
      ObjectCreated(queue, created);

      mAddTileMapWidget.SafeDestroy();

      Z::gEditor->SelectOnly(created);
    }
  }
}

TileMap* TileEditor2D::GetTileMap()
{
  // Disabled creation
  TileMap* tileMap = static_cast<TileMap*>(Tool::GetOrCreateEditComponent(ZilchTypeId(TileMap), cDefaultName, cDefaultArchetype, mLastEdited, false));
  if (tileMap == NULL && mAddTileMapWidget.IsNull())
  {
    mAddTileMapWidget = Tool::CreateViewportTextWidget("No TileMap Object, Add New +");
    if (mAddTileMapWidget != nullptr)
    {
      ConnectThisTo((ViewportTextWidget*)mAddTileMapWidget, Events::LeftMouseDown, OnLeftMouseDownAddTileMapWidget);
      ConnectThisTo((ViewportTextWidget*)mAddTileMapWidget, Events::LeftMouseUp, OnLeftClickAddTileMapWidget);
    }
  }
  return tileMap;
}

IntVec2 TileEditor2D::GridPositionFromWorld(Vec3 pos)
{
  return IntVec2(int(floor(pos.x)), int(floor(pos.y)));
}

IntVec2 TileEditor2D::GetMouseGridPosition(TileMap* map)
{
  Vec3 tileSpace = map->GetOwner()->has(Transform)->TransformPointInverse(mMousePos);
  return GridPositionFromWorld(tileSpace);
}

const TileMapSelection& TileEditor2D::GetSelection()
{
  return mTileMapSelection;
}

void TileEditor2D::SetSelection(const TileMapSelection& selection)
{
  mTileMapSelection = selection;
}

TileEditor2DSubToolType::Enum TileEditor2D::GetToolType()
{
  return mToolType;
}

void TileEditor2D::SetToolType(TileEditor2DSubToolType::Enum type)
{
  mToolType = type;
  mCurrentTool = mSubTools[type];
}

bool TileEditor2D::GetShowCollision()
{
  return mShowCollision;
}

void TileEditor2D::SetShowCollision(bool showCollision)
{
  mShowCollision = showCollision;
  mTilePalatte->SetShowCollision(mShowCollision);
}

bool TileEditor2D::GetShowCoordinates()
{
  return mShowCoordinates;
}

void TileEditor2D::SetShowCoordinates(bool showCoordinates)
{
  mShowCoordinates = showCoordinates;
}

bool TileEditor2D::TiledDrawing()
{
  return mTiledDrawing;
}

UvRect TileEditor2D::GetCollisionTextureUv(StringParam resourceName)
{
  if (mCollisionTextureUv.Find(resourceName).Empty())
  {
    UvRect noValidUv = {Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f)};
    return noValidUv;
  }
  else
  {
    return mCollisionTextureUv[resourceName];
  }
}

void TileEditor2D::SetCustomSelection()
{
  mTilePalatte->SetSelectionBorderActive(false);
}

void TileEditor2D::CommitOperation(TileMap* map, Array<TileMapChange>& changes)
{
  if (changes.Empty())
    return;

  TileEditor2DOperation* op = new TileEditor2DOperation;
  op->mTileMapHandle = map->GetOwner();
  op->mChanges.Assign(changes.All());

  map->GetSpace()->MarkModified();

  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->Queue(op);
}

}//namespace Zero
