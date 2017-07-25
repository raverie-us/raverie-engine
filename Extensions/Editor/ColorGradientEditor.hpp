///////////////////////////////////////////////////////////////////////////////
///
/// \file ColorGradientEditor.hpp
/// Declaration of the ColorGradientEditor Composite.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ColorGradientEditor;
class PixelBuffer;
class DisplayVisual;
class ColorEvent;

//------------------------------------------------------------------ GradientKey
namespace GradientEditing
{

class GradientKey : public Spacer
{
public:
  typedef GradientKey ZilchSelf;
  GradientKey(ColorGradientEditor* parent, Vec4Param color, float interpolant);

  /// Update the translation of the object when anything changes
  void UpdateTransform() override;
  
  /// Mouse event response.
  void OnMouseDrag(MouseEvent* event);
  void OnRightClick(MouseEvent* event);

  /// Opens up the Color Picker to edit the color for this key
  void OnLeftClick(MouseEvent* event);

  /// Update the interpolant based on the given position (from mouse movement).
  void MoveTo(Vec2Param worldPos);

  /// Sets the transform to be centered around the given position.
  void SetCenter(Vec2Param worldPos);

  /// Responding to events from the color picker.
  void OnFinalColorPicked(ColorEvent* event);
  void OnColorChanged(ColorEvent* event);
  void OnColorPickCancelled(ColorEvent* event);

  /// The color of the key.
  Vec4 mColor;

  /// Used for when the color picker is canceled.
  Vec4 mPreviousColor;

  /// The interpolant of the color on the gradient.
  float mInterpolant;

  /// Whether or not the color picker is open for this key so that we can
  /// close the color picker if this is deleted.
  bool mPickingColor;

  ColorGradientEditor* mGradientEditor;
};

}//namespace GradientEditing

//------------------------------------------------------------------- Key Drawer
class GradientKeyDrawer : public Widget
{
public:
  GradientKeyDrawer(ColorGradientEditor* gradientEditor);

  /// Draw the widget.
  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

private:
  ColorGradientEditor* mGradientEditor;
};

//-------------------------------------------------------- Color Gradient Editor
class ColorGradientEditor : public Composite
{
public:
  typedef ColorGradientEditor ZilchSelf;
  typedef Gradient<Vec4> GradientType;
  typedef GradientEditing::GradientKey InternalKey;

  ColorGradientEditor(Composite* parent, ColorGradient* gradient);
  ~ColorGradientEditor();

  /// Updates the resource to match the editors keys.
  void Update();

  /// Gets the world position of the widget from the given keys interpolant.
  Vec2 GetKeyWorldPos(InternalKey* key);

  /// Gets the interpolant value of the given world position.
  float GetInterpolantAtWorldPos(Vec2Param worldPos);

  /// Should be called when the gradient is modified in any way.
  void Modified();

private:
  friend class GradientKeyDrawer;
  friend class GradientEditing::GradientKey;
  friend class SliderManipulator;

  /// Called when the editor is saved.
  void OnSave(Event* e);

  /// Updates the pixel buffer that displays the gradient.
  void UpdateGradientBlock();

  /// Loads the given gradient into the editor.
  void LoadGradient(ColorGradient* gradient);

  /// Adds the given key to the resource.
  uint AddKey(InternalKey* key, bool sendEvent = true);

  /// Removes the given key from the resource.
  bool RemoveKey(InternalKey* key);

  void UpdateTransform() override;

  /// Clears all keys.
  void Clear();

  /// Used for creating a new key.
  void OnGradientBlockMouseDown(MouseEvent* event);

private:
  GradientKeyDrawer* mKeyDrawer;

  // Gradient display block
  PixelBuffer* mGradientBlockBuffer;
  TextureView* mGradientBlockDisplay;

  // The gradient being modified
  HandleOf<ColorGradient> mGradient;

  /// Used to sort the gradient keys by the x-position.
  struct SortByX
  {
    bool operator()(const InternalKey* left, const InternalKey* right)
    {
      return left->mInterpolant < right->mInterpolant;
    }
  };

  typedef Array<InternalKey*> GradientKeyArray;
  GradientKeyArray mKeys;
};

void DrawColorGradient(ColorGradient* gradient, PixelBuffer* buffer);

}//namespace Zero
