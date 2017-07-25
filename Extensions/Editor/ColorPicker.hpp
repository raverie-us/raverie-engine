///////////////////////////////////////////////////////////////////////////////
///
/// \file ColorPicker.hpp
/// Declaration of the ColorPicker Composite.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class PixelBuffer;
class DisplayVisual;
class CheckBox;
class Label;
class TextBox;
class TextButton;

namespace Events
{
  DeclareEvent(FinalColorPicked);
  DeclareEvent(ColorChanged);
  DeclareEvent(ColorPickCancelled);
}

class ColorEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ColorEvent(Vec4Param color){Color = color;}
  Vec4 Color;
};

//---------------------------------------------------------------- ColorDropper

// Eye Dropper for picking colors from desktop
class ColorEyeDropper : public MouseManipulation
{
public:
  typedef ColorEyeDropper ZilchSelf;
  Widget* mOwner;
  ColorEyeDropper(Mouse* mouse, Composite* parent);
  void SetColorEvent(StringParam eventName);
  void OnMouseUp(MouseEvent* event) override;
  void OnMouseMove(MouseEvent* event) override;
};

// Open the eye dropper
ColorEyeDropper* OpenEyeDropper(Mouse* mouse, Composite* parent);

//---------------------------------------------------------------- Color Display

// Widget that display color in a box
class ColorDisplay : public Composite
{
public:
  ColorDisplay(Composite* parent, uint pixelWidth, uint pixelHeight);
  ~ColorDisplay();

  void SetInvalid();
  void UpdateTransform();
  void SetColor(Vec4Param color);
private:
  PixelBuffer* mColorBuffer;
  TextureView* mColorDisplay;
  Label* mText;
};

//----------------------------------------------------------------- Color Picker

/// Color Picker widget
class ColorPicker : public Composite
{
public:
  typedef ColorPicker ZilchSelf;

  ColorPicker(Composite* parent);
  ~ColorPicker();

  void Update();

  /// Sets the target.
  static ColorPicker* EditColor(Widget* target, Vec4 startingColor);
  static bool sOpenInNewOsWindow;

  /// Closes the color picker
  static void Close();

  /// Gets the final color as a ByteColor.
  ByteColor GetColor();
  /// Sets the final color from a ByteColor.
  void SetColor(ByteColor color);

  /// Gets the final color as a Vec4.
  Vec4 GetFloatColor();
  /// Sets the final color from a Vec4.
  void SetColor4(Vec4Param color);

  /// Gets the final color as a Vec4 without Hdr.
  Vec4 GetFlatFloatColor();

  /// Gets the colors alpha value.
  float GetAlpha();
  /// Sets the colors alpha value.
  void SetAlpha(float alpha);

  /// Determines whether or not the color can be an Hdr color.
  void SetAllowsHdr(bool state);

  /// Gets the colors Hdr value.
  float GetHdr();
  /// Sets the colors Hdr value.
  void SetHdr(float hdr);

  /// Gets the state on whether we are accepting the changes of the picked color
  bool IsColorPicked();
private:
  friend class BlockManipulator;
  friend class SliderManipulator;

  /// Sets the current editing mode.
  void SetMode(uint mode);

  /// Updates the final color from RGB.
  void UpdateFinalColor();
  /// Draws to the color slider buffer.
  void UpdateColorSlider();
  /// Draws to the alpha slider buffer.
  void UpdateAlphaSlider();
  /// Draws to the color block buffer.
  void UpdateColorBlock();
  /// Updates all values in the text boxes.
  void UpdateTextBoxes();

  /// Converts RGB to HSV and stores them.
  void UpdateHSVFromRGB();
  /// Converts HSV to RGB and stores them.
  void UpdateRGBFromHSV();

  void ColorChanged();
  void ColorPicked();
  void UpdateSelectionFromHSV();
  void UpdateSelectionFromRGB();
  void SelectionChanged();
  void UpdateHSVFromSelection();
  void UpdateRGBFromSelection();

  /// Used by the manipulators to set the selections.
  void SetColorBlockSelection(Vec2Param pos);
  void SetColorSliderSelection(float pos);
  void SetAlphaSliderSelection(float pos);

  /// Button response.
  void OnOkClicked(Event* event);
  void OnCloseClicked(Event* event);
  void OnEyeDropper(MouseEvent* event);
  void OnEyeDropColorChagned(ColorEvent* event);

  /// Handles the mouse down event on the three sliders.
  void OnColorSliderMouseDown(MouseEvent* event);
  void OnAlphaSliderMouseDown(MouseEvent* event);
  void OnColorBlockMouseDown(MouseEvent* event);

  /// Check box event response.
  void OnHueClicked(Event* event);
  void OnSaturationClicked(Event* event);
  void OnValueClicked(Event* event);
  void OnRedClicked(Event* event);
  void OnGreenClicked(Event* event);
  void OnBlueClicked(Event* event);

  /// Event response for text box changes.
  void OnTextBoxChanged(uint mode, float range);
  void OnHueChanged(Event* event);
  void OnSaturationChanged(Event* event);
  void OnValueChanged(Event* event);
  void OnRedChanged(Event* event);
  void OnGreenChanged(Event* event);
  void OnBlueChanged(Event* event);
  void OnAlphaChanged(Event* event);
  void OnHdrChanged(Event* event);
  void OnHexChanged(Event* event);

  void OnKeyDown(KeyboardEvent* event);

  /// Dispatches an event on the current target.
  void DispatchOnTarget(StringParam event);

private:
  /// Static instance of the color picker.
  static ColorPicker* Instance;

  enum {Hue, Saturation, Value, Red, Green, Blue};

  /// The target that we're modifying.
  HandleOf<Widget> mTarget;

  // Final color display
  ColorDisplay* mFinalColorDisplay;

  // Color block
  PixelBuffer* mColorBlockBuffer;
  TextureView* mColorBlockDisplay;
  Vec2 mColorBlockSelection;

  // Color slider
  PixelBuffer* mColorSliderBuffer;
  TextureView* mColorSliderDisplay;
  float mColorSliderSelection;

  // Alpha slider
  PixelBuffer* mAlphaSliderBuffer;
  TextureView* mAlphaSliderDisplay;
  float mAlphaSliderSelection;
  
  // Alpha text box
  Label* mAlphaLabel;
  TextBox* mAlphaTextBox;

  // HDR text box
  Label* mHdrLabel;
  TextBox* mHdrTextBox;

  // Hex editor
  Label* mHexLabel;
  TextBox* mHexTextBox;

  // Buttons
  TextButton* mOkButton;
  TextButton* mCancelButton;

  /// Hue Saturation Brightness.
  CheckBox* mCheckBoxes[6];
  Label* mLabels[6];
  TextBox* mTextBoxes[6];
  float mValues[6];

  /// The final alpha of the color.
  float mAlpha;

  /// The Hdr scalar of the color.
  float mHdr;

  /// Determines whether or not the color can be Hdr.
  bool mAllowsHdr;

  /// The current mode we're in.
  uint mMode;

  /// Used to determine if we should send a cancel 
  /// event when the window is closed.
  bool mColorPicked;
};

}//namespace Zero
