///////////////////////////////////////////////////////////////////////////////
///
/// \file ColorGradientEditor.cpp
/// Implementation of the ColorGradientEditor Composite.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const static float sGradientKeySize = Pixels(8);

void DrawCheckers2(PixelBuffer* buffer, uint checkerSize);
void DrawKey(DisplayRender* render, Vec3Param pos, float size, ByteColor color);

//------------------------------------------------------------------ GradientKey
namespace GradientEditing
{

class GradientKeyManipulator : public MouseManipulation
{
public:
  GradientKey* mTarget;
  Vec2 mStartingPosition;
  Vec2 mCurrentPosition;

  GradientKeyManipulator(Mouse* mouse, Composite* owner, GradientKey* target)
    : MouseManipulation(mouse, owner)
  {
    mTarget = target;
    mStartingPosition = GetLocalPos(mouse->GetClientPosition());
    mCurrentPosition = mStartingPosition;
  }

  void OnMouseMove(MouseEvent* event) override
  {
    // We want the position in our parents local space (the curve editor)
    mCurrentPosition = GetLocalPos(event->Position);
    mTarget->MoveTo(mCurrentPosition);
  }

  Vec2 GetLocalPos(Vec2Param pos)
  {
    return mTarget->GetParent()->ToLocal(pos);
  }
};

GradientKey::GradientKey(ColorGradientEditor* parent, 
                         Vec4Param color, float interpolant) 
  : Spacer(parent)
{
  SetNotInLayout(true);

  ConnectThisTo(this, Events::LeftMouseDrag, OnMouseDrag);
  ConnectThisTo(this, Events::RightClick, OnRightClick);
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);

  ConnectThisTo(this, Events::FinalColorPicked, OnFinalColorPicked);
  ConnectThisTo(this, Events::ColorChanged, OnColorChanged);
  ConnectThisTo(this, Events::ColorPickCancelled, OnColorPickCancelled);

  SetSize(Vec2(1,1) * sGradientKeySize);

  mGradientEditor = parent;
  mColor = color;
  mInterpolant = interpolant;
  mPickingColor = false;
}

void GradientKey::OnMouseDrag(MouseEvent* event)
{
  event->Handled = true;
  new GradientKeyManipulator(event->GetMouse(), GetParent(), this);
}

void GradientKey::OnRightClick(MouseEvent* event)
{
  if(mPickingColor)
    ColorPicker::Close();

  if(mGradientEditor->RemoveKey(this))
    Destroy();
}

void GradientKey::OnLeftClick(MouseEvent* event)
{
  mPreviousColor = mColor;
  mPickingColor = true;
  ColorPicker::EditColor(this, mColor);
}

void GradientKey::UpdateTransform()
{
  SetCenter(mGradientEditor->GetKeyWorldPos(this));
  Spacer::UpdateTransform();
}

void GradientKey::MoveTo(Vec2Param worldPos)
{
  mInterpolant = mGradientEditor->GetInterpolantAtWorldPos(worldPos);
  mInterpolant = Math::Clamp(mInterpolant, 0.0f, 1.0f);
  MarkAsNeedsUpdate();
  mGradientEditor->MarkAsNeedsUpdate();
  mGradientEditor->Modified();
}

void GradientKey::SetCenter(Vec2Param worldPos)
{
  Vec3 halfSize = ToVector3(mSize) * 0.5f;
  mTranslation = ToVector3(worldPos) - halfSize;
}

void GradientKey::OnFinalColorPicked(ColorEvent* event)
{
  mPickingColor = false;
  mGradientEditor->Modified();
  mGradientEditor->MarkAsNeedsUpdate();
}

void GradientKey::OnColorChanged(ColorEvent* event)
{
  mColor = event->Color;
  mGradientEditor->MarkAsNeedsUpdate();
}

void GradientKey::OnColorPickCancelled(ColorEvent* event)
{
  mColor = mPreviousColor;
  mPickingColor = false;
  mGradientEditor->MarkAsNeedsUpdate();
}

}//namespace GradientEditing

//------------------------------------------------------------------- Key Drawer
GradientKeyDrawer::GradientKeyDrawer(ColorGradientEditor* gradientEditor) 
  : Widget(gradientEditor)
{
  mGradientEditor = gradientEditor;
}

void GradientKeyDrawer::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  Array<StreamedVertex> lines;
  Array<StreamedVertex> triangles;

  ColorGradientEditor::GradientKeyArray& keys = mGradientEditor->mKeys;
  for (uint i = 0; i < keys.Size(); ++i)
  {
    // Get the keys position
    GradientEditing::GradientKey* key = keys[i];
    Vec3 pos = ToVector3(mGradientEditor->GetKeyWorldPos(key));
    Vec4 color = RemoveHdrFromColor(key->mColor);
    color.w = 1.0f;
    float halfSize = sGradientKeySize * 0.5f;
    StreamedVertex v0(SnapToPixels(pos + Vec3(-halfSize, -halfSize, 0)), Vec2(0, 0), color);
    StreamedVertex v1(SnapToPixels(pos + Vec3(-halfSize, halfSize, 0)), Vec2(0, 1), color);
    StreamedVertex v2(SnapToPixels(pos + Vec3(halfSize, halfSize, 0)), Vec2(1, 1), color);
    StreamedVertex v3(SnapToPixels(pos + Vec3(halfSize, -halfSize, 0)), Vec2(1, 0), color);

    Vec3 top = SnapToPixels(pos - Vec3(0, halfSize * 2.0f, 0));
    Vec3 triOffset = -Pixels(-0.25f, 1, 0);

    triangles.PushBack(v0);
    triangles.PushBack(v1);
    triangles.PushBack(v2);
    triangles.PushBack(v2);
    triangles.PushBack(v3);
    triangles.PushBack(v0);

    triangles.PushBack(StreamedVertex(v0.mPosition + triOffset, Vec2(0, 0), ToFloatColor(Color::Black)));
    triangles.PushBack(StreamedVertex(v3.mPosition + triOffset, Vec2(0, 0), ToFloatColor(Color::Black)));
    triangles.PushBack(StreamedVertex(top + triOffset, Vec2(0, 0), ToFloatColor(Color::Black)));

    v0.mColor = ToFloatColor(Color::Black);
    v1.mColor = ToFloatColor(Color::Black);
    v2.mColor = ToFloatColor(Color::Black);
    v3.mColor = ToFloatColor(Color::Black);
    lines.PushBack(v0);
    lines.PushBack(v1);
    lines.PushBack(v1);
    lines.PushBack(v2);
    lines.PushBack(v2);
    lines.PushBack(v3);
    lines.PushBack(v3);
    lines.PushBack(v0);
  }

  CreateRenderData(viewBlock, frameBlock, clipRect, triangles, PrimitiveType::Triangles);
  CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);
}

//-------------------------------------------------------- Color Gradient Editor
ColorGradientEditor::ColorGradientEditor(Composite* parent, ColorGradient* gradient)
  : Composite(parent)
{
  SetSize(parent->GetSize());

  // Create the color buffer
  mGradientBlockBuffer = new PixelBuffer(Color::Black, 256, 35);

  // Create the display and set the texture
  mGradientBlockDisplay = new TextureView(this);
  mGradientBlockDisplay->SetTexture(mGradientBlockBuffer->Image);
  mGradientBlockDisplay->SetTranslation(Pixels(25,0,0));
  mGradientBlockDisplay->SetSize(Vec2(GetSize().x, 0) + Pixels(-50, 35));

  mKeyDrawer = new GradientKeyDrawer(this);
  mKeyDrawer->SetTranslation(Pixels(0,0,0));
  mKeyDrawer->SetSize(Vec2(GetSize().x, 0) + Pixels(0,50));
  ConnectThisTo(mKeyDrawer, Events::DoubleClick, OnGradientBlockMouseDown);
  ConnectThisTo(Z::gEditor, Events::Save, OnSave);

  LoadGradient(gradient);
}

ColorGradientEditor::~ColorGradientEditor()
{
  SafeDelete(mGradientBlockBuffer);
}

void ColorGradientEditor::Update()
{
  if(ColorGradient* gradient = mGradient)
  {
    gradient->mGradient.Clear();
    for(uint i = 0; i < mKeys.Size(); ++i)
    {
      InternalKey* key = mKeys[i];
      gradient->Insert(key->mColor, key->mInterpolant);
    }
  }
}

Vec2 ColorGradientEditor::GetKeyWorldPos(InternalKey* key)
{
  Vec2 pos(key->mInterpolant * mKeyDrawer->GetSize().x, Pixels(45));
  Vec2 offset = Pixels(25, 0);
  pos.x *= (GetSize().x - Pixels(50.0f)) / GetSize().x;
  pos.x += offset.x;
  return pos;
}

float ColorGradientEditor::GetInterpolantAtWorldPos(Vec2Param worldPos)
{
  Vec2 offset = Pixels(25, 0);
  Vec2 pos = worldPos;
  pos.x -= offset.x;
  pos.x /= (GetSize().x - Pixels(50.0f)) / GetSize().x;
  pos.x /= mKeyDrawer->GetSize().x;

  pos.x = Math::Clamp(pos.x, 0.0f, 1.0f);
  return pos.x;
}

void ColorGradientEditor::Modified()
{
  if(ColorGradient* gradient = mGradient)
  {
    MetaOperations::NotifyObjectModified(gradient);

    TabModifiedEvent eventToSend(true);
    DispatchBubble(Events::TabModified, &eventToSend);
  }
}

void ColorGradientEditor::OnSave(Event* e)
{
  if(ColorGradient* gradient = mGradient)
  {
    ContentItem* contentItem = gradient->mContentItem;
    if(contentItem)
      contentItem->SaveContent();

    TabModifiedEvent eventToSend(false);
    DispatchBubble(Events::TabModified, &eventToSend);
  }
}

void ColorGradientEditor::UpdateGradientBlock()
{
  ColorGradient* gradient = mGradient;
  if(gradient == nullptr)
    return;

  DrawColorGradient(gradient, mGradientBlockBuffer);
}

void ColorGradientEditor::LoadGradient(ColorGradient* gradient)
{
  // Set the gradient
  mGradient = gradient;

  // Clear any existing data
  Clear();

  GradientType::KeyArray& keys = gradient->mGradient.mControlPoints;
  for(uint i = 0; i < keys.Size(); ++i)
  {
    GradientType::GradientKey& key = keys[i];

    // Create a new internal control point
    InternalKey* newKey = new InternalKey(this, key.Value, key.Interpolant);

    AddKey(newKey, false);
  }
}

uint ColorGradientEditor::AddKey(InternalKey* key, bool sendEvent)
{
  mKeys.PushBack(key);
  key->MarkAsNeedsUpdate();
  Sort(mKeys.All(), SortByX());
  MarkAsNeedsUpdate();
  if(sendEvent)
    Modified();
  return mKeys.FindIndex(key);
}

bool ColorGradientEditor::RemoveKey(InternalKey* key)
{
  if(mKeys.Size() <= 2)
    return false;
  mKeys.EraseValueError(key);
  MarkAsNeedsUpdate();
  Modified();
  return true;
}

void ColorGradientEditor::UpdateTransform()
{
  SetSize(GetParent()->GetSize());

  // Stretch the gradient to the window
  mGradientBlockDisplay->SetSize(Vec2(GetSize().x, 0) + Pixels(-50, 35));
  mKeyDrawer->SetSize(Vec2(GetSize().x, 0) + Pixels(0,50));

  // Update the resource
  Update();

  // Update the gradient
  UpdateGradientBlock();

  Composite::UpdateTransform();
}

void ColorGradientEditor::Clear()
{
  // Delete each key
  for(uint i = 0; i < mKeys.Size(); ++i)
    delete mKeys[i];

  // Clear the array
  mKeys.Clear();
}

void ColorGradientEditor::OnGradientBlockMouseDown(MouseEvent* event)
{
  ColorGradient* gradient = mGradient;
  if(gradient == nullptr)
    return;

  Vec2 world = ToLocal(event->Position);
  float interpolant = GetInterpolantAtWorldPos(world);

  // Sample for the color
  Vec4 color = gradient->Sample(interpolant);

  // Create a new internal control point
  InternalKey* newKey = new InternalKey(this, color, interpolant);

  AddKey(newKey);

  newKey->OnMouseDrag(event);
}

/// Draws a checker pattern onto the given buffer(does NOT upload the buffer).
void DrawCheckers2(PixelBuffer* buffer, uint checkerSize)
{
  uint width = buffer->Width;
  uint height = buffer->Height;

  ByteColor white = Color::White;
  ByteColor gray = Color::LightGray;

  for(uint y = 0; y < height; ++y)
  {
    for(uint x = 0; x < width; ++x)
    {
      uint offset = ((y / checkerSize) % 2) * checkerSize;

      if(((x + offset) / checkerSize) % 2 == 0)
        buffer->SetPixel(x, y, white);
      else
        buffer->SetPixel(x, y, gray);
    }
  }
}

void DrawColorGradient(ColorGradient* gradient, PixelBuffer* buffer)
{
  Gradient<Vec4>::KeyArray& keys = gradient->mGradient.mControlPoints;

  // If there's no keys, just draw checkers
  if (keys.Empty())
  {
    DrawCheckers2(buffer, 5);
    buffer->Upload();
    return;
  }

  // Only render a second gradient for alpha if any of the keys have different alpha
  bool differentAlpha = false;
  bool differentColor = false;

  Vec4 firstKey = keys.Front().Value;
  float alpha = firstKey.w;
  Vec3 color = ToVector3(firstKey);

  for(uint i = 1; i < keys.Size(); ++i)
  {
    Vec4 currentKey = keys[i].Value;
    float currAlpha = currentKey.w;
    Vec3 currColor = ToVector3(currentKey);

    // Check alpha difference
    if(Math::Abs(currAlpha - alpha) > 0.0001f)
      differentAlpha = true;

    // Check color difference
    if(Math::Distance(color, currColor) > 0.0001f)
      differentColor = true;

    // If they're both different, no need to continue searching
    if (differentAlpha && differentColor)
      break;
  }

  // No need to draw the checkers if we're just overwriting it with a solid color
  if(differentAlpha)
    DrawCheckers2(buffer, 5);

  uint height = buffer->Height;

  if(differentAlpha && differentColor)
    height = (uint)(buffer->Height * 0.5f);

  // Draw the raw color
  for (uint x = 0; x < buffer->Width; ++x)
  {
    float t = (float)x / (float)buffer->Width;

    Vec4 floatColor = RemoveHdrFromColor(gradient->Sample(t));
    ByteColor color = ToByteColor(floatColor);

    for (uint y = 0; y < height; ++y)
    {
      buffer->AddToPixel(x, y, color);

      // Draw a line between them
      if (differentAlpha && differentColor && y == height - 1)
        buffer->SetPixel(x, y, Color::Black);
    }
  }

  // Draw the color with full alpha
  if(differentAlpha && differentColor)
  {
    for (uint x = 0; x < buffer->Width; ++x)
    {
      float t = (float)x / (float)buffer->Width;
      Vec4 floatColor = RemoveHdrFromColor(gradient->Sample(t));
      ByteColor color = ToByteColor(floatColor);
      SetAlphaByte(color, 255);

      for (uint y = height; y < buffer->Height; ++y)
        buffer->AddToPixel(x, y, color);
    }
  }

  buffer->Upload();
}

}//namespace Zero

