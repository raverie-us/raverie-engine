// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(FinalColorPicked);
DefineEvent(ColorChanged);
DefineEvent(ColorPickCancelled);
} // namespace Events

ZilchDefineType(ColorEvent, builder, type)
{
}

void DrawCheckers(PixelBuffer* buffer, uint checkerSize);

ColorDisplay::ColorDisplay(Composite* parent, uint pixelWidth, uint pixelHeight) : Composite(parent)
{
  static const String className = "ColorPicker";
  mDefSet = GetDefinitionSet()->GetDefinitionSet(className);

  // Create the color buffer
  mColorBuffer = new PixelBuffer(Color::Black, pixelWidth, pixelHeight);

  // Create the color display and set the texture
  mColorDisplay = new TextureView(this);
  mColorDisplay->SetTexture(mColorBuffer->Image);

  mText = new Label(this, cText);
}

ColorDisplay::~ColorDisplay()
{
  SafeDelete(mColorBuffer);
}

void ColorDisplay::SetInvalid()
{
  SetColor(Vec4(0.7f, 0.7f, 0.7f, 0.8f));
  mText->SetActive(true);
  mText->SetText("-");
}

void ColorDisplay::UpdateTransform()
{
  mColorDisplay->SetTranslation(Pixels(0, 0, 0));
  mColorDisplay->SetSize(GetSize());
  mText->SetTranslation(Pixels(3, 0, 0));
  mText->SetSize(GetSize());
  Composite::UpdateTransform();
}

void ColorDisplay::SetColor(Vec4Param hdrColor)
{
  mText->SetActive(false);

  // Draw a checkered background
  DrawCheckers(mColorBuffer, 5);

  Vec4 color = RemoveHdrFromColor(hdrColor);

  // The color without transparency
  ByteColor trueColor = ToByteColor(Vec4(color.x, color.y, color.z, 1.0f));

  // The actual color
  ByteColor finalColor = ToByteColor(color);

  // Get the dimensions of the buffer
  uint width = mColorBuffer->Width;
  uint height = mColorBuffer->Height;
  uint widthMid = width / 2;

  for (uint y = 0; y < height; ++y)
  {
    for (uint x = 0; x < widthMid; ++x)
      mColorBuffer->SetPixel(x, y, trueColor);

    for (uint x = widthMid; x < width; ++x)
      mColorBuffer->AddToPixel(x, y, finalColor);
  }

  mColorBuffer->Upload();
}

ColorEyeDropper* OpenEyeDropper(Mouse* mouse, Composite* parent)
{
  mouse->SetCursor(Cursor::Cross);
  return new ColorEyeDropper(mouse, parent);
}

ColorEyeDropper::ColorEyeDropper(Mouse* mouse, Composite* parent) : MouseManipulation(mouse, parent)
{
  mOwner = parent;
  mouse->SetCursor(Cursor::Cross);
}

void ColorEyeDropper::SetColorEvent(StringParam eventName)
{
  ByteColor color = Z::gEngine->has(OsShell)->GetColorAtMouse();
  ColorEvent event(ToFloatColor(color));
  mOwner->DispatchEvent(eventName, &event);
}

void ColorEyeDropper::OnMouseUp(MouseEvent* event)
{
  SetColorEvent(Events::ColorChanged);
  SetColorEvent(Events::FinalColorPicked);
  this->Destroy();
}

void ColorEyeDropper::OnMouseMove(MouseEvent* event)
{
  SetColorEvent(Events::ColorChanged);
}

class SliderManipulator : public MouseManipulation
{
public:
  typedef void (ColorPicker::*SetCallback)(float);

  ColorPicker* mColorPicker;
  Widget* mDisplay;
  SetCallback mCallback;

  SliderManipulator(Mouse* mouse, ColorPicker* owner, Widget* display, SetCallback callback) :
      MouseManipulation(mouse, owner)
  {
    mColorPicker = owner;
    mDisplay = display;
    mCallback = callback;

    // Set the color on the initial mouse down
    SetColor(mDisplay->ToLocal(mouse->GetClientPosition()));
  }

  void OnMouseMove(MouseEvent* event) override
  {
    // We want the local space of the display
    Vec2 localPosition = mDisplay->ToLocal(event->Position);

    SetColor(localPosition);
  }

  void SetColor(Vec2Param localPos)
  {
    float val = 1.0f - localPos.y / mDisplay->GetSize().y;
    // Clamp the value to the range [0, 1]
    val = Math::Clamp(val, 0.0f, 1.0f);
    (mColorPicker->*mCallback)(val);
    mColorPicker->SelectionChanged();
  }

  void OnMouseUp(MouseEvent* event) override
  {
    this->Destroy();
  }
};

class BlockManipulator : public MouseManipulation
{
public:
  ColorPicker* mColorPicker;
  Widget* mDisplay;

  BlockManipulator(Mouse* mouse, ColorPicker* owner, Widget* display) : MouseManipulation(mouse, owner)
  {
    mColorPicker = owner;
    mDisplay = display;

    // Set the color on the initial mouse down
    SetColor(mDisplay->ToLocal(mouse->GetClientPosition()));
  }

  void OnMouseMove(MouseEvent* event) override
  {
    // We want the local space of the display
    Vec2 localPos = mDisplay->ToLocal(event->Position);

    SetColor(localPos);
  }

  void SetColor(Vec2Param localPos)
  {
    Vec2 val = localPos;
    val.x = val.x / mDisplay->GetSize().x;
    val.y = 1.0f - val.y / mDisplay->GetSize().y;

    // Clamp the value to the range [0, 1]
    val.x = Math::Clamp(val.x, 0.0f, 1.0f);
    val.y = Math::Clamp(val.y, 0.0f, 1.0f);

    mColorPicker->SetColorBlockSelectionValue(val);
    mColorPicker->SelectionChanged();
  }

  void OnMouseUp(MouseEvent* event) override
  {
    this->Destroy();
  }
};

ColorPicker* ColorPicker::Instance;
bool ColorPicker::sOpenInNewOsWindow = false;
const float ColorPicker::cColorDisplaySize = 200.0f;
const float ColorPicker::cLuminanceThreshold = 0.5f;

ColorPicker::ColorPicker(Composite* parent) : Composite(parent)
{
  Instance = this;

  mColorPicked = false;
  parent->SetSize(Pixels(410, 235));
  mValues[Hue] = 0.0f;
  mValues[Saturation] = 0.96f;
  mValues[Value] = 0.85f;
  mValues[Red] = 1.0f;
  mValues[Green] = 1.0f;
  mValues[Blue] = 1.0f;
  mAlpha = 1.0f;
  mHdr = 1.0f;

  mFinalColorDisplay = new ColorDisplay(this, 70, 20);
  mFinalColorDisplay->SetTranslation(Pixels(239, 1, 0));
  mFinalColorDisplay->SetSize(Pixels(70, 20));

  // Parent composite for the color block area to allow for proper widget
  // clipping
  mColorBlockArea = new Composite(this);
  mColorBlockArea->SetTranslation(Pixels(0, 0, 0));
  mColorBlockArea->SetSize(Pixels(200, 200));
  mColorBlockArea->SetClipping(true);
  // Create the color buffer
  mColorBlockBuffer = new PixelBuffer(Color::Black, 256, 256);
  // Create the display and set the texture
  mColorBlockDisplay = new TextureView(mColorBlockArea);
  mColorBlockDisplay->SetTexture(mColorBlockBuffer->Image);
  mColorBlockDisplay->SetSize(Pixels(200, 200));
  // Create the selection widget for the color block
  mColorBlockSelection = new ImageWidget(mColorBlockArea, "WhiteSquareBorder");
  mColorBlockSelection->SetSize(Pixels(5, 5));
  mColorBlockSelection->SetDisplayOrigin(DisplayOrigin::Center);
  mColorBlockSelection->SetColor(ToFloatColor(Color::Black));

  ConnectThisTo(mColorBlockDisplay, Events::LeftMouseDown, OnColorBlockMouseDown);

  // Create the color buffer
  mColorSliderBuffer = new PixelBuffer(Color::Red, 1, 256);
  // Create the display and set the texture
  mColorSliderDisplay = new TextureView(this);
  mColorSliderDisplay->SetTexture(mColorSliderBuffer->Image);
  mColorSliderDisplay->SetTranslation(Pixels(205, 0, 0));
  mColorSliderDisplay->SetSize(Pixels(15, 200));
  // Create the selection widget for the color bar
  mColorSliderSelection = new ImageWidget(this, "White");
  mColorSliderSelection->SetSize(Pixels(15, 1));
  mColorSliderSelection->SetTranslation(Pixels(205, 0, 0));
  mColorSliderSelection->SetColor(ToFloatColor(Color::Black));

  ConnectThisTo(mColorSliderDisplay, Events::LeftMouseDown, OnColorSliderMouseDown);

  // Create the color buffer
  mAlphaSliderBuffer = new PixelBuffer(Color::Red, 20, 256);
  // Create the display and set the texture
  mAlphaSliderDisplay = new TextureView(this);
  mAlphaSliderDisplay->SetTexture(mAlphaSliderBuffer->Image);
  mAlphaSliderDisplay->SetTranslation(Pixels(225, 0, 0));
  mAlphaSliderDisplay->SetSize(Pixels(7, 200));
  // Create the selection widget for the alpha bar
  mAlphaSliderSelection = new ImageWidget(this, "White");
  mAlphaSliderSelection->SetSize(Pixels(7, 1));
  mAlphaSliderSelection->SetTranslation(Pixels(225, 0, 0));
  mAlphaSliderSelection->SetColor(ToFloatColor(Color::Black));

  ConnectThisTo(mAlphaSliderDisplay, Events::LeftMouseDown, OnAlphaSliderMouseDown);

  Vec3 offset = Pixels(250, 25, 0);

  for (uint i = 0; i < 6; ++i)
  {
    Vec3 pos = Pixels(0, i * 25.0f, 0);

    // Create the check-boxes
    mCheckBoxes[i] = new CheckBox(this);
    mCheckBoxes[i]->SetTranslationAndSize(pos + Pixels(-7, 9, 0) + offset, Vec2(0.08f, 0.08f));

    // Create the label
    mLabels[i] = new Label(this);
    mLabels[i]->SetSize(Pixels(20, 20));
    mLabels[i]->SetTranslation(pos + offset);

    // Create the text box
    mTextBoxes[i] = new TextBox(this);
    mTextBoxes[i]->SetText(String::Format("%0.2f", mValues[i]));
    mTextBoxes[i]->SetEditable(true);
    mTextBoxes[i]->SetSize(Pixels(40, 20));
    mTextBoxes[i]->SetTranslation(Pixels(20, 0, 0) + pos + offset);
  }

  mLabels[Hue]->SetText("H:");
  mLabels[Saturation]->SetText("S:");
  mLabels[Value]->SetText("V:");
  mLabels[Red]->SetText("R:");
  mLabels[Green]->SetText("G:");
  mLabels[Blue]->SetText("B:");

  // Connect to check boxes
  ConnectThisTo(mCheckBoxes[Hue], Events::ValueChanged, OnHueClicked);
  ConnectThisTo(mCheckBoxes[Saturation], Events::ValueChanged, OnSaturationClicked);
  ConnectThisTo(mCheckBoxes[Value], Events::ValueChanged, OnValueClicked);
  ConnectThisTo(mCheckBoxes[Red], Events::ValueChanged, OnRedClicked);
  ConnectThisTo(mCheckBoxes[Green], Events::ValueChanged, OnGreenClicked);
  ConnectThisTo(mCheckBoxes[Blue], Events::ValueChanged, OnBlueClicked);

  // Connect to text boxes
  ConnectThisTo(mTextBoxes[Hue], Events::TextBoxChanged, OnHueChanged);
  ConnectThisTo(mTextBoxes[Saturation], Events::TextBoxChanged, OnSaturationChanged);
  ConnectThisTo(mTextBoxes[Value], Events::TextBoxChanged, OnValueChanged);
  ConnectThisTo(mTextBoxes[Red], Events::TextBoxChanged, OnRedChanged);
  ConnectThisTo(mTextBoxes[Green], Events::TextBoxChanged, OnGreenChanged);
  ConnectThisTo(mTextBoxes[Blue], Events::TextBoxChanged, OnBlueChanged);

  mAlphaLabel = new Label(this);
  mAlphaLabel->SetText("A:");
  mAlphaLabel->SetTranslationAndSize(Pixels(250, 175, 0), Pixels(20, 20));

  mAlphaTextBox = new TextBox(this);
  mAlphaTextBox->SetText("1.0");
  mAlphaTextBox->SetEditable(true);
  mAlphaTextBox->SetTranslationAndSize(Pixels(270, 175, 0), Pixels(40, 20));
  ConnectThisTo(mAlphaTextBox, Events::TextBoxChanged, OnAlphaChanged);

  mHdrLabel = new Label(this);
  mHdrLabel->SetText("Hdr:");
  mHdrLabel->SetTranslationAndSize(Pixels(320, 150, 0), Pixels(30, 20));

  mHdrTextBox = new TextBox(this);
  mHdrTextBox->SetText("1.0");
  mHdrTextBox->SetEditable(true);
  mHdrTextBox->SetTranslationAndSize(Pixels(355.0f, 150.0f, 0.0f), Pixels(40, 20));
  ConnectThisTo(mHdrTextBox, Events::TextBoxChanged, OnHdrChanged);

  mHexLabel = new Label(this);
  mHexLabel->SetText("#");
  mHexLabel->SetTranslationAndSize(Pixels(325, 175, 0), Pixels(20, 20));

  mHexTextBox = new TextBox(this);
  mHexTextBox->SetText("FF0013");
  mHexTextBox->SetEditable(true);
  mHexTextBox->SetTranslationAndSize(Pixels(340, 175, 0), Pixels(55, 19));
  ConnectThisTo(mHexTextBox, Events::TextBoxChanged, OnHexChanged);

  mOkButton = new TextButton(this);
  mOkButton->SetText("OK");
  mOkButton->SetTranslationAndSize(Pixels(315, 0, 0), Pixels(80, 20));
  ConnectThisTo(mOkButton, Events::ButtonPressed, OnOkClicked);

  mCancelButton = new TextButton(this);
  mCancelButton->SetText("Cancel");
  mCancelButton->SetTranslationAndSize(Pixels(315, 23, 0), Pixels(80, 20));
  ConnectThisTo(mCancelButton, Events::ButtonPressed, OnCloseClicked);

  Element* dropButton = CreateAttached<Element>("EyeDropper");
  dropButton->SetTranslation(Pixels(315, 46, 0));
  ConnectThisTo(dropButton, Events::LeftMouseDown, OnEyeDropper);

  ConnectThisTo(this, Events::ColorChanged, OnEyeDropColorChagned);

  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  SetMode(Hue);
}

ColorPicker::~ColorPicker()
{
  if (!mColorPicked)
    DispatchOnTarget(Events::ColorPickCancelled);
  Instance = nullptr;

  SafeDelete(mColorBlockBuffer);
  SafeDelete(mColorSliderBuffer);
  SafeDelete(mAlphaSliderBuffer);
}

void ColorPicker::Update()
{
  UpdateFinalColor();
  UpdateColorBlock();
  UpdateColorSlider();
  UpdateAlphaSlider();
  UpdateTextBoxes();
  UpdateColorSliderIndicator();
  UpdateAlphaSliderIndicator();
  UpdateBlockIndicator();
}

ColorPicker* ColorPicker::EditColor(Widget* target, Vec4 color)
{
  // If the instance is null, create a new window
  if (Instance == nullptr)
  {
    Composite* parent = nullptr;

    if (sOpenInNewOsWindow)
    {
      OsShell* osShell = Z::gEngine->has(OsShell);
      IntVec2 size = IntVec2(400, 210);
      IntVec2 position = IntVec2(250, 200);

      BitField<WindowStyleFlags::Enum> style;
      style.SetFlag(WindowStyleFlags::MainWindow);
      style.SetFlag(WindowStyleFlags::Resizable);
      style.SetFlag(WindowStyleFlags::OnTaskBar);
      style.SetFlag(WindowStyleFlags::Close);
      style.SetFlag(WindowStyleFlags::ClientOnly);

      OsWindow* window = osShell->CreateOsWindow(
          "TweakablesWindow", size, position, target->GetRootWidget()->GetOsWindow(), style.Field);
      window->SetMinClientSize(size);

      MainWindow* rootWidget = new MainWindow(window);
      rootWidget->SetTitle("Color Picker");

      parent = rootWidget;
    }
    else
    {
      Window* window = new Window(target->GetRootWidget());
      window->SetTranslationAndSize(Pixels(200, 200, 0), Pixels(280, 400));

      parent = window;
    }

    Instance = new ColorPicker(parent);
  }

  // Set the target
  Instance->mTarget = target;

  Instance->SetColor4(color);

  return Instance;
}

void ColorPicker::Close()
{
  if (Instance != NULL)
  {
    CloseTabContaining(Instance);

    // Close the Os window if it was opened in its own Os window
    if (sOpenInNewOsWindow)
      Instance->GetRootWidget()->GetOsWindow()->Close();
  }
}

ByteColor ColorPicker::GetColor()
{
  return ToByteColor(GetFloatColor());
}

void ColorPicker::SetColor(ByteColor color)
{
  SetColor4(ToFloatColor(color));
}

Vec4 ColorPicker::GetFloatColor()
{
  return Vec4(mValues[Red] * mHdr, mValues[Green] * mHdr, mValues[Blue] * mHdr, mAlpha);
}

void ColorPicker::SetColor4(Vec4Param color)
{
  // Get the HDR from the color
  mHdr = GetHdrFromColor(color);

  // Set RGBA
  mValues[Red] = color.x / mHdr;
  mValues[Green] = color.y / mHdr;
  mValues[Blue] = color.z / mHdr;
  mAlpha = color.w;

  UpdateHSVFromRGB();
  ColorChanged();
}

Vec4 ColorPicker::GetFlatFloatColor()
{
  return Vec4(mValues[Red], mValues[Green], mValues[Blue], mAlpha);
}

float ColorPicker::GetAlpha()
{
  return mAlpha;
}

void ColorPicker::SetAlpha(float alpha)
{
  mAlpha = alpha;
  SetAlphaSliderSelectionValue(alpha);
  Update();
}

void ColorPicker::SetAllowsHdr(bool state)
{
  mAllowsHdr = state;
}

float ColorPicker::GetHdr()
{
  return mHdr;
}

void ColorPicker::SetHdr(float hdr)
{
  mHdr = hdr;
  Update();
}

bool ColorPicker::IsColorPicked()
{
  return mColorPicked;
}

void ColorPicker::SetMode(uint mode)
{
  for (uint i = 0; i < 6; ++i)
    mCheckBoxes[i]->SetCheckedDirect(false);

  mMode = mode;
  mCheckBoxes[mode]->SetCheckedDirect(true);

  SetColorSliderSelectionValue(mValues[mode]);

  ColorChanged();
  UpdateColorBlock();
  UpdateColorSlider();
  UpdateAlphaSlider();
}

void ColorPicker::UpdateFinalColor()
{
  // Set the final color in the display
  Vec4 finalColor = GetFlatFloatColor();
  mFinalColorDisplay->SetColor(finalColor);

  // Tell the target that we updated the color
  DispatchOnTarget(Events::ColorChanged);
}

void ColorPicker::DispatchOnTarget(StringParam event)
{
  if (Widget* widget = mTarget)
  {
    ColorEvent e(GetFloatColor());
    widget->DispatchEvent(event, &e);
  }
}

void ColorPicker::UpdateColorSlider()
{
  // Dimensions of the slider
  uint height = mColorSliderBuffer->Height;
  uint width = mColorSliderBuffer->Width;

  // The step
  float step = 1.0f / float(height);

  // Walk the height
  for (uint y = 0; y < height; ++y)
  {
    float yPos = float(y) * step;

    Vec4 color;
    if (mMode == Hue)
      color = HSVToFloatColor(yPos, 1.0f, 1.0f);
    else if (mMode == Saturation)
      color = HSVToFloatColor(mValues[Hue], yPos, Math::Max(0.27f, mValues[Value]));
    else if (mMode == Value)
      color = HSVToFloatColor(mValues[Hue], mValues[Saturation], yPos);
    else // The mode is R, G, or B
    {
      color = Vec4(mValues[Red], mValues[Green], mValues[Blue], 1.0f);
      color[mMode - 3] = yPos;
    }

    // Set the color for each pixel wide
    for (uint x = 0; x < width; ++x)
      mColorSliderBuffer->SetPixel(x, height - 1 - y, ToByteColor(color));
  }

  // Upload the changes
  mColorSliderBuffer->Upload();
}

void ColorPicker::UpdateAlphaSlider()
{
  DrawCheckers(mAlphaSliderBuffer, 10);
  Vec4 color = GetFloatColor();

  // Dimensions of the block
  uint height = mAlphaSliderBuffer->Height;
  uint width = mAlphaSliderBuffer->Width;

  // Walk the texture
  for (uint y = 0; y < height; ++y)
  {
    float yPos = float(y) / (height - 1);

    for (uint x = 0; x < width; ++x)
    {
      color.w = yPos;
      mAlphaSliderBuffer->AddToPixel(x, height - 1 - y, ToByteColor(color));
    }
  }

  mAlphaSliderBuffer->Upload();
}

void ColorPicker::UpdateColorBlock()
{
  // Dimensions of the block
  uint height = mColorBlockBuffer->Height;
  uint width = mColorBlockBuffer->Width;

  // Walk the texture
  for (uint y = 0; y < height; ++y)
  {
    float valY = float(y) / (height - 1);

    for (uint x = 0; x < width; ++x)
    {
      float valX = float(x) / (width - 1);

      Vec4 color;

      if (mMode == Hue)
        color = HSVToFloatColor(Vec4(mValues[Hue], valX, valY, 1.0f));
      else if (mMode == Saturation)
        color = HSVToFloatColor(Vec4(valX, mValues[Saturation], valY, 1.0f));
      else if (mMode == Value)
        color = HSVToFloatColor(Vec4(valX, valY, mValues[Value], 1.0f));
      else if (mMode == Red)
        color = Vec4(mValues[Red], valX, valY, 1.0f);
      else if (mMode == Green)
        color = Vec4(valX, mValues[Green], valY, 1.0f);
      else if (mMode == Blue)
        color = Vec4(valX, valY, mValues[Blue], 1.0f);

      // Set the color of the pixel
      mColorBlockBuffer->SetPixel(x, height - 1 - y, ToByteColor(color));
    }
  }

  // Upload the changes
  mColorBlockBuffer->Upload();
}

void ColorPicker::UpdateTextBoxes()
{
  mTextBoxes[Hue]->SetText(String::Format("%0.0f", mValues[Hue] * 360.0f));

  for (uint i = 1; i < 3; ++i)
    mTextBoxes[i]->SetText(String::Format("%0.0f", mValues[i] * 100.0f));

  for (uint i = 3; i < 6; ++i)
    mTextBoxes[i]->SetText(String::Format("%0.0f", mValues[i] * 255.0f));

  // Update the alpha text box
  mAlphaTextBox->SetText(String::Format("%1.2f", mAlpha));

  // Update the Hdr text box
  mHdrTextBox->SetText(String::Format("%1.2f", mHdr));

  // Update the hex text box
  ByteColor color = ToByteColor(Vec4(mValues[Blue], mValues[Green], mValues[Red], 0.0f));
  mHexTextBox->SetText(String::Format("%06X", color));
}

void ColorPicker::UpdateBlockIndicator()
{
  // Using the RGB of the currently selected color find the luminance
  // and select an appropriate color the selection box
  Vec3 rgbColor(mValues[Red], mValues[Blue], mValues[Green]);
  if (Math::Luminance(rgbColor) < cLuminanceThreshold)
    mColorBlockSelection->SetColor(ToFloatColor(Color::White));
  else
    mColorBlockSelection->SetColor(ToFloatColor(Color::Black));
}

void ColorPicker::UpdateColorSliderIndicator()
{
  float yPos = mColorSliderSelection->GetTranslation().y;

  // Get the color of the color slider that the selection is over and check the
  // luminance to determine the color of the sliders selection indicator
  ByteColor color = mColorSliderBuffer->GetPixel(0, (uint)yPos);
  Vec4 floatColor = ToFloatColor(color);
  Vec3 rgbColor(floatColor.x, floatColor.y, floatColor.z);
  if (Math::Luminance(rgbColor) < cLuminanceThreshold)
    mColorSliderSelection->SetColor(ToFloatColor(Color::White));
  else
    mColorSliderSelection->SetColor(ToFloatColor(Color::Black));
}

void ColorPicker::UpdateAlphaSliderIndicator()
{
  float yPos = mColorSliderSelection->GetTranslation().y;

  // Get the value of the alpha slider that the selection is over and check the
  // luminance to determine the color of the sliders selection indicator
  ByteColor color = mAlphaSliderBuffer->GetPixel(0, (uint)yPos);
  Vec4 floatColor = ToFloatColor(color);
  Vec3 rgbColor(floatColor.x, floatColor.y, floatColor.z);
  if (Math::Luminance(rgbColor) < cLuminanceThreshold)
    mAlphaSliderSelection->SetColor(ToFloatColor(Color::White));
  else
    mAlphaSliderSelection->SetColor(ToFloatColor(Color::Black));
}

void ColorPicker::UpdateHSVFromRGB()
{
  // Calculate HSV
  Vec4 hsv = FloatColorToHSV(mValues[Red], mValues[Green], mValues[Blue]);
  mValues[Hue] = hsv.x;
  mValues[Saturation] = hsv.y;
  mValues[Value] = hsv.z;
}

void ColorPicker::UpdateRGBFromHSV()
{
  // Calculate RGB
  Vec4 rgb = HSVToFloatColor(mValues[Hue], mValues[Saturation], mValues[Value]);
  mValues[Red] = rgb.x;
  mValues[Green] = rgb.y;
  mValues[Blue] = rgb.z;
}

void ColorPicker::ColorChanged()
{
  // If the mode is in HSV
  if (mMode <= Value)
  {
    UpdateSelectionFromHSV();
    UpdateRGBFromHSV();
  }
  else // Mode is RGB
  {
    UpdateSelectionFromRGB();
    UpdateHSVFromRGB();
  }

  Update();
}

void ColorPicker::ColorPicked()
{
  // Send the final color
  DispatchOnTarget(Events::FinalColorPicked);

  // So that we don't send the cancel event in the destructor
  mColorPicked = true;

  // Close the window
  Close();
}

void ColorPicker::UpdateSelectionFromHSV()
{
  SetColorSliderSelectionValue(mValues[mMode]);
  if (mMode == Hue)
    SetColorBlockSelectionValue(Vec2(mValues[Saturation], mValues[Value]));
  else if (mMode == Saturation)
    SetColorBlockSelectionValue(Vec2(mValues[Hue], mValues[Value]));
  else if (mMode == Value)
    SetColorBlockSelectionValue(Vec2(mValues[Hue], mValues[Saturation]));
  SetAlphaSliderSelectionValue(mAlpha);
}

void ColorPicker::UpdateSelectionFromRGB()
{
  SetColorSliderSelectionValue(mValues[mMode]);
  if (mMode == Red)
    SetColorBlockSelectionValue(Vec2(mValues[Green], mValues[Blue]));
  else if (mMode == Green)
    SetColorBlockSelectionValue(Vec2(mValues[Red], mValues[Blue]));
  else if (mMode == Blue)
    SetColorBlockSelectionValue(Vec2(mValues[Red], mValues[Green]));
  SetAlphaSliderSelectionValue(mAlpha);
}

void ColorPicker::SelectionChanged()
{
  // Update the alpha
  mAlpha = GetAlphaSliderSelectionValue();

  // If the mode is in HSV
  if (mMode <= Value)
  {
    UpdateHSVFromSelection();
    UpdateRGBFromHSV();
  }
  else // Mode is RGB
  {
    UpdateRGBFromSelection();
    UpdateHSVFromRGB();
  }

  Update();
}

void ColorPicker::UpdateHSVFromSelection()
{
  Vec2 colorBlockVal = GetColorBlockSelectionValue();
  float colorSliderVal = GetColorSliderSelectionValue();

  if (mMode == Hue)
  {
    mValues[Hue] = colorSliderVal;
    mValues[Saturation] = colorBlockVal.x;
    mValues[Value] = colorBlockVal.y;
  }
  else if (mMode == Saturation)
  {
    mValues[Hue] = colorBlockVal.x;
    mValues[Saturation] = colorSliderVal;
    mValues[Value] = colorBlockVal.y;
  }
  else if (mMode == Value)
  {
    mValues[Hue] = colorBlockVal.x;
    mValues[Saturation] = colorBlockVal.y;
    mValues[Value] = colorSliderVal;
  }
}

void ColorPicker::UpdateRGBFromSelection()
{
  Vec2 colorBlockVal = GetColorBlockSelectionValue();
  float colorSliderVal = GetColorSliderSelectionValue();

  if (mMode == Red)
  {
    mValues[Red] = colorSliderVal;
    mValues[Green] = colorBlockVal.x;
    mValues[Blue] = colorBlockVal.y;
  }
  else if (mMode == Green)
  {
    mValues[Red] = colorBlockVal.x;
    mValues[Green] = colorSliderVal;
    mValues[Blue] = colorBlockVal.y;
  }
  else if (mMode == Blue)
  {
    mValues[Red] = colorBlockVal.x;
    mValues[Green] = colorBlockVal.y;
    mValues[Blue] = colorSliderVal;
  }
}

void ColorPicker::SetColorBlockSelectionValue(Vec2Param val)
{
  // Set the selection
  Vec3 newPos(val.x * cColorDisplaySize, (1.0f - val.y) * cColorDisplaySize, 0);
  mColorBlockSelection->SetTranslation(newPos);
}

Vec2 ColorPicker::GetColorBlockSelectionValue()
{
  Vec3 curPos = mColorBlockSelection->GetTranslation();
  return Vec2(curPos.x / cColorDisplaySize, 1.0f - curPos.y / cColorDisplaySize);
}

void ColorPicker::SetColorSliderSelectionValue(float val)
{
  // Set the selection
  Vec3 newPos = mColorSliderSelection->GetTranslation();
  newPos.y = (1.0f - val) * cColorDisplaySize;
  mColorSliderSelection->SetTranslation(newPos);

  UpdateColorSliderIndicator();
}

float ColorPicker::GetColorSliderSelectionValue()
{
  return 1.0f - mColorSliderSelection->GetTranslation().y / cColorDisplaySize;
}

void ColorPicker::SetAlphaSliderSelectionValue(float val)
{
  // Set the selection
  Vec3 newPos = mAlphaSliderSelection->GetTranslation();
  newPos.y = (1.0f - val) * cColorDisplaySize;
  mAlphaSliderSelection->SetTranslation(newPos);

  UpdateAlphaSliderIndicator();
}

float ColorPicker::GetAlphaSliderSelectionValue()
{
  return 1.0f - mAlphaSliderSelection->GetTranslation().y / cColorDisplaySize;
}

void ColorPicker::OnOkClicked(Event* event)
{
  ColorPicked();
}

void ColorPicker::OnCloseClicked(Event* event)
{
  // Close the window
  Close();

  // A cancel event will be sent out in our destructor
}

void ColorPicker::OnEyeDropper(MouseEvent* event)
{
  OpenEyeDropper(Mouse::GetInstance(), this);
}

void ColorPicker::OnEyeDropColorChagned(ColorEvent* event)
{
  SetColor4(event->Color);
}

void ColorPicker::OnColorSliderMouseDown(MouseEvent* event)
{
  new SliderManipulator(event->GetMouse(), this, mColorSliderDisplay, &ColorPicker::SetColorSliderSelectionValue);
}

void ColorPicker::OnAlphaSliderMouseDown(MouseEvent* event)
{
  new SliderManipulator(event->GetMouse(), this, mAlphaSliderDisplay, &ColorPicker::SetAlphaSliderSelectionValue);
}

void ColorPicker::OnColorBlockMouseDown(MouseEvent* event)
{
  new BlockManipulator(event->GetMouse(), this, mColorBlockDisplay);
}

void ColorPicker::OnHueClicked(Event* event)
{
  SetMode(Hue);
}

void ColorPicker::OnSaturationClicked(Event* event)
{
  SetMode(Saturation);
}

void ColorPicker::OnValueClicked(Event* event)
{
  SetMode(Value);
}

void ColorPicker::OnRedClicked(Event* event)
{
  SetMode(Red);
}

void ColorPicker::OnGreenClicked(Event* event)
{
  SetMode(Green);
}

void ColorPicker::OnBlueClicked(Event* event)
{
  SetMode(Blue);
}

void ColorPicker::OnTextBoxChanged(uint mode, float range)
{
  String text = mTextBoxes[mode]->GetText();
  ToValue(text.All(), mValues[mode]);
  mValues[mode] = Math::Clamp(mValues[mode] / range, 0.0f, 1.0f);

  if (mode < 3)
    UpdateRGBFromHSV();
  else
    UpdateHSVFromRGB();

  ColorChanged();
}

void ColorPicker::OnHueChanged(Event* event)
{
  OnTextBoxChanged(Hue, 360);
}

void ColorPicker::OnSaturationChanged(Event* event)
{
  OnTextBoxChanged(Saturation, 100);
}

void ColorPicker::OnValueChanged(Event* event)
{
  OnTextBoxChanged(Value, 100);
}

void ColorPicker::OnRedChanged(Event* event)
{
  OnTextBoxChanged(Red, 255);
}

void ColorPicker::OnGreenChanged(Event* event)
{
  OnTextBoxChanged(Green, 255);
}

void ColorPicker::OnBlueChanged(Event* event)
{
  OnTextBoxChanged(Blue, 255);
}

void ColorPicker::OnAlphaChanged(Event* event)
{
  // Get the text from the text box
  String text = mAlphaTextBox->GetText();

  // Convert it to a float
  float val;
  ToValue(text.All(), val);

  // Clamp it and set it
  SetAlpha(Math::Clamp(val, 0.0f, 1.0f));
}

void ColorPicker::OnHdrChanged(Event* event)
{
  // Get the text from the text box
  String text = mHdrTextBox->GetText();

  // Convert it to a float
  float val;
  ToValue(text.All(), val);

  // Clamp it and set it
  SetHdr(Math::Clamp(val, 1.0f, 10000.0f));
}

void ColorPicker::OnHexChanged(Event* event)
{
  String text = mHexTextBox->GetText();
  u64 val = (u64)ReadHexString(text);

  const float InvFactor = 1.0f / 255.0f;
  float r = float((val >> 16) & 0xFF) * InvFactor;
  float g = float((val >> 8) & 0xFF) * InvFactor;
  float b = float((val >> 0) & 0xFF) * InvFactor;
  float a = mAlpha;

  Vec4 color(r, g, b, a);

  SetColor4(color);
}

void ColorPicker::OnKeyDown(KeyboardEvent* event)
{
  if (event->Key == Keys::Enter)
    ColorPicked();
  if (event->Key == Keys::Escape)
    Close();
}

/// Draws a checker pattern onto the given buffer(does NOT upload the buffer).
void DrawCheckers(PixelBuffer* buffer, uint checkerSize)
{
  uint width = buffer->Width;
  uint height = buffer->Height;

  ByteColor white = Color::White;
  ByteColor gray = Color::LightGray;

  for (uint y = 0; y < height; ++y)
  {
    for (uint x = 0; x < width; ++x)
    {
      uint offset = ((y / checkerSize) % 2) * checkerSize;

      if (((x + offset) / checkerSize) % 2 == 0)
        buffer->SetPixel(x, y, white);
      else
        buffer->SetPixel(x, y, gray);
    }
  }
}

} // namespace Zero
